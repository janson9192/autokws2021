import os
import sys
import shutil
import glob
import pdb


def read_wav_scp(scp_file):
    root_dir = os.path.dirname(scp_file)
    h_scp_file = open(scp_file, 'r')
    wav_scp_dic = {}
    for line_str in h_scp_file:
        if line_str.find("\t") == -1:
            utt, path = line_str.strip().split(" ", 1)
        else:
            utt, path = line_str.strip().split("\t", 1)
        wav_scp_dic[utt] = path.replace('wav/', root_dir + '/')
    h_scp_file.close()
    return wav_scp_dic


def read_utt2spk_and_text(data_dir):
    h_file = open(os.path.join(data_dir, 'TRANS.txt'),'r')
    head_flag = 1
    utt2spk_dic = {}
    text_dic = {}
    for line_str in h_file:
        if head_flag == 1:
            head_flag = 0
            continue
        utt, spk, text = line_str.strip().split('\t')
        utt2spk_dic[utt] = spk
        text_dic[utt] = text
    h_file.close()
    return utt2spk_dic, text_dic


def utt2x_to_x2utt(utt2x_dict):
    x2utt_dict = {}
    for utt, x in utt2x_dict.items():
        if not x in x2utt_dict:
            x2utt_dict[x] = []
        x2utt_dict[x].append(utt)
    return x2utt_dict


def output(output_dir, wav_scp_dic, text_dic, utt2spk_dic, spk2utt_dic):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # wav_scp
    h_file = open(os.path.join(output_dir, 'wav.scp'),'w')
    for utt in sorted(wav_scp_dic.keys()):
        h_file.write("%s %s\n" % (utt, wav_scp_dic[utt]))
    h_file.close()

    # text
    h_file = open(os.path.join(output_dir, 'text'),'w')
    for utt in sorted(text_dic.keys()):
        h_file.write("%s %s\n" % (utt, text_dic[utt]))
    h_file.close()

    # utt2spk
    h_file = open(os.path.join(output_dir, 'utt2spk'),'w')
    for utt in sorted(utt2spk_dic.keys()):
        h_file.write("%s %s\n" % (utt, utt2spk_dic[utt]))
    h_file.close()

    # spk2utt
    h_file = open(os.path.join(output_dir, 'spk2utt'),'w')
    for spk in sorted(spk2utt_dic.keys()):
        h_file.write("%s %s\n" % (spk, " ".join(spk2utt_dic[spk])))
    h_file.close()
       

def main():
    root_data_dir = sys.argv[1]
    output_dir = []
    output_dir.append(sys.argv[2])
    output_dir.append(sys.argv[3])
    output_dir.append(sys.argv[4])

    a = 0
    for x in ['train','dev','test']:
        wav_scp_dic = read_wav_scp(os.path.join(root_data_dir,x+'.scp'))
        utt2spk_dic, text_dic = read_utt2spk_and_text(os.path.join(root_data_dir,x))
        spk2utt_dic = utt2x_to_x2utt(utt2spk_dic)
        output(output_dir[a], wav_scp_dic, text_dic, utt2spk_dic, spk2utt_dic)
        a += 1


if __name__ == "__main__":
    main()