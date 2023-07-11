import argparse
import subprocess
import os

N_TASKS = 30
#N_ITERS = 10
N_ITERS = 2

if __name__ == "__main__":
    modes = ["no-preemption", "preemption"]
    priorities = ["no-priority", "priority"]

    parser = argparse.ArgumentParser()
    parser.add_argument("seed", type=int)
    args = parser.parse_args()

    seed = args.seed

    try:
        os.mkdir("service_times")
    except:
        pass

#for rr in [1, 2]:
#    for m in [0, 1]:
for rr in [2]:
    for m in [0, 1]:
        for p in [1]:
            for size in [600]:
                for time in [0.1, 0.5, 0.8]:
                    for iter in range(N_ITERS):
                        print(f"service_times/service_times-{rr}rr-{modes[m]}-{time}-{size}-{iter}-{priorities[p]}-{seed}.txt")
                        output_file = open(f"service_times/service_times-{rr}rr-{modes[m]}-{time}-{size}-{iter}-{priorities[p]}-{seed}.txt", "w")
                        #print(f"python3 postprocess.py results/seed{seed}/{size}/{priorities[p]}-exp_{N_TASKS}-{rr}rr-{modes[m]}-{time}-{size}-{iter}.txt avg_service_time {p}")
                        print(f"python3 postprocess.py results/seed{seed}/{size}/{priorities[p]}-exp_{N_TASKS}-{rr}rr-{modes[m]}-{time}-{size}-{iter}.txt avg_service_time {p}")
                        subprocess.call(f"python3 postprocess.py results/seed{seed}/{size}/{priorities[p]}-exp_{N_TASKS}-{rr}rr-{modes[m]}-{time}-{size}-{iter}.txt avg_service_time {p}", stdout=output_file, shell=True)
                        output_file.close()

# Average them all
for p in [1]:
    for rr in [2]:
        for m in [0, 1]:
            for size in [600]:
                for time in [0.1, 0.5, 0.8]:
                    output_file = open(f"service_times/avg_service_times-{rr}rr-{modes[m]}-{time}-{size}-{priorities[p]}-{seed}.txt", "w")
                    subprocess.call(f"python3 average_service_times.py {N_ITERS} {size} {time} {m} {rr} {p} {seed}", stdout=output_file, shell=True)
                    output_file.close()
