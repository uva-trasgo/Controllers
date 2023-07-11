#!/bin/bash

N_RR=$1
for time in 0.1 0.5 0.8; do
    for size in 200 300 400 500 600; do
        python3 postprocess.py seed15/$size/priority-exp_30tasks-${N_RR}rr-no-preemption-${time}-${size}-1.txt count_queue > stats/serving_times-no-preemption-${time}-${size}.txt
        echo "===" >> stats/serving_times-no-preemption-${time}-${size}.txt
        python3 postprocess.py seed15/$size/priority-exp_30tasks-${N_RR}rr-no-preemption-${time}-${size}-1.txt n_interrupts >> stats/serving_times-no-preemption-${time}-${size}.txt
        echo "===" >> stats/serving_times-no-preemption-${time}-${size}.txt
        python3 postprocess.py seed15/$size/priority-exp_30tasks-${N_RR}rr-no-preemption-${time}-${size}-1.txt n_swaps >> stats/serving_times-no-preemption-${time}-${size}.txt


        python3 postprocess.py seed15/$size/priority-exp_30tasks-${N_RR}rr-preemption-${time}-${size}-1.txt count_queue > stats/serving_times-preemption-${time}-${size}.txt
        echo "===" >> stats/serving_times-preemption-${time}-${size}.txt
        python3 postprocess.py seed15/$size/priority-exp_30tasks-${N_RR}rr-preemption-${time}-${size}-1.txt n_interrupts >> stats/serving_times-preemption-${time}-${size}.txt
        echo "===" >> stats/serving_times-preemption-${time}-${size}.txt
        python3 postprocess.py seed15/$size/priority-exp_30tasks-${N_RR}rr-preemption-${time}-${size}-1.txt n_swaps >> stats/serving_times-preemption-${time}-${size}.txt
    done
done
