# author:wangjingsong@4paradigm.com
import torch
import fairseq
import librosa
from dtw import *
from pydub import AudioSegment
from pydub.utils import db_to_float
from pydub.silence import detect_silence,detect_nonsilent
import os
import numpy as np
import itertools
import sys

submission_dir = os.path.dirname(os.path.realpath(__file__))
wav2vec_model_path = submission_dir + '/wav2vec_small.pt'
def load_model(model_path):
    model, cfg, task = fairseq.checkpoint_utils.load_model_ensemble_and_task([model_path])
    model = model[0]
    model.eval()
    return model

def feature_extract(model,wav):
    f1=model.feature_extractor(torch.tensor(wav).unsqueeze(0)).transpose(1, 2)
    f2=torch.nn.LayerNorm(512, 1e-5, True)(f1)
    f3=model.quantizer.forward_idx(f2)
    f4=model.project_q(f3[0])
    return f4.squeeze(0).detach().numpy()

def trim_silence(wav):
    start_end = detect_nonsilent(wav, 250, -40, 1)
    start_end = [se for se in start_end if se[1] - se[0] > 50]
    while len(start_end)==0 or wav.dBFS > 0: #if can't detect nonsilent
        wav = wav + 5
        start_end = detect_nonsilent(wav, 250, -40, 1)
        start_end = [se for se in start_end if se[1] - se[0] > 50]
    start = min(start_end)[0]
    end = max(start_end)[1]
    wav = wav[start-50:end+50]
    wav = np.array(wav.get_array_of_samples(),dtype='float32')/ 2**15
    return wav

def select_template(enroll_wavs, model):
    #extract features first
    features = [feature_extract(model, wav) for wav in enroll_wavs]
    
    #find the best template
    distance_each=[]
    for i,j in list(itertools.combinations(list(range(len(features))), 2)):
        alignment=dtw(features[i], features[j], keep_internals=True, distance_only=True, dist_method='cosine')
        distance_each.append((i, alignment.normalizedDistance))
        distance_each.append((j, alignment.normalizedDistance))
    
    low_scores=list(zip(*sorted(distance_each, key=lambda x:x[1])[:6]))[0]
    feature_index = np.argmax(np.bincount(low_scores))
    select_feature = features[feature_index]
    
    #save the largest distance as threshold
    largest_distance = max(list(zip(*distance_each))[1])
    return select_feature, largest_distance

    
def enroll(enroll_data_dir, work_dir):
    print('#load and trim data')
    enroll_wavs = []
    for wav_file in os.listdir(enroll_data_dir):
        wav=AudioSegment.from_wav(os.path.join(enroll_data_dir,wav_file))
        enroll_wavs.append(trim_silence(wav))
    
    print('#load model')
    model = load_model(wav2vec_model_path)
    
    print('#select template')
    template, largest_distance = select_template(enroll_wavs, model)
    
    print('#save template and threshold')
    np.save(os.path.join(work_dir, 'template.npy'), template)
    with open(os.path.join(work_dir, 'threshold'), 'w') as f:
        f.write(str(largest_distance))
    
    

def predict(test_data_dir, work_dir, predictoin_path):
    print('#load model and template')
    model = load_model(wav2vec_model_path)
    template = np.load(os.path.join(work_dir,'template.npy'))
    with open(os.path.join(work_dir, 'threshold')) as f:
        threshold = float(f.readline())
    threshold_max, threshold_min = 0.6, 0.45 #manually set super-hyparams
    threshold = max(threshold_min, min(threshold_max, threshold))
    
    print('#do predict')
    predict_results = []
    _len = len(template)
    _step = max(1, _len//5)

    for test_wav in sorted(os.listdir(test_data_dir)):
        wav = librosa.load(os.path.join(test_data_dir,test_wav),16000)[0]
        feature = feature_extract(model, wav)
        all_dis = []
        for i in range(0, max(len(feature)-_len,_step), _step):
            alignment = dtw(feature[i:min(i+_len,len(feature))],
                    template, keep_internals=True, dist_method='cosine')
            all_dis.append(alignment.normalizedDistance)
        
        #predict_results.append((test_wav, str(min(all_dis)), '1' if min(all_dis) < threshold else '0'))
        predict_results.append((test_wav, '1' if min(all_dis) < threshold else '0'))
    
    print('#save result')
    with open(prediction_path, 'w') as f:
        for predict_result in predict_results:
            f.write('\t'.join(predict_result) + '\n')
    
    
if __name__ == '__main__':
    if sys.argv[1] == 'enroll': # for enrollment step
        enroll_data_dir = sys.argv[2]
        work_dir = sys.argv[3]
        if not os.path.exists(work_dir):
            os.mkdir(work_dir)

        enroll(enroll_data_dir, work_dir)

    if sys.argv[1] == 'test': # for predict step
        test_data_dir = sys.argv[2]
        work_dir = sys.argv[3]
        prediction_path = sys.argv[4]

        predict(test_data_dir, work_dir, prediction_path)


        
    
    
    
    
    
