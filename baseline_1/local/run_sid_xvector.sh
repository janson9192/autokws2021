#!/bin/bash
# run_sid.sh

stage_start=0
stage_end=9
nj=40

data_dir=
model_dir=
mfcc_flag=1
mfcc_config=
vad_flag=0
vad_config=
online_cmvn_config=
xvector_dim=100
min_len=200

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

exp_base_dir=${model_dir}
data_sid=${exp_base_dir}/data
conf_dir=${exp_base_dir}/conf
mfcc_dir=${exp_base_dir}/mfcc
mfcc_log=${mfcc_dir}/log
extractor_dir=${exp_base_dir}
report_dir=${exp_base_dir}/report

# rm dir
if [ $stage_start -le 0 -a $stage_end -ge 0 ]; then
    rm -rf ${exp_base_dir}
    echo "rm -rf ${exp_base_dir}"
    echo 'run_xvector_sid stage 0 finished' `date`
fi

# conf
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    if [ ! -d ${conf_dir} ];then
        mkdir -p ${conf_dir}
    fi

    # if mfcc_config is empty
    if [ ! -n "$mfcc_config" ]; then
        echo "create mfcc.conf"
        cat <<EOF > ${conf_dir}/mfcc.conf
--sample-frequency=16000
--frame-length=25 # the default is 25
--low-freq=20 # the default.
--high-freq=7600 # the default is zero meaning use the Nyquist (8k in this case).
--num-mel-bins=30
--num-ceps=30
--snip-edges=false
--allow-downsample=true
--allow-upsample=true
EOF
    else
        echo "cp mfcc.conf"
        cp $mfcc_config ${conf_dir}/mfcc.conf
    fi

    # if vad_config is empty
    if [ ! -n "$vad_config" ]; then
        echo "create vad.conf"
        cat <<EOF > ${conf_dir}/vad.conf
--vad-energy-threshold=5.5
--vad-energy-mean-scale=0.5
--vad-proportion-threshold=0.12
--vad-frames-context=2
EOF
    else
        echo "cp vad.conf"
        cp $vad_config ${conf_dir}/vad.conf
    fi

    # if online_cmvn_config is empty
    if [ ! -n "$online_cmvn_config" ]; then
        echo "create online_cmvn.conf"
        cat <<EOF > ${conf_dir}/online_cmvn.conf
# configuration file for apply-cmvn-online, used when invoking online2-wav-nnet3-latgen-faster.
EOF
    else
        echo "cp online_cmvn.conf"
        cp $online_cmvn_config ${conf_dir}/online_cmvn.conf
    fi

    echo 'run_xvector_sid stage 1 finished' `date`
fi

# copy data
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    mkdir -p ${data_sid}
    cp -arf ${data_dir}/* ${data_sid}
    echo 'run_xvector_sid stage 2 finished' `date`
fi

# make mfcc
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    # if mfcc_flag is 1, we need extract mfcc feats
    # if mfcc_flag is 0, we use the feats.cp in data dir
    if [ $mfcc_flag -eq 1 ]; then
        steps/make_mfcc.sh --mfcc-config ${conf_dir}/mfcc.conf \
        --nj ${nj} --cmd "$train_cmd" \
        ${data_sid} \
        ${mfcc_log} \
        ${mfcc_dir} || exit 1;

        utils/data/fix_data_dir.sh ${data_sid} || exit 1;
    fi

    echo "run_xvector_sid.sh stage 3 finished " `date`
fi

# cmvn
if [ $stage_start -le 4 -a $stage_end -ge 4 ]; then
    steps/compute_cmvn_stats.sh ${data_sid} ${mfcc_log} ${mfcc_dir}
    echo "run_xvector_sid.sh stage 4 finished " `date`
fi

# vad
if [ $stage_start -le 5 -a $stage_end -ge 5 ]; then
    if [ $vad_flag -eq 1 ];then
        sid/compute_vad_decision.sh --nj ${nj} --cmd "$train_cmd" \
        --vad_config ${conf_dir}/vad.conf \
        ${data_sid} ${mfcc_log} ${mfcc_dir} || exit 1;
    fi

    echo "run_xvector_sid.sh stage 5 finished " `date`
fi

# This script applies CMVN and removes nonspeech frames (if vad_flag=0).  Note that this is somewhat
# wasteful, as it roughly doubles the amount of training data on disk.  After
# creating training examples, this can be removed.
if [ $stage_start -le 6 -a $stage_end -ge 6 ]; then
    local/xvector/prepare_feats_for_egs.sh --nj 20 --cmd "$train_cmd" \
    ${data_sid} ${data_sid}_cmvn ${mfcc_dir}_cmvn \
    || exit 1;

    utils/fix_data_dir.sh ${data_sid}_cmvn || exit 1;
    echo "run_xvector_sid.sh stage 6 finished " `date`
fi

# Now, we need to remove features that are too short after removing silence
# frames.  We want atleast 2s (200 frames) per utterance.
if [ $stage_start -le 7 -a $stage_end -ge 7 ]; then
    echo "min frames per utt: ${min_len}"
    mv ${data_sid}_cmvn/utt2num_frames ${data_sid}_cmvn/utt2num_frames.bak

    awk -v min_len=${min_len} '$2 > min_len {print $1, $2}' ${data_sid}_cmvn/utt2num_frames.bak > ${data_sid}_cmvn/utt2num_frames
    
    utils/filter_scp.pl ${data_sid}_cmvn/utt2num_frames ${data_sid}_cmvn/utt2spk > ${data_sid}_cmvn/utt2spk.new
    
    mv ${data_sid}_cmvn/utt2spk.new ${data_sid}_cmvn/utt2spk
    
    utils/fix_data_dir.sh ${data_sid}_cmvn

    echo "run_xvector_sid.sh stage 7 finished " `date`
fi

# train
if [ $stage_start -le 8 -a $stage_end -ge 8 ]; then
    # stage set 2 means run without make egs
    
    local/xvector/run_xvector.sh \
    --min_len ${min_len} \
    --data ${data_sid}_cmvn \
    --nnet-dir $extractor_dir \
    --xvector_dim ${xvector_dim} \
    --num_jobs_initial 1 \
    --num_jobs_final 1 \
    || exit 1;

    echo "run_xvector_sid.sh stage 8 finished " `date`
fi

# report
if [ $stage_start -le 9 -a $stage_end -ge 9 ]; then
    steps/nnet3/report/generate_plots.py \
    ${extractor_dir} ${report_dir} \
    || exit 1;

    echo "run_xvector_sid.sh stage 9 finished " `date`
fi
