# usage:python local/make_augment_utt2lang_from_utt2uniq_and_utt2lang.py utt2uniq utt2lang outdir
import os
import sys

def Read_Data(utt2uniq, utt2lang):
    h_utt2uniq = open(utt2uniq, 'r')
    utt2uniq_dict = {}
    for line_str in h_utt2uniq:
        if line_str.find("\t") == -1:
            utt1, utt0 = line_str.strip().split(" ", 1)
        else:
            utt1, utt0 = line_str.strip().split("\t", 1)
        utt2uniq_dict[utt1] = utt0
    h_utt2uniq.close()

    h_utt2lang = open(utt2lang, 'r')
    utt2lang_dict = {}
    for line_str in h_utt2lang:
        if line_str.find("\t") == -1:
            utt0, lang = line_str.strip().split(" ", 1)
        else:
            utt0, lang = line_str.strip().split("\t", 1)
        utt2lang_dict[utt0] = lang
    h_utt2lang.close()

    return utt2uniq_dict, utt2lang_dict


def Output_Data(outdir, utt2uniq_dict, utt2lang_dict):
    h_utt2lang = open(os.path.join(outdir, 'utt2lang'), 'w')
    for utt1, utt0 in utt2uniq_dict.items():
        lang = utt2lang_dict[utt0]
        h_utt2lang.write("%s %s\n" % (utt1, lang))
    h_utt2lang.close()


def main():
    utt2uniq = sys.argv[1]
    utt2lang = sys.argv[2]
    outdir = sys.argv[3]

    utt2uniq_dict, utt2lang_dict = Read_Data(utt2uniq, utt2lang)
    Output_Data(outdir, utt2uniq_dict, utt2lang_dict)


if __name__ == "__main__":
    main()
