#include "scheduler.h"

void insert_task(schedulerTask * task, schedulerQueue * queue) {
    queue->queue[queue->tail++] = task;
}

schedulerTask * pop_task(schedulerQueue * queue) {
    if(queue->head < queue->tail)
        return queue->queue[queue->head++];
    else
        return NULL;
}

schedulerTask * get_arrived_task(int n_tasks, int * tasks_to_arrive) {
    static int n_current_task = 0;

#ifdef _DEBUG
    printf("Returning arrived task \n");
#endif
    return &pregeneratedTasks.all_tasks[n_current_task++];
}

int has_tasks(schedulerQueue * queue) {
    if(queue->head < queue->tail)
        return 1;
    else
        return 0;
}

schedulerTask * get_task_from_queue(int enabled_priorities, int enabled_preemption, int use_CPU) {
#ifdef _DEBUG
    printf("Returning task from queue\n");
#endif
    int priority = 0;

    if(enabled_priorities) {
        for(int p = 0; p < N_PRIORITIES; p++) {
            if(has_tasks(&task_queue[p])) {
                priority = p;
                break;
            }
        }
    }

    /*
    // Return task from CPU if there is no other with higher priority in the queue and 
    // there is an available region in the FPGA.
    if(enabled_preemption && use_CPU) {
        int available_region = 0;
        for(int rr = 0; rr < N_RR+1; rr++) {
            if(FPGA.loaded_kernel_state[rr] == FINISHED) {
                available_region = 1;
                break;
            }
        }

        if(available_region) {
            int CPU_highest_priority = 1;
            for(int p = 0; p < N_PRIORITIES; p++) {
                if(has_tasks(&task_queue[p])) {
                    CPU_highest_priority = 0;
                    break;
                }
            }

            if(CPU_highest_priority && FPGA.running_tasks[N_RR] != NULL) {
                printf("$$$$ MIGRATION FROM CPU TO FPGA\n\n");
                #pragma omp atomic write
                host_task_ready = 0;

                reset_host = 1;
                FPGA.running_tasks[N_RR]->context = RR[N_RR].context;
                FPGA.running_tasks[N_RR]->preempted = 1;

                FPGA.loaded_kernel_state[N_RR] = FINISHED;

                reset_host = 0;
                #pragma omp atomic write
                host_task_ready = 1;

                return FPGA.running_tasks[N_RR];
            }
        }
    }
    */

    schedulerTask * return_task = pop_task(&task_queue[priority]);

#ifdef _DEBUG
    printf("return_task: %p\n", return_task);
#elif _EXP_MODE
    if(return_task != NULL)
        fprintf(exp_file, "{ \"op\":\"dequeue_task\", \"task_id\":%d, \"priority\":%d }\n", return_task->id, priority);
#endif

    return return_task;
}

void enqueue(schedulerTask * task) {
    int priority = task->priority;
#ifdef _DEBUG
    printf("Enqueing task. priority: %d, id: %d\n", priority, task->id);
#elif _EXP_MODE
    fprintf(exp_file, "{ \"op\":\"enqueue_task\", \"task_id\":%d, \"priority\":%d }\n", task->id, priority);
#endif
    insert_task(task, &task_queue[priority]);
}

int get_queue_size() {
    int size = 0;
    for(int priority = 0; priority < N_PRIORITIES; priority++) {
        int head = task_queue[priority].head;
        int tail = task_queue[priority].tail;
        size += tail - head;
    }

    return size;
}
