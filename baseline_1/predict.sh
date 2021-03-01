#!/usr/bin/env bash
# author:qjshao@npu-aslp.org

# this code that must be submitted

test_data=$1
workdir=$2
prediction_file=$3

submission_dir=$(cd "$(dirname "$0")"; pwd)
cd $submission_dir
. ./path.sh
. ./cmd.sh

stage_start=1
stage_end=7
nj=10

data_dir=${workdir}/data/test
mfcc_dir=${workdir}/mfcc/test
mfcc_conf=conf/mfcc_30_16k.conf
bnf_extractor_dir=exp/bnf_extractor
bnf_dir=${workdir}/bnf/test
std_dir=${workdir}/std
feat_type="bnf"
distance_type="cosion"
do_mvn=0
vad_fag=0
result_dir=${workdir}/result
threshold_value_1=0.80
threshold_value_2=0.83
xvector_extractor_dir=exp/xvector_extractor
enroll_xvector_dir=${workdir}/xvector/enroll
xvector_dir=${workdir}/xvector/test

# prepare_data
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    if [ ! -d  ${data_dir} ];then
        mkdir -p ${data_dir}
    fi

    python local/prepare_test_for_one_spk.py \
    ${test_data} \
    ${data_dir} \
    || exit;

    utils/data/fix_data_dir.sh ${data_dir} || exit 1;

    echo "pradict.sh stage 1 finished " `date`
fi

# make mfcc
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    if [ ! -d ${mfcc_dir} ];then
        mkdir -p ${mfcc_dir}
    fi

    steps/make_mfcc.sh --cmd "$train_cmd" \
    --nj ${nj} \
    --mfcc-config ${mfcc_conf} \
    ${data_dir} \
    ${mfcc_dir}/log \
    ${mfcc_dir} \
    || exit 1;

    steps/compute_cmvn_stats.sh \
    ${data_dir} \
    ${mfcc_dir}/log \
    ${mfcc_dir} \
    || exit 1;

    utils/fix_data_dir.sh ${data_dir} || exit 1;

    echo "pradict.sh stage 2 finished " `date`
fi

# get_bottleneck_features
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    if [ ! -d ${bnf_dir} ];then
        mkdir -p ${bnf_dir}
    fi

    local/get_bottleneck_features.sh \
    --nj ${nj} \
    --data_dir ${data_dir} \
    --model_dir ${bnf_extractor_dir} \
    --bnf_dir ${bnf_dir} \
    --log_dir ${bnf_dir}/log \
    --bnf_data_dir ${bnf_dir}/data \
    --bnf_htk_dir ${bnf_dir}/htk \
    || exit 1;

    echo "predict.sh.sh stage 3 finished " `date`
fi

# prapare the input file of STD_v5 code, which specially made for
# Q by E task. The main functions of STD_v5 are: 1.averaging the enroll templates, 
# 2.caculate the similarity of templates and test utterance feature.
# More details about STD code: https://github.com/jingyonghou/XY_QByE_STD
if [ $stage_start -le 4 -a $stage_end -ge 4 ]; then
    if [ ! -d ${std_dir} ];then
        mkdir -p ${std_dir}
    fi

    awk '{print $1}' ${data_dir}/utt2spk > ${std_dir}/test_list

    echo "predict.sh stage 4 finished " `date`
fi


# dtw caculate
if [ $stage_start -le 5 -a $stage_end -ge 5 ]; then
    echo keyword_n > ${std_dir}/template_list

    ./STD_v5/dtw_std \
    ${std_dir}/ \
    ${std_dir}/template_list \
    ${bnf_dir}/htk/ \
    ${std_dir}/test_list \
    $feat_type $distance_type $do_mvn \
    ${std_dir}/ || exit 1;

    echo "predict.sh stage 5 finished " `date`
fi

# decide whether to wake up
if [ $stage_start -le 6 -a $stage_end -ge 6 ]; then
    if [ ! -d ${result_dir} ];then
        mkdir -p ${result_dir}
    fi  

    python local/decide_wake_up.py \
    ${std_dir}/test_list \
    ${std_dir}/keyword_n.RESULT \
    ${threshold_value_1} \
    ${result_dir}/wake_up_result \
    || exit 1;

    echo "predict.sh stage 6 finished " `date`
fi

# use xvector to verify the wake up is from enroll speaker.
if [ $stage_start -le 7 -a $stage_end -ge 7 ]; then
    if [ ! -d ${xvector_dir} ];then
        mkdir -p ${xvector_dir}
    fi

    local/verify_wake_up_speaker.sh \
    --input_wake_up_result_file ${result_dir}/wake_up_result \
    --enroll_xvector_dir ${enroll_xvector_dir} \
    --data_dir ${data_dir} \
    --model_dir ${xvector_extractor_dir} \
    --output_xvector_dir ${xvector_dir} \
    --threshold_value ${threshold_value_2} \
    --output_wake_up_result_file ${result_dir}/wake_up_result \
    || exit 1;

    sed "s/test_\(.*\) /\1.wav /" ${result_dir}/wake_up_result > $prediction_file
    echo "predict.sh stage 7 finished " `date`
fi


