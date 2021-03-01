# make_utt2vad.py
# trans the kaldi-format vad to STD-format

import sys
import os
from kaldi_python_io import ScriptReader, ArchiveReader
import pdb


def read_vad(vad_scp):
    scp_reader = ScriptReader(vad_scp)
    vad_dict = {}
    for utt, vad in scp_reader:
        vad_dict[utt] = vad
    return vad_dict


def get_start_and_end_point(vad_dict, decision_times=3):
    start_dict = {}
    end_dict = {}
    for utt, vad in vad_dict.items():
        start_flag = 0
        for a in range(0,len(vad),1):
            if vad[a] == 1:
                start_flag += 1
                if start_flag == decision_times:
                    start_dict[utt] = a - (decision_times - 1)
                    break
            elif start_flag != 0: 
                # vad[a] == 0 and start_flag != 0 means the times of
                # vad[a] == 1 is less than decision_times, so we need to
                # reset start_flag to 0
                start_flag = 0
        end_flag = 0
        for a in range(len(vad)-1,-1,-1):
            if vad[a] == 1:
                end_flag += 1
                if end_flag == decision_times:
                    end_dict[utt] = a + (decision_times - 1)
                    break
            elif end_flag != 0: 
                # vad[a] == 0 and end_flag != 0 means the times of
                # vad[a] == 1 is less than decision_times, so we need to
                # reset end_flag to 0
                end_flag = 0
    return start_dict, end_dict
        

def output_data(utt2vad, start_dict, end_dict):
    h_utt2vad = open(utt2vad, 'w')
    for utt in sorted(start_dict.keys()):
        h_utt2vad.write("%s %d %d\n" % (utt, start_dict[utt], end_dict[utt]))
    h_utt2vad.close()


def main():
    vad_scp = sys.argv[1]
    utt2vad = sys.argv[2]
    vad_dict = read_vad(vad_scp)
    start_dict, end_dict = get_start_and_end_point(vad_dict, decision_times=30)
    output_data(utt2vad, start_dict, end_dict)


if __name__ == "__main__":
    main()