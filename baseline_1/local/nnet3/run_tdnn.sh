#!/usr/bin/env bash

# This script is based on swbd/s5c/local/nnet3/run_tdnn.sh

# this is the standard "tdnn" system, built in nnet3; it's what we use to
# call multi-splice.

nj=40
stage_start=1
stage_end=4

asr_train_dir=
asr_dev_dir=
asr_test_dir=
mfcc_root_dir=
mfcc_conf=
mfcc_dim=
bnf_dim=
model_dir=
egs_dir=
ali_dir=
graph_dir=

# training options
train_stage=-10
initial_effective_lrate=0.0015
final_effective_lrate=0.00015
num_epochs=4
num_jobs_initial=2
num_jobs_final=2
remove_egs=false

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

    echo "run_tdnn.sh stage 1 finished " `date`
fi

mfcc_train_dir=${mfcc_root_dir}/`echo ${asr_train_dir} | awk -F/ '{print $NF}'`/data
mfcc_dev_dir=${mfcc_root_dir}/`echo ${asr_dev_dir} | awk -F/ '{print $NF}'`/data
mfcc_test_dir=${mfcc_root_dir}/`echo ${asr_test_dir} | awk -F/ '{print $NF}'`/data

# creating neural net configs
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    echo "$0: creating neural net configs";

    num_targets=$(tree-info $ali_dir/tree |grep num-pdfs|awk '{print $2}')

    mkdir -p $model_dir/configs
    cat <<EOF > $model_dir/configs/network.xconfig
input dim=${mfcc_dim} name=input

# please note that it is important to have input layer with the name=input
# as the layer immediately preceding the fixed-affine-layer to enable
# the use of short notation for the descriptor
fixed-affine-layer name=lda input=Append(-2,-1,0,1,2) affine-transform-file=$model_dir/configs/lda.mat

# the first splicing is moved before the lda layer, so no splicing here
relu-batchnorm-layer name=tdnn1 dim=850
relu-batchnorm-layer name=tdnn2 dim=850 input=Append(-1,0,2)
relu-batchnorm-layer name=tdnn3 dim=850 input=Append(-3,0,3)
relu-batchnorm-layer name=tdnn4 dim=850 input=Append(-7,0,2)
relu-batchnorm-layer name=tdnn5 dim=850 input=Append(-3,0,3)
affine-layer name=bnf dim=${bnf_dim} input=tdnn5
relu-batchnorm-layer name=tdnn6 dim=850 input=bnf
output-layer name=output input=tdnn6 dim=$num_targets max-change=1.5
EOF
    steps/nnet3/xconfig_to_configs.py --xconfig-file $model_dir/configs/network.xconfig --config-dir $model_dir/configs/
    echo "run_tdnn.sh stage 2 finished " `date`
fi

# trian
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    steps/nnet3/train_dnn.py \
    --stage $train_stage \
    --cmd "$cuda_cmd" \
    --feat.cmvn-opts "--norm-means=false --norm-vars=false" \
    --trainer.num-epochs $num_epochs \
    --trainer.optimization.num-jobs-initial $num_jobs_initial \
    --trainer.optimization.num-jobs-final $num_jobs_final \
    --trainer.optimization.initial-effective-lrate $initial_effective_lrate \
    --trainer.optimization.final-effective-lrate $final_effective_lrate \
    --egs.dir "$egs_dir" \
    --cleanup.remove-egs $remove_egs \
    --cleanup.preserve-model-interval 500 \
    --use-gpu wait \
    --feat-dir ${mfcc_train_dir} \
    --ali-dir $ali_dir \
    --lang data/lang \
    --dir $model_dir  || exit 1;

    echo "run_tdnn.sh stage 3 finished " `date`
fi


if [ $stage_start -le 4 -a $stage_end -ge 4 ]; then
    # this version of the decoding treats each utterance separately
    # without carrying forward speaker information.
    decode_dir=${model_dir}/decode_test
    steps/nnet3/decode.sh --nj 8 --cmd "$decode_cmd" \
    $graph_dir ${mfcc_test_dir} $decode_dir || exit 1;

    echo "run_tdnn.sh stage 4 finished " `date`
fi