#include "scheduler.h"

int finished_scheduler = 0;
int tasks_to_arrive = 1;

void PYNQ_scheduler(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread group, int n_tasks, int use_CPU, int n_rr, int enabled_priorities, int enabled_preemption, int enabled_partial) {
    setup_PYNQ_scheduler(ctrl, threads, group, n_tasks, use_CPU, n_rr);

    int last_arrived = 0;
    schedulerTask * task;
    int arrived;

    update_timeout(&timeout, last_arrived);

    while(1) {
        if(!finished_scheduler) { // if full reconfiguration is enabled the scheduler could have already finished
            wait_finish(&timeout);
            finished_scheduler = has_finished(n_tasks, use_CPU, n_rr, &tasks_to_arrive, 0);
        }
        
        if(finished_scheduler) 
            break;

        if(tasks_to_arrive && timeout.tv_sec == 0 && timeout.tv_usec == 0) {
#ifdef _DEBUG
            printf("TIMEOUT!\n");
#endif
            task = get_arrived_task();
#ifdef _EXP_MODE
            fprintf(exp_file, "{ \"op\":\"task_arrival\", \"task_id\":%d, \"task_priority\":%d, \"time\":%lf }\n", task->id, task->priority, omp_get_wtime() - init_time);
#endif
            last_arrived++;
            arrived = 1;
            serve_task(task, arrived, use_CPU, n_rr, enabled_preemption, enabled_partial, n_tasks);
        }
        else {
            for(int rr = 0; rr < n_rr; rr++) {
                if (FPGA.loaded_kernel_state[rr] == FINISHED) {
                    task = get_task_from_queue(enabled_priorities, enabled_preemption, use_CPU);
                    arrived = 0;
                    serve_task(task, arrived, use_CPU, n_rr, enabled_preemption, enabled_partial, n_tasks);
                }
            }
        }
        update_timeout(&timeout, last_arrived);
#ifdef _EXP_MODE
        int size = get_queue_size();
        fprintf(exp_file, "{ \"op\": \"queue_size\", \"size\": %d }\n", size);

        printf("---- End of scheduler iteration | last arrived: %d ----\n", last_arrived);
#endif
    }

#ifdef _EXP_MODE
    fprintf(exp_file, "{ \"op\":\"end\", \"time\":%lf }\n", omp_get_wtime() - init_time);
    fclose(exp_file);
#endif
}
