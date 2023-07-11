#define _CTRL_PYNQ_KERNEL_FILE_
#include "Ctrl.h"

//#define N 1024
//#define H_NCOL (N+2)

#define ap_wait()

Ctrl_NewType(int);

#include <string.h>

CTRL_KERNEL_FUNCTION(MedianBlur, PYNQ, DEFAULT, KTILE_ARGS(KHitTile_int input_array, KHitTile_int output_array), INT_ARGS(int H, int W, int iters), FLOAT_ARGS(NO_FLOAT_ARG)) {
  DEF_KTILE_INTERFACES(input_array, output_array);
  DEF_INT_INTERFACES(H, W, iters);
  DEF_FLOAT_INTERFACES(NO_FLOAT_INTERFACE);
  DEF_RETURN_INTERFACE();
   
    KTILE(input_array, int); 
    KTILE(output_array, int); 

    int k, row, col;
    context_vars(k, row, col);

    int H_NROW = H+2;
    int H_NCOL = W+2;

    int window[9]; 
    for_save(k, 0, iters, 1) {
        for_save(row, 1, H+1, 1) {
            for_save(col, 1, W+1, 1) {
                if(*ktile_args_0_data == 1 && row == 123 && col == 123) {
                    return;
                }
                window[0] = hit(input_array,(row-1)*H_NCOL+col-1);
                window[1] = hit(input_array,(row-1)*H_NCOL+col);
                window[2] = hit(input_array,(row-1)*H_NCOL+col+1);
                window[3] = hit(input_array,row*H_NCOL+col-1);
                window[4] = hit(input_array,row*H_NCOL+col);
                window[5] = hit(input_array,row*H_NCOL+col+1);
                window[6] = hit(input_array,(row+1)*H_NCOL+col-1);
                window[7] = hit(input_array,(row+1)*H_NCOL+col);
                window[8] = hit(input_array,(row+1)*H_NCOL+col+1);

                // insertion sort
                int i, element, j;
                for (i = 1; i < 9; i++) {
                        element = window[i];
                        j = i - 1;
                        while (j >= 0 && window[j] > element) {
                                window[j + 1] = window[j];
                                j = j - 1;
                        }
                        window[j + 1] = element;
                }
                
                hit(output_array,row*H_NCOL+col) = window[4];

                for_checkpoint(col);
            }
            for_checkpoint(row);
        }
        for_checkpoint(k);
    }
}
int main() {
    FILE * image;
    image = fopen("Saltpepper.pgm", "r");
    //image = fread("Saltpepper.pgm", "r");
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
            in_matrix_data[i * H_NCOL + j] = data[(i-1) * W + j];
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
    int iters = 2;

    MedianBlur(in_matrix_wrapper, in_matrix_data, out_matrix_wrapper, out_matrix_data, out_matrix_wrapper, &stop, H, W, iters, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &context, &return_value);

    //printf("\nCONTEXT: ");
    //for(int i = 0; i < 10; i++)
    //    printf("%d ", context.var[i]);
    //printf("\n");

    stop = 0;
    MedianBlur(in_matrix_wrapper, in_matrix_data, out_matrix_wrapper, out_matrix_data, out_matrix_wrapper, &stop, H, W, iters, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &context, &return_value);

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
