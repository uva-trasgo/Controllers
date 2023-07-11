#!/bin/bash

N_RR=$1

#./gen_serving_time_data.sh ${N_RR}
./gen_stats.sh ${N_RR}

#python3 plots.py ${N_RR}
python3 stats_table.py > stats/stats_table-${N_RR}rr.txt
