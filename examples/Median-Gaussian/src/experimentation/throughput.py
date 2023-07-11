import sys
from statistics import mean
from statistics import stdev

N_RR = sys.argv[1]
time = sys.argv[2]
preemption = int(sys.argv[3])
n_iters=10

n_tasks=30

if preemption == 0:
    mode = "no-preemption"
else:
    mode = "preemption"


if __name__ == "__main__":
    for size in [200, 300, 400, 500, 600]:
        end_time = []
        inv_end_time = []
        for iter in range(1,n_iters+1):
            #f = open(f"{size}/exp_30tasks-{N_RR}rr-{mode}-{time}-{size}-{iter}.txt")
            f = open(f"seed15/{size}/priority-exp_30tasks-{N_RR}rr-{mode}-{time}-{size}-{iter}.txt")

            last_line = f.read().split("\n")[-2]

            end_time.append(float(last_line.split()[1]))
            inv_end_time.append(1/end_time[-1])

        print(f"{n_tasks / mean(end_time)} {stdev(inv_end_time)}")

