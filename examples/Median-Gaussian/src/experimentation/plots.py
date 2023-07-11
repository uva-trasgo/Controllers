import matplotlib.pyplot as plt
import matplotlib
import sys
import numpy as np

N_TASKS = 30
times = [0.1,0.5,0.8]

N_PRIORITIES = 5


colours = ['b', 'r', 'g', 'y', 'c']

#N_RR = int(sys.argv[2])



if sys.argv[1] == "stacked":
    #size = 600
    x = np.arange(3)-0.8
    mode_idx = 0
    for N_RR in [1, 2]:
        #for size in [200, 300, 400, 500, 600]:
        for size in [600]:
            #plt.figure(figsize=(5,6))
            plt.rcParams.update({'font.size': 15})
            plt.xlabel("Rate of task arrival")
            plt.ylabel("Service time (s)")

            previous_serving_times = [0,0,0,0,0,0]

            serving_times = [[] for i in range(N_PRIORITIES)]
            times_std = [[] for i in range(N_PRIORITIES)]

            #For seed 15 and 2 RR
            if N_RR == 2:
                plt.ylim([0,60])
            #For seed 15 and 1 RR
            if N_RR == 1:
                plt.ylim([0,120])


            for p in range(N_PRIORITIES):
                for i in range(len(times)):
                    for mode in ["no-preemption", "preemption"]:
                        #f = open(f"serving_times/serving_times-{mode}-0_{int(times[i]*10)}.txt")
                        f = open(f"serving_times/avg_service_times-{N_RR}rr-{mode}-{times[i]}-{size}-priority-15.txt")

                        # Discard previous priorities lines
                        for j in range(p):
                            f.readline()
                        line = f.readline().strip()
                        mean_time = float(line.split()[0])
                        std_time = float(line.split()[1])
                        serving_times[p].append(mean_time)
                        times_std[p].append(std_time)

                    current_colour = list(colours[p])

                print(serving_times[p][::2])
                print(serving_times[p][1::2])

                width = 0.4

                plt.bar(x-0.2, serving_times[p][::2], width, yerr=times_std[p][::2], bottom=previous_serving_times[::2], color=current_colour, label=f"Priority {p}", edgecolor='black')
                plt.bar(x+0.2, serving_times[p][1::2], width, yerr=times_std[p][1::2], bottom=previous_serving_times[1::2], color=current_colour, hatch="//", edgecolor='black')
                for i in range(2*len(times)):
                    previous_serving_times[i] += serving_times[p][i]

            plt.legend()

            #plt.show()
            plt.xticks(x, ["Busy", "Medium", "Idle"])

            #plt.savefig(f"serving_times/service_times-{mode}-{N_RR}rr-{size}-15.pdf")
            plt.savefig(f"serving_times/service_times-{N_RR}rr-{size}-15.pdf")
            plt.clf()

elif sys.argv[1] == "points":
    sizes = [200,300,400,500,600]
    arrival_rates = ["Busy", "Medium", "Idle"]

    #fig, axs = plt.subplots(2, 2)
    fig = plt.figure(figsize=(6.4,4.2))
    gs = fig.add_gridspec(2, 2, hspace=0.05, wspace=0.05)
    #(ax1, ax2), (ax3, ax4) = gs.subplots(sharex='col', sharey='row')
    axs = gs.subplots(sharex='col', sharey='row')
    plot_coords = [[0,0],[0,1],[1,0],[1,1]]
    _x = [0,0,1,1]
    _y = [0,1,0,1]
    n_plot = 0

    fontsize=8


    for N_RR in [1,2]:
        for mode in ["no-preemption", "preemption"]:
            ##For seed 15 and 2 RR
            if N_RR == 2:
                axs[_x[n_plot]][_y[n_plot]].set_ylim([0,5.8])
            #For seed 15 and 1 RR
            if N_RR == 1:
                axs[_x[n_plot]][_y[n_plot]].set_ylim([0,3.6])


            axs[_x[n_plot]][_y[n_plot]].set_xlabel("Size (NxN)", size=fontsize)
            if mode == "no-preemption":
                axs[_x[n_plot]][_y[n_plot]].set_ylabel("Throughput (tasks/s)", size=fontsize)
            axs[_x[n_plot]][_y[n_plot]].tick_params(axis='both', labelsize=fontsize)
            time_counter = 0

            for time in [0.1, 0.5, 0.8]:
                mean_throughput = []
                std_throughput = []

                f = open(f"throughput/throughput-{N_RR}-{mode}-{time}.txt")

                for size in range(len([200,300,400,500,600])):
                    line = f.readline()
                    mean_throughput.append(float(line.split()[0]))
                    std_throughput.append(float(line.split()[1]))

                axs[_x[n_plot]][_y[n_plot]].errorbar([200,300,400,500,600], mean_throughput, yerr=std_throughput, marker='o', label=f"{arrival_rates[time_counter]}",lw=0.8,ms=2)
                time_counter += 1
            if N_RR == 1 and mode == "preemption":
                axs[_x[n_plot]][_y[n_plot]].legend()

            f = open(f"throughput/full_{N_RR}rr-{mode}-0.1.txt")
            mean_throughput = []
            for size in sizes:
                mean_throughput.append(float(f.readline()))
            axs[_x[n_plot]][_y[n_plot]].errorbar([200,300,400,500,600], mean_throughput, yerr=std_throughput, marker='o', linestyle="dashed", label=f"Full reconfiguration", lw=0.8, ms=2)
            if N_RR == 1 and mode == "preemption":
                axs[_x[n_plot]][_y[n_plot]].legend(prop={'size': fontsize})

            n_plot += 1

            #axs[_x[n_plot]][_y[n_plot]].savefig(f"throughput/throughput-{N_RR}rr-{mode}.pdf")
            #axs[_x[n_plot]][_y[n_plot]].clf()
    #plt.show()
    fig.tight_layout()
    plt.savefig(f"throughput/throughput.pdf")

