#define _CTRL_PYNQ_KERNEL_FILE_

#include "median-blur.h"

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
