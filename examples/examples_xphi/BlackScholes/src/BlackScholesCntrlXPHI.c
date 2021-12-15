#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include <hitmap.h>
#include "CAL.h"
#include "CAL_KernelXPHI.h"


 int  NUM_ITERATIONS = 1; //512;
float      RISKFREE = 0.02f;
float    VOLATILITY = 0.30f;


hit_tileNewType( float );

double  mainClock;
double  initClock;
double  sequentialClock;

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



////////////////////////////////////////////////////////////////////////////////
// Helper function, returning uniformly distributed
// random float in [low, high] range
////////////////////////////////////////////////////////////////////////////////
float RandFloat(float low, float high)
{
    float t = (float)rand() / (float)RAND_MAX;
    return (1.0f - t) * low + t * high;
}


////////////////////////////////////////////////////////////////////////////////
// Main program
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{

  if ( argc < 3 ) {
       fprintf(stderr, "\nUsage: %s <numOpts> <numIter>\n", argv[0]);
       exit(EXIT_FAILURE);
  }
  int OPT_N = atoi( argv[1] );
  NUM_ITERATIONS = atoi( argv[2] );
  
  CAL_CntrlInit(1);

    // Start logs

    //'h_' prefix - XPHI (host) memory space
    HitTile_float
    //Results calculated by XPHI for reference
    h_CallResultXPHI,
    h_PutResultXPHI,
    //XPHI instance of input data
    h_StockPrice,
    h_OptionStrike,
    h_OptionYears,
    // Tiles representing single constants
    h_RiskFree,
    h_Volatility;

    int i;
    // Arturo: Timers
    double totalClock;
    double mainClock;
    totalClock = omp_get_wtime();

    hit_tileDomainAlloc( &h_CallResultXPHI, float, 1, OPT_N);
    hit_tileDomainAlloc( &h_PutResultXPHI,  float, 1, OPT_N);
    hit_tileDomainAlloc( &h_StockPrice,    float, 1, OPT_N);
    hit_tileDomainAlloc( &h_OptionStrike,  float, 1, OPT_N);
    hit_tileDomainAlloc( &h_OptionYears,   float, 1, OPT_N);
    
    HitTile foo1, foo2;
    hit_tileSingle( &foo1, RISKFREE, sizeof(float) );
    h_RiskFree = *(HitTile_float *)&foo1;
    hit_tileSingle( &foo2, VOLATILITY, sizeof(float) );
    h_Volatility = *(HitTile_float *)&foo2;
	

    srand(5347);
    //Generate options set
    for (i = 0; i < OPT_N; i++)
    {
        hit_tileElemAtNoStride( h_CallResultXPHI, 1, i ) = 0.0f;
        hit_tileElemAtNoStride( h_PutResultXPHI, 1, i )  = -1.0f;
        hit_tileElemAtNoStride( h_StockPrice, 1, i )    = RandFloat(5.0f, 30.0f);
        hit_tileElemAtNoStride( h_OptionStrike, 1, i )  = RandFloat(1.0f, 100.0f);
        hit_tileElemAtNoStride( h_OptionYears, 1, i )   = RandFloat(0.25f, 10.0f);
    }

    //Calculate options values on XPHI
    mainClock = omp_get_wtime();
    
    CALCntrl comm;
    CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
    CAL_CntrlAttach(&comm, (HitTile*)&h_CallResultXPHI);
    CAL_CntrlAttach(&comm, (HitTile*)&h_PutResultXPHI);
    CAL_CntrlAttach(&comm, (HitTile*)&h_StockPrice);
    CAL_CntrlAttach(&comm, (HitTile*)&h_OptionStrike);
    CAL_CntrlAttach(&comm, (HitTile*)&h_OptionYears);
    CAL_CntrlAttach(&comm, (HitTile*)&h_RiskFree);
    CAL_CntrlAttach(&comm, (HitTile*)&h_Volatility);

    CALThread threads;
    CALThreadInit(threads, 1, OPT_N);

          
    for (i = 0; i < NUM_ITERATIONS; i++)
    {
      CAL_CntrlLaunch(comm, BlackScholesKernel, threads, 7, &h_CallResultXPHI, &h_PutResultXPHI, &h_StockPrice, &h_OptionStrike, &h_OptionYears, &h_RiskFree, &h_Volatility );
    }

    // Copy result from device memory to host memory
    CAL_CntrlDetach(&comm, (HitTile*)&h_CallResultXPHI);
    CAL_CntrlDetach(&comm, (HitTile*)&h_PutResultXPHI);


	
    CAL_CntrlDetach(&comm, (HitTile*)&h_StockPrice);
    CAL_CntrlDetach(&comm, (HitTile*)&h_OptionStrike);
    CAL_CntrlDetach(&comm, (HitTile*)&h_OptionYears);
    CAL_CntrlDetach(&comm, (HitTile*)&h_RiskFree);
    CAL_CntrlDetach(&comm, (HitTile*)&h_Volatility);
    CAL_CntrlDestroy(&comm);
    
    mainClock = omp_get_wtime() - mainClock;
    totalClock = omp_get_wtime() - totalClock;


        // Calculate NORM
        double resultado=0,suma=0;
        for (int i=0; i<OPT_N; i++){
                suma+= pow( (hit_tileElemAt((h_CallResultXPHI), 1, i) + hit_tileElemAt((h_PutResultXPHI), 1, i)), 2 );
        }

        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");


    hit_tileFree(h_CallResultXPHI);
    hit_tileFree(h_PutResultXPHI);
    hit_tileFree(h_StockPrice);
    hit_tileFree(h_OptionStrike);
    hit_tileFree(h_OptionYears);


    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );

    CAL_CntrlFinish();

    exit(EXIT_SUCCESS);
}

