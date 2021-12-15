#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include <hitmap.h>
#include "CAL.h"

int  NUM_ITERATIONS = 512;
float      RISKFREE = 0.02f;
float    VOLATILITY = 0.30f;


hit_tileNewType( float );
hit_ktileNewType( float );

double  mainClock;
double  initClock;
double  sequentialClock;

/* A. Function: Polynomial approximation of cumulative normal distribution function */
static double CND(double d)
{
    const double       A1 = 0.31938153;
    const double       A2 = -0.356563782;
    const double       A3 = 1.781477937;
    const double       A4 = -1.821255978;
    const double       A5 = 1.330274429;
    const double RSQRT2PI = 0.39894228040143267793994605993438;

    double
    K = 1.0 / (1.0 + 0.2316419 * fabs(d));

    double
    cnd = RSQRT2PI * exp(- 0.5 * d * d) *
          (K * (A1 + K * (A2 + K * (A3 + K * (A4 + K * A5)))));

    if (d > 0)
        cnd = 1.0 - cnd;

    return cnd;
}


/* B. Kernel function */
CAL_KERNEL(BlackScholesKernel, CPU, 
	  KHitTile_float callResult, 
	  KHitTile_float putResult, 
	  KHitTile_float Sf,  //Stock price
	  KHitTile_float Xf,  //Option strike
	  KHitTile_float Tf,  //Option years
	  KHitTile_float Rf,  //Riskless rate
	  KHitTile_float Vf   //Volatility rate
) {
    double S = hit_tileElemAtNoStride( Sf, 1, threadId.x );
    double X = hit_tileElemAtNoStride( Xf, 1, threadId.x );
    double T = hit_tileElemAtNoStride( Tf, 1, threadId.x );
    double R = hit_tileElemAtNoStride( Rf, 1, 0 );
    double V = hit_tileElemAtNoStride( Vf, 1, 0 );

    double sqrtT = sqrt(T);
    double    d1 = (log(S / X) + (R + 0.5 * V * V) * T) / (V * sqrtT);
    double    d2 = d1 - V * sqrtT;
    double CNDD1 = CND(d1);
    double CNDD2 = CND(d2);

    //Calculate Call and Put simultaneously
    double expRT = exp(- R * T);
    hit_tileElemAtNoStride( callResult, 1, threadId.x ) = (float)(S * CNDD1 - X * expRT * CNDD2);
    hit_tileElemAtNoStride( putResult, 1, threadId.x )  = (float)(X * expRT * (1.0 - CNDD2) - S * (1.0 - CNDD1));
}

/* C. Defining kernel protoypes */
CAL_KERNEL_PROTO( BlackScholesKernel,                           // Name
                  1, CPU,                                       // Implementations
                  7,         
			OUT, HitTile_float, callResult,
        		OUT, HitTile_float, putResult,
        		IN,  HitTile_float, Sf,  //Stock price
        		IN,  HitTile_float, Xf,  //Option strike
        		IN,  HitTile_float, Tf,  //Option years
        		IN,  HitTile_float, Rf,  //Riskless rate
        		IN,  HitTile_float, Vf   
    		);


/* D. Helper function, returning uniformly distributed random float in [low, high] range */
float RandFloat(float low, float high)
{
    float t = (float)rand() / (float)RAND_MAX;
    return (1.0f - t) * low + t * high;
}


/*
 * Main program
 */
int main(int argc, char **argv)
{

    // 1. Init Controllers library
    CAL_CntrlInit(1);
  
    // 2. Taking arguments
    if ( argc != 4 ) {
       fprintf(stderr, "\nUsage: %s <numOpts> <numIters> <numThreads>\n", argv[0]);
       exit(EXIT_FAILURE);
    }
    int OPT_N = atoi( argv[1] );
    NUM_ITERATIONS = atoi( argv[2]);
    int num_threads=atoi( argv[3]);
      
    int i;

    double totalClock;
    double mainClock;
    totalClock = omp_get_wtime();

    // 3. Declare and initialize full matrices and domains
    	//'h_' prefix - CPU (host) memory space
    HitTile_float
    	//Results calculated by CPU for reference
    	h_CallResultCPU,
    	h_PutResultCPU,
    	//CPU instance of input data
    	h_StockPrice,
    	h_OptionStrike,
    	h_OptionYears,
    	// Tiles representing single constants
    	h_RiskFree,
    	h_Volatility;

    hit_tileDomainAlloc( &h_CallResultCPU, float, 1, OPT_N);
    hit_tileDomainAlloc( &h_PutResultCPU,  float, 1, OPT_N);
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
        hit_tileElemAtNoStride( h_CallResultCPU, 1, i ) = 0.0f;
        hit_tileElemAtNoStride( h_PutResultCPU, 1, i )  = -1.0f;
        hit_tileElemAtNoStride( h_StockPrice, 1, i )    = RandFloat(5.0f, 30.0f);
        hit_tileElemAtNoStride( h_OptionStrike, 1, i )  = RandFloat(1.0f, 100.0f);
        hit_tileElemAtNoStride( h_OptionYears, 1, i )   = RandFloat(0.25f, 10.0f);
    }


    // 4. Init computation threads 
    CALThread threads;
    CALThreadInit(threads, 1, OPT_N);
    
    // 5. Create controller object
    CALCntrl comm;
    CAL_CntrlCreate(&comm, CAL_CNTRL_CPU, num_threads);
    
    // 6. Attach data structures to the controllers
    CAL_CntrlAttach(&comm, (HitTile*)&h_CallResultCPU);
    CAL_CntrlAttach(&comm, (HitTile*)&h_PutResultCPU);
    CAL_CntrlAttach(&comm, (HitTile*)&h_StockPrice);
    CAL_CntrlAttach(&comm, (HitTile*)&h_OptionStrike);
    CAL_CntrlAttach(&comm, (HitTile*)&h_OptionYears);
    CAL_CntrlAttach(&comm, (HitTile*)&h_RiskFree);
    CAL_CntrlAttach(&comm, (HitTile*)&h_Volatility);


    mainClock = omp_get_wtime();

    // 7. Launch the task
    for (i = 0; i < NUM_ITERATIONS; i++)
           CAL_CntrlLaunch(comm, BlackScholesKernel, threads, h_CallResultCPU, h_PutResultCPU, h_StockPrice, h_OptionStrike, h_OptionYears, h_RiskFree, h_Volatility );

    // 8. Copy result from device memory to host memory
    CAL_CntrlDetach(&comm, (HitTile*)&h_CallResultCPU);
    CAL_CntrlDetach(&comm, (HitTile*)&h_PutResultCPU);


    mainClock = omp_get_wtime() - mainClock;

    // 9. Destroy the controller	
    CAL_CntrlDestroy(&comm);
    
    // 10. TIMES
    totalClock = omp_get_wtime() - totalClock;
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );

    // 11. Calculate NORM
    double resultado=0,suma=0;
    for (int i=0; i<OPT_N; i++){
            suma+= pow( (hit_tileElemAt((h_CallResultCPU), 1, i) + hit_tileElemAt((h_PutResultCPU), 1, i)), 2 );
    }

    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");

    // 12. Free data structures
    hit_tileFree(h_CallResultCPU);
    hit_tileFree(h_PutResultCPU);
    hit_tileFree(h_StockPrice);
    hit_tileFree(h_OptionStrike);
    hit_tileFree(h_OptionYears);

    // 13. Finish the controller library
    CAL_CntrlFinish();

    exit(EXIT_SUCCESS);
}

