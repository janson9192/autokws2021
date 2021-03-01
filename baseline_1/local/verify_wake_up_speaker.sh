#!/usr/bin/env bash

stage_start=1
stage_end=3
nj=1

input_wake_up_result_file=
enroll_xvector_dir=
data_dir=
model_dir=
output_xvector_dir=
threshold_value=
output_wake_up_result_file=

mfcc_conf=conf/mfcc_30_16k.conf
vad_conf=conf/vad.conf

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

vad_dir=${output_xvector_dir}/vad

# prepare sub data dir for wake up data
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    tmpdir=temp_${RANDOM}
    mkdir $tmpdir
    trap 'rm -rf "$tmpdir"' EXIT

    awk '{if ($2==1) print $1}' ${input_wake_up_result_file} > ${tmpdir}/wake_up_list

    utils/data/subset_data_dir.sh \
    --utt-list ${tmpdir}/wake_up_list \
    ${data_dir} \
    ${output_xvector_dir}/data \
    || exit 1;

    echo `date`
    echo 'get_enroll_xvextor stage 1 finished'
fi

# get xvextor
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    sid/compute_vad_decision.sh --nj ${nj} --cmd "${train_cmd}" \
    --vad-config ${vad_conf} \
    ${output_xvector_dir}/data ${vad_dir}/log ${vad_dir} || exit 1;

    sid/nnet3/xvector/extract_xvectors.sh --cmd "$train_cmd" --nj ${nj} \
    ${model_dir} ${output_xvector_dir}/data ${output_xvector_dir} || exit 1;

    echo `date`
    echo 'get_enroll_xvextor stage 2 finished'
fi

# compare test xvector and enroll xvector and re-decide wake up 
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    python local/decide_wake_up_again.py \
    ${enroll_xvector_dir}/spk_xvector.scp \
    ${output_xvector_dir}/xvector.scp \
    ${threshold_value} \
    ${input_wake_up_result_file} \
    ${output_wake_up_result_file} \
    || exit 1;
    
    echo `date`
    echo 'get_enroll_xvextor stage 3 finished'
fi

