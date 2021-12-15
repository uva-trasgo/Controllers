#include "kernel_xphi.h"
#include "CAL_KernelXPHI.h"
#include "CAL_CntrlXPHI.h"


CAL_KERNEL_XPHI(MatSum, 3, IN, HitTile_float*, A, IN, HitTile_float*, B, OUT, HitTile_float*, C){
 
 int k;

  int nIter=1;

  #pragma vector aligned
  #pragma ivdep
  for(k=0;k< nIter;k++){
        hit_tileElemAt(C, 2, threadId.x, threadId.y) = hit_tileElemAt(C, 2, threadId.x, threadId.y) + hit_tileElemAt(A, 2, threadId.x, threadId.y) + hit_tileElemAt(B, 2, threadId.x, threadId.y) ;
  }
/*
        char michostname[100];
        gethostname(michostname, sizeof(michostname));
        printf("MIC: Hello world from MIC. I am %s and I have %ld logical cores. I was called from host: CHIMERA \n", michostname, sysconf(_SC_NPROCESSORS_ONLN));
*/
}








