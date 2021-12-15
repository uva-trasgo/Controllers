#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"
#include "CAL_KernelGPU.h"
#include "CAL_KernelCPU.h"
#include "CAL_CntrlDev.h"



int  NUM_ITERATIONS = 1;
float      RISKFREE = 0.02f;
float    VOLATILITY = 0.30f;


//hit_tileNewType( float );


///////////////////////////////////////////////////////////////////////////////
// Polynomial approximation of cumulative normal distribution function
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
// Kernel function
///////////////////////////////////////////////////////////////////////////////
CAL_KERNEL_CPU(BlackScholesKernel, 7,
        OUT, HitTile_float*, callResult,
        OUT, HitTile_float*, putResult,
        IN,  HitTile_float*, Sf,  //Stock price
        IN,  HitTile_float*, Xf,  //Option strike
        IN,  HitTile_float*, Tf,  //Option years
        IN,  HitTile_float*, Rf,  //Riskless rate
        IN,  HitTile_float*, Vf   //Volatility rate
) {
    double S = hit_tileElemAtNoStride( *Sf, 1, threadId.x );
    double X = hit_tileElemAtNoStride( *Xf, 1, threadId.x );
    double T = hit_tileElemAtNoStride( *Tf, 1, threadId.x );
    double R = hit_tileElemAtNoStride( *Rf, 1, 0 );
    double V = hit_tileElemAtNoStride( *Vf, 1, 0 );

    double sqrtT = sqrt(T);
    double    d1 = (log(S / X) + (R + 0.5 * V * V) * T) / (V * sqrtT);
    double    d2 = d1 - V * sqrtT;
    double CNDD1 = CND(d1);
    double CNDD2 = CND(d2);

    //Calculate Call and Put simultaneously
    double expRT = exp(- R * T);
    hit_tileElemAtNoStride( *callResult, 1, threadId.x ) = (float)(S * CNDD1 - X * expRT * CNDD2);
    hit_tileElemAtNoStride( *putResult, 1, threadId.x )  = (float)(X * expRT * (1.0 - CNDD2) - S * (1.0 - CNDD1));
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




/* GPU KERNELS */
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
CAL_KERNEL_GPU_CHAR_STATIC(BlackScholesKernel_GPU, 1, medium, low, low);
CAL_KERNEL_GPU(BlackScholesKernel_GPU, 7,
        OUT, HitTile_float*, callResult,
        OUT, HitTile_float*, putResult,
        IN,  HitTile_float*, Sf,  //Stock price
        IN,  HitTile_float*, Xf,  //Option strike
        IN,  HitTile_float*, Tf,  //Option years
        IVAL,  float, Rf,  //Riskless rate
        IVAL,  float, Vf   //Volatility rate
) {
        int pos = threadId.x;


        BlackScholesBodyGPU(
                &(hit_ktileElemAt( callResult, 1, pos )),
                &(hit_ktileElemAt( putResult, 1, pos )),
                hit_ktileElemAt( Sf, 1, pos ),
                hit_ktileElemAt( Xf, 1, pos ),
                hit_ktileElemAt( Tf, 1, pos ),
                Rf, Vf
        );
}



HitClock commClock;
HitClock seqClock;
HitClock mainClock;


/**
 MAIN PROGRAM 
*/
int main(int argc, char *argv[]) {

	setbuf(stdout, NULL);

	omp_set_nested(1);
	#pragma omp parallel num_threads(2)
	{
	#pragma omp single nowait
	{

	if ( argc<6 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <numIter> <num_cpus> <per CPU> <per GPU>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	 int OPT_N = atoi( argv[1] );
	NUM_ITERATIONS = atoi( argv[2] );
	int num_cpus = atoi( argv[3] );
	float perCPU = atof( argv[4] );
	float perGPU = atof( argv[5] );

//	printf("\n %lf %lf\n", perCPU,perGPU);

	// Needed internally by the controllers
        hit_clockReset(mainClock);
        hit_clockReset(seqClock);
        hit_clockReset(commClock);

	// 1. Part the domain in two 
	HitShape domain= hit_shapeStd1(OPT_N);
    	HitShape domains_cntrl[2];
    	hit_Cntrlpartf(domains_cntrl, domain, perCPU,perGPU);

#ifdef DEBUG    	
	dumpShape(domains_cntrl[0]);
    	dumpShape(domains_cntrl[1]);
	printf("\n Cards: %d %d\n", hit_shapeSigCard(domains_cntrl[1],0), hit_shapeSigCard(domains_cntrl[1],0)); 
#endif



	// 2. Determine the threads to launch. TODO with a layout
	CALThreadDev threads; 
	CALThreadInitDev(2, threads, domains_cntrl);

	// 3. Create the multiple-device controller
	CALMCntrl cntrlMult;
	CAL_MCntrlCreate2(cntrlMult, CAL_CNTRL_CPU, num_cpus, CAL_CNTRL_GPU, 0 );	

	// 4. Define data structures shared between several devices in a node
	CHitTile_float  
    	//Results calculated by CPU for reference
    	h_CallResult,
    	h_PutResult,
    	//CPU instance of input data
    	h_StockPrice,
    	h_OptionStrike,
    	h_OptionYears;
	
	HitTile_float 
    	// Tiles representing single constants
    	h_RiskFree,
    	h_Volatility;

	// Data structures, number of devices, type, how part the structure
	// TODO All matrix are allocated in both CPU and GPU
	hit_CtileDomainAlloc(h_CallResult, 2, float, domains_cntrl);	
	hit_CtileDomainAlloc(h_PutResult, 2, float, domains_cntrl);	
	hit_CtileDomainAlloc(h_StockPrice, 2, float, domains_cntrl);	
	hit_CtileDomainAlloc(h_OptionStrike, 2, float, domains_cntrl);	
	hit_CtileDomainAlloc(h_OptionYears, 2, float, domains_cntrl);	
	
    	HitTile foo1, foo2;
    	hit_tileSingle( &foo1, RISKFREE, sizeof(float) );
    	h_RiskFree = *(HitTile_float *)&foo1;
    	hit_tileSingle( &foo2, VOLATILITY, sizeof(float) );
    	h_Volatility = *(HitTile_float *)&foo2;
	
	
	// Initialize
        srand(5347);
	int dim0= hit_shapeSigCard(domains_cntrl[0],0);
	int dim0_GPU= hit_shapeSigCard(domains_cntrl[1],0);
    	//Generate options set
	int i; 
    	for (i = 0; i < OPT_N; i++)
    	{
		if(i<dim0   ){
        		hit_tileElemAtNoStride( h_CallResult.DS_dev[0], 1, i ) = 0.0f;
        		hit_tileElemAtNoStride( h_PutResult.DS_dev[0], 1, i )  = -1.0f;
        		hit_tileElemAtNoStride( h_StockPrice.DS_dev[0], 1, i )    = RandFloat(5.0f, 30.0f);
        		hit_tileElemAtNoStride( h_OptionStrike.DS_dev[0], 1, i )  = RandFloat(1.0f, 100.0f);
        		hit_tileElemAtNoStride( h_OptionYears.DS_dev[0], 1, i )   = RandFloat(0.25f, 10.0f);
			}
		else{

        		hit_tileElemAtNoStride( h_CallResult.DS_dev[1], 1, i-dim0 ) = 0.0f;
        		hit_tileElemAtNoStride( h_PutResult.DS_dev[1], 1, i-dim0 )  = -1.0f;
        		hit_tileElemAtNoStride( h_StockPrice.DS_dev[1], 1, i-dim0 )    = RandFloat(5.0f, 30.0f);
        		hit_tileElemAtNoStride( h_OptionStrike.DS_dev[1], 1, i-dim0 )  = RandFloat(1.0f, 100.0f);
        		hit_tileElemAtNoStride( h_OptionYears.DS_dev[1], 1, i-dim0 )   = RandFloat(0.25f, 10.0f);
		}

	}

	cudaDeviceSynchronize();
        hit_clockStart( mainClock );
	
	// 5. Attach the data structures to a multiple-device controller
	CAL_MCntrlAttach(h_CallResult, cntrlMult);
	CAL_MCntrlAttach(h_PutResult, cntrlMult);
	CAL_MCntrlAttach(h_StockPrice, cntrlMult);
	CAL_MCntrlAttach(h_OptionStrike, cntrlMult);
	CAL_MCntrlAttach(h_OptionYears, cntrlMult);
	
	//CUDA_CHECK();
        cudaDeviceSynchronize();
        hit_clockStart( seqClock );

	// 6. Invoke kernels
	// Compute
	for (i = 0; i < NUM_ITERATIONS; i++){

		CAL_MCntrlLaunch(cntrlMult, threads,0, BlackScholesKernel, 7, &hit_Ctile(h_CallResult,0), &hit_Ctile(h_PutResult,0), &hit_Ctile(h_StockPrice,0), &hit_Ctile(h_OptionStrike,0), &hit_Ctile(h_OptionYears,0), &(h_RiskFree), &(h_Volatility) );

		CAL_MCntrlLaunch(cntrlMult, threads,1, BlackScholesKernel_GPU, 7, &hit_Ctile(h_CallResult,1), &hit_Ctile(h_PutResult,1), &hit_Ctile(h_StockPrice,1), &hit_Ctile(h_OptionStrike,1), &hit_Ctile(h_OptionYears,1), RISKFREE, VOLATILITY );
	}	

        CAL_CntrlDestroy(&cntrlMult.controller[0]);

        cudaDeviceSynchronize();
        hit_clockStop( seqClock );


	// 7. Copy result from device memory to host memory
	CAL_MCntrlDetach(h_CallResult, cntrlMult);
	CAL_MCntrlDetach(h_PutResult, cntrlMult);
	CAL_MCntrlDetach(h_StockPrice, cntrlMult);
	CAL_MCntrlDetach(h_OptionStrike, cntrlMult);
	CAL_MCntrlDetach(h_OptionYears, cntrlMult);

	// 8. Destroy multiple-device controller	
        CAL_CntrlDestroy(&cntrlMult.controller[1]);

        cudaDeviceSynchronize();
        hit_clockStop( mainClock );

	
	//CAL_MDestroy2(cntrlMult);
	printf("\nBlackScholes OPT_N: %d \n", OPT_N );
        printf("\n%d-CPU: %f%\tGPU %f%\n", num_cpus, perCPU, perGPU );
        printf("Clock main: %.8lf\n", mainClock.seconds);
        printf("Clock comm: %.8lf\n", commClock.seconds);
        printf("Clock seq: %.8lf\n", seqClock.seconds);


#ifndef NDEBUG
	cudaDeviceSynchronize();
	printf("\n Checking\n");


	// Calculate NORM
    	double resultado=0,suma=0;
        for (int i=0; i<dim0; i++){
		suma+= pow( 0.1*(hit_tileElemAt((h_CallResult.DS_dev[0]), 1, i) + hit_tileElemAt((h_PutResult.DS_dev[0]), 1, i)), 2 );
	}
        for (int i=dim0; i<OPT_N; i++){
		suma+= pow( 0.1*(hit_tileElemAt((h_CallResult.DS_dev[1]), 1, i-dim0) + hit_tileElemAt((h_PutResult.DS_dev[1]), 1, i-dim0) ), 2 );
	}
	
    	printf("\n ----------------------- NORM ----------------------- \n");
    	printf("\n Acumulated sum: %lf",suma);

    	resultado=sqrt( suma );
    	printf("\n Result: %lf \n",resultado);

    	printf("\n ---------------------------------------------------- \n");


#endif
	
	// 9. Free resources
	hit_CtileFree(h_CallResult, cntrlMult);
	hit_CtileFree(h_PutResult, cntrlMult);
	hit_CtileFree(h_StockPrice, cntrlMult);
	hit_CtileFree(h_OptionStrike, cntrlMult);
	hit_CtileFree(h_OptionYears, cntrlMult);
	

}}

	return 0;
}
