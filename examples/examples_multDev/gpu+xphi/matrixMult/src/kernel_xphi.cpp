#include "kernel_xphi.h"
#include "CAL_KernelXPHI.h"
#include "CAL_CntrlXPHI.h"


CAL_KERNEL_XPHI(MatMul, 3, IN, HitTile_float*, A, IN, HitTile_float*, B, OUT, HitTile_float*, C){
 

        int k=0;
        //#pragma vector aligned
        #pragma ivdep
        for(k=0; k< hit_tileDimCard(C,1); k++ ){
                hit_tileElemAtNoStride(C,2, threadId.x, k) =hit_tileElemAtNoStride(C, 2, threadId.x, k) +
                         hit_tileElemAtNoStride(A, 2, threadId.x, threadId.y) * hit_tileElemAtNoStride(B, 2, threadId.y, k);
        }


}








