#include "scheduler.h"
#ifdef _TEST_PARTIAL
#include <time.h>
#endif // _TEST_PARTIAL

//#include "kernels.h"

CTRL_HOST_TASK(Zero_Array, HitTile_int input_image, int H, int W) {
//void initialise_image(HitTile_int image, int H, int W, int init_zeros) {
    for(int i = 0; i < H; i++) {
        for(int j = 0; j < W; j++) {
                hit(input_image, i, j) = 0;
        }
    }
}

CTRL_HOST_TASK_PROTO( Zero_Array, 3,
    OUT, HitTile_int, in_matrix,
    INVAL, int, H,
    INVAL, int, W
);

CTRL_SWAP_PROTO(ABC, 1, PYNQ, DEFAULT, 1, OUT, HitTile_int, dummy_tile);
//CTRL_KERNEL_SYNC_PROTO(NONAME, 1, PYNQ, DEFAULT, 1, OUT, HitTile_int, dummy_tile);
 


void restore_context(schedulerTask * task, int rr, int n_rr) {
    if(task->preempted) {
#ifdef _DEBUG
        printf("(rr %d) Kernel %d was evicted before, running_task: %d\n\n", rr, task->id, FPGA.running_tasks[rr]->id);
#endif
        RR[rr].context = task->context;
        if(rr < n_rr)
            PYNQ_writeMMIO(&BRAMController[rr].bram, &RR[rr].context, 0x0, sizeof(struct context));
    }
    else {
#ifdef _DEBUG
        printf("(restore_context) Before cleaining BRAM\n");
#endif
        clean_bram(rr);
#ifdef _DEBUG
        printf("(restore_context) After cleaining BRAM\n");
#endif
    }
}

void evict_kernel(int rr, int n_rr, schedulerTask * incoming_task) {

#ifdef _DEBUG
    printf("(evict_kernel) Before reset\n");
#endif
    reset(rr);
    FPGA.loaded_kernel_state[rr] = FINISHED;
#ifdef _DEBUG
    printf("(evict_kernel) After reset\n");
#elif _EXP_MODE
    fprintf(exp_file, "{ \"op\":\"reset\", \"rr\":%d, \"time\":%lf }\n", rr, omp_get_wtime() - init_time);
#endif


    _print_output(rr);


    // The context is saved for future use when the task is swapped in again
    PYNQ_readMMIO(&BRAMController[rr].bram, &RR[rr].context, 0x0, sizeof(struct context));
    FPGA.running_tasks[rr]->context = RR[rr].context;
    FPGA.running_tasks[rr]->preempted = 1;

    if(incoming_task != NULL)
        enqueue(FPGA.running_tasks[rr]);

    if(incoming_task != NULL)
        restore_context(incoming_task, rr, n_rr);
#ifdef _DEBUG
    if(incoming_task != NULL)
        printf("(evict_kernel) Context after restoring, (incoming_task: %d, running_task: %d):\n", incoming_task->id, FPGA.running_tasks[rr]->id);
    else
        printf("(evict_kernel) Context after restoring, (incoming_task: -, running_task: %d):\n", FPGA.running_tasks[rr]->id);
#elif _EXP_MODE
    if(incoming_task != NULL)
        fprintf(exp_file, "{ \"op\":\"evict_task\", \"running_task_id\":%d, \"incoming_task_id\":%d, \"rr\":%d, \"time\": %lf }\n", FPGA.running_tasks[rr]->id, incoming_task->id, rr, omp_get_wtime() - init_time);
    else
        fprintf(exp_file, "{ \"op\":\"evict_task\", \"running_task_id\":%d, \"incoming_task_id\":-1, \"rr\":%d, \"time\": %lf }\n", FPGA.running_tasks[rr]->id, rr, omp_get_wtime() - init_time);
#endif
#ifdef _DEBUG
    _print_output(rr);
#endif

#ifdef _DEBUG
    if(incoming_task != NULL)
        printf("evicted kernel %d to replace with %d\n", FPGA.running_tasks[rr]->id, incoming_task->id);
    else
        printf("evicted kernel %d\n", FPGA.running_tasks[rr]->id);
#endif
}


void serve_task(schedulerTask * task, int arrived, int use_CPU, int n_rr, int enabled_preemption, int enabled_partial, int n_tasks) {
    if(task != NULL) {
#ifdef _DEBUG
        if(FPGA.running_tasks[0] != NULL && FPGA.running_tasks[1] != NULL)
            printf("(serve_task) arrived_task %d, running_tasks: %d, %d\n", task->id, FPGA.running_tasks[0]->id, FPGA.running_tasks[1]->id);
        else
            printf("(serve_task) arrived_task %d\n", task->id);
#endif
        int rr;
        int available_region = 0;

        // Try to find a region with no kernel running
        for(rr = 0; rr < ((use_CPU) ? N_RR+1 : n_rr); rr++) {
            if(FPGA.loaded_kernel_state[rr] == FINISHED) {
                available_region = 1;
                break;
            }
        }

        // The context is restored if the kernel had previously been run
        if(enabled_preemption && available_region) {
#ifdef _DEBUG
            printf("(serve_task) found an available region\n");
            _print_output(rr);
#endif
            restore_context(task, rr, n_rr);
#ifdef _DEBUG
            _print_output(rr);
#endif
        }

        if(enabled_preemption && !available_region) {
            // Restore context if the task was interrupted in the past
            for(rr = 0; rr < ((use_CPU) ? N_RR+1 : n_rr); rr++) {
                if(FPGA.loaded_kernel_state[rr] == RUNNING && task->priority < FPGA.running_tasks[rr]->priority) {
                    evict_kernel(rr, n_rr, task);

                    available_region = 1;
                    break;
                }
            }
        }

        
        if(available_region)
            run(task, rr, n_rr, enabled_partial, n_tasks);
        else
            enqueue(task);

#ifdef _DEBUG
        printf("(serve_task) bye\n");
#endif
    }
}

int reset_host = 0;

void swap(schedulerTask * task, int rr, int n_rr, int enabled_partial, int n_tasks) {
    int do_swap = 0;

    if(task->k_data.task_type <= 2 && FPGA.loaded_kernels[rr] == GAUSSIAN) {
        FPGA.loaded_kernels[rr] = MEDIAN;
        do_swap = 1;
    }
    else if(task->k_data.task_type == 3 && FPGA.loaded_kernels[rr] == MEDIAN)  {
        FPGA.loaded_kernels[rr] = GAUSSIAN;
        do_swap = 1;
    }

    char bitstream_path[512];

    if(do_swap) {
        if(enabled_partial) {
            sprintf(bitstream_path, "/home/xilinx/Bitstreams-axi_firewall/%s-%s_pblock_%s_partial.bit", kernel_name[FPGA.loaded_kernels[0]], kernel_name[FPGA.loaded_kernels[1]], FPGA.pblock_names[rr]);

    #ifdef _DEBUG
            printf("(rr: %d) %s\n", task->rr, bitstream_path);
    #endif

            Ctrl_Task ctrl_task = Ctrl_KernelTaskCreate_Swap(task->rr, bitstream_path, &dummy_tile);

            #ifdef _DEBUG
            printf("----> Before launching swap task. rr: %d\n", rr);
            #endif // _DEBUG
            Ctrl_LaunchKernel(CtrlConstructs.ctrl, ctrl_task);

            Ctrl_WaitTile(CtrlConstructs.ctrl, dummy_tile);
        }
        else { // Full reconfiguration. Done in the main thead, as it stalls the FPGA anyways
            int evicted_kernels[n_rr];
            for(int i = 0; i < n_rr; i++)
                evicted_kernels[i] = 0;

            for(int other_rr = 0; other_rr < n_rr; other_rr++) {
                if(FPGA.loaded_kernel_state[other_rr] == RUNNING && other_rr != rr) {
                    evict_kernel(other_rr, n_rr, NULL);
                    evicted_kernels[other_rr] = 1;
                }
            }

            sprintf(bitstream_path, "/home/xilinx/Bitstreams-axi_firewall/%s-%s.bit", kernel_name[FPGA.loaded_kernels[0]], kernel_name[FPGA.loaded_kernels[1]]);
            double begin_swap_time = omp_get_wtime();
            PYNQ_loadBitstream(bitstream_path, 0);

            setup_scheduler(0);
            for(int _rr = 0; _rr < n_rr; _rr++)
                setup_reconfigurable_region(_rr);

            for(int other_rr = 0; other_rr < n_rr; other_rr++) {
                if(evicted_kernels[other_rr] && other_rr != rr) {
                    restore_context(FPGA.running_tasks[other_rr], other_rr, n_rr); 
                    launch(FPGA.running_tasks[other_rr], other_rr, n_rr);
                }
            }
            #ifdef _EXP_MODE
            fprintf(exp_file, "{ \"op\":\"full_swap\", \"bitstream_path\":\"%s\", \"start_time\":%lf, \"finish_time\":%lf }\n", bitstream_path, begin_swap_time - init_time, omp_get_wtime() - init_time);
            #endif
        }
    }
}

void launch(schedulerTask * p_task, int rr, int n_rr) {
    if(rr < n_rr)
        current_launch_task_id = p_task->id;
    else
        current_host_task_id = p_task->id;

#ifdef _DEBUG
    printf("Before launching. current_launch_task_id: %d\n", current_launch_task_id);
#endif

    (*f_launch[p_task->k_data.task_type])(p_task, CtrlConstructs.ctrl, CtrlConstructs.threads, CtrlConstructs.group);

    Ctrl_WaitTile(CtrlConstructs.ctrl, *p_task->output_image); 

    FPGA.running_tasks[rr] = p_task;
    FPGA.loaded_kernel_state[rr] = RUNNING;
}

void run(schedulerTask * task, int rr, int n_rr, int enabled_partial, int n_tasks) {
    task->rr = rr;

    if(rr < n_rr)
        swap(task, rr, n_rr, enabled_partial, n_tasks);
   
    launch(task, rr, n_rr); 
}
