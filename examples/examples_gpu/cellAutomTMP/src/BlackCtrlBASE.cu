#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <assert.h>
#include <hitmap.h>
#include "timer.h"
#include "CAL.h"

int  NUM_ITERATIONS = 512;
const float      RISKFREE = 0.02f;
const float    VOLATILITY = 0.30f;


hit_tileNewType( float );
hit_ktileNewType( float );

double  mainClock;
double  initClock;
double  sequentialClock;


///////////////////////////////////////////////////////////////////////////////
// CPU: Polynomial approximation of cumulative normal distribution function
///////////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////////
// GPU: Polynomial approximation of cumulative normal distribution function
///////////////////////////////////////////////////////////////////////////////
__device__ inline float cndGPU(float d)
{
    const float       A1 = 0.31938153f;
    const float       A2 = -0.356563782f;
    const float       A3 = 1.781477937f;
    const float       A4 = -1.821255978f;
    const float       A5 = 1.330274429f;
    const float RSQRT2PI = 0.39894228040143267793994605993438f;

    float
    K = __fdividef(1.0f, (1.0f + 0.2316419f * fabsf(d)));

    float
    cnd = RSQRT2PI * __expf(- 0.5f * d * d) *
          (K * (A1 + K * (A2 + K * (A3 + K * (A4 + K * A5)))));

    if (d > 0)
        cnd = 1.0f - cnd;

    return cnd;
}

///////////////////////////////////////////////////////////////////////////////
// Black-Scholes formula for both call and put
///////////////////////////////////////////////////////////////////////////////
__device__ inline void BlackScholesBodyGPU(
    float *CallResult,
    float *PutResult,
    float S, //Stock price
    float X, //Option strike
    float T, //Option years
    float R, //Riskless rate
    float V  //Volatility rate
)
{
    float sqrtT, expRT;
    float d1, d2, CNDD1, CNDD2;

    sqrtT = __fdividef(1.0F, rsqrtf(T));
    d1 = __fdividef(__logf(S / X) + (R + 0.5f * V * V) * T, V * sqrtT);
    d2 = d1 - V * sqrtT;

    CNDD1 = cndGPU(d1);
    CNDD2 = cndGPU(d2);

    //Calculate Call and Put simultaneously
    expRT = __expf(- R * T);
    *CallResult = S * CNDD1 - X * expRT * CNDD2;
    *PutResult  = X * expRT * (1.0f - CNDD2) - S * (1.0f - CNDD1);
}

///////////////////////////////////////////////////////////////////////////////
// Kernel function
///////////////////////////////////////////////////////////////////////////////
/* A. CPU Kernel charazterization */
CAL_KERNEL_GPU_CHAR_STATIC(BlackScholesKernel, 1, medium, low, low);

/* B. GPU Kernel implementation */
CAL_KERNEL(BlackScholesKernel, dGPU, 
		 KHitTile_float callResult, 
		 KHitTile_float putResult, 
		 KHitTile_float Sf,  //Stock price
	 	 KHitTile_float Xf,  //Option strike
	  	 KHitTile_float Tf,  //Option years
	  	 float Rf,  //Riskless rate
	  	 float Vf   //Volatility rate
) {
	int pos = threadId.x;


	BlackScholesBodyGPU(
		&(hit_tileElemAtNoStride( callResult, 1, pos )),
		&(hit_tileElemAtNoStride( putResult, 1, pos )),
    		hit_tileElemAtNoStride( Sf, 1, pos ),
    		hit_tileElemAtNoStride( Xf, 1, pos ),
    		hit_tileElemAtNoStride( Tf, 1, pos ),
		Rf, Vf
	);
}

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO(BlackScholesKernel, 
	1, dGPU,
	7, 
	OUT, HitTile_float, callResult, 
	OUT, HitTile_float, putResult, 
	IN,  HitTile_float, Sf,  //Stock price
	IN,  HitTile_float, Xf,  //Option strike
	IN,  HitTile_float, Tf,  //Option years
	INVAL,  float, Rf,  //Riskless rate
	INVAL,  float, Vf   //Volatility rate
	);

///////////////////////////////////////////////////////////////////////////////
// Black-Scholes formula for both call and put, FOR RESULTS CHECKING ONLY
///////////////////////////////////////////////////////////////////////////////
static void BlackScholesBodyCPU(
    float *callResult,
    float *putResult,
    float Sf, //Stock price
    float Xf, //Option strike
    float Tf, //Option years
    float Rf, //Riskless rate
    float Vf  //Volatility rate
)
{
    double S = Sf;
    double X = Xf; 
    double T = Tf;
    double R = Rf; 
    double V = Vf;

    double sqrtT = sqrt(T);
    double    d1 = (log(S / X) + (R + 0.5 * V * V) * T) / (V * sqrtT);
    double    d2 = d1 - V * sqrtT;
    double CNDD1 = CND(d1);
    double CNDD2 = CND(d2);

    //Calculate Call and Put simultaneously
    double expRT = exp(- R * T);
    *callResult   = (float)(S * CNDD1 - X * expRT * CNDD2);
    *putResult    = (float)(X * expRT * (1.0 - CNDD2) - S * (1.0 - CNDD1));
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
int main(int argc, char *argv[]) {
	
        // 1. Init Controllers library
        //CAL_CntrlInit(2);
        omp_set_nested(1);
        omp_set_num_threads(3);
        #pragma omp parallel 
        {
        #pragma omp single 
         {	

        // 2. Taking arguments
  	if ( argc != 4 ) {
  	     fprintf(stderr, "\nUsage: %s <numOpts> <numIter> <GPU>\n", argv[0]);
  	     exit(EXIT_FAILURE);
  	}
  	int OPT_N = atoi( argv[1] );
  	NUM_ITERATIONS = atoi( argv[2] );
	int GPU = atoi( argv[3] );

    	//'h_' prefix - CPU (host) memory space
    	HitTile_float
    	//Results calculated by CPU for reference
    	h_CallResult,
    	h_PutResult,
    	//CPU instance of input data
    	h_StockPrice,
    	h_OptionStrike,
    	h_OptionYears;
    	// Tiles representing single constants

    	int i;
    	// Arturo: Timers
    	Timer tTotal;
    	TimerCreate(tTotal);
    	
    	
		// 3. Declare and initialize full matrices and domains
    	hit_tileDomainAlloc( &h_CallResult, float, 1, OPT_N);
    	hit_tileDomainAlloc( &h_PutResult,  float, 1, OPT_N);
    	hit_tileDomainAlloc( &h_StockPrice,    float, 1, OPT_N);
    	hit_tileDomainAlloc( &h_OptionStrike,  float, 1, OPT_N);
    	hit_tileDomainAlloc( &h_OptionYears,   float, 1, OPT_N);

    	srand(5347);

    	//Generate options set
    	for (i = 0; i < OPT_N; i++)
    	{
    	    hit_tileElemAtNoStride( h_CallResult, 1, i ) = 0.0f;
    	    hit_tileElemAtNoStride( h_PutResult, 1, i )  = -1.0f;
    	    hit_tileElemAtNoStride( h_StockPrice, 1, i )    = RandFloat(5.0f, 30.0f);
    	    hit_tileElemAtNoStride( h_OptionStrike, 1, i )  = RandFloat(1.0f, 100.0f);
    	    hit_tileElemAtNoStride( h_OptionYears, 1, i )   = RandFloat(0.25f, 10.0f);
    	}    		
	
    	// Initialize the device before measuring
    	cudaDeviceSynchronize();
    	TimerStart(tTotal);
        
		// 4. Init computation threads
    	CALThread threads;
    	CALThreadInit(threads, 1, OPT_N);	

		// 5. Create controller object
		CALCntrl comm;
		CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU );
  
		// 6. Attach data structures to the controllers
		CAL_CntrlAttach(&comm, (HitTile*)&h_CallResult);
    	CAL_CntrlAttach(&comm, (HitTile*)&h_PutResult);
    	CAL_CntrlAttach(&comm, (HitTile*)&h_StockPrice);
    	CAL_CntrlAttach(&comm, (HitTile*)&h_OptionStrike);
    	CAL_CntrlAttach(&comm, (HitTile*)&h_OptionYears);
	
		// 7. Launch the task: invoking the kernel
    	for (i = 0; i < NUM_ITERATIONS; i++) {
    		   CAL_CntrlLaunch(comm, BlackScholesKernel, threads, h_CallResult, h_PutResult, 
								h_StockPrice, h_OptionStrike, h_OptionYears, RISKFREE, VOLATILITY );
    	}

		// 8. Copy result from device memory to host memory
    	CAL_CntrlDetach(&comm, (HitTile*)&h_CallResult);
    	CAL_CntrlDetach(&comm, (HitTile*)&h_PutResult);

		// 9. Destroy the controller
    	CAL_CntrlDetach(&comm, (HitTile*)&h_StockPrice);
    	CAL_CntrlDetach(&comm, (HitTile*)&h_OptionStrike);
    	CAL_CntrlDetach(&comm, (HitTile*)&h_OptionYears);
    	CAL_CntrlDestroy(&comm);

		// 10. TIMES
    	cudaDeviceSynchronize();
    	TimerStop( tTotal );
    
    	printf("SIZE %d %d\n", OPT_N, NUM_ITERATIONS );
    	printf("Clock main:  %.8lf\n", TimerGetTime(tTotal) );

		// 11. Calculate NORM
        double resultado=0,suma=0;
        for (int i=0; i<OPT_N; i++){
                suma+= pow(0.1*( (hit_tileElemAt((h_CallResult), 1, i) + hit_tileElemAt((h_PutResult), 1, i))), 2 );
        }

        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");

		// 12. Free data structures
    	hit_tileFree(h_CallResult);
    	hit_tileFree(h_PutResult);
    	hit_tileFree(h_StockPrice);
		hit_tileFree(h_OptionStrike);
		hit_tileFree(h_OptionYears);
    	TimerDestroy( tTotal );
	
		printf("hola mundo 4\n");
	
		
	CAL_CntrlFinish();
	return 0;
}

