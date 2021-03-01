#!/usr/bin/env bash
# author:qjshao@npu-aslp.org

# if you want to run shell script like this:
# ./a.sh
# you should add the following command.
# or run shell script like:
# bash a.sh
find . |xargs chmod +x

# this code must be submitted
pip install jieba
pip install kaldi-python-io
