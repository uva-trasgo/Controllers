#ifndef _SCHEDULER_
#define _SCHEDULER_

#include "Ctrl.h"
#include "median-blur.h"
#include "gaussian-blur.h"

#define N_PRIORITIES 5
#define MAX_N_TASK 50

#define NAME_BUFFER 50

char initial_configuration[256];

#define LAUNCH_FULL_CONFIGURATION( config ) PYNQ_loadBitstream(config, 0);

#define SET_INITIAL_CONFIGURATION( N_RR, ... ) _SET_INITIAL_CONFIGURATION( N_RR, __VA_ARGS__ )
#define _SET_INITIAL_CONFIGURATION( N_RR, ... ) SET_INITIAL_CONFIGURATION_##N_RR( __VA_ARGS__ )
#define SET_INITIAL_CONFIGURATION_4 ( k1, k2, k3, k4 ) sprintf(initial_configuration, "/home/xilinx/Bitstreams-axi_firewall/%s-%s-%s-%s.bit", #k1, #k2, #k3, #k4); LAUNCH_FULL_CONFIGURATION(initial_configuration);
#define SET_INITIAL_CONFIGURATION_3 ( k1, k2, k3 ) sprintf(initial_configuration, "/home/xilinx/Bitstreams-axi_firewall/%s-%s-%s.bit", #k1, #k2, #k3); LAUNCH_FULL_CONFIGURATION(initial_configuration);
#define SET_INITIAL_CONFIGURATION_2( k1, k2 ) sprintf(initial_configuration, "/home/xilinx/Bitstreams-axi_firewall/%s-%s.bit", #k1, #k2); LAUNCH_FULL_CONFIGURATION(initial_configuration);
#define SET_INITIAL_CONFIGURATION_1( k ) sprintf(initial_configuration, "/home/xilinx/Bitstreams-axi_firewall/%s.bit", #k); LAUNCH_FULL_CONFIGURATION(initial_configuration);

#define _COUNT(v1,v2,v3,v4,v5,v6,v7,v8,N,...) N
#define COUNT(...) _COUNT(__VA_ARGS__, 8,7,6,5,4,3,2,1)

extern int finished_scheduler;
extern int tasks_to_arrive;


int H, W;
int H_NROW, H_NCOL;

HitTile_int dummy_tile;
//HitTile_int dummy_sync_tile[N_RR];

// Scheduler task types
// User defined
typedef enum schedulerTaskType {
   SCHEDULER_MEDIAN_1,
   SCHEDULER_MEDIAN_2,
   SCHEDULER_MEDIAN_3,
   SCHEDULER_GAUSSIAN,
   number_of_sch_task_types
} schedulerTaskType;

typedef enum kernels {
    MEDIAN,
    GAUSSIAN,
    number_of_kernel_types
} kernels;


///////////////////////
typedef struct kernel_data {
    int iters;
    schedulerTaskType task_type;
    double arrival_time;
    double absolute_arrival_time;   // used to keep the original absolute time after calculating time diffs
} kernel_data;

typedef struct schedulerTask {
    int id;
    kernel_data k_data;
    struct context context;
    HitTile_int * input_image;
    HitTile_int * output_image;
    int rr;
    int priority;
    int enqueued;
    int preempted;
} schedulerTask;

typedef struct schedulerQueue {
    schedulerTask * queue[MAX_N_TASK];
    int head;
    int tail;
} schedulerQueue;

schedulerQueue task_queue[N_PRIORITIES];


typedef enum kernel_state {
    RUNNING,
    FINISHED,
    UNAVAILABLE
} kernel_state;

char kernel_name[number_of_kernel_types][NAME_BUFFER];

typedef struct t_FPGA {
    schedulerTask * running_tasks[N_RR+1];
    kernels loaded_kernels[N_RR+1];
    kernel_state loaded_kernel_state[N_RR+1];
    char pblock_names[N_RR][NAME_BUFFER];
} t_FPGA;

t_FPGA FPGA;

typedef struct t_pregeneratedTasks {
    schedulerTask all_tasks[N_PRIORITIES * MAX_N_TASK];
    double * arrival_times;
    HitTile_int * input_images;
    HitTile_int * output_images;
    //kernel_data * k_data;
} t_pregeneratedTasks;

t_pregeneratedTasks pregeneratedTasks;


struct timeval timeout;

typedef struct t_CtrlConstructs {
    PCtrl ctrl;
    Ctrl_Thread threads;
    Ctrl_Thread group;
} t_CtrlConstructs;

t_CtrlConstructs CtrlConstructs;

extern int reset_host;

// For testing purposes
HitTile_int partial_output;


// Launch functions defined by the user
void (*f_launch[20])(schedulerTask *, PCtrl ctrl, Ctrl_Thread, Ctrl_Thread);


#define SET_KERNEL_TASKS( ... ) _SET_KERNEL_TASKS( COUNT( __VA_ARGS__ ), __VA_ARGS__ )
#define _SET_KERNEL_TASKS( N, ... ) __SET_KERNEL_TASKS( N, __VA_ARGS__ )
#define __SET_KERNEL_TASKS( N, ... ) SET_KERNEL_TASKS_##N( __VA_ARGS__ )
#define SET_KERNEL_TASKS_4( f_launch4, ... ) f_launch[3] = &f_launch4; SET_KERNEL_TASKS_3( __VA_ARGS__ )
#define SET_KERNEL_TASKS_3( f_launch3, ... ) f_launch[2] = &f_launch3; SET_KERNEL_TASKS_2( __VA_ARGS__ )
#define SET_KERNEL_TASKS_2( f_launch2, ... ) f_launch[1] = &f_launch2; SET_KERNEL_TASKS_1( __VA_ARGS__ )
#define SET_KERNEL_TASKS_1( f_launch1 ) f_launch[0] = &f_launch1;




// Scheduler functions
void task_generator(PCtrl ctrl, int n_tasks, double total_time, int enabled_priorities, int H, int W, int seed);
void insert_task(schedulerTask * task, schedulerQueue * queue);
schedulerTask * pop_task(schedulerQueue * queue);
void PYNQ_scheduler(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread group, int n_tasks, int use_CPU, int n_rr, int enabled_priorities, int enabled_preemption, int enabled_partial);
void setup_PYNQ_scheduler(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread group, int n_tasks, int use_CPU, int n_rr);
void update_timeout(struct timeval *, int last_arrived);
int has_finished(int n_tasks, int use_CPU, int n_rr, int * tasks_to_arrive, int from_swap);
schedulerTask * get_arrived_task();
schedulerTask * get_task_from_queue(int enabled_priorities, int enabled_preemption, int use_CPU);
void serve_task(schedulerTask * task, int arrived, int use_CPU, int n_rr, int enabled_preemption, int enabled_partial, int n_tasks);
void enqueue(schedulerTask * task);
int get_queue_size();
void swap(schedulerTask * task, int rr, int n_rr, int enabled_partial, int n_tasks);
void launch(schedulerTask * task, int rr, int n_rr);
void run(schedulerTask * task, int rr, int n_rr, int enabled_partial, int n_tasks);
void context_switch_thread();
// For testing purposes
void run_host_task(int n_task, int kernel);
void run_kernel_task(int n_task);
#endif // _SCHEDULER_
