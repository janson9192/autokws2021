#!/bin/bash
# author:wangjingsong@4paradigm.com

enrollment_dir=$1
work_dir=$2

export MKL_SERVICE_FORCE_INTEL=1 #for the error https://github.com/pytorch/pytorch/issues/37377

# use conda env py36
source activate py36

# cd the submission dir
submission_dir=$(cd "$(dirname "$0")"; pwd)
cd $submission_dir
python wav2vec_dtw.py enroll $enrollment_dir $work_dir
