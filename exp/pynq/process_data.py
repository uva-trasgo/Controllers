import json
import matplotlib.pyplot as plt
from itertools import product
import numpy as np
import pandas as pd
from SyncRNG import SyncRNG

N_TASKS = 30
P = [1]
TIME = [0.1, 0.5, 0.8]
RR = 2
SIZE = 600
N_RR = 2


class Task:
    def __init__(self, task_id, priority, generation_time, colour):
        self.id = task_id
        self.priority = priority
        self.generation_time = generation_time
        self.rr = []
        self.launch = []
        self.colour = colour
        self.hatch = None

        self.launch_counter = 0

        self.evictions = []

    def set_arrival_time(self, time):
        self.arrival_time = time

    def add_launch_start(self, start_time, rr):
        self.launch.append(start_time)
        self.rr.append(rr)

    def add_launch_finish(self, end_time):
        start_time = self.launch[self.launch_counter]
        self.launch[self.launch_counter] = (start_time, end_time)
        self.launch_counter += 1

    def get_service_time(self):
        return self.launch[0][0] - self.arrival_time

    def add_eviction(self, eviction):
        self.evictions.append(eviction)


class Swap:
    def __init__(self, start_time, end_time, rr, trigger_time):
        self.start_time = start_time
        self.end_time = end_time
        self.duration = end_time - start_time
        self.rr = rr
        self.trigger_time = trigger_time


class Eviction:
    def __init__(self, running_task_id, incoming_task_id, time):
        self.running_task_id = running_task_id
        self.incoming_task_id = incoming_task_id
        self.time = time

    def __repr__(self):
        return f"{self.time}"

def get_cmap(n, name='hsv'):
    return plt.cm.get_cmap(name, n)

def gen_tasks(filename, gen_hatch=False):
    patterns_iter = iter(['/', '\\', '|', '-', '+', 'x', 'o', 'O', '.', '*'])

    ops = []

    for line in open(filename):
        ops.append(json.loads(line))

    tasks = []
    swaps = []
    #evictions = []

    cmap = get_cmap(N_TASKS)

    # Stores the launch time of the task that triggered a reconfiguration
    trigger_time = None

    for json_obj in ops:
        if json_obj['op'] == 'task_generation':
            task_id = json_obj['task_id']
            task = Task(task_id, json_obj['task_priority'], json_obj['time'], cmap(task_id))
            tasks.append(task)

        if json_obj['op'] == 'task_arrival':
            task = tasks[json_obj['task_id']]
            task.arrival_time = json_obj['time']
            trigger_time = json_obj['time']

        if json_obj['op'] == 'launch_task':
            task = tasks[json_obj['task_id']]

            if 'start_time' in json_obj.keys():
                task.add_launch_start(json_obj['start_time'], json_obj['rr'])
            elif 'finish_time' in json_obj.keys():
                task.add_launch_finish(json_obj['finish_time'])
                trigger_time = json_obj['finish_time']

        if json_obj['op'] == 'evict_task':
            task = tasks[json_obj['running_task_id']]
            task.add_launch_finish(json_obj['time'])
            eviction = Eviction(json_obj['running_task_id'], json_obj['incoming_task_id'], json_obj['time'])
            #evictions.append(eviction)
            incoming_task = tasks[json_obj['incoming_task_id']]
            incoming_task.add_eviction(eviction)
            trigger_time = json_obj['time']

        if json_obj['op'] == 'partial_swap':
            swap = Swap(json_obj['start_time'], json_obj['finish_time'], json_obj['rr'], trigger_time)
            swaps.append(swap)

        if json_obj['op'] == 'full_swap':
            swap = Swap(json_obj['start_time'], json_obj['finish_time'], None)
            swaps.append(swap)

        if json_obj['op'] == 'end':
            end_scheduler = json_obj['time']


    #for task in tasks:
    #    print(f"id: {task.id}, launches: {task.launch}, service_time: {task.get_service_time()}")
    #print("SWAP TIMES")
    #for swap in swaps:
    #    print((swap.start_time, swap.end_time))

    if gen_hatch:
        for task in tasks:
            task.hatch = next(patterns_iter) if len(task.launch) > 1 else ''

    return [tasks, swaps, end_scheduler]

def get_avg_interrupt_to_reconfigure():
    N_ITERS = 10

    n_tasks = 0
    mean_mapping_overhead = []
    percent_mean_mapping_overhead = []
    
    seeds = [28871727]
   
    s = SyncRNG(seed=seeds[0]) 
    for i in range(9):
        seeds.append(s.randi())

    mean_ov = []
    std_ov = []

    for seed in seeds:
        for rate in [0.1,0.5,0.8]:
            for rr in [1,2]:
                for it in range(N_ITERS):
                    n_tasks = 0
                    mean_mapping_overhead.append(0)
                    percent_mean_mapping_overhead.append(0)
                    tasks, swaps, end_scheduler = gen_tasks(f"results-review/seed{seed}/600/priority-exp_30-{rr}rr-preemption-{rate}-600-{it}-1.txt")
                    
                    for swap in swaps:    
                        mean_mapping_overhead[-1] += (swap.start_time - swap.trigger_time)
                        n_tasks += 1

                    mean_mapping_overhead[-1] /= n_tasks
                    percent_mean_mapping_overhead[-1] = mean_mapping_overhead[-1] / end_scheduler * 100

    overall_mean_mapping_overhead = np.mean(mean_mapping_overhead)
    overall_std_mapping_overhead = np.std(mean_mapping_overhead)
    overall_percent_mean_mapping_overhead = np.mean(percent_mean_mapping_overhead)
    overall_percent_std_mapping_overhead = np.std(percent_mean_mapping_overhead)

    print("Overall mean mapping overhead: ", overall_mean_mapping_overhead)
    print("Overall std mapping overhead: ", overall_std_mapping_overhead)
    print("Overall percent. mean mapping overhead: ", overall_percent_mean_mapping_overhead)
    print("Overall percent. std mapping overhead: ", overall_percent_std_mapping_overhead)

    return overall_mean_mapping_overhead, overall_std_mapping_overhead, overall_percent_mean_mapping_overhead, overall_percent_std_mapping_overhead


     

def get_avg_service_times(tasks_lst, n_priorities):
    avg_service_times = [0] * n_priorities
    n_tasks_priority = [0] * n_priorities

    for task in tasks_lst:
        avg_service_times[task.priority] += task.get_service_time()
        n_tasks_priority[task.priority] += 1
    
    for p in range(n_priorities):
        avg_service_times[p] /= n_tasks_priority[p]

    return np.array(avg_service_times)

def gen_preemption_overhead(seed, rr):
    N_ITERS = 10

    for rate in [0.1,0.5,0.8]:
        for it in range(N_ITERS):
            tasks, swaps, end_scheduler = gen_tasks(f"results-review/seed{seed}/600/priority-exp_30-{rr}rr-preemption-{rate}-600-{it}-1.txt")

            all_exec_times = []

            total_exec_time = 0
            for task in tasks:
                exec_time = 0 
                for launch_times in task.launch:
                    exec_time += launch_times[1] - launch_times[0]
            
            all_exec_times
            n_evictions = 0
            overhead = 0
            for task in tasks:
                if task.evictions:
                    for i in range(len(task.evictions)):
                        overhead += task.launch[i][0] - task.evictions[i].time
                        n_evictions += 1

    if n_evictions > 0:
        overhead = overhead / n_evictions
    else:
        overhead = -1

    return overhead

def gen_overall_overhead(seed):
    N_ITERS = 10

    overhead = []

    for rate in [0.1, 0.5, 0.8]:
        for it in range(N_ITERS):
            tasks, swaps, end_scheduler = gen_tasks(f"results-review/seed{seed}/600/priority-exp_30-1rr-preemption-0.1-600-{it}-1.txt")

            exec_times = []

            total_exec_time = 0
            for task in tasks:
                for launch_times in task.launch:
                    total_exec_time += launch_times[1] - launch_times[0]

            overhead.append((end_scheduler - total_exec_time) / total_exec_time)

    mean_overhead = np.mean(overhead)
    std_overhead = np.std(overhead)

    print(f"SEED: {seed}")
    print(mean_overhead)
    print(std_overhead)
    print()

    return mean_overhead, std_overhead

        

    
            
        
# This function would calculate the overhead with tasks running concurrently. It is hard to implement and test, 
# so we opted for the simplified version with 1 RR.
#def gen_overall_overhead(seed, rr):
#    tasks, swaps, end_scheduler = gen_tasks(f"results-review/seed{seed}/600/priority-exp_30-1rr-preemption-0.1-600-0-1.txt")
#    
#    exec_time_pieces = []
#
#    def check_overlap(new_piece, old_piece):
#        overlap = False
#
#        if new_piece[0] > old_piece[0] and new_piece[0] < old_piece[1]:
#            if new_piece[1] > old_piece[1]:
#                old_piece[1] = new_piece[1]
#                overlap = True
#        if new_piece[1] < old_piece[1] and new_piece[1] > old_piece[1]:
#            if new_piece[0] < old_piece[0]:
#                old_piece[0] = new_piece[0]
#            
#               
#    # Overlap tasks executing concurrently 
#    for task in tasks:
#        overlap = False
#        for launch in task.launch:
#            for piece in exec_time_pieces:
#                overlap = check_overlap(launch, exec_time_pieces)
#
#            if not overlap:
#                exec_times_pieces.append(launch)
#
#    # Unify times that still overlap
#    for piece in exec_time_pieces:
#        for i in range(1, len(exec_time_pieces)):
#            if piece[1] > exec_time_pieces[0] and piece
#               ...



def gen_avg_service_times(seed, rr):
    modes = ["no-preemption", "preemption"]
    priorities = ["no-priority", "priority"]

    try:
        os.mkdir("service_times")
    except:
        pass
    modes = ["no-preemption", "preemption"]
    priorities = ["no-priority", "priority"]

    # Program params
    RR = [rr]
    SIZE = [600]
    P = [1] # priorities. 0=disabled, 1=enabled
    M = [0, 1] # preemption (mode). 0=disabled, 1=enabled
    TIME = [0.1, 0.5, 0.8]
    #PARTIAL = [0, 1]
    PARTIAL = [1]

    N_ITERS = 1
    N_PRIORITIES = 5

    avg_service_times = {}

    avg_service_times_accum_df = {} 
    for params in product(RR, SIZE, P, M, TIME, PARTIAL):
        rr = params[0]
        size = params[1]
        p = params[2]
        m = params[3]
        time = params[4]
        partial = params[5]

        tasks_for_params_lst = []

        for curr_iter in range(N_ITERS):
            filename = f"results/seed{seed}/{size}/{priorities[p]}-exp_{N_TASKS}-{rr}rr-{modes[m]}-{time}-{size}-{curr_iter}-{partial}.txt"
            tasks_lst, swap_lst, end_scheduler  = gen_tasks(filename) 
            tasks_for_params_lst.append(tasks_lst)

            if params in avg_service_times_accum_df.keys():
                avg_service_times_2darray = get_avg_service_times(tasks_lst, N_PRIORITIES)
                df = avg_service_times_accum_df[params]
                avg_service_times_accum_df[params].loc[len(avg_service_times_accum_df[params].index)] = avg_service_times_2darray
            else:
                avg_service_times_2darray = [get_avg_service_times(tasks_lst, N_PRIORITIES)]
                avg_service_times_accum_df[params] = pd.DataFrame(avg_service_times_2darray)
                avg_service_times_accum_df[params].rename_axis("priority", axis="columns")
                avg_service_times_accum_df[params].rename_axis("iteration", axis="rows")

    avg_service_times_df = {}
    for params in avg_service_times_accum_df.keys():
        avg_service_times_df[params] = avg_service_times_accum_df[params].mean(axis=0)
        avg_service_times_df[params].rename_axis("priority")

    return avg_service_times_df
