# splice all the utt in enroll data dir to a long wav, and
# use this long wav to get xvector. It will get better performance
# note:the input source dir is a kaldi format data dir
import os
import sys
import shutil
import glob
import pdb


def read_data_dir(source_dir):
    wav_scp_dict = {}
    h_wav_scp = open(os.path.join(source_dir, "wav.scp"), 'r')
    for line_str in h_wav_scp:
        if line_str.find("\t") == -1:
            utt, path = line_str.strip().split(" ", 1)
        else:
            utt, path = line_str.strip().split("\t", 1)
        wav_scp_dict[utt] = path
    h_wav_scp.close()
    return wav_scp_dict


def output(output_dir, wav_scp_dic):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # wav_scp
    h_file = open(os.path.join(output_dir, 'wav.scp'),'w')
    wav_list = []
    for utt, wav in wav_scp_dic.items():
        wav_list.append(wav)
    h_file.write("%s %s %s\n" % ('enroll_splice sox', ' '.join(wav_list), ' -t wav - |'))
    h_file.close()

    # utt2spk
    h_file = open(os.path.join(output_dir, 'utt2spk'),'w')
    h_file.write("%s %s\n" % ('enroll_splice', 'enroll'))
    h_file.close()

    # spk2utt
    h_file = open(os.path.join(output_dir, 'spk2utt'),'w')
    h_file.write("%s %s\n" % ('enroll', 'enroll_splice'))
    h_file.close()

def main():
    source_dir = sys.argv[1]
    output_dir = sys.argv[2]

    wav_scp_dict = read_data_dir(source_dir)
    output(output_dir, wav_scp_dict)

if __name__ == "__main__":
    main()