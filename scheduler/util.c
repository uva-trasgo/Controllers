#include "scheduler.h"
#include <string.h>

void setup_PYNQ_scheduler(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread group, int n_tasks, int use_CPU, int n_rr) {
    currently_swapping = 0;

    dummy_tile = Ctrl_DomainAlloc(ctrl, int, hitShapeSize(1));

    // Setup of FPGA descriptor
    sprintf(FPGA.pblock_names[0], "medianblur_0");
    sprintf(FPGA.pblock_names[1], "gaussianblur_0");

    FPGA.loaded_kernels[0] = GAUSSIAN;
    FPGA.loaded_kernels[1] = GAUSSIAN;
    FPGA.loaded_kernels[2] = GAUSSIAN;

    FPGA.loaded_kernel_state[0] = FINISHED;
    FPGA.loaded_kernel_state[1] = FINISHED;

    for(int rr = 0; rr < n_rr; rr++)
        FPGA.running_tasks[rr] = NULL;

    if(use_CPU) {
        FPGA.loaded_kernel_state[N_RR] = FINISHED;
    }
    else {
        FPGA.loaded_kernel_state[N_RR] = UNAVAILABLE;
    }

    for(int i = ((use_CPU) ? N_RR+1 : n_rr); i < N_RR+1; i++) 
        FPGA.loaded_kernel_state[i] = UNAVAILABLE;

    for(int rr = 0; rr < ((use_CPU) ? N_RR+1 : n_rr); rr++) {
        FPGA.running_tasks[rr] = NULL;
    }

    // Setup of available kernels
    FILE * kernel_names = fopen("/home/xilinx/controller-DPR/controllers/scheduler/kernel_names.txt", "r");

    int n_kernel = 0;
    while(fgets(kernel_name[n_kernel], NAME_BUFFER, kernel_names)) {
        // Remove trailing newline character
        kernel_name[n_kernel][strcspn(kernel_name[n_kernel], "\n")] = 0;
        n_kernel++;
#ifdef _DEBUG
        printf("kernel_name: %s\n", kernel_name[n_kernel-1]);
#endif
    }
    fclose(kernel_names);

    CtrlConstructs.ctrl = ctrl;
    CtrlConstructs.threads = threads;
    CtrlConstructs.group = group;

    current_launch_task_id = -1;

    evicted_host_task_id = -2;
    current_host_task_id = -1;

#ifdef _EXP_MODE
    exp_file = fopen("/tmp/ramdisk/exp_file.txt", "w");
    fprintf(exp_file, "{ \"op\":\"params\", \"n_tasks\":%d, \"n_rr\":%d }\n", n_tasks, n_rr);

    #ifdef _EXP_MODE
    for(int i = 0; i < n_tasks; i++) {
        schedulerTask * task = &pregeneratedTasks.all_tasks[i];
        fprintf(exp_file, "{ \"op\":\"task_generation\", \"task_id\":%d, \"task_priority\":%d, \"time\":%lf }\n", task->id, task->priority, task->k_data.arrival_time);
    }
    #endif // _EXP_MODE

    init_time = omp_get_wtime();
#endif
}

void update_timeout(struct timeval * timeout, int last_arrived) {
    static int current_task = -1;
    int new_task = 0;

    if(last_arrived > current_task) {
        current_task = last_arrived;
        new_task = 1;
    }

    if(new_task) {
        int sec = (int)floor(pregeneratedTasks.arrival_times[last_arrived]);
        int usec = (pregeneratedTasks.arrival_times[last_arrived] - sec) * 100000; 

        timeout->tv_sec = sec;
        timeout->tv_usec = usec;
    }
}

int has_finished(int n_tasks, int use_CPU, int n_rr, int * tasks_to_arrive, int from_swap) {
    static int finished_tasks = 0;
    static int n_current_task = 0;

    if(!from_swap && timeout.tv_sec == 0 && timeout.tv_usec == 0)
        n_current_task++;

    if(n_current_task <= n_tasks)
       *tasks_to_arrive = 1;
    else
       *tasks_to_arrive = 0; 

    for(int rr = 0; rr < ((use_CPU) ? N_RR+1 : n_rr); rr++) {
        if(generated_interrupts[rr]) {
#ifdef _DEBUG
            printf("FINISHED RR %d\n", rr);
#endif
#if _EXP_MODE
            fprintf(exp_file, "{ \"op\":\"launch_task\", \"task_id\":%d, \"rr\":%d, \"finish_time\":%lf }\n", RR[rr].running_task_id, rr, omp_get_wtime() - init_time);
#endif
            generated_interrupts[rr] = 0;
            if(rr < N_RR) {
#ifdef _DEBUG 
	        printf("Unsetting lock_RR[%d]\n", rr);
#endif
            }

            FPGA.loaded_kernel_state[rr] = FINISHED;

            finished_tasks++;
        }
    }

    if(finished_tasks == n_tasks)
        return 1;
    else
        return 0;
}

void context_switch_thread() {
    struct timespec eviction_time = {
        .tv_sec = 0,
        .tv_nsec = 1
    };

    nanosleep(&eviction_time, NULL);
}


