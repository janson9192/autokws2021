Auto-KWS 2021 Challenge starting kit
======================================
## About Challenge:
https://www.4paradigm.com/competition/autospeech2021

## Contents
ingestion/: The code and libraries used on Codalab to run your submmission.

scoring/: The code and libraries used on Codalab to score your submmission.

code_submission/: An example of code submission you can use as template.

sample_data/: Some sample data to test your code before you submit it.

run_local_test.py: A python script to simulate the runtime in codalab.

## Local development and testing
1. The audios in sample_data are the same as those in Practice Dataset.

2. You can also make other data in the same format as sample_data and debug the code locally.

3. To make your own submission to Auto-KWS 2021 challenge, you need to modify the file 
`initialize.sh`, `enrollment.sh` and `predict.sh` in `code_submission/`, which implements your 
algorithm.

1) `initialize.sh` is for preparing the environment, you can `pip install` or `make` your lib 
or even create a new virtual environment.
2) `enrollment.sh ${enrollment_dir} ${work_dir}` is for loading enrollment data for one speaker, 
and implement the enrollment strategy. Enrollment features, and any temporary files can be 
saved in work_dir.
3) `predict.sh ${test_dir} ${work_dir} ${prediction_file}` is for inference, you can use the 
fintuned model or features to predict the labels of audio in test_dir. and output predictions
need to be written into prediction_file.
Note: For each speaker, platform will run enrollment.sh and predict.sh once (in ingestion.py),
then score.py will calculate final scores by all prediction files and all true labels.

4. Test the algorithm on your local computer using Docker,
in the exact same environment as on the Automl.ai challenge platform. Advanced
users can also run local test without Docker, if they install all the required
packages.

5. If you are new to docker, install docker from https://docs.docker.com/get-started/.
Then, at the shell, run:
```
cd path/to/autokws2021_starting_kit/
docker run --gpus '"device=0"'  -it -v "$(pwd):/app/auto-kws" janson91/autokws2021:gpu /bin/bash
```
The option `-v "$(pwd):/app/codalab"` mounts current directory
(`autospeech_starting_kit/`) as `/app/codalab`. If you want to mount other
directories on your disk, please replace `$(pwd)` by your own directory.

The Docker image
```
janson91/autokws2021:gpu
```
has Nvidia GPU supports. see (https://hub.docker.com/r/janson91/autokws2021)
to check installed packages in the docker image.
Docker based nvidia/cuda:10.0-cudnn7-devel-ubuntu16.04.
Kaldi (https://github.com/kaldi-asr/kaldi.git)  and Anaconda3 has been built in docker.
And We have create an anaconda environment named py36, which contains:
    tensorflow-gpu==1.14
    pytorch==1.5
    librosa==0.7.0

Another tip for docker beginner: please download and unzip the baseline files to your starting kit dir if you want to test out the baseline locally. 

6. You will then be able to run the `ingestion program` (to produce predictions)
and the `scoring program` (to evaluate your predictions) on sample data.
In the Auto-KWS 2021 challenge, both two programs will run in parallel. So we provide 
a Python script to simulate this behavior. To test locally, run:
```
python run_local_test.py
```
Then you can view the scores and detialed results in `scoring_output/`.

The full usage is
```
python run_local_test.py --dataset_dir=`pwd`/sample_data/practice 
    --solution_dir=`pwd`/sample_data/practice_solution --code_dir=./code_submission
```
You can change the argument `dataset_dir` to other datasets. On the other hand,
you can also modify the directory containing your other sample code.

## Prepare a ZIP file for submission on Automl.ai
Zip the contents of `code_submission` without the directory structure:
```
cd code_submission/
zip -r mysubmission.zip *
```
then use the "Upload a Submission" button to make a submission to the
competition page on Automl.ai platform.

Tip: to look at what's in your submission zip file without unzipping it, you
can do
```
unzip -l mysubmission.zip
```
Note: It is recommended to control the size of the uploaded compressed package (less than 5GB), which 
is too large to cause the risk of upload failure.

## Baseline
There are two baselines:
- baseline_1 consists of a query by example (QbyE) system and a speaker verification (SV) system,
which use Kaldi framework.
- baseline_2 extract features by wav2vec 2.0 model and calculate similarity by dtw.
you can download baselines and follow the steps above to test them.
More details can be found in README in baseline dir.

you can 

```
cd starting_kit
unzip baseline.zip -d baseline
python run_local_test.py --code_dir=./baseline
```

to test out the baseline model.

## Contact us
If you have any questions, please contact us via:
<autospeech2021@4paradigm.com>

