#!/bin/bash
# author:wangjingsong@4paradigm.com

test_dir=$1
work_dir=$2
prediction_file=$3

echo ===== In enrollment.sh =====

submission_dir=$(cd "$(dirname "$0")"; pwd)
cd $submission_dir

# you can run your predict code here, for example:
python predict.py $test_dir $work_dir $prediction_file
