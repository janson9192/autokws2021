#!/usr/bin/env bash
# author:qjshao@npu-aslp.org

# This code is for reference only and does not need to be submitted

# Train a speaker verification model. This model will used after
# After the utterance is judged as wake-up, this model will be used 
# for re-verification.

nj=40
stage_start=2
stage_end=2

# you need to change these path to yours.
source_sv_data_dir=DATASET/MAGICDATA_SLR68
RIRS_NOISES_root=DATASET/NoiseAndRIRs/RIRS_NOISES
Musan_root=DATASET/NoiseAndRIRs/Musan

sv_train_dir=data/magicdata_train
sv_dev_dir=data/magicdata_dev
sv_test_dir=data/magicdata_test
xvector_dim=100
model_dir=exp/xvector_extractor
mfcc_root_dir=mfcc/mfcc_30_16k
mfcc_conf=conf/mfcc_30_16k.conf
musan_data=data/musan

echo "run_xvector_extractor.sh start " `date`

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

mfcc_train_dir=${mfcc_root_dir}/`echo ${sv_train_dir} | awk -F/ '{print $NF}'`
mfcc_dev_dir=${mfcc_root_dir}/`echo ${sv_dev_dir} | awk -F/ '{print $NF}'`
mfcc_test_dir=${mfcc_root_dir}/`echo ${sv_test_dir} | awk -F/ '{print $NF}'`

# if you didn't run "run_bnf_extractor.sh", you need run this stage to 
# prepare data dir of MAGICDATA. Otherwise, you can skip this stage
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    if [ ! -d data/ ];then
        mkdir data
    fi

    python local/prepare_magicdata.py \
    ${source_sv_data_dir} \
    ${sv_train_dir} \
    ${sv_dev_dir} \
    ${sv_test_dir} \
    || exit 1;

    python local/fix_text.py ${sv_train_dir} || exit 1;
    utils/data/fix_data_dir.sh ${sv_train_dir} || exit 1;

    echo "run_xvector_extractor.sh stage 1 finished " `date`
fi

# data augment
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    ./local/augment_data_dir.sh \
    --source_data_dir ${sv_train_dir} \
    --out_data_dir ${sv_train_dir}_aug \
    --combine_data_times 3 \
    --RIRS_NOISES_root ${RIRS_NOISES_root} \
    --Musan_root ${Musan_root} \
    --musan_data ${musan_data} \
    || exit 1;

    echo "run_xvector_extractor.sh stage 2 finished " `date`
fi

# make mfcc
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    out_dir=`echo ${sv_train_dir}_aug | awk -F/ '{print $NF}'`
    if [ ! -d ${mfcc_root_dir}/${out_dir} ];then
        mkdir -p ${mfcc_root_dir}/${out_dir}
        cp -arf ${sv_train_dir}_aug ${mfcc_root_dir}/${out_dir}/data
        echo "mkdir -p ${mfcc_root_dir}/${out_dir}"
    fi
    steps/make_mfcc.sh --cmd "$train_cmd" --nj ${nj} \
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

    done

    echo "run_xvector_extractor.sh stage 3 finished " `date`
fi

# run_sid_xvector
if [ $stage_start -le 4 -a $stage_end -ge 4 ]; then
    local/run_sid_xvector.sh \
    --data_dir ${mfcc_train_dir}/data \
    --model_dir ${model_dir} \
    --mfcc_flag 0 \
    --vad_flag 1 \
    --xvector_dim ${xvector_dim} \
    --min_len 200 \
    || exit 1;
   
    echo "run_xvector_extractor.sh stage 4 finished " `date`
fi
