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

CAL_KERNEL_HEADER_XPHI(BlackScholesKernel, 7,
        OUT, HitTile_float*, callResult,
        OUT, HitTile_float*, putResult,
        IN,  HitTile_float*, Sf,  //Stock price
        IN,  HitTile_float*, Xf,  //Option strike
        IN,  HitTile_float*, Tf,  //Option years
        IN,  HitTile_float*, Rf,  //Riskless rate
        IN,  HitTile_float*, Vf   //Volatility rate
)

