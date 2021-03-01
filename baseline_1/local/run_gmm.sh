#!/bin/bash

stage_start=1
stage_end=5

source_asr_data_dir=
asr_train_dir=
asr_dev_dir=
asr_test_dir=
mfcc_root_dir=
mfcc_conf=

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

# nj for train, dev and test
nj[0]=40
nj[1]=$(wc -l ${asr_dev_dir}/spk2utt | awk '{print $1}' || exit 1;)
nj[2]=$(wc -l ${asr_test_dir}/spk2utt | awk '{print $1}' || exit 1;)

# make MFCC features.
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    a=0
    for data_dir in ${asr_train_dir} ${asr_dev_dir} ${asr_test_dir}; do
        out_dir=`echo ${data_dir} | awk -F/ '{print $NF}'`
        if [ ! -d ${mfcc_root_dir}/${out_dir} ];then
            mkdir -p ${mfcc_root_dir}/${out_dir}
            cp -arf ${data_dir} ${mfcc_root_dir}/${out_dir}/data
            echo "mkdir -p ${mfcc_root_dir}/${out_dir}"
        fi
        steps/make_mfcc.sh --cmd "$train_cmd" --nj ${nj[$a]} \
        --mfcc-config ${mfcc_conf} \
        ${mfcc_root_dir}/${out_dir}/data \
        ${mfcc_root_dir}/${out_dir}/log \
        ${mfcc_root_dir}/${out_dir} \
        || exit 1;

        steps/compute_cmvn_stats.sh \
        ${mfcc_root_dir}/${out_dir}/data \
        ${mfcc_root_dir}/${out_dir}/log \
        ${mfcc_root_dir}/${out_dir} \
        || exit 1;

        utils/fix_data_dir.sh ${mfcc_root_dir}/${out_dir}/data || exit 1;

        a=$[$a+1]
    done

    echo "run_gmm.sh stage 1 finished " `date`
fi

mfcc_train_dir=${mfcc_root_dir}/`echo ${asr_train_dir} | awk -F/ '{print $NF}'`
mfcc_dev_dir=${mfcc_root_dir}/`echo ${asr_dev_dir} | awk -F/ '{print $NF}'`
mfcc_test_dir=${mfcc_root_dir}/`echo ${asr_test_dir} | awk -F/ '{print $NF}'`

# mono
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    # training
    steps/train_mono.sh --cmd "$train_cmd" --nj $nj \
    ${mfcc_train_dir}/data data/lang exp/mono || exit 1;

    # decoding
    utils/mkgraph.sh data/lang_test exp/mono exp/mono/graph || exit 1;

    steps/decode.sh --cmd "$decode_cmd" --config conf/decode.conf --nj 8 \
    exp/mono/graph ${mfcc_test_dir}/data exp/mono/decode_test

    # alignment
    steps/align_si.sh --cmd "$train_cmd" --nj ${nj[0]} \
    ${mfcc_train_dir}/data data/lang exp/mono exp/mono_ali || exit 1;

    echo "run_gmm.sh stage 2 finished " `date`
fi 

# tri1
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    # training
    steps/train_deltas.sh --cmd "$train_cmd" \
    4000 32000 ${mfcc_train_dir}/data data/lang exp/mono_ali exp/tri1 || exit 1;

    # decoding
    utils/mkgraph.sh data/lang_test exp/tri1 exp/tri1/graph || exit 1;

    steps/decode.sh --cmd "$decode_cmd" --config conf/decode.conf --nj 8 \
    exp/tri1/graph ${mfcc_test_dir}/data exp/tri1/decode_test

    # alignment
    steps/align_si.sh --cmd "$train_cmd" --nj ${nj[0]} \
    ${mfcc_train_dir}/data data/lang exp/tri1 exp/tri1_ali || exit 1;

    echo "run_gmm.sh stage 3 finished " `date`
fi

# tri2
if [ $stage_start -le 4 -a $stage_end -ge 4 ]; then
    # training
    steps/train_deltas.sh --cmd "$train_cmd" \
    7000 56000 ${mfcc_train_dir}/data data/lang exp/tri1_ali exp/tri2 || exit 1;

    # decoding
    utils/mkgraph.sh data/lang_test exp/tri2 exp/tri2/graph

    steps/decode.sh --cmd "$decode_cmd" --config conf/decode.conf --nj 8 \
    exp/tri2/graph ${mfcc_test_dir}/data exp/tri2/decode_test

    # alignment
    steps/align_si.sh --cmd "$train_cmd" --nj ${nj[0]} \
    ${mfcc_train_dir}/data data/lang exp/tri2 exp/tri2_ali || exit 1;

    echo "run_gmm.sh stage 4 finished " `date`
fi

# tri3
if [ $stage_start -le 5 -a $stage_end -ge 5 ]; then
    # training [LDA+MLLT]
    steps/train_lda_mllt.sh --cmd "$train_cmd" \
    10000 80000 ${mfcc_train_dir}/data data/lang exp/tri2_ali exp/tri3 || exit 1;

    # decoding
    utils/mkgraph.sh data/lang_test exp/tri3 exp/tri3/graph || exit 1;

    steps/decode.sh --cmd "$decode_cmd" --config conf/decode.conf --nj 8 \
    exp/tri3/graph ${mfcc_test_dir}/data exp/tri3/decode_test

    # alignment
    steps/align_si.sh --cmd "$train_cmd" --nj ${nj[0]} \
    ${mfcc_train_dir}/data data/lang exp/tri3 exp/tri3_ali || exit 1;

    steps/align_si.sh --cmd "$train_cmd" --nj 5 \
    ${mfcc_dev_dir}/data data/lang exp/tri3 exp/tri3_ali_dev || exit 1;

    echo "run_gmm.sh stage 5 finished " `date`
fi