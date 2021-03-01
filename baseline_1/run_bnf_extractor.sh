#!/usr/bin/env bash
# author:qjshao@npu-aslp.org

# This code is for reference only and does not need to be submitted

# Train a bnf (bottleneck feature) extractor by the steps of ASR task
# and then use the bnf to caculate dtw score. It will get better result
# than use MFCC feature directly.

# We use magicdata dataset (http://www.openslr.org/68/) in this
# case. You can train another bnf extractor by your data or method

nj=40
stage_start=0
stage_end=3

# you need to change these path to yours.
source_asr_data_dir=DATASET/MAGICDATA_SLR68

asr_train_dir=data/magicdata_train
asr_dev_dir=data/magicdata_dev
asr_test_dir=data/magicdata_test
bnf_extractor_dir=exp/bnf_extractor
bnf_dim=50

echo "run.sh start " `date`

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

# prepare_asr_all
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    local/prepare_asr_all.sh \
    --source_asr_data_dir ${source_asr_data_dir} \
    --asr_train_dir ${asr_train_dir} \
    --asr_dev_dir ${asr_dev_dir} \
    --asr_test_dir ${asr_test_dir} \
    || exit 1;

    echo "run.sh stage 1 finished " `date`
fi

# run gmm
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    local/run_gmm.sh \
    --asr_train_dir ${asr_train_dir} \
    --asr_dev_dir ${asr_dev_dir} \
    --asr_test_dir ${asr_test_dir} \
    --mfcc_root_dir mfcc/mfcc_13_16k \
    --mfcc_conf conf/mfcc_13_16k.conf \
    || exit 1;
    echo "run.sh stage 2 finished " `date`
fi

# run tdnn
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    local/nnet3/run_tdnn.sh \
    --asr_train_dir ${asr_train_dir} \
    --asr_dev_dir ${asr_dev_dir} \
    --asr_test_dir ${asr_test_dir} \
    --mfcc_root_dir mfcc/mfcc_30_16k \
    --mfcc_conf conf/mfcc_30_16k.conf \
    --mfcc_dim 30 \
    --bnf_dim ${bnf_dim} \
    --model_dir ${bnf_extractor_dir} \
    --ali_dir exp/tri3_ali \
    --graph_dir exp/tri3/graph \
    --num_jobs_initial 1 \
    --num_jobs_final 1 \
    --remove_egs false \
    || exit 1;
    echo "run.sh stage 3 finished " `date`
fi
