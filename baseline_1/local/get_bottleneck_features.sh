#!/bin/bash
# local/get_bottleneck_features

nj=40
stage_start=0
stage_end=2

data_dir=
model_dir=
bnf_dir=
log_dir=
bnf_data_dir=
bnf_htk_dir=

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

# make sure bnf_dir is absolute path
bnf_dir=`perl -e '($dir,$pwd)= @ARGV; if($dir!~m:^/:) { $dir = "$pwd/$dir"; } print $dir; ' ${bnf_dir} ${PWD}`

if [ $stage_start -le 0 -a $stage_end -ge 0 ]; then
    echo "rm ${bnf_data_dir} ${bnf_dir} ${bnf_htk_dir}"
    rm -rf ${bnf_data_dir} ${bnf_dir} ${bnf_htk_dir}
    echo `date`
    echo 'get_bottleneck_features stage 0 finished'
fi

# use the trained nnet3 model to caculate the bottleneck features
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    steps/nnet3/make_bottleneck_features.sh \
    --cmd "$decode_cmd" --nj ${nj}\
    "bnf" \
    $data_dir \
    $bnf_data_dir \
    $model_dir \
    $log_dir \
    $bnf_dir \
    || exit 1;
    
    echo `date`
    echo "bottleneck features prepared successfully"
fi

# translate the .ark features to htk format file
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    if [ ! -d $bnf_htk_dir ];then
        mkdir -p $bnf_htk_dir
    fi

    ls $bnf_dir/raw_*.scp | sort > ${bnf_htk_dir}/ark_list
    
    for ark_scp in `cat ${bnf_htk_dir}/ark_list`;
    do
        echo "translating $ark_scp"
        copy-feats-to-htk --output-dir=${bnf_htk_dir} --output-ext=bnf  scp:${ark_scp} || exit 1;
    done
   
    echo "bottleneck features translated to htk format successfully"
    echo `date`
fi