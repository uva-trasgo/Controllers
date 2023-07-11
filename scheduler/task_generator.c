#include "Ctrl.h"
#include "scheduler.h"

uint32_t lfsr113(uint64_t **);
void lfsr113_seed(uint32_t, uint64_t **);

uint64_t * state;

#ifdef _EXTERNAL_IMG
CTRL_HOST_TASK( Init_Images, HitTile_int in_matrix, HitTile_int out_matrix, int H, int W) { 
    FILE * image;
    image = fopen("/home/xilinx/controller-DPR/controllers/examples/Median-Gaussian/src/Saltpepper.pgm", "r");

    size_t len = 100;
    char line[len];

    fgets(line, len, image);
    
    int file_H, file_W;
    fscanf(image, "%d", &file_H);
    fscanf(image, "%d", &file_W);

    fgets(line, len, image);

    int * data = malloc(H_NROW*H_NCOL * sizeof(int));

    for(int i = 0; i < H_NROW; i++) {
        for(int j = 0; j < H_NCOL; j++) {
            hit(in_matrix, i, j) = 0;
            hit(out_matrix, i, j) = 0;
        }
    }   

    for(int i = 1; i < H_NROW-1; i++) {
        for(int j = 1; j < H_NCOL-1; j++) {
            fscanf(image, "%d", &data[i*file_W+j]);
        }
    }   

    for(int i = 1; i < H_NROW-1; i++) {
        for(int j = 1; j < H_NCOL-1; j++) {
            hit(in_matrix, i, j) = data[i*file_W+j]; 
            hit(out_matrix, i, j) = 0;
        }
    }  
}
#else
CTRL_HOST_TASK(Init_Images, HitTile_int input_image, HitTile_int output_image, int H, int W) {
//void initialise_image(HitTile_int image, int H, int W, int init_zeros) {
    for(int i = 0; i < H; i++) {
        for(int j = 0; j < W; j++) {
                hit(input_image, i, j) = lfsr113(&state) % 256;
                hit(output_image, i, j) = 0;
        }
    }
}
#endif // _EXTERNAL_IMG

CTRL_HOST_TASK_PROTO( Init_Images, 4,
        OUT, HitTile_int, input_image,
        OUT, HitTile_int, output_image,
        INVAL, int, H,
        INVAL, int, W
);

void clean_context(struct context * context) {
    for(int i = 0; i < 10; i++) {
        context->var[i] = 0;
        context->incr_var[i] = 0;
        context->init_var[i] = 0;
        context->saved[i] = 0;
    }

    context->valid = 1;
}

void task_generator(PCtrl ctrl, int n_tasks, double total_time, int enabled_priorities, int H, int W, int seed) {
    state = (uint64_t *) malloc(4 * sizeof(uint64_t));

    lfsr113_seed(seed, &state);

    int H_NROW = H+2;
    int H_NCOL = W+2;

    //struct context contexts[n_tasks];
    pregeneratedTasks.input_images = (HitTile_int*)malloc(n_tasks * sizeof(HitTile_int));
    pregeneratedTasks.output_images = (HitTile_int*)malloc(n_tasks * sizeof(HitTile_int));
    pregeneratedTasks.arrival_times = (double *)malloc(n_tasks * sizeof(double));

    int priority;

    // For testing purposes
    //partial_output = Ctrl_DomainAlloc(ctrl, int, hitShapeSize(H_NROW, H_NCOL));

    for(int task = 0; task < n_tasks; task++) {
        kernel_data k_data;
        k_data.task_type = lfsr113(&state) % 4; 
        k_data.arrival_time = (double)lfsr113(&state) * 2.3283064365387e-10 * (double)total_time * 60.0;

        if(k_data.task_type <= 2) // MEDIAN
            k_data.iters = k_data.task_type+1;

        pregeneratedTasks.input_images[task] = Ctrl_DomainAlloc(ctrl, int, hitShapeSize(H_NROW, H_NCOL));
        pregeneratedTasks.output_images[task] = Ctrl_DomainAlloc(ctrl, int, hitShapeSize(H_NROW, H_NCOL));

        if(enabled_priorities)
            priority = lfsr113(&state) % 5; 
        else
            priority = 0;
        
        pregeneratedTasks.all_tasks[task].id = task;
        pregeneratedTasks.all_tasks[task].k_data = k_data;
        pregeneratedTasks.all_tasks[task].input_image = &pregeneratedTasks.input_images[task];
        pregeneratedTasks.all_tasks[task].output_image = &pregeneratedTasks.output_images[task];
        pregeneratedTasks.all_tasks[task].priority = priority;
        pregeneratedTasks.all_tasks[task].enqueued = 0;
        pregeneratedTasks.all_tasks[task].preempted = 0;

        clean_context(&pregeneratedTasks.all_tasks[task].context);
    }    

    for(int task = 0; task < n_tasks; task++) {
        Ctrl_HostTask(ctrl, Init_Images, pregeneratedTasks.input_images[task], pregeneratedTasks.output_images[task], H_NROW, H_NCOL);
#ifdef _DEBUG
        printf("Generated image %d\n", task);
#endif
        //Ctrl_HostTask(ctrl, Init_Tiles, input_images[task], output_images[task]);
    }
    Ctrl_GlobalSync(ctrl);
   
    int j_min;
    // Sort tasks by arrival time
    for(int i = 0; i < n_tasks; i++) {
        j_min = i; 
        for(int j = i+1; j < n_tasks; j++) {
            if(pregeneratedTasks.all_tasks[j].k_data.arrival_time < pregeneratedTasks.all_tasks[j_min].k_data.arrival_time)
                j_min = j; 
        }

        HitTile_int * aux_input_image = pregeneratedTasks.all_tasks[i].input_image;
        HitTile_int * aux_output_image = pregeneratedTasks.all_tasks[i].output_image;
        kernel_data aux_k_data = pregeneratedTasks.all_tasks[i].k_data;

        pregeneratedTasks.all_tasks[i].input_image = pregeneratedTasks.all_tasks[j_min].input_image;
        pregeneratedTasks.all_tasks[i].output_image = pregeneratedTasks.all_tasks[j_min].output_image;
        pregeneratedTasks.all_tasks[i].k_data = pregeneratedTasks.all_tasks[j_min].k_data;

        pregeneratedTasks.all_tasks[j_min].input_image = aux_input_image;
        pregeneratedTasks.all_tasks[j_min].output_image = aux_output_image;
        pregeneratedTasks.all_tasks[j_min].k_data = aux_k_data;
    }

    double previous_time = 0.0;
    for(int task = 0; task < n_tasks; task++) {
        double absolute_time = pregeneratedTasks.all_tasks[task].k_data.arrival_time;
        pregeneratedTasks.all_tasks[task].k_data.absolute_arrival_time = absolute_time;
        pregeneratedTasks.all_tasks[task].k_data.arrival_time -= previous_time;
        previous_time = absolute_time;
    }

    for(int task = 0; task < n_tasks; task++)
        pregeneratedTasks.arrival_times[task] = pregeneratedTasks.all_tasks[task].k_data.arrival_time;

    for(int priority = 0; priority < N_PRIORITIES; priority++) {
        task_queue[priority].head = 0;
        task_queue[priority].tail = 0;
    }

    // Test sorted
#ifdef _DEBUG
    printf("TASKS TIMES\n");
    for(int task = 0; task < n_tasks; task++) {
        printf("%lf\n", pregeneratedTasks.all_tasks[task].k_data.arrival_time);
    }
#endif

    //all_tasks[0].k_data.arrival_time = 1.5;
    //arrival_times[0] = 1.5;


}
