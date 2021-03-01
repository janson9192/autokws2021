import os
import sys
import shutil
import glob
import pdb


def read_data_dir(source_dir):
    wav_list = glob.glob(source_dir + "/*.wav")
    return wav_list

def utt2x_to_x2utt(utt2x_dict):
    x2utt_dict = {}
    for utt, x in utt2x_dict.items():
        if not x in x2utt_dict:
            x2utt_dict[x] = []
        x2utt_dict[x].append(utt)
    return x2utt_dict

def analysis_wav_list(wav_list):
    wav_scp_dic = {}
    utt2spk_dic = {}

    for wav in wav_list:
        utt = "test_" + os.path.basename(wav).replace(".wav", "")
        spk = utt
        
        wav_scp_dic[utt] = wav
        utt2spk_dic[utt] = spk
    spk2utt_dic = utt2x_to_x2utt(utt2spk_dic)
    return wav_scp_dic, utt2spk_dic, spk2utt_dic

def output(output_dir, wav_scp_dic, utt2spk_dic, spk2utt_dic):
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # wav_scp
    h_file = open(os.path.join(output_dir, 'wav.scp'),'w')
    for utt in sorted(wav_scp_dic.keys()):
        h_file.write("%s %s\n" % (utt, wav_scp_dic[utt]))
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
    source_dir = sys.argv[1]
    output_dir = sys.argv[2]
    print(source_dir)
    print(output_dir)

    wav_list = read_data_dir(source_dir)

    wav_scp_dic, utt2spk_dic, spk2utt_dic = analysis_wav_list(wav_list)
    output(output_dir, wav_scp_dic, utt2spk_dic, spk2utt_dic)

if __name__ == "__main__":
    main()
