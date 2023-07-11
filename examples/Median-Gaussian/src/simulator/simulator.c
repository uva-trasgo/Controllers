#include <stdlib.h>
#include <stdio.h>

double time_swap = 0.06879160000000001;
double time_gaussian = 1.173695;
double time_median_1 = 0.8515248;
double time_median_2 = 1.3885841;
double time_median_3 = 1.925119;

double time_tasks[] = {
    time_median_1,
    time_median_2,
    time_median_3,
    time_gaussian
};

double time_to_finish_region[2];
int task_runninng_idx[2];
int type_task_running[2];

#define FINISHED 0
#define RUNNING 1

int region_state[2];

void main(int argc, char * argv[]) {
    int n_tasks = atoi(argv[1]);
    int seed = atoi(argv[2]);

    region_state[0] = FINISHED;
    region_state[1] = FINISHED;

    srand(seed);

    int * task_type = (int *)malloc(n_tasks * sizeof(int));
    double * arrival_times = (double *)malloc(n_tasks * sizeof(double));
    int * priorities = (int *)malloc(n_tasks * sizeof(int));


    for(int task = 0; task < n_tasks; task++) {
        task_type[task] = rand() % 4;
        arrival_times[task] = (double)rand() / (double)RAND_MAX * (double)total_time * 60.0;
        priorities[task] = rand() % 5;
    }

    // Sort according to arrival time
    double * old_task_type = (double *)malloc(n_tasks * sizeof(int));
    double * old_arrival_times = (double *)malloc(n_tasks * sizeof(double));
    double * old_priorities = (double *)malloc(n_tasks * sizeof(int));
    memcpy(old_task_type, task_type, n_tasks * sizeof(int));
    memcpy(old_arrival_times, arrival_times, n_tasks * sizeof(double));
    memcpy(old_priorities, priorities, n_tasks * sizeof(int));

    int j_min;

    int aux_task_type;
    double aux_arrival_time;
    int aux_priority;

    int n_arriving_task;

    for(int i = 0; i < n_tasks; i++) {
        j_min = i;
        for(int j = i+1; j < n_tasks; j++) {
            if(arrival_times[j] < arrival_times[j_min])
                j_min = j;
        }
        
        aux_task_type = task_type[i];
        aux_arrival_time = arrival_times[i];
        aux_priority = priorities[i];

        task_type[i] = task_type[jmin];
        arrival_times[i] = arrival_times[jmin];
        priorities[i] = priorities[jmin];

        task_type[jmin] = aux_task_type;
        arrival_times[jmin] = aux_arrival_times;
        priorities[jmin] = aux_priorities;
    }

    for(int i = 0; i < 2; i++) {
        task_runninng_idx[i] = n_arriving_task;
        type_task_running[i] = task_type[n_arriving_task];
        time_to_finish_region[i] = arrival_times[n_arriving_task] + time_tasks[type_task_running[i]];
    }

    while(1) {
        // Task arrives
        for(int i = 0; i < 2; i++)
            if(region_state[i] == FINISHED)
                task_runninng_idx[i] = n_arriving_task;

    } 
}
