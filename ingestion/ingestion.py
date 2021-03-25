import os
from os.path import join
import sys
from sys import path
import timing
from timing import Timer
from common import get_logger, init_usermodel
import argparse
from filelock import FileLock
import subprocess
import yaml
import time

# Verbosity level of logging:
# Can be: NOTSET, DEBUG, INFO, WARNING, ERROR, CRITICAL
VERBOSITY_LEVEL = 'INFO'
LOGGER = get_logger(VERBOSITY_LEVEL, __file__)

# 30 minutes for initialize.sh
INIT_TIME_BUDGET = 1800
# 5 minutes for each enrollment.sh
ENROLL_TIME_BUDGET = 300 * 5
# 0.25 is real time ratio, and 866 is total duration of all test audio
PREDICT_TIME_BUDGET = 60 + 0.25 * 866 
PREDICTION_FILE = 'predictions.txt'

def _here(*args):
    """Helper function for getting the current directory of this script."""
    here = os.path.dirname(os.path.realpath(__file__))
    return os.path.abspath(os.path.join(here, *args))

def write_start_file(output_dir):
    """write start file"""
    start_filepath = os.path.join(output_dir, 'start.txt')
    lockfile = os.path.join(output_dir, 'start.txt.lock')
    ingestion_pid = os.getpid()

    with FileLock(lockfile):
        with open(start_filepath, 'w') as ftmp:
            ftmp.write(str(ingestion_pid))

    LOGGER.info('===== Finished writing "start.txt" file.')


class IngestionError(RuntimeError):
    """Model api error"""

def _parse_args():
    root_dir = _here(os.pardir)
    default_dataset_dir = join(root_dir, "sample_data")
    default_output_path = join(root_dir, "sample_result_submission")
    default_ingestion_program_dir = join(root_dir, "ingestion_program")
    default_code_dir = join(root_dir, "code_submission")
    default_score_dir = join(root_dir, "scoring_output")
    default_temp_dir = join(root_dir, 'temp_output')
    parser = argparse.ArgumentParser()
    parser.add_argument('--dataset_dir', type=str,
                        default=default_dataset_dir,
                        help="Directory storing the dataset (containing "
                             "e.g. enrollment/)")
    parser.add_argument('--output_dir', type=str,
                        default=default_output_path,
                        help="Directory storing the predictions. It will "
                             "contain e.g. [start.txt, predictions, end.yaml]"
                             "when ingestion terminates.")
    parser.add_argument('--ingestion_program_dir', type=str,
                        default=default_ingestion_program_dir,
                        help="Directory storing the ingestion program "
                             "`ingestion.py` and other necessary packages.")
    parser.add_argument('--code_dir', type=str,
                        default=default_code_dir,
                        help="Directory storing the submission code "
                             "`model.py` and other necessary packages.")
    parser.add_argument('--score_dir', type=str,
                        default=default_score_dir,
                        help="Directory storing the scoring output "
                             "e.g. `scores.txt` and `detailed_results.html`.")
    parser.add_argument('--temp_dir', type=str,
                        default=default_temp_dir,
                        help="Directory storing the temporary output."
                             "e.g. save the participants` model after "
                             "trainning.")

    args = parser.parse_args()
    LOGGER.debug(f'Parsed args are: {args}')
    LOGGER.debug("-" * 50)
    if (args.dataset_dir.endswith('run/input') and
            args.code_dir.endswith('run/program')):
        LOGGER.debug("Since dataset_dir ends with 'run/input' and code_dir "
                     "ends with 'run/program', suppose running on "
                     "CodaLab platform. Modify dataset_dir to 'run/input_data'"
                     " and code_dir to 'run/submission'. "
                     "Directory parsing should be more flexible in the code of"
                     " compute worker: we need explicit directories for "
                     "dataset_dir and code_dir.")

        args.dataset_dir = args.dataset_dir.replace(
            'run/input', 'run/input_data')
        args.code_dir = args.code_dir.replace(
            'run/program', 'run/submission')

        # Show directories for debugging
        LOGGER.debug(f"sys.argv = {sys.argv}")
        LOGGER.debug(f"Using dataset_dir: {args.dataset_dir}")
        LOGGER.debug(f"Using output_dir: {args.output_dir}")
        LOGGER.debug(
            f"Using ingestion_program_dir: {args.ingestion_program_dir}")
        LOGGER.debug(f"Using code_dir: {args.code_dir}")
    return args

def _init_python_path(args):
    path.append(args.ingestion_program_dir)
    path.append(args.code_dir)
    os.makedirs(args.output_dir, exist_ok=True)
    os.makedirs(args.temp_dir, exist_ok=True)

def _init_timer(PREDICT_TIME_BUDGET):
    timer = Timer()
    timer.add_process('initialize.sh', INIT_TIME_BUDGET, timing.CUM)
    LOGGER.debug(
        f"init time budget of initialize.sh: {INIT_TIME_BUDGET} "
        f"mode: {timing.CUM}")
    timer.add_process('enrollment.sh', ENROLL_TIME_BUDGET, timing.CUM)
    LOGGER.debug(
        f"init time budget of enrollment.sh: {ENROLL_TIME_BUDGET} "
        f"mode: {timing.CUM}")
    timer.add_process('predict.sh', PREDICT_TIME_BUDGET, timing.CUM)
    LOGGER.debug(
        f"init time budget of predict.sh: {PREDICT_TIME_BUDGET} "
        f"mode: {timing.CUM}")
    return timer

def run_command(com,timer):
    with timer.time_limit(com.split()[1].split('/')[-1]):
        subprocess.run(com,shell=True,check=True)

def _finalize(args, timer):
    # Finishing ingestion program
    end_time = time.time()

    time_stats = timer.get_all_stats()
    for pname, stats in time_stats.items():
        for stat_name, val in stats.items():
            LOGGER.info(f'the {stat_name} of duration in {pname}: {val} sec')

    overall_time_spent = timer.get_overall_duration()

    # Write overall_time_spent to a end.yaml file
    end_filename = 'end.yaml'
    lockfile = os.path.join(args.output_dir, 'end.yaml.lock')
    content = {
        'ingestion_duration': overall_time_spent,
        'time_stats': time_stats,
        'end_time': end_time}
    with FileLock(lockfile):
        with open(join(args.output_dir, end_filename), 'w') as ftmp:
            yaml.dump(content, ftmp)
            LOGGER.info(
                f'Wrote the file {end_filename} marking the end of ingestion.')

            LOGGER.info("[+] Done. Ingestion program successfully terminated.")
            LOGGER.info(f"[+] Overall time spent {overall_time_spent:5.2} sec")

    # Copy all files in output_dir to score_dir
    os.system(
        f"cp -R {os.path.join(args.output_dir, '*')} {args.score_dir}")
    LOGGER.debug(
        "Copied all ingestion output to scoring output directory.")

    LOGGER.info("[Ingestion terminated]")

def main():
    """main entry"""
    LOGGER.info('===== Start ingestion program.')
    # Parse directories from input arguments
    LOGGER.info('===== Initialize args.')
    args = _parse_args()


    #prepare for processing
    _init_python_path(args)
    write_start_file(args.output_dir)
    timer = _init_timer(PREDICT_TIME_BUDGET)

    command_initialize = f'/bin/bash {args.code_dir}/initialize.sh'
    #do init
    run_command(command_initialize, timer)


    prediction_dir = join(args.output_dir, 'predictions')
    os.mkdir(prediction_dir)
    for speaker in sorted(os.listdir(args.dataset_dir)):
        enrollment_dir = join(args.dataset_dir, speaker, 'enroll')
        test_dir = join(args.dataset_dir, speaker, 'test')
        work_dir = join(args.temp_dir, speaker)
        prediction_path = join(prediction_dir, f'{speaker}_{PREDICTION_FILE}')

        command_enrollment=f'/bin/bash {args.code_dir}/enrollment.sh {enrollment_dir} {work_dir}'
        command_predict=f'/bin/bash {args.code_dir}/predict.sh {test_dir} {work_dir} {prediction_path}'
        #do enrollment
        run_command(command_enrollment,timer)
        #do test
        run_command(command_predict, timer)

    _finalize(args, timer)


if __name__ == "__main__":
    main()
