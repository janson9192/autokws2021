import numpy as np 
import sys
import os
import shutil


INPUT_FILE = sys.argv[1]
OUT_DIR = sys.argv[2]
SLIDING_WINDOW = int(sys.argv[3])

# this script is used to output roc curve data, the input file of
# this script includes three columns data, the first colunm is uttrance id,
# (but in fact this colunm is not used, so it can be whatever string you like)，
# and the second column is if the test uttrance include  keyword， the rest 
# clounm is confidence score of each frame.

def Get_Roc_Data(result_file, out_dir):
    roc_data = []
    for a in range(1000, -5, -5):
        threshold_value = float(a) / 1000.0
        false_alarm_num = 0
        false_reject_num = 0
        fact_negative = 0
        fact_positive = 0

        false_reject_list = []
        false_alarm_list = []

        # for b in range(test_num):
        h_test = open(result_file, 'r')
        for line_str in h_test:
            line_data = line_str.strip().split(" ")
            utt = line_data[0]
            keyword_flag = int(line_data[1])
            score = [float(x) for x in line_data[2:]]

            if keyword_flag == 1:
                fact_positive += 1
                if max(score) < threshold_value:
                    false_reject_num += 1
                    false_reject_list.append(utt)
            else: # keyword_flag[b] != 1
                fact_negative += len(score)
                tag = 0
                for x in score:
                    if tag > 0: 
                        tag -= 1
                        continue
                    if x >= threshold_value: 
                        #print(line_dec[0], float(i))
                        false_alarm_num += 1
                        false_alarm_list.append(utt)
                        tag = SLIDING_WINDOW
        h_test.close()
        negative_time = fact_negative * 0.01/3600 # frame shift is 10ms, and the Unit of negative_time is Hour
        print(threshold_value, false_alarm_num, negative_time, false_reject_num,fact_positive)
        Output_Fa_Fr_List(threshold_value, false_alarm_list, false_reject_list, out_dir)
        roc_data.append([threshold_value, false_alarm_num/negative_time, false_reject_num/fact_positive])
    return np.array(roc_data)

def Output_Fa_Fr_List(threshold, false_alarm_list, false_reject_list, out_dir):
    fa_fr_dir = os.path.join(out_dir, "fa_fr_list")

    threshold_dir = os.path.join(fa_fr_dir, str(threshold))
    if os.path.exists(threshold_dir):
        shutil.rmtree(threshold_dir)
    os.makedirs(threshold_dir)

    h_fa = open(os.path.join(threshold_dir, 'false_alarm_list.txt'), 'w')
    for utt in false_alarm_list:
        h_fa.write("%s\n" % utt)
    h_fa.close()

    h_fr = open(os.path.join(threshold_dir, 'false_reject_list.txt'), 'w')
    for utt in false_reject_list:
        h_fr.write("%s\n" % utt)
    h_fr.close()
   

def Output_ROC(roc_data, out_dir):
    data_len = len(roc_data)
    out_file = os.path.join(out_dir, 'roc.dat')
    h_out = open(out_file, 'w')
    for a in range(data_len):
        h_out.write("%.3f %.3f %.3f\n" % (roc_data[a, 0], roc_data[a, 1], roc_data[a, 2]))
    h_out.close()


def main():
    if os.path.exists(OUT_DIR):
        shutil.rmtree(OUT_DIR)
    os.makedirs(OUT_DIR)

    roc_data = Get_Roc_Data(INPUT_FILE, OUT_DIR)
    Output_ROC(roc_data, OUT_DIR)  
    print("local/roc.py successed")


if __name__ == "__main__":
    main()
