#!/bin/bash

mkdir -p /tmp/ramdisk
sudo mount -t tmpfs -o size=5m myramdisk /tmp/ramdisk

BUILD_DIR=/home/xilinx/controller-DPR/controllers/build/examples/Median-Gaussian
EXP_DIR=/home/xilinx/controller-DPR/controllers/examples/Median-Gaussian/src/experimentation
CURR_DIR=$(pwd)

cpu=0
#n_rr=2
#priorities=$1
#preemption=$2
n_tasks=30
#time=$4

echo $time


cd $BUILD_DIR

cp Median-Gaussian_Pynq_Ctrl-backup Median-Gaussian_Pynq_Ctrl 

modes=("no-preemption" "preemption")
priorities=("no-priority" "priority")

for seed in 15; do
    #for n_rr in 1 2; do
    for n_rr in 2; do
        #for p in 0 1; do
        for p in 1; do
            #for mode in 0 1; do
            for mode in 0; do
            #for size in 200 300 400 500 600; do
            for size in 200; do
                mkdir -p $EXP_DIR/$size

                #for time in 0.1 0.5 0.8; do
                for time in 0.1; do
                for iter in {1..10}; do
                    echo "Size: ${size}. Time: ${time}"
                    repeat=1
                    while [[ repeat -eq 1 ]]; do
                    sudo timeout 90 ./Median-Gaussian_Pynq_Ctrl $size $size ~/controller-DPR/scheduler/Saltpepper.pgm $cpu $n_rr $p $mode $n_tasks $time $seed

                    exit_code=$?

                    if [ $exit_code -eq 135 ] || [ $exit_code -eq 124 ]; then echo "REPEATING!"; repeat=1; else repeat=0; fi
                    done

                    cp /tmp/ramdisk/exp_file.txt $EXP_DIR/seed$seed/$size/${priorities[p]}-exp_30tasks-${n_rr}rr-${modes[mode]}-$time-$size-$iter.txt
                done
                done
            done
            done
        done
    done
done

cd $(pwd)
