import json
from itertools import product
import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from process_data import gen_tasks, Task, Swap, gen_avg_service_times

N_TASKS = 30
P = [1]
TIME = [0.1, 0.5, 0.8]
RR = 2
SIZE = 600
N_RR = 2

def plot_points(seed):
    SIZES = [200, 300, 400, 500, 600]
    labels = {0.1:'Busy', 0.5:'Medium', 0.8:'Idle'}

    for time in [0.1, 0.5, 0.8]:
        throughputs = []
        for size in SIZES:
            filename = f"results/seed{seed}/{size}/priority-exp_30-2rr-preemption-{time}-{size}-0-1.txt"

            _, _, end_scheduler_time = gen_tasks(filename)

            throughputs.append(N_TASKS/end_scheduler_time)

        plt.plot(SIZES, throughputs, label = labels[time], marker='o')

    # Full reconfiguration
    throughputs = []
    for size in SIZES:
        filename = f"results/seed{seed}/{size}/priority-exp_30-2rr-preemption-0.1-{size}-0-0.txt"

        _, _, end_scheduler_time = gen_tasks(filename)

        throughputs.append(N_TASKS/end_scheduler_time)

    plt.plot(SIZES, throughputs, label = 'Full (busy)', marker='o')

    plt.xlabel("Size (NxN)")
    plt.ylabel("Throughput (Tasks/s)")
    plt.legend()
    plt.savefig(f"../../../AMA-lato_New/plots/throughput-{seed}.pdf")


def plot_stacked(seed, partial):
    N_PRIORITIES = 5
    times = [0.1, 0.5, 0.8]
    modes = ["no-preemption", "preemption"]

    colours = ['b', 'r', 'g', 'y', 'c']
    x = np.arange(3)-0.8
    mode_idx = 0
    for N_RR in [1,2]:
        avg_service_times_df_dict = gen_avg_service_times(seed, N_RR)
        #for size in [200, 300, 400, 500, 600]:
        for size in [600]:
            #plt.figure(figsize=(5,6))
            plt.rcParams.update({'font.size': 15})
            plt.xlabel("Rate of task arrival")
            plt.ylabel("Service time (log(s))")

            previous_service_times = [0,0,0,0,0,0]

            service_times = [[] for i in range(N_PRIORITIES)]
            times_std = [[] for i in range(N_PRIORITIES)]

            plt.yscale('log', base=2)
            #For seed 15 and 2 RR
            if N_RR == 2:
                if partial:
                    plt.ylim([1,50])
                else:
                    plt.ylim([1,100])
            #For seed 15 and 1 RR
            if N_RR == 1:
                plt.ylim([1,100])
            

            for p in range(N_PRIORITIES):
                for i in range(len(times)):
                    for m in [0,1]:
                        service_times[p].append(avg_service_times_df_dict[(N_RR,size, 1, m, times[i], partial)][p])

                    current_colour = list(colours[p])


                width = 0.4

                plt.bar(x-0.2, service_times[p][::2], width, bottom=previous_service_times[::2], color=current_colour, label=f"Priority {p}", edgecolor='black')
                plt.bar(x+0.2, service_times[p][1::2], width, bottom=previous_service_times[1::2], color=current_colour, hatch="//", edgecolor='black')
                for i in range(2*len(times)):
                    previous_service_times[i] += service_times[p][i]

            plt.legend(fontsize=10)

            plt.xticks(x, ["Busy", "Medium", "Idle"])
            #plt.show()

            plt.savefig(f"../../../AMA-lato_New/plots/service_times-{N_RR}rr-{size}-{seed}.pdf")

            #plt.savefig(f"service_times/service_times-{mode}-{N_RR}rr-{size}-15.pdf")
            #plt.savefig(f"service_times/service_times-{N_RR}rr-{size}-{seed}.pdf")
            plt.clf()

def plot_gantt(seed, enabled_partial):
    filename = f"results/seed{seed}/600/priority-exp_30-2rr-preemption-0.1-600-0-{enabled_partial}.txt"
    tasks_lst, swap_lst, end_scheduler = gen_tasks(filename, True) 



    # Declaring a figure "gnt"
    fig, gnt = plt.subplots()

    # Setting Y-axis limits
    gnt.set_ylim(5, 15 + 10 * N_RR)


    # Setting X-axis limits
    gnt.set_xlim(0, end_scheduler + 0.1)

    # Setting labels for x-axis and y-axis
    gnt.set_xlabel('seconds since start')
    gnt.set_ylabel('Reconfigurable Region')

    # Setting ticks on y-axis
    gnt.set_yticks([(15 + 10 * i) for i in range(N_RR)])
    # Labeling ticks of y-axis
    gnt.set_yticklabels(str(i) for i in range(N_RR))

    # Setting graph attribute
    gnt.grid(True)


    for task in tasks_lst:
        evicted = False if len(task.launch) == 1 else True
        for i in range(len(task.launch)):
            start, end = task.launch[i]
            duration = end - start
            gnt.broken_barh([(start, duration)], (10 * (1 + task.rr[i]), 9), facecolors = task.colour, hatch=task.hatch)
    for swap in swap_lst:
        if swap.rr == None:
            rr = 0
            gnt.broken_barh([(swap.start_time, swap.duration)], (10 * (1+rr), 9), facecolors = 'black')
            rr = 1
            gnt.broken_barh([(swap.start_time, swap.duration)], (10 * (1+rr), 9), facecolors = 'black')
        else:
            rr = swap.rr
            gnt.broken_barh([(swap.start_time, swap.duration)], (10 * (1+rr), 9), facecolors = 'black')

    #plt.show()
    endname = "full" if partial == 0 else "partial"
    plt.savefig(f"../../../AMA-lato_New/plots/gantt-{endname}.pdf")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("seed", type=int)
    parser.add_argument("partial", type=int)
    parser.add_argument("plot_type", type=str)
    args = parser.parse_args()

    seed = args.seed
    partial = args.partial
    plot_type = args.plot_type

    if plot_type == 'stacked':
        plot_stacked(seed, partial)

    if plot_type == 'gantt':
        for partial in [0,1]:
            plot_gantt(seed, partial)

    if plot_type == 'points':
        plot_points(seed)
