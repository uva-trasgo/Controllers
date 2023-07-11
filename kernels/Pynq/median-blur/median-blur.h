#include "Ctrl.h"

#ifndef _CTRL_NEWTYPE_INT_
#define _CTRL_NEWTYPE_INT_
Ctrl_NewType(int);
#endif // _CTRL_NEWTYPE_INT_

CTRL_KERNEL_FUNCTION(MedianBlur, PYNQ, DEFAULT, KTILE_ARGS(KHitTile_int input_array, KHitTile_int output_array), INT_ARGS(int H, int W, int iters), FLOAT_ARGS(NO_FLOAT_ARG));
