#!/bin/bash

nj=40
stage_start=1
stage_end=6

source_asr_data_dir=
asr_train_dir=
asr_dev_dir=
asr_test_dir=

. ./cmd.sh
. ./path.sh
. ./utils/parse_options.sh

# prepare data
if [ $stage_start -le 1 -a $stage_end -ge 1 ]; then
    if [ ! -d data/ ];then
        mkdir data
    fi

    python local/prepare_magicdata.py \
    ${source_asr_data_dir} \
    ${asr_train_dir} \
    ${asr_dev_dir} \
    ${asr_test_dir} \
    || exit 1;

    python local/fix_text.py ${asr_train_dir} || exit 1;
    python local/fix_text.py ${asr_dev_dir} || exit 1;
    python local/fix_text.py ${asr_test_dir} || exit 1;
    
    utils/data/fix_data_dir.sh ${asr_train_dir} || exit 1;
    utils/data/fix_data_dir.sh ${asr_dev_dir} || exit 1;
    utils/data/fix_data_dir.sh ${asr_test_dir} || exit 1;

    echo "prepare_asr_all.sh stage 1 finished " `date`
fi

# download DaCiDian raw resources, convert to Kaldi lexicon format
if [ $stage_start -le 2 -a $stage_end -ge 2 ]; then
    local/prepare_dict.sh || exit 1;
    echo "prepare_asr_all.sh stage 2 finished " `date`
fi

# word-segmente
if [ $stage_start -le 3 -a $stage_end -ge 3 ]; then
    python -c "import jieba" 2>/dev/null || \
    (echo "jieba is not found. you can use 'pip install jieba' to install this python moudle." && exit 1;)

    tmpdir=temp_${RANDOM}
    mkdir $tmpdir

    awk '{print $1}' data/local/dict/lexicon.txt | sort | uniq | awk '{print $1,99}'> \
    ${tmpdir}/word_seg_vocab.txt || exit 1;

    for data_dir in ${asr_train_dir} ${asr_dev_dir} ${asr_test_dir}; do
        python local/word_segmentation.py ${tmpdir}/word_seg_vocab.txt ${data_dir}/text > \
        ${tmpdir}/text || exit 1;
        mv ${tmpdir}/text ${data_dir}/text || exit 1;
    done

    rm -rf $tmpdir
    echo "prepare_asr_all.sh stage 3 finished " `date`
fi

# L
if [ $stage_start -le 4 -a $stage_end -ge 4 ]; then
    utils/prepare_lang.sh --position-dependent-phones false \
    data/local/dict "<UNK>" data/local/lang data/lang || exit 1;
    echo "prepare_asr_all.sh stage 4 finished " `date`
fi

# arpa LM
if [ $stage_start -le 5 -a $stage_end -ge 5 ]; then
    local/train_lms.sh \
    data/local/dict/lexicon.txt \
    ${asr_train_dir}/text \
    data/local/lm || exit 1;
    echo "prepare_asr_all.sh stage 5 finished " `date`
fi

# G compilation, check LG composition
if [ $stage_start -le 6 -a $stage_end -ge 6 ]; then
    utils/format_lm.sh data/lang data/local/lm/3gram-mincount/lm_unpruned.gz \
    data/local/dict/lexicon.txt data/lang_test || exit 1;
    echo "prepare_asr_all.sh stage 6 finished " `date`
fi

