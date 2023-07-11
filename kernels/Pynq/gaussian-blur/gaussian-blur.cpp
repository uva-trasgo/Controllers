#define _CTRL_PYNQ_KERNEL_FILE_

#include "gaussian-blur.h"
#include <string.h>
#include <math.h>

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

  int i, j;

  context_vars(i,j);

  int new_value;

  int window[9];


  for_save(i, 1, H_NROW-1, 1) {
    for_save(j, 1, H_NCOL-1, 1) {
      window[4] = hit(in_matrix, i * H_NCOL + j);
      window[1] = hit(in_matrix, (i-1) * H_NCOL + j);
      window[0] = hit(in_matrix, (i-1) * H_NCOL + (j-1));
      window[2] = hit(in_matrix, (i-1) * H_NCOL + (j+1));
      window[7] = hit(in_matrix, (i+1) * H_NCOL + j);
      window[6] = hit(in_matrix, (i+1) * H_NCOL + (j-1));
      window[8] = hit(in_matrix, (i+1) * H_NCOL + (j+1));
      window[3] = hit(in_matrix, i * H_NCOL + (j-1));
      window[5] = hit(in_matrix, i * H_NCOL + (j+1));

      new_value = 0;
      for(int k = 0; k < 9; k++)
          new_value += floor(weights[k] * window[k]);

      hit(out_matrix, i * H_NCOL + j) = new_value;

      for_checkpoint(j);
    }
    for_checkpoint(i);
  }
}
