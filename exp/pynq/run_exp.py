import os
import subprocess
import argparse
from SyncRNG import SyncRNG


BASE_DIR = "../../build/examples/Median-Gaussian"
IMG = "../../../scheduler/Saltpepper.pgm"
CPU = 0
N_RR = 2
N_TASKS = 30
SIZES = [200, 300, 400, 500, 600]
N_ITERS = 10

modes = ["no-preemption", "preemption"]
priorities = ["no-priority", "priority"]


times = [0.1, 0.5, 0.8]
n_seeds = 10
seeds = [28871727]

if __name__ == "__main__":
    try:
        os.mkdir("/tmp/ramdisk")
    except:
        pass


    s = SyncRNG(seed=seeds[0])
    for i in range(n_seeds-1):
        new_seed = s.randi()
        seeds.append(new_seed)

    subprocess.call("sudo mount -t tmpfs -o size=5m myramdisk /tmp/ramdisk", shell=True)

    #parser = argparse.ArgumentParser()
    #parser.add_argument("n_rr", type=int)
    #parser.add_argument("seed", type=int)

    #args = parser.parse_args()
    #n_rr = args.n_rr
    #seed = args.seed

timeout_times = {1:{0.1:50, 0.5:56, 0.8:60}, 2:{0.1:35, 0.5:45, 0.8:55}}

for partial in [1]:
    for seed in seeds:
        EXP_DIR = f"./results-review/seed{seed}"

        try:
            os.makedirs(EXP_DIR)
        except:
            pass

        for rr in [1,2]:
            for p in [1]:
                for m in [0,1]:
                    for size in SIZES:
                        try:
                            output_dir = f"{EXP_DIR}/{size}/"
                            os.makedirs(output_dir)
                        except:
                            pass

                        for iter in range(N_ITERS):
                        #for iter in [1]:
                            for time in times:
                                print(f"Running seed: {seed}, p: {p}, m: {m}, rr: {rr}, time: {time}, iter: {iter}", flush=True)
                                repeat = 1
                                while repeat == 1:
                                    print(f"sudo {BASE_DIR}/Median-Gaussian_Pynq_Ctrl {size} {size} {IMG} {CPU} {rr} {p} {m} {N_TASKS} {time} {seed} {partial}")
                                    exit_code = subprocess.call(f"sudo timeout --signal=SIGINT -k 10 90 {BASE_DIR}/Median-Gaussian_Pynq_Ctrl {size} {size} {IMG} {CPU} {rr} {p} {m} {N_TASKS} {time} {seed} {partial}", shell=True)

                                    if exit_code != 0:
                                        repeat = 1
                                    else:
                                        repeat = 0

                                subprocess.call(f"cp /tmp/ramdisk/exp_file.txt {output_dir}/{priorities[p]}-exp_{N_TASKS}-{rr}rr-{modes[m]}-{time}-{size}-{iter}-{partial}.txt", shell=True)
