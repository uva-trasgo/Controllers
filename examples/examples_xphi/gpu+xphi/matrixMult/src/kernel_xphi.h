#include <hitmap.h>
#include <math.h>
#include <omp.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CAL_Cntrl.h"
#include "hit_ktile.h"
#include "CAL_header.h"

hit_tileNewType( float );

CAL_KERNEL_HEADER_XPHI(MatMul, 3, IN, HitTile_float*, A, IN, HitTile_float*, B, OUT, HitTile_float*, C);

