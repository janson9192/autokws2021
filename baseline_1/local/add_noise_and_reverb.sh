# !/bin/bash
# add_noise_and_reverb.sh

stage_start=0
stage_end=7

source_data_dir=
aug_data_dir=
aug_times=  
source_data_sampling_rate=16000 

RIRS_NOISES_root=DATASET/NoiseAndRIRs/RIRS_NOISES
Musan_root=DATASET/NoiseAndRIRs/Musan
musan_data=data/musan

smallroom_rir_times=1
mediumroom_rir_times=1
largeroom_rir_times=0
musan_noise_times=1
musan_music_times=1
musan_speech_times=0
speed_perturb=1
volume_perturb=0

fg_snrs="15:10:5"
bg_snrs="20:15:10"
speech_num="1:3:5"

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

tmpdir=temp_${RANDOM}
mkdir $tmpdir
trap 'rm -rf "$tmpdir"' EXIT


# rm dir
if [ $stage_start -le 0 -a $stage_end -ge 0 ]; then
    rm RIRS_NOISES
    rm -rf ${musan_data}
    echo "add_noise_and_reverb.sh stage 0 finished" `date`
fi


# initial
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    if [ ! -d RIRS_NOISES ];then
        ln -s $RIRS_NOISES_root ./
        echo "link RIRS_NOISES_root"
    fi

    # Prepare the MUSAN corpus, which consists of music, speech, and noise
    # suitable for augmentation.
    if [ ! -d data/musan ];then 
        steps/data/make_musan.sh --sampling_rate ${source_data_sampling_rate} \
        ${Musan_root} ${musan_data}
        echo "make musan data"
    fi

    # Get the duration of the MUSAN recordings.  This will be used by the
    # script augment_data_dir.py.
    for name in speech noise music; do
        if [ ! -d ${musan_data}/musan_${name}/reco2dur ];then
            utils/data/get_utt2dur.sh ${musan_data}/musan_${name}
            mv ${musan_data}/musan_${name}/utt2dur ${musan_data}/musan_${name}/reco2dur
        fi
    done
    echo "add_noise_and_reverb.sh stage 1 finished" `date`
fi


# add reverberation
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    # smallroom
    for a in `seq 1 ${smallroom_rir_times}`;do
        tmp_out_dir=smallroom_${a}
        # Note that we don't add any additive noise here.
        steps/data/reverberate_data_dir.py \
        --rir-set-parameters "1, RIRS_NOISES/simulated_rirs/smallroom/rir_list" \
        --speech-rvb-probability 1 \
        --pointsource-noise-addition-probability 0 \
        --isotropic-noise-addition-probability 0 \
        --num-replications 1 \
        --source-sampling-rate ${source_data_sampling_rate} \
        ${source_data_dir} ${tmpdir}/${tmp_out_dir}

        utils/copy_data_dir.sh --utt-suffix "-${tmp_out_dir}" ${tmpdir}/${tmp_out_dir} ${tmpdir}/${tmp_out_dir}.new
        rm -rf ${tmpdir}/${tmp_out_dir}
        mv ${tmpdir}/${tmp_out_dir}.new ${tmpdir}/${tmp_out_dir}
    done

    # mediumroom
    for a in `seq 1 ${mediumroom_rir_times}`;do
        tmp_out_dir=mediumroom_${a}
        # Note that we don't add any additive noise here.
        steps/data/reverberate_data_dir.py \
        --rir-set-parameters "1, RIRS_NOISES/simulated_rirs/mediumroom/rir_list" \
        --speech-rvb-probability 1 \
        --pointsource-noise-addition-probability 0 \
        --isotropic-noise-addition-probability 0 \
        --num-replications 1 \
        --source-sampling-rate ${source_data_sampling_rate} \
        ${source_data_dir} ${tmpdir}/${tmp_out_dir}

        utils/copy_data_dir.sh --utt-suffix "-${tmp_out_dir}" ${tmpdir}/${tmp_out_dir} ${tmpdir}/${tmp_out_dir}.new
        rm -rf ${tmpdir}/${tmp_out_dir}
        mv ${tmpdir}/${tmp_out_dir}.new ${tmpdir}/${tmp_out_dir}
    done

    # largeroom
    for a in `seq 1 ${largeroom_rir_times}`;do
        tmp_out_dir=largeroom_${a}
        # Note that we don't add any additive noise here.
        steps/data/reverberate_data_dir.py \
        --rir-set-parameters "1, RIRS_NOISES/simulated_rirs/largeroom/rir_list" \
        --speech-rvb-probability 1 \
        --pointsource-noise-addition-probability 0 \
        --isotropic-noise-addition-probability 0 \
        --num-replications 1 \
        --source-sampling-rate ${source_data_sampling_rate} \
        ${source_data_dir} ${tmpdir}/${tmp_out_dir}

        utils/copy_data_dir.sh --utt-suffix "-${tmp_out_dir}" ${tmpdir}/${tmp_out_dir} ${tmpdir}/${tmp_out_dir}.new
        rm -rf ${tmpdir}/${tmp_out_dir}
        mv ${tmpdir}/${tmp_out_dir}.new ${tmpdir}/${tmp_out_dir}
    done
  
    echo "add_noise_and_reverb.sh stage 2 finished" `date`
fi


# add noise
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    # musan_noise
    for a in `seq 1 ${musan_noise_times}`;do
        tmp_out_dir=noise_${a}

        steps/data/augment_data_dir.py \
        --utt-suffix "${tmp_out_dir}" \
        --fg-interval 1 \
        --fg-snrs "${fg_snrs}" \
        --fg-noise-dir "${musan_data}/musan_noise" \
        ${source_data_dir} ${tmpdir}/${tmp_out_dir} \
        || exit 1;
    done
    
    # musan_music_times
    for a in `seq 1 ${musan_music_times}`;do
        tmp_out_dir=music_${a}
        
        steps/data/augment_data_dir.py \
        --utt-suffix "${tmp_out_dir}" \
        --num-bg-noises "1" \
        --bg-snrs "${bg_snrs}" \
        --bg-noise-dir "${musan_data}/musan_music" \
        ${source_data_dir} ${tmpdir}/${tmp_out_dir} \
        || exit 1;
    done

    # musan_speech_times
    for a in `seq 1 ${musan_speech_times}`;do
        tmp_out_dir=speech_${a}
        
        steps/data/augment_data_dir.py \
        --utt-suffix "${tmp_out_dir}" \
        --num-bg-noises "${speech_num}" \
        --bg-snrs "${bg_snrs}" \
        --bg-noise-dir "${musan_data}/musan_speech" \
        ${source_data_dir} ${tmpdir}/${tmp_out_dir} \
        || exit 1;
    done

    echo "add_noise_and_reverb.sh stage 3 finished" `date`
fi


# speed perturb
if [ $stage_start -le 4 -a $stage_end -ge 4 ]; then
    if [ $speed_perturb -eq 1 ];then        
        utils/data/perturb_data_dir_speed.sh 0.9 $source_data_dir ${tmpdir}/speed_0.9 || exit 1;
        utils/data/perturb_data_dir_speed.sh 1.1 $source_data_dir ${tmpdir}/speed_1.1 || exit 1;

        echo "add_noise_and_reverb.sh stage 4 finished" `date`
    fi
fi


# volume perturb
if [ $stage_start -le 5 -a $stage_end -ge 5 ]; then
    if [ $volume_perturb -eq 1 ];then
        utils/copy_data_dir.sh --utt-suffix "-volume_perturb" $source_data_dir ${tmpdir}/volume_perturb
        utils/data/perturb_data_dir_volume.sh ${tmpdir}/volume_perturb || exit 1;
        echo "add_noise_and_reverb.sh stage 5 finished" `date`
    fi
fi


# combine data
if [ $stage_start -le 6 -a $stage_end -ge 6 ]; then
    ls_list=`ls ${tmpdir}`
    dir_list=
    for sub_dir in ${ls_list};do
        dir_list+="${tmpdir}/$sub_dir "
    done

    utils/data/combine_data.sh  ${tmpdir}/all "$dir_list"

    echo "add_noise_and_reverb.sh stage 6 finished" `date`
fi

# subset data
if [ $stage_start -le 7 -a $stage_end -ge 7 ]; then
    source_num=`wc -l ${source_data_dir}/utt2spk | awk '{print $1}' `
    aug_num=`awk 'BEGIN{printf("%d", "'$source_num'"*"'$aug_times'")}'`

    utils/data/subset_data_dir.sh ${tmpdir}/all ${aug_num} ${aug_data_dir}

    echo "add_noise_and_reverb.sh stage 7 finished" `date`
fi


