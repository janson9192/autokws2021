# usage: local/fix_text.py data_dir

import os
import re
import sys

def Read_Data(data_dir):
    h_text = open(os.path.join(data_dir, 'text'), 'r')
    text_list = []
    for line_str in h_text:
        str0 = line_str.strip().split()
        utt = str0[0]
        text = " ".join(str0[1:])
        text = text.replace("，", "")
        text = text.replace("。", "")
        text = text.replace("？", "")
        text = text.replace("?", "")
        text = text.replace("！", "")
        text = text.replace("！", "")
        text = text.replace(" ", "")
        text = text.replace(" ", "")
        if text != "":
            text_list.append("%s %s" % (utt, text))
    h_text.close()

    return text_list


def Output_Data(data_dir, text_list):
    h_wav_scp = open(os.path.join(data_dir, "text"), 'w')
    for line in text_list:
        h_wav_scp.write("%s\n" % line)
    h_wav_scp.close()


def main():
    data_dir = sys.argv[1]

    text_list = Read_Data(data_dir)
    Output_Data(data_dir, text_list)


if __name__ == "__main__":
    main()