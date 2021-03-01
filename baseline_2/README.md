This is the baseline system for Auto-KWS 2021 Challenge, for details you can visit https://www.4paradigm.com/competition/autospeech 2021

## About the baseline
The baseline use wav2vec 2.0 (https://arxiv.org/pdf/2006.11477.pdf) as feature extractor. After feature extraction, DTW is used for feature comparison. For the longer test audio, it will be compared with the enrollment audio segment by segment, and the minimum score will be taken as the score of test audio. If the score bigger than threshold, the audio is judged to be awake.

## Model
wav2vec model download from: https://dl.fbaipublicfiles.com/fairseq/wav2vec/wav2vec_small.pt

## Other files
- initialize.sh: for preparing your environment.
- enrollment.sh: you need to process the input source enrollment data of one speaker here. The input-dir is like practice/P0001/enroll. And the output-dir is used to store what you need for predict.sh. Here are no mandatory requirements, except the output-dir must be named by the speaker.
- predict.sh: you need to process the input source test data of one speaker here. The input-dir is like practice/P0001/test. The output-dir must be named by the speaker. And the result file must in the spk/result dir and named wake_up_result. The result-file has two columns, the first column is utterance-id and the second is the wake-up result (1 is wake up and 0 is not).



