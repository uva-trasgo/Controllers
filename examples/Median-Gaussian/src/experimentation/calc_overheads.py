import sys
from statistics import mean
from statistics import stdev

N_RR = sys.argv[2]
seed = 15
modes = ["no-preemption", "preemption"]
times = [0.1, 0.5, 0.8]
sizes = [200,300,400,500,600]

if sys.argv[1] == "throughput": 
    #for mode in modes:
    for time in times:
        throughput = [[] for i in range(len(modes))]
        for mode_idx in range(len(modes)):
            f = open(f"throughput/throughput-{N_RR}-{modes[mode_idx]}-{time}.txt")

            for size in sizes:
                line = f.readline()
                mean_value = float(line.split(" ")[0])
                throughput[mode_idx].append(mean_value)

        print(throughput)
        print(f"Time {time}")
        for size_idx in range(len(sizes)):
            print(f"Size {sizes[size_idx]}: {(throughput[0][size_idx] - throughput[1][size_idx]) / throughput[0][size_idx]}")
elif sys.argv[1] == "total":

    all_overheads = []
    for size in sizes:
        for time in times:
            exec_times = [[] for i in range(len(modes))]
            for mode_idx in range(len(modes)):
                for it in range(1,11):
                    f = open(f"seed15/{size}/priority-exp_30tasks-{N_RR}rr-{modes[mode_idx]}-{time}-{size}-{it}.txt")

                    all_lines = f.read().split("\n")
                    exec_times[mode_idx].append(float(all_lines[-2].split(" ")[1]))

            #overhead = mean(exec_times[1]) / mean(exec_times[0])
            overheads = []
            for i in range(len(exec_times[0])):
                overheads.append(exec_times[1][i] / exec_times[0][i])
            print(exec_times[1])
            print(exec_times[0])
            print(f"size: {size}, N_RR: {N_RR}, mode: {modes[mode_idx]}, time: {time}, overhead: {mean(overheads)}, std overhead: {stdev(overheads)}")
            all_overheads.append(mean(overheads))

    print(f"all overheads mean: {mean(all_overheads)}, stdev: {stdev(all_overheads)}")

    #priority-exp_30tasks-2rr-no-preemption-0.8-600-9.txt

