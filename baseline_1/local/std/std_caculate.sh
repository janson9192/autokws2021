#!/bin/bash
# loca/std_caculate_v2.sh

stage_start=1
stage_end=3

feat_type="bnf"
distance_type="cosion"
keyword="keyword"
do_mvn=0
vad_fag=1
slide_window=100
bnf_enrollment_dir=
bnf_test_dir=
std_enrollment_dir=
std_test_dir=
std_average_dir=
std_result_dir=
std_roc_dir=
test_label=

. ./path.sh
. ./utils/parse_options.sh

# Create a temporary folder to store variable files
tmpdir=temp_${RANDOM}/
mkdir $tmpdir
trap 'rm -rf "$tmpdir"' EXIT

# enrollment template fusion
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    if [ -d $std_average_dir ];then
        rm -rf $std_average_dir
    fi
    mkdir -p $std_average_dir

    ls $std_enrollment_dir > ${tmpdir}/enroll_dir_list
    for spk_dir in ` awk '{print $1}' ${tmpdir}/enroll_dir_list `; do
        keyword_list_file=${std_enrollment_dir}/${spk_dir}/list.txt
        avr_out_dir=${std_average_dir}/${spk_dir}
        mkdir $avr_out_dir

        ./STD_v5/template_avg_hierarchical ${bnf_enrollment_dir}/ $keyword_list_file ${keyword}  \
        $feat_type $distance_type $do_mvn $vad_fag ${avr_out_dir}/ || exit 1;
    done
    echo enrollment template fusion successfully 
    echo `date`
fi

# dtw caculate
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    if [ -d $std_result_dir ];then
        rm -rf $std_result_dir
    fi
    mkdir -p $std_result_dir

    ls $std_test_dir > ${tmpdir}/test_dir_list
    echo ${keyword}_n > ${tmpdir}/avr_list
    for spk_dir in ` awk '{print $1}' ${tmpdir}/test_dir_list `; do
        test_list_file=${std_test_dir}/${spk_dir}/list.txt
        avr_out_dir=${std_average_dir}/${spk_dir}
        result_out_dir=${std_result_dir}/${spk_dir}
        mkdir $result_out_dir
     
        ./STD_v5/dtw_std ${avr_out_dir}/ ${tmpdir}/avr_list ${bnf_test_dir}/ \
        $test_list_file $feat_type $distance_type $do_mvn $result_out_dir/ || exit 1;

    done
    echo dtw caculate successfully 
    echo `date`
fi

# roc caculate
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    python local/std/prepare_roc_input_list.py \
    ${std_test_dir} \
    ${std_result_dir} \
    ${keyword} \
    ${test_label} \
    ${tmpdir} \
    || exit 1;

    python local/std/roc_frame.py \
    ${tmpdir}/roc_input_file \
    $std_roc_dir \
    $slide_window \
    || exit 1;
    
    echo roc caculate successfully 
    echo `date`
fi