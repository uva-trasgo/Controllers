/*
 * Code adapted to compile for CPU only in C99 
 * 	by Arturo Gonzalez-Escribano, Aug 2016
 *
 * Copyright 1993-2015 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 */

/*
 * This sample evaluates fair call and put prices for a
 * given set of European options by Black-Scholes formula.
 * See supplied whitepaper for more explanations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

////////////////////////////////////////////////////////////////////////////////
// Data configuration
////////////////////////////////////////////////////////////////////////////////
const int  NUM_ITERATIONS = 512;


const float      RISKFREE = 0.02f;
const float    VOLATILITY = 0.30f;

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
// Black-Scholes formula for both call and put
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
int main(int argc, char **argv)
{

    if ( argc != 3 ) {
       fprintf(stderr, "\nUsage: %s <numOpts> <numThreads>\n", argv[0]);
       exit(EXIT_FAILURE);
    }
    int OPT_N = atoi( argv[1] );
    int OPT_SZ = OPT_N * sizeof(float);
    int num_threads=atoi( argv[2]);
    omp_set_num_threads( num_threads );

    // Start logs
    //printf("[%s] - Starting...\n", argv[0]);

    //'h_' prefix - CPU (host) memory space
    float
    //Results calculated by CPU for reference
    *h_CallResultCPU,
    *h_PutResultCPU,
    //CPU instance of input data
    *h_StockPrice,
    *h_OptionStrike,
    *h_OptionYears;

    int i;
    // Arturo: Timers
    double mainClock;
    double totalClock;
    totalClock = omp_get_wtime();

    //printf("Initializing data...\n");
    //printf("...allocating CPU memory for options.\n");
    h_CallResultCPU = (float *)malloc(OPT_SZ);
    h_PutResultCPU  = (float *)malloc(OPT_SZ);
    h_StockPrice    = (float *)malloc(OPT_SZ);
    h_OptionStrike  = (float *)malloc(OPT_SZ);
    h_OptionYears   = (float *)malloc(OPT_SZ);

    //printf("...generating input data in CPU mem.\n");
    srand(5347);

    //Generate options set
    for (i = 0; i < OPT_N; i++)
    {
        h_CallResultCPU[i] = 0.0f;
        h_PutResultCPU[i]  = -1.0f;
        h_StockPrice[i]    = RandFloat(5.0f, 30.0f);
        h_OptionStrike[i]  = RandFloat(1.0f, 100.0f);
        h_OptionYears[i]   = RandFloat(0.25f, 10.0f);
    }

    //printf("Executing Black-Scholes CPU kernel (%i iterations)...\n", NUM_ITERATIONS);

    //printf("...running CPU calculations.\n\n");
    //Calculate options values on CPU
    
    mainClock = omp_get_wtime();

    for (i = 0; i < NUM_ITERATIONS; i++) {
	int opt;
        #pragma omp parallel for
        for ( opt = 0; opt < OPT_N; opt++)
            BlackScholesBodyCPU(
                &h_CallResultCPU[opt],
                &h_PutResultCPU[opt],
                h_StockPrice[opt],
                h_OptionStrike[opt],
                h_OptionYears[opt],
                RISKFREE,
                VOLATILITY
            );
    }

    mainClock = omp_get_wtime() - mainClock;
    totalClock = omp_get_wtime() - totalClock;

    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", totalClock );
    printf("Clock seq: %lf\n", mainClock );

        // Calculate NORM
        double resultado=0,suma=0;
        for (int i=0; i<OPT_N; i++){
                suma+= pow( (h_CallResultCPU[i] + h_PutResultCPU[i]), 2 );
        }

        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");

    //printf("...releasing CPU memory.\n");
    free(h_OptionYears);
    free(h_OptionStrike);
    free(h_StockPrice);
    free(h_PutResultCPU);
    free(h_CallResultCPU);
    //printf("Shutdown done.\n");

    //printf("\n[BlackScholes] - Test Summary\n");

    exit(EXIT_SUCCESS);
}
