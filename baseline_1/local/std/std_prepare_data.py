# std_prepare_data.py

import sys
import os
import shutil

# usage: std_prepare_data.py  enrollment_data_dir test_data_dir output_enrollment_dir output_test_dir
   
def read_spk2utt_list(data_dir):
    spk2utt_list = {}
    h_file = open(os.path.join(data_dir, "spk2utt"), 'r')
    for line_str in h_file:
        utt_list = []
        if line_str.find("\t") == -1:
            spk = line_str.strip().split(" ", 1)[0]
            utt_str = line_str.strip().split(" ", 1)[1]
            utt_list = utt_str.split()
        else:
            spk = line_str.strip().split("\t", 1)[0]
            utt_str = line_str.strip().split("\t", 1)[1]
            utt_list = utt_str.split()
        spk2utt_list[spk] = utt_list
    h_file.close()
    return spk2utt_list


def read_utt2vad(data_dir):
    utt2vad = {}
    h_file = open(os.path.join(data_dir, "utt2vad"), 'r')
    for line_str in h_file:
        if line_str.find("\t") == -1:
            utt, vad_start, vad_end = line_str.strip().split(" ")
        else:
            utt, vad_start, vad_end = line_str.strip().split("\t")
        utt2vad[utt] = ("%s %s") % (vad_start, vad_end)
    h_file.close()
    return utt2vad


def output_list(data_dir, output_dir, vad_flag):
    if os.path.exists(output_dir):
        shutil.rmtree(output_dir)
    os.makedirs(output_dir)

    if vad_flag == 1:
        spk2utt_list = read_spk2utt_list(data_dir)
        utt2vad = read_utt2vad(data_dir)
        for spk, utt_list in spk2utt_list.items():
            spk_dir = os.path.join(output_dir, spk)
            os.makedirs(spk_dir)
            list_file = open(os.path.join(spk_dir, "list.txt"), 'w')
            for utt in utt_list:
                list_file.write("%s %s\n" % (utt, utt2vad[utt]))
            list_file.close()
    else:
        spk2utt_list = read_spk2utt_list(data_dir)
        for spk, utt_list in spk2utt_list.items():
            spk_dir = os.path.join(output_dir, spk)
            os.makedirs(spk_dir)
            list_file = open(os.path.join(spk_dir, "list.txt"), 'w')
            for utt in utt_list:
                list_file.write("%s\n" % utt)
            list_file.close()


def main():
    enrollment_data_dir = sys.argv[1]
    test_data_dir = sys.argv[2]
    output_enrollment_dir = sys.argv[3]
    output_test_dir= sys.argv[4]
   
    output_list(enrollment_data_dir, output_enrollment_dir, 1)
    output_list(test_data_dir, output_test_dir, 0)


if __name__ == "__main__":
    main()
