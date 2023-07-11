#!/bin/bash

#N_RR=$1

modes=("no-preemption" "preemption")
priorities=("no-priority" "priority")

for seed in 15; do
    #for p in 0 1; do
    for p in 1; do
        for N_RR in 1 2; do
            for mode in 0 1; do
                for size in 200 300 400 500 600; do
                    for time in 0.1 0.5 0.8; do
                    echo "Working on size ${size}, time ${time}"
                    for iter in {1..10}; do
                        python3 postprocess.py seed${seed}/$size/${priorities[p]}-exp_30tasks-${N_RR}rr-${modes[mode]}-${time}-${size}-${iter}.txt avg_serving_time ${p} > serving_times/serving_times-${N_RR}rr-${modes[mode]}-${time}-${size}-${iter}-${priorities[p]}-${seed}.txt
                    done
                    done
                done

                #Average them all
                for size in 200 300 400 500 600; do
                    for time in 0.1 0.5 0.8; do
                    	python3 average_service_times.py 10 ${size} ${time} ${mode} ${N_RR} ${p} ${seed} > serving_times/avg_service_times-${N_RR}rr-${modes[mode]}-${time}-${size}-${priorities[p]}-${seed}.txt
                    done
                done
            done
        done
    done
done
