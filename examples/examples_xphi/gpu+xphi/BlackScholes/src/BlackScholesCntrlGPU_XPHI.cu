//#define _BSD_SOURCE
#include <omp.h>
#include <hitmap.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "kernel_xphi.h"
#include "CAL_KernelGPU.h"
#include "CAL_KernelXPHI.h"


double  mainClock;
double  initClock;
double  sequentialClock;

float      RISKFREE = 0.02f;
float    VOLATILITY = 0.30f;





////////////////////////////////////////////////////////////////////////////////
// Helper function, returning uniformly distributed
// random float in [low, high] range
////////////////////////////////////////////////////////////////////////////////
float RandFloat(float low, float high)
{
    float t = (float)rand() / (float)RAND_MAX;
    return (1.0f - t) * low + t * high;
}




double t_start, t_end;

int main(int argc, char *argv[])
{

    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
    #endif

//    CAL_CntrlInit(3);

    omp_set_nested(1);
    #pragma omp parallel num_threads(3)
    {
    #pragma omp single nowait
    {

    if ( argc < 5 ) {
                fprintf(stderr, "\nUsage: %s <numRows> <iter> <perGPU> <perXPHI> \n", argv[0]);
                exit(EXIT_FAILURE);
    }
    int rows = atoi( argv[1] );
    int OPT_N=rows;
    int NIter = atoi( argv[2] );
    int columns = rows;
    float perGPU = atof( argv[3] );
    float perXPHI = atof( argv[4] );

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

    
    /* 1. DIVIDE COMPUTATION */
    HitShape Xphi_shape=hit_shapeStd1(rows); //HIT_SHAPE_NULL;
    HitShape GPU_shape=hit_shapeStd1(rows); //HIT_SHAPE_NULL;
    HitLayout weightLay;


    /* 2. DECLARE  MATRIX  */
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

    hit_tileDomainShapeAlloc( &h_CallResultXPHI, float, Xphi_shape);
    hit_tileDomainShapeAlloc( &h_PutResultXPHI,  float, Xphi_shape);
    hit_tileDomainShapeAlloc( &h_StockPrice,    float, Xphi_shape);
    hit_tileDomainShapeAlloc( &h_OptionStrike,  float, Xphi_shape);
    hit_tileDomainShapeAlloc( &h_OptionYears,   float, Xphi_shape);

    
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


    for (i = 0; i < NIter; i++)
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


        // Calculate NORM
    double resultado=0,suma=0;
    for (int i=0; i<OPT_N; i++){
                suma+= pow( (hit_tileElemAt((h_CallResultXPHI), 1, i) + hit_tileElemAt((h_PutResultXPHI), 1, i)), 2 );
    }

    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum XPHI: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result XPHI: %lf \n",resultado);
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

  return 0;
}
