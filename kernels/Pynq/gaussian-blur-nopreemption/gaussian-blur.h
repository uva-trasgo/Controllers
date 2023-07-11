#include "Ctrl.h"

#ifndef _CTRL_NEWTYPE_INT_
#define _CTRL_NEWTYPE_INT_
Ctrl_NewType(int);
#endif // _CTRL_NEWTYPE_INT_

CTRL_KERNEL_FUNCTION(GaussianBlur, PYNQ, DEFAULT, KTILE_ARGS(KHitTile_int in_matrix, KHitTile_int out_matrix), INT_ARGS(int H, int W), FLOAT_ARGS(NO_FLOAT_ARG));
