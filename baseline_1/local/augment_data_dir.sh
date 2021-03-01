#!/bin/bash

stage_start=0
stage_end=4

source_data_dir=
out_data_dir=
combine_data_times= 

smallroom_rir_times=1
mediumroom_rir_times=1
largeroom_rir_times=0
musan_noise_times=1
musan_music_times=1
musan_speech_times=0
speed_perturb=0
volume_perturb=0
source_data_sampling_rate=16000 
RIRS_NOISES_root=
Musan_root=
musan_data=

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

echo "augment_data_dir.sh start " `date`

# initial
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    rm -rf $data_dir
    echo "rm -rf $data_dir"
    echo "augment_data_dir.sh stage 1 finished " `date`
fi

# add_noise_and_reverb
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    tmpdir=temp_${RANDOM}
    mkdir $tmpdir
    trap 'rm -rf "$tmpdir"' EXIT

    aug_times=`echo "scale=2;$combine_data_times-1" | bc`

    ./local/add_noise_and_reverb.sh \  
    --RIRS_NOISES_root ${RIRS_NOISES_root} \
    --Musan_root ${Musan_root} \
    --musan_data ${musan_data} \
    --smallroom_rir_times ${smallroom_rir_times} \
    --mediumroom_rir_times ${mediumroom_rir_times} \
    --largeroom_rir_times ${largeroom_rir_times} \
    --musan_noise_times ${musan_noise_times} \
    --musan_music_times ${musan_music_times} \
    --musan_speech_times ${musan_speech_times} \
    --speed_perturb ${speed_perturb} \
    --volume_perturb ${volume_perturb} \
    --source_data_sampling_rate ${source_data_sampling_rate} \
    --source_data_dir ${source_data_dir} \
    --aug_data_dir ${tmpdir}/aug_data \
    --aug_times ${aug_times} \
    || exit 1;

    echo "augment_data_dir.sh stage 2 finished " `date`
fi

# combine_data
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    utils/data/combine_data.sh ${out_data_dir} \
    ${tmpdir}/aug_data ${source_data_dir} \
    || exit 1;

    echo "augment_data_dir.sh stage 3 finished " `date`
fi

# make_augment_utt2lang_from_utt2uniq_and_utt2lang
if [ $stage_start -le 4 -a $stage_end -ge 4 ]; then
    if [ -f ${source_data_dir}/utt2lang ];then
        ./local/make_augment_utt2lang_from_utt2uniq_and_utt2lang.py \
        ${out_data_dir}/utt2uniq \
        ${source_data_dir}/utt2lang \
        ${out_data_dir} \
        || exit 1;
    fi

    echo "augment_data_dir.sh stage 4 finished " `date`
fi
