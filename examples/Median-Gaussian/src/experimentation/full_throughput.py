import sys
from statistics import mean

N_RR = int(sys.argv[1])

modes = ["no-preemption", "preemption"]
sizes = [200, 300, 400, 500, 600]
times = [0.1, 0.5, 0.8]

FULL_TIME=float(0.2160231)
PARTIAL_TIME=float(0.06879160000000001)

N_PRIORITIES = 5

#serving_times/avg_service_times-1rr-no-preemption-0.1-200-priority-15.txt

for mode_idx in range(len(modes)):
    for time_idx in range(len(times)):
        #n_reconfigs = []
        full_throughputs = []

        for size_idx in range(len(sizes)):
            #Calculate average execution time
            #priority-exp_30tasks-1rr-no-preemption-0.8-600-10.txt
            avg_exec_time = []
            for it in range(1,11):
                f = open(f"seed15/{sizes[size_idx]}/priority-exp_30tasks-{N_RR}rr-{modes[mode_idx]}-{times[time_idx]}-{sizes[size_idx]}-{it}.txt")
                mean_times = float(f.read().split("\n")[-2].split(" ")[1])
                #print(f"meantimes: {mean_times}")
                avg_exec_time.append(mean_times)


            f_table = open(f"stats/stats_table-{N_RR}rr.txt")
            all_tablelines = f_table.read().split("\n")
            #print(all_tablelines[6 * (size_idx+1)-1])
            swap_line = all_tablelines[6 * (size_idx+1)-1].split("&")

            #print(f"Size: {sizes[size_idx]}, time: {times[time_idx]}, mode: {modes[mode_idx]}")
            #print(swap_line[1+mode_idx*3+time_idx])
            n_reconfigs = float(swap_line[1+mode_idx*3+time_idx])
            #print(f"n_reconfigs: {n_reconfigs}")
            #print(f"mean_avg_exec: {mean(avg_exec_time)}")
            #print(f"full - partial: {(FULL_TIME - PARTIAL_TIME)}")

            full_partial = FULL_TIME - PARTIAL_TIME
            avg_exec = mean(avg_exec_time)

            simulated_time = mean(avg_exec_time) + (FULL_TIME - PARTIAL_TIME) * n_reconfigs
            #print(f"Avg. exec time: {mean(avg_exec_time)}")
            #print(f"Simulated time: {simulated_time}")
            full_throughputs.append(30 / simulated_time)

        print(f"Mode: {modes[mode_idx]}, time: {times[time_idx]}")
        print(full_throughputs)

        f_out = open(f"throughput/full_{N_RR}rr-{modes[mode_idx]}-{times[time_idx]}.txt", "w")
        for i in range(len(full_throughputs)):
            f_out.write(str(full_throughputs[i]))
            f_out.write("\n")
