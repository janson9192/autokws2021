# example for predict
# author:wangjingsong@4paradigm.com
import os
import sys

if __name__ == '__main__':
    print("== predict file: {}; test_dir: {}; work_dir: {}; prediction_file: {}; =="
            .format(sys.argv[0],sys.argv[1],sys.argv[2],sys.argv[3]))

    test_dir = sys.argv[1]
    prediction_path = sys.argv[3]
    # fake prediction file
    test_files = sorted(os.listdir(test_dir))
    with open(prediction_path, 'w') as f:
        fake_label = '1'
        for test_file in test_files:
            f.write(f'{test_file}\t{fake_label}\n')


