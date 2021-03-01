#!/usr/bin/env bash

stage_start=1
stage_end=2
nj=1

data_dir=
model_dir=
output_dir=
mfcc_conf=conf/mfcc_30_16k.conf
vad_conf=conf/vad.conf

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

mfcc_dir=${output_dir}/mfcc

# splice enroll wav together
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    python local/prepare_xvector_data.py \
    ${data_dir} \
    ${output_dir}/data \
    || exit 1;

    echo `date`
    echo 'get_enroll_xvextor stage 1 finished'
fi

# get xvextor
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    
    steps/make_mfcc.sh --nj ${nj} --cmd "$train_cmd" \
    --mfcc-config ${mfcc_conf} \
    ${output_dir}/data ${mfcc_dir}/log ${mfcc_dir} || exit 1;

    sid/compute_vad_decision.sh --nj ${nj} --cmd "${train_cmd}" \
    --vad-config ${vad_conf} \
    ${output_dir}/data ${mfcc_dir}/log ${mfcc_dir} || exit 1;

    sid/nnet3/xvector/extract_xvectors.sh --cmd "$train_cmd" --nj ${nj} \
    ${model_dir} ${output_dir}/data ${output_dir} || exit 1;

    echo `date`
    echo 'get_enroll_xvextor stage 2 finished'
fi
