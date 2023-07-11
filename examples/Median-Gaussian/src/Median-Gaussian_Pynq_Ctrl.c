/*
 * <license>
 * 
 * Controller v2.1
 * 
 * This software is provided to enhance knowledge and encourage progress in the scientific
 * community. It should be used only for research and educational purposes. Any reproduction
 * or use for commercial purpose, public redistribution, in source or binary forms, with or 
 * without modifications, is NOT ALLOWED without the previous authorization of the copyright 
 * holder. The origin of this software must not be misrepresented; you must not claim that you
 * wrote the original software. If you use this software for any purpose (e.g. publication),
 * a reference to the software package and the authors must be included.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Copyright (c) 2007-2020, Trasgo Group, Universidad de Valladolid.
 * All rights reserved.
 * 
 * More information on http://trasgo.infor.uva.es/
 * 
 * </license>
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>
#include <assert.h>
#include "Ctrl.h"
#include <signal.h>

#define SEED 6834723
#define EPSILON 0.0001

#define HCOL (1024+2)

double main_clock;
double exec_clock;

#include "median-blur.h"
#include "gaussian-blur.h"

#include <sys/time.h>
#include <signal.h>

#include <time.h>

#include "scheduler.h"
#include "kernels.h"

//int H, W;
//int H_NROW, H_NCOL;
char * image_path;

//CTRL_KERNEL(SwapKernelDummy, CPU, DEFAULT, KHitTile_int dummy, {
//
//});
//
//CTRL_KERNEL_PROTO( SwapKernelDummy,
//        1, CPU, DEFAULT, 1,
//        OUT, HitTile_int, dummy);


CTRL_HOST_TASK( Init_Tiles, HitTile_int in_matrix, HitTile_int out_matrix ) {
    FILE * image;
    image = fopen("/home/xilinx/controller-DPR/controllers/examples/Median-Gaussian/src/Saltpepper.pgm", "r");

    size_t len = 100;
    char line[len];

    fgets(line, len, image);
    printf("%s\n", line);

    
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
    printf("Finishing Init_Tiles\n");
}

CTRL_HOST_TASK( Write_Image, HitTile_int image_matrix, int H, int W, int number ) {
    FILE * output;

    char filename[100];
    sprintf(filename, "output_image_%d.ppm", number);
    //output = fopen("output_image.ppm", "w");
    output = fopen(filename, "w");

    int i, j;
    
    int H_NROW = H+2;
    int H_NCOL = W+2;

    fwrite("P2\n", sizeof(char), 3, output);
    fprintf(output, "%d ", H);
    fprintf(output, "%d\n", W);
    fprintf(output, "%d\n", 255);

    for(i = 1; i < H_NROW-1; i++) {
        for(j = 1; j < H_NCOL-1; j++) {
            fprintf(output, "%d ", hit(image_matrix, i, j));
        }
    }
    fprintf(output, "\n");

    fclose(output);
}



CTRL_HOST_TASK_PROTO( Init_Tiles, 2,
	OUT, HitTile_int, in_matrix,
	OUT, HitTile_int, out_matrix
);


CTRL_HOST_TASK_PROTO( Write_Image, 4,
        IN, HitTile_int, image_matrix,
        INVAL, int, H,
        INVAL, int, W,
        INVAL, int, number
);

void launch_medianblur1(schedulerTask * p_task, PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread group) {
    //printf("Before launching MedianBlur 1. iters: %d\n", p_task->k_data.iters);
    Ctrl_Launch(ctrl, MedianBlur, threads, group, p_task->rr, *p_task->input_image, *p_task->output_image, H, W, p_task->k_data.iters);
}

void launch_medianblur2(schedulerTask * p_task, PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread group) {
    //printf("Before launching MedianBlur 2. iters: %d\n", p_task->k_data.iters);
    Ctrl_Launch(ctrl, MedianBlur, threads, group, p_task->rr, *p_task->input_image, *p_task->output_image, H, W, p_task->k_data.iters);
}

void launch_medianblur3(schedulerTask * p_task, PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread group) {
    //printf("Before launching MedianBlur 3. iters: %d\n", p_task->k_data.iters);
    Ctrl_Launch(ctrl, MedianBlur, threads, group, p_task->rr, *p_task->input_image, *p_task->output_image, H, W, p_task->k_data.iters);
}

void launch_gaussianblur(schedulerTask * p_task, PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread group) {
    //printf("Before launching GaussianBlur. task_id: %d. rr: %d\n", p_task->id, p_task->rr);
    Ctrl_Launch(ctrl, GaussianBlur, threads, group, p_task->rr, *p_task->input_image, *p_task->output_image, H, W);
}

int n_tasks;
PCtrl ctrl;

void intHandler(int dummy) {
    for(int task = 0; task < n_tasks; task++) {
        Ctrl_Free(ctrl, pregeneratedTasks.input_images[task], pregeneratedTasks.output_images[task]);
    }
    exit(666);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, intHandler);
	main_clock = omp_get_wtime();

    int SIZE = 1024;

    if(argc < 12) {
        printf("Median-Gaussian_Pynq_Ctrl <H> <W> <image_path> <use_CPU> <n_rr> <enabled_priorities> <enabled_preemption> <n_tasks> <minutes> <seed> <enabled_partial>\n");
        return -1;
    }

    H = atoi(argv[1]);
    W = atoi(argv[2]);
    H_NROW = H + 2;
    H_NCOL = W + 2;

    image_path = argv[3];

    int use_CPU = atoi(argv[4]);
    int n_rr = atoi(argv[5]);
    int enabled_priorities = atoi(argv[6]);
    int enabled_preemption = atoi(argv[7]);
    //int n_tasks = atoi(argv[8]);
    n_tasks = atoi(argv[8]);
    double minutes = atof(argv[9]);
    int seed = atoi(argv[10]);
    int enabled_partial = atoi(argv[11]);


	Ctrl_Thread threads;
	Ctrl_ThreadInit(threads, SIZE, SIZE);

	Ctrl_Thread group;
	Ctrl_ThreadInit(group, LOCAL_SIZE_0, LOCAL_SIZE_1);


	__ctrl_block__(1,1)
	{
        SET_INITIAL_CONFIGURATION(N_RR, GaussianBlur, GaussianBlur);
        SET_KERNEL_TASKS( launch_gaussianblur, launch_medianblur3, launch_medianblur2, launch_medianblur1 );



        Ctrl_Policy policy = 1;
        int device = 0;
		//PCtrl ctrl = Ctrl_Create(CTRL_TYPE_PYNQ, policy, device);
		ctrl = Ctrl_Create(CTRL_TYPE_PYNQ, policy, device);

        //printf("Before domainalloc\n");
        //dummy_tile = Ctrl_DomainAlloc(ctrl, int, hitShapeSize(1));
        //for(int rr = 0; rr < n_rr; rr++)
        //    dummy_sync_tile[rr] = Ctrl_DomainAlloc(ctrl, int, hitShapeSize(1));
        //printf("After DomainAlloc\n");

        Ctrl_GlobalSync(ctrl);
        exec_clock = omp_get_wtime();


        srand(seed);

        task_generator(ctrl, n_tasks, minutes, enabled_priorities, H, W, seed);
        PYNQ_scheduler(ctrl, threads, group, n_tasks, use_CPU, n_rr, enabled_priorities, enabled_preemption, enabled_partial);


		Ctrl_GlobalSync(ctrl);
        exec_clock = omp_get_wtime() - exec_clock;

        //#ifdef _DEBUG_OUTPUTS
        //for(int task = 0; task < n_tasks; task++) {
        //    Ctrl_HostTask(ctrl, Write_Image, pregeneratedTasks.output_images[task], H, W, task);
        //    //Ctrl_HostTask(ctrl, Write_Image, pregeneratedTasks.input_images[task], H, W, task);
        //}
        //#endif // _DEBUG_OUTPUTS

        #ifdef _DEBUG_PARTIAL
        //int task = 666;
        //for(int i = 0; i < H_NROW; i++) {
        //    for(int j = 0; j < H_NCOL; j++) {
        //        printf("%d ", hit(partial_output, i, j));
        //    }
        //}
        //printf("\n");
        //Ctrl_HostTask(ctrl, Write_Image, partial_output, H, W, task);
        #endif // _DEBUG_PARTIAL

		Ctrl_GlobalSync(ctrl);


        for(int task = 0; task < n_tasks; task++) {
            Ctrl_Free(ctrl, pregeneratedTasks.input_images[task], pregeneratedTasks.output_images[task]);
        }
        Ctrl_Free(ctrl, dummy_tile);

        Ctrl_Destroy(ctrl);
    }

	main_clock = omp_get_wtime() - main_clock;

	#ifdef _CTRL_EXAMPLES_EXP_MODE_
		printf("%lf, %lf\n", main_clock, exec_clock);
		fflush(stdout);
	#else
		printf("\n ----------------------- TIME ----------------------- \n\n");
		printf(" Clock main: %lf\n", main_clock);
		printf(" Clock exec: %lf\n", exec_clock);
		printf("\n ---------------------------------------------------- \n");
	#endif

	return 0;
}
