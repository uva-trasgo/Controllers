#include "kernel_xphi.h"
#include "CAL_KernelXPHI.h"
#include "CAL_CntrlXPHI.h"

///////////////////////////////////////////////////////////////////////////////
// Polynomial approximation of cumulative normal distribution function
///////////////////////////////////////////////////////////////////////////////
static float __attribute__((target(mic))) CND(float d)
{
    const float       A1 = 0.31938153;
    const float       A2 = -0.356563782;
    const float       A3 = 1.781477937;
    const float       A4 = -1.821255978;
    const float       A5 = 1.330274429;
    const float RSQRT2PI = 0.39894228040143267793994605993438;

    float
    K = 1.0 / (1.0 + 0.2316419 * fabs(d));

    float
    cnd = RSQRT2PI * exp(- 0.5 * d * d) *
          (K * (A1 + K * (A2 + K * (A3 + K * (A4 + K * A5)))));

    if (d > 0)
        cnd = 1.0 - cnd;

    return cnd;
}

///////////////////////////////////////////////////////////////////////////////
// Kernel function
///////////////////////////////////////////////////////////////////////////////

CAL_KERNEL_XPHI(BlackScholesKernel, 7,
        OUT, HitTile_float*, callResult,
        OUT, HitTile_float*, putResult,
        IN,  HitTile_float*, Sf,  //Stock price
        IN,  HitTile_float*, Xf,  //Option strike
        IN,  HitTile_float*, Tf,  //Option years
        IN,  HitTile_float*, Rf,  //Riskless rate
        IN,  HitTile_float*, Vf   //Volatility rate
) {
 // int i;
  //for (i = 0; i < NUM_ITERATIONS; i++)
  {
    float S = hit_tileElemAtNoStride( Sf, 1, threadId.x );
    float X = hit_tileElemAtNoStride( Xf, 1, threadId.x );
    float T = hit_tileElemAtNoStride( Tf, 1, threadId.x );
    float R = hit_tileElemAtNoStride( Rf, 1, 0 );
    float V = hit_tileElemAtNoStride( Vf, 1, 0 );

    float sqrtT = sqrt(T);
    float    d1 = (log(S / X) + (R + 0.5 * V * V) * T) / (V * sqrtT);
    float    d2 = d1 - V * sqrtT;
    float CNDD1 = CND(d1);
    float CNDD2 = CND(d2);

    //Calculate Call and Put simultaneously
    float expRT = exp(- R * T);
    hit_tileElemAtNoStride( callResult, 1, threadId.x ) = (float)(S * CNDD1 - X * expRT * CNDD2);
    hit_tileElemAtNoStride( putResult, 1, threadId.x )  = (float)(X * expRT * (1.0 - CNDD2) - S * (1.0 - CNDD1));
  }
}










