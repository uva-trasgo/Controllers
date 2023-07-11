Dynamic Partial Reconfiguration scheduler
====
Our DPR scheduler evicts and reallocates tasks in the device based on their priority upon arrival, therefore improving the throughput whith respect to the traditional full reconfiguration and improving the flexibility of workload management in a comparable manner to the preemption mechanisms of CPU and GPU.

Each application that makes use of this scheduler must include statements as the following in the Controller block (check the following [example](../examples/Median-Gaussian/src/Median-Gaussian_Pynq_Ctrl.c)):

```task_generator(ctrl, n_tasks, minutes, enabled_priorities, H, W, seed);```

```PYNQ_scheduler(ctrl, threads, group, n_tasks, use_CPU, n_rr, enabled_priorities, enabled_preemption, enabled_partial);```

Note how we keep our scheduler generic through the ```schedulerTask``` structure

```
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
```

This is a task agnostic structure that keeps track of the state of the task and it used to make decisions about relocation in the device and enqueing based on the preemption policy. The steps to get tasks running different than the ones presented here are straightforward. Nevertheless, we encourage authors to contacts us if they need any help during this process.