import os
import sys
import pdb


def Read_Data(test_list, dtw_result_file):
    h_test_list = open(test_list, 'r')
    h_dtw_result_file = open(dtw_result_file, 'r')
    utt2score = {}
    for line_str1 in h_test_list:
        utt = line_str1.strip()
        line_str2 = h_dtw_result_file.readline()
        score_str = line_str2.strip().split()
        score_list = [(1.0 - float(score)) for score in score_str]
        utt2score[utt] = max(score_list)
    h_test_list.close()
    h_dtw_result_file.close()
    return utt2score


def Output(threshold_value, output_file, utt2score):
    h_output = open(output_file, 'w')
    for utt,score in utt2score.items():
        if score >= threshold_value:
            h_output.write("%s %d\n" % (utt, 1))
        else:
            h_output.write("%s %d\n" % (utt, 0))
    h_output.close()


def main():
    test_list = sys.argv[1]
    dtw_result_file = sys.argv[2]
    threshold_value = float(sys.argv[3])
    output_file = sys.argv[4]

    utt2score = Read_Data(test_list, dtw_result_file)
    Output(threshold_value, output_file, utt2score)


if __name__ == "__main__":
    main()