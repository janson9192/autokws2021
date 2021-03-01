# prepare_roc_input_list.py

# usage:  python prepare_roc_input_list.py std_test_dir result_dir keyword out_dir
 
import os
import sys
import shutil


def Read_Data(std_test_dir, result_dir, keyword, test_label):
    # read the speaker list
    spk_list = os.listdir(std_test_dir)

    spk_list_dict = {}
    for spk in spk_list:
        utt_list = []
        h_list = open(os.path.join(std_test_dir, spk, "list.txt"), 'r')
        for line_str in h_list:
            utt = line_str.strip()
            utt_list.append(utt)
        h_list
        spk_list_dict[spk] = utt_list

    spk_score_dict = {}
    for spk in spk_list:
        score_list = []
        h_score = open(os.path.join(result_dir, spk, keyword + "_n.RESULT"), 'r')
        for line_str in h_score:
            score = line_str.strip()
            score_list.append(score)
        h_score.close()
        spk_score_dict[spk] = score_list
    
    utt2label_dict = {}
    h_label = open(test_label, 'r')
    for line_str in h_label:
        utt, label = line_str.strip().split()
        utt2label_dict[utt] = label
    h_label.close()

    return spk_list_dict, spk_score_dict, utt2label_dict


def Get_Roc_In(spk_list_dict, spk_score_dict, test_label, utt2label_dict):
    roc_in_list = []
    for spk in sorted(spk_list_dict.keys()):
        utt_list = spk_list_dict[spk]
        score_list = spk_score_dict[spk]
        list_len = len(utt_list)
        for a in range(list_len):
            utt = utt_list[a]
            flag = utt2label_dict[utt]
            score0 = score_list[a].split(" ")
            score1 = ["%.4f" % (1 - float(score0[b])) for b in range(len(score0))]
            score2 = " ".join(score1)
            roc_in_list.append("%s %s %s" % (utt, flag, score2))
    return roc_in_list


def Output(roc_in_list, out_dir):
    h_roc_in_file = open(os.path.join(out_dir, 'roc_input_file'), 'w')
    for line_str in roc_in_list:
        h_roc_in_file.write('%s\n' % line_str)
    h_roc_in_file.close()
            

def main():
    std_test_dir = sys.argv[1]
    result_dir = sys.argv[2]
    keyword = sys.argv[3]
    test_label = sys.argv[4]
    out_dir = sys.argv[5]

    spk_list_dict, spk_score_dict, utt2label_dict = Read_Data(std_test_dir, result_dir, keyword, test_label)
    roc_in_list = Get_Roc_In(spk_list_dict, spk_score_dict, test_label, utt2label_dict)
    Output(roc_in_list, out_dir)
    

if __name__ == "__main__":
    main()