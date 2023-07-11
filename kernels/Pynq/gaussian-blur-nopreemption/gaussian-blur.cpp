#define _CTRL_PYNQ_KERNEL_FILE_

#include <string.h>
#include <math.h>

void GaussianBlur(int * in_matrix, int * out_matrix, int H, int W) {
  int H_NCOL = W + 2;
  int H_NROW = H + 2;
  
  float weights[] = {0.07, 0.12, 0.07,
                    0.12, 0.19, 0.12,
                    0.07, 0.12, 0.07};

  int i, j;

  int new_value;

  int window[9];


  for(int i = 1; i < H_NROW-1; i++) {
    for(int j = 1; j < H_NCOL-1; j++) {
      window[4] = in_matrix[i * H_NCOL + j];
      window[1] = in_matrix[(i-1) * H_NCOL + j];
      window[0] = in_matrix[(i-1) * H_NCOL + (j-1)];
      window[2] = in_matrix[(i-1) * H_NCOL + (j+1)];
      window[7] = in_matrix[(i+1) * H_NCOL + j];
      window[6] = in_matrix[(i+1) * H_NCOL + (j-1)];
      window[8] = in_matrix[(i+1) * H_NCOL + (j+1)];
      window[3] = in_matrix[i * H_NCOL + (j-1)];
      window[5] = in_matrix[i * H_NCOL + (j+1)];

      new_value = 0;
      for(int k = 0; k < 9; k++)
          new_value += floor(weights[k] * window[k]);

      out_matrix[i * H_NCOL + j] = new_value;
    }
  }
}
