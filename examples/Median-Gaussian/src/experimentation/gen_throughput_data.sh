#!/bin/bash

modes=("no-preemption" "preemption")

mkdir -p throughput

for N_RR in 1 2; do
	for mode in 0 1; do
		for time in 0.1 0.5 0.8; do
			python3 throughput.py ${N_RR} ${time} ${mode} > throughput/throughput-${N_RR}-${modes[mode]}-${time}.txt
		done
	done
done
