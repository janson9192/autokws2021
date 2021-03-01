#!/bin/bash
# author:wangjingsong@4paradigm.com

enrollment_dir=$1
work_dir=$2

echo ===== In enrollment.sh =====

submission_dir=$(cd "$(dirname "$0")"; pwd)
cd $submission_dir

# you can run your enroll code here, for example:
python enroll.py $enrollment_dir $work_dir
