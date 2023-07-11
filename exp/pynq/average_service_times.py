import sys
from statistics import mean
from statistics import stdev


if __name__ == "__main__":
    n_iters = int(sys.argv[1])
    size = int(sys.argv[2])
    time = float(sys.argv[3])
    preemption = int(sys.argv[4])
    N_RR = int(sys.argv[5])
    enabled_priorities = int(sys.argv[6])
    seed = int(sys.argv[7])

    if enabled_priorities == 1:
        N_PRIORITIES = 5
    elif enabled_priorities == 0:
        N_PRIORITIES = 1
    if preemption == 0:
        mode = "no-preemption"
    else:
        mode = "preemption"


    times = [[] for i in range(N_PRIORITIES)]
    
    for iter in range(n_iters):
    #for iter in range(1,2):
        if enabled_priorities:
            f = open(f"service_times/service_times-{N_RR}rr-{mode}-{time}-{size}-{iter}-priority-{seed}.txt")
        elif not enabled_priorities:
            f = open(f"service_times/service_times-{N_RR}rr-{mode}-{time}-{size}-{iter}-no-priority-{seed}.txt")
        for p in range(N_PRIORITIES):
            times[p].append(float(f.readline().strip()))

    for p in range(N_PRIORITIES):
        #times[p] /= n_iters
        print(f"{mean(times[p])} {stdev(times[p])}")
        #print(f"{mean(times[p])} 0")
