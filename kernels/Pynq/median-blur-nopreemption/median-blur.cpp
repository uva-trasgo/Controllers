#define _CTRL_PYNQ_KERNEL_FILE_

void MedianBlur(int * input_array, int * output_array, int H, int W, int iters) {
    int k, row, col;

    int H_NROW = H+2;
    int H_NCOL = W+2;

    int window[9]; 
    for(int k = 0; k < iters; k++) {
        for(int row = 1; row < H+1; row++) {
            for(int col = 1; col < W+1; col++) {
                window[0] = input_array[(row-1)*H_NCOL+col-1];
                window[1] = input_array[(row-1)*H_NCOL+col];
                window[2] = input_array[(row-1)*H_NCOL+col+1];
                window[3] = input_array[row*H_NCOL+col-1];
                window[4] = input_array[row*H_NCOL+col];
                window[5] = input_array[row*H_NCOL+col+1];
                window[6] = input_array[(row+1)*H_NCOL+col-1];
                window[7] = input_array[(row+1)*H_NCOL+col];
                window[8] = input_array[(row+1)*H_NCOL+col+1];

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
                output_array[row*H_NCOL+col] = window[4];
            }
        }
    }   
}
