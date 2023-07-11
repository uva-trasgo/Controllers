#define _CTRL_PYNQ_KERNEL_FILE_
#include "Ctrl.h"
#include <math.h>

//#define NROW 512
//#define NCOL 512
//#define H_NROW (NROW+2)
//#define H_NCOL (NCOL+2)

#define ap_wait()

Ctrl_NewType(int);

#include <string.h>

#define UP 1
#define UP_LEFT 0
#define UP_RIGHT 2
#define DOWN 7
#define DOWN_LEFT 6
#define DOWN_RIGHT 8
#define LEFT 3
#define RIGHT 5
#define CURRENT 4

CTRL_KERNEL_FUNCTION(GaussianBlur, PYNQ, DEFAULT, KTILE_ARGS(KHitTile_int in_matrix, KHitTile_int out_matrix), INT_ARGS(int H, int W), FLOAT_ARGS(NO_FLOAT_ARG)) {
  DEF_KTILE_INTERFACES(in_matrix, out_matrix);
  DEF_INT_INTERFACES(H, W);
  DEF_FLOAT_INTERFACES(NO_FLOAT_INTERFACE);
  DEF_RETURN_INTERFACE();

  KTILE(in_matrix, int);
  KTILE(out_matrix, int);

  int H_NCOL = W + 2;
  int H_NROW = H + 2;
  
  //float weights[] = {0.07, 0.12, 0.07,
  //                  0.12, 0.19, 0.12,
  //                  0.07, 0.12, 0.07};
  context->var[0] = 4;

  int weights[] = {0, 0, 0,
                    0, 1, 0,
                    0, 0, 0};
  int sum;

  int up, up_left, up_right, down, down_left, down_right, left, right, current;

  int i, j;

  //context_vars(i,j);


  //for_save(i, 1, H_NROW-1, 1) {
  for(int i = 1; i < H_NROW-1; i++) {
    #pragma HLS pipeline off
    //for_save(j, 1, H_NCOL-1, 1) {
    for(int j = 1; j < H_NCOL-1; j++) {
      #pragma HLS pipeline off
      up = (i-1) * H_NCOL + j;
      up_left = (i-1) * H_NCOL + (j-1);
      up_right = (i-1) * H_NCOL + (j+1);
      down = (i+1) * H_NCOL + j;
      down_left = (i+1) * H_NCOL + (j-1);
      down_right = (i+1) * H_NCOL + (j+1);
      left = i * H_NCOL + (j-1);
      right = i * H_NCOL + (j+1);
      current = i * H_NCOL + j;
      hit(out_matrix, current) = weights[1] * hit(in_matrix, up) +
                                       weights[0] * hit(in_matrix, up_left) +
                                       weights[2] * hit(in_matrix, up_right) +
                                       weights[7] * hit(in_matrix, down) +
                                       weights[6] * hit(in_matrix, down_left) +
                                       weights[8] * hit(in_matrix, down_right) +
                                       weights[3] * hit(in_matrix, left) +
                                       weights[5] * hit(in_matrix, right) +
                                       weights[4] * hit(in_matrix, current);
      //for_checkpoint(j);
    }
    //for_checkpoint(i);
  }
}
/*
CTRL_KERNEL_FUNCTION(GaussianBlur, PYNQ, DEFAULT, KTILE_ARGS(KHitTile_int in_matrix, KHitTile_int out_matrix), INT_ARGS(int H, int W), FLOAT_ARGS(NO_FLOAT_ARG)) {
  DEF_KTILE_INTERFACES(in_matrix, out_matrix);
  DEF_INT_INTERFACES(H, W);
  DEF_FLOAT_INTERFACES(NO_FLOAT_INTERFACE);
  DEF_RETURN_INTERFACE();

  KTILE(in_matrix, int);
  KTILE(out_matrix, int);

  int H_NCOL = W + 2;
  int H_NROW = H + 2;
  
  float weights[] = {0.07, 0.12, 0.07,
                    0.12, 0.19, 0.12,
                    0.07, 0.12, 0.07};

  int sum;

  int up, up_left, up_right, down, down_left, down_right, left, right, current;

  int i, j;

  context_vars(i,j);


  for_save(i, 1, H_NROW-1, 1) {
    for_save(j, 1, H_NCOL-1, 1) {
      up = (i-1) * H_NCOL + j;
      up_left = (i-1) * H_NCOL + (j-1);
      up_right = (i-1) * H_NCOL + (j+1);
      down = (i+1) * H_NCOL + j;
      down_left = (i+1) * H_NCOL + (j-1);
      down_right = (i+1) * H_NCOL + (j+1);
      left = i * H_NCOL + (j-1);
      right = i * H_NCOL + (j+1);
      current = i * H_NCOL + j;
      hit(out_matrix, current) = floor(weights[1] * hit(in_matrix, up) +
                                       weights[0] * hit(in_matrix, up_left) +
                                       weights[2] * hit(in_matrix, up_right) +
                                       weights[7] * hit(in_matrix, down) +
                                       weights[6] * hit(in_matrix, down_left) +
                                       weights[8] * hit(in_matrix, down_right) +
                                       weights[3] * hit(in_matrix, left) +
                                       weights[5] * hit(in_matrix, right) +
                                       weights[4] * hit(in_matrix, current));
      for_checkpoint(j);
    }
    for_checkpoint(i);
  }
}
*/

int main(int argc, char * argv[]) {
    if(argc < 2) {
        printf("gaussian-blur_cpu <context_switch>\n");
        return 1; 
    }

    int context_switch = atoi(argv[1]);

    FILE * image;
    image = fopen("baboon.ascii.pgm", "r");
    //image = fopen("Saltpepper.pgm", "r");
    char * line = NULL;
    size_t len = 0;
    ssize_t size = getline(&line, &len, image);
                                                                                                              
    int H, W;
    fscanf(image, "%d", &H);
    fscanf(image, "%d", &W);

    int H_NROW = H+2;
    int H_NCOL = W+2;
                           
    getline(&line, &len, image);
                              
    int * data = malloc(H*W * sizeof(int));
    int * output = malloc(H*W * sizeof(int));
                                            
    for(int i = 0; i < H; i++) {
        for(int j = 0; j < W; j++) {
                fscanf(image, "%d", &data[i*W+j]);
        }
    }

    data_KHitTile_int in_matrix_data = malloc(H_NROW*H_NCOL * sizeof(int));
    data_KHitTile_int out_matrix_data = malloc(H_NROW*H_NCOL * sizeof(int));
    for(int i = 0; i < H*W; i++) in_matrix_data[i] = 0;
    //for(int i = 0; i < H*W; i++) in_matrix_data[i] = data[i]; 
    for(int i = 1; i < H_NROW-1; i++) {
        for(int j = 1; j < H_NROW-1; j++) {
            in_matrix_data[i * H_NCOL + j] = data[(i-1) * W + (j-1)];
        }
    }
    for(int i = 0; i < H*W; i++) out_matrix_data[i] = 0;


    fpga_wrapper_KHitTile_int in_matrix_wrapper;
    fpga_wrapper_KHitTile_int out_matrix_wrapper;

    for(int i = 0; i < 4; i++) {
        in_matrix_wrapper.origAcumCard[i] = 0;
        out_matrix_wrapper.origAcumCard[i] = 0;
    }   
    for(int i = 0; i < 3; i++) {
        in_matrix_wrapper.card[i] = 0;
        out_matrix_wrapper.card[i] = 0;
    }   
    in_matrix_wrapper.offset = 0;
    out_matrix_wrapper.offset = 0;

    struct context context;
    for(int i = 0; i < 10; i++) {
        context.var[i] = 0;
        context.init_var[i] = 0;
        context.incr_var[i] = 0;
        context.saved[i] = 0;
    }   
    context.valid = 1;

    int return_value = 0;
    
    int stop = 1; 

    if(context_switch) {

        GaussianBlur(in_matrix_wrapper, in_matrix_data, out_matrix_wrapper, out_matrix_data, out_matrix_wrapper, &stop, H, W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &context, &return_value);
    
        //printf("\nCONTEXT: ");
        //for(int i = 0; i < 10; i++)
        //    printf("%d ", context.var[i]);
        //printf("\n");
    }

    stop = 0;
    GaussianBlur(in_matrix_wrapper, in_matrix_data, out_matrix_wrapper, out_matrix_data, out_matrix_wrapper, &stop, H, W, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &context, &return_value);

    
    printf("P2\n");
    printf("%d %d\n", H, W);
    printf("255\n");

    for(int i = 1; i < H_NROW-1; i++) {
        for(int j = 1; j < H_NCOL-1; j++) {
                printf("%d ", out_matrix_data[i*H_NCOL+j]);
        }
        printf("\n");
    }
    

    fclose(image);
    free(line);
}
