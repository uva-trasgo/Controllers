import sys
from statistics import mean

N_PRIORITIES = 5
MAX_N_TASKS = 50

# This function returns the maximum number of elements enqueued the priority queues per priority
def count_queue_elements():
    queues = []
    # The number of elements is calculated per sequence of enqueues (we move to the next sequence when we dequeue an element)
    current_accumulation = []
    for q in range(N_PRIORITIES):
        queues.append([0])
        current_accumulation.append(0)

    #print(queues)

    for line in all_lines:
        if line[0] == "EQ":
            priority = int(line[1])
            queues[priority][current_accumulation[priority]] += 1
        if line[0] == "DQ":
            priority = int(line[1])
            queues[priority].append(queues[priority][current_accumulation[priority]] - 1)
            current_accumulation[priority] += 1

    current_priority = 0
    for q in queues:
        #print(f"max. number of elements queue {current_priority} = {max(q)}")
        print(f"{max(q)}")
        current_priority += 1

def count_interruptions():
    # Collects how many times each task was interrupted
    tasks = [0] * MAX_N_TASKS

    for line in all_lines:
        if line[0] == "EV":
            tasks[int(line[2])] += 1

    for n_task in range(len(tasks)):
        if tasks[n_task] > 0:
            #print(f"task {n_task} interrupted {tasks[n_task]} times")
            print(f"{tasks[n_task]}")

def count_swaps():
    n_swaps = 0

    for line in all_lines:
        if line[0] == "S":
            n_swaps += 1

    print(f"{n_swaps}")
            

def calculate_average_serving_time():
    enabled_priority = int(sys.argv[3])

    if enabled_priority == 1:
        N_PRIORITIES = 5
    elif enabled_priority == 0:
        N_PRIORITIES = 1

    times = []
    for q in range(N_PRIORITIES):
        times.append([])

    served_tasks = set()
    priority_per_task = {}
    arrival_times = {}
    service_times = {}

    arrival_line = 0

    for line in all_lines:
        if line[0] == "ARR":
            task_id = int(line[2])
            launch_line = arrival_line
            priority_per_task[task_id] = int(line[1])

            arrival_times[task_id] = float(line[3])

            priority = int(line[1])
            task_id = int(line[2])
            arrival_time = float(line[3])
            serve_time = 0
        
        if line[0] == "L":
            task_id = int(line[4])

            if not task_id in served_tasks:
                launch_time = float(line[3])
                service_times[task_id] = launch_time - arrival_times[task_id]
                priority = priority_per_task[task_id]
                times[priority].append(service_times[task_id])

                served_tasks.add(task_id)

    for p in range(N_PRIORITIES):
        print(f"{mean(times[p])}")


if __name__ == "__main__":
    filename = sys.argv[1]

    f = open(filename)

    all_lines = []

    for line in f:
        all_lines.append(line.strip().split())


    if sys.argv[2] == "count_queue":
        count_queue_elements()

    if sys.argv[2] == "n_interrupts":
        count_interruptions()

    if sys.argv[2] == "avg_serving_time":
        calculate_average_serving_time()
    
    if sys.argv[2] == "n_swaps":
        count_swaps()
