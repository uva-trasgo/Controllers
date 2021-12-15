#define _BSD_SOURCE
#include <omp.h>
#include <hitmap.h>
#include <math.h>
//#include <mkl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "CAL.h"
#include "CAL_CntrlCPU.h"
#include "CAL_KernelCPU.h"
//#include "CAL_AttachXPHI.h"
//#include "../../../cal/xphi/CAL_AttachXPHI.h"

hit_tileNewType( float );
hit_tileNewType( int );


double  mainClock;
double  initClock;
double  sequentialClock;
/*
CAL_KERNEL_XPHI(Mandelbrot2, 8, OUT, HitTile_float*, resultx,  OUT, HitTile_float*, resulty, IVAL, float, begin_x, IVAL , float, begin_y, IVAL, float, end_x, IVAL, float, end_y,IVAL,float, xsize,IVAL,float, ysize){

//	int sizex = (int)hit_tileElemAtNoStride( xsize, 1, 0 );
//	int sizey = (int)hit_tileElemAtNoStride( ysize, 1, 0 );

      float x0 =  begin_x + (end_x -  begin_x)/xsize * threadId.x;
     
      float y0 =  begin_y+ ( end_y - begin_y)/ysize * threadId.y;
      
       float x = 0;
      float y = 0;
      int maxiter2=2000;
      int iteration = 0;
      while (x*x + y*y < 2*2 && iteration < maxiter2 ) {
             float xtemp = x*x - y*y + x0;
             y = 2*x*y + y0;
             x = xtemp;
            iteration = iteration + 1;
       }
       hit_tileElemAt(resultx,2, threadId.x, threadId.y) = x;
       hit_tileElemAt(resulty,2, threadId.x, threadId.y) = y;

}
*/
CAL_KERNEL_CPU(Mandelbrot, 8, IN, HitTile_float*, begin_x, IN,HitTile_float*, begin_y, IN, HitTile_float*, end_x, IN,HitTile_float*, end_y,IN,HitTile_float*, xsize,IN,HitTile_float*, ysize,  OUT, HitTile_float*, resultx,  OUT, HitTile_float*, resulty){

//	int sizex = (int)hit_tileElemAtNoStride( xsize, 1, 0 );
//	int sizey = (int)hit_tileElemAtNoStride( ysize, 1, 0 );

      float x0 = hit_tileElemAtNoStride( begin_x, 1, 0 )+ ( hit_tileElemAtNoStride( end_x, 1, 0 ) - hit_tileElemAtNoStride( begin_x, 1, 0 ))/hit_tileElemAtNoStride( xsize, 1, 0 ) * threadId.x;
      float y0 = hit_tileElemAtNoStride( begin_y, 1, 0 )+ ( hit_tileElemAtNoStride( end_y, 1, 0 ) - hit_tileElemAtNoStride( begin_y, 1, 0 ))/hit_tileElemAtNoStride( ysize, 1, 0 ) * threadId.y;
      
       float x = 0;
      float y = 0;
      int maxiter2=2000;
      int iteration = 0;
      while (x*x + y*y < 2*2 && iteration < maxiter2 ) {
             float xtemp = x*x - y*y + x0;
             y = 2*x*y + y0;
             x = xtemp;
            iteration = iteration + 1;
       }
       hit_tileElemAt(resultx,2, threadId.x, threadId.y) = x;
       hit_tileElemAt(resulty,2, threadId.x, threadId.y) = y;

}


double t_start, t_end;

int main(int argc, char *argv[])
{

//    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
  //  #endif

    CAL_CntrlInit(1);
           if ( argc != 9 ) {
                fprintf(stderr, "Usage: %s <maxiter> <xsize> <ysize> <begin_x> <end_x> <begin_y> <end_y> <numThreads>\n", argv[0]);
                exit (EXIT_FAILURE);
        }

        int maxiter = atoi( argv[1] );
        int xsize = atoi( argv[2] );
        int ysize = atoi( argv[3] );
        float begin_x = (float)atof( argv[4] );
        float end_x = (float)atof( argv[5] );
        float begin_y = (float)atof( argv[6] );
        float end_y = (float)atof( argv[7] );
        int numThreads = atoi( argv[8] );

  
//  int nIter = atoi( argv[2] );
//    int num_threads=atoi( argv[3]);

    int i, j, k;

    
    /* 2. DECLARE FULL MATRIX AND COPY */
    HitTile_float resultx, resulty;
    hit_tileDomain( &resultx, float, 2, xsize, ysize );
    hit_tileDomain( &resulty, float, 2, xsize, ysize );

    hit_tileAlloc( &resultx );
    hit_tileAlloc( &resulty );

    CALCntrl comm;
    CALThread threads;

    CALThreadInit(threads, 2, xsize, ysize);
    HitTile_float bx,by,ex,ey; 
    HitTile foo1, foo2, foo3, foo4, foo5, foo6; 
    
    hit_tileSingle( &foo1, begin_x, sizeof(float) );
    bx = *(HitTile_float *)&foo1;
    hit_tileSingle( &foo2, begin_y, sizeof(float) );
    by = *(HitTile_float *)&foo2;
    hit_tileSingle( &foo3, end_x, sizeof(float) );
    ex = *(HitTile_float *)&foo3;
    hit_tileSingle( &foo4, end_y, sizeof(float) );
    ey = *(HitTile_float *)&foo4;

    HitTile_float sx,sy;
    float sizex=(float) xsize;
    float sizey=(float) ysize;
    hit_tileSingle( &foo5, sizex, sizeof(float) );
    sx = *(HitTile_float *)&foo5;
    hit_tileSingle( &foo6, sizey, sizeof(float) );
    sy = *(HitTile_float *)&foo6;


    /* CREATE CNTRLS */
    mainClock = omp_get_wtime();
    
    CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
    CAL_CntrlAttach(&comm, (HitTile*)&resultx);
    CAL_CntrlAttach(&comm, (HitTile*)&resulty);
    CAL_CntrlAttach(&comm, (HitTile*)&bx);
    CAL_CntrlAttach(&comm, (HitTile*)&by);
    CAL_CntrlAttach(&comm, (HitTile*)&ex);
    CAL_CntrlAttach(&comm, (HitTile*)&ey);
    CAL_CntrlAttach(&comm, (HitTile*)&sx);
    CAL_CntrlAttach(&comm, (HitTile*)&sy);
  
    CAL_CntrlLaunch(comm, Mandelbrot, threads, 8, &bx,&by,&ex,&ey,&sx,&sy, &resultx, &resulty);	
   // CAL_CntrlLaunch(comm, Mandelbrot2, threads, 8,&resultx, &resulty, begin_x,begin_y,end_x,end_y,sizex,sizey);	

    CAL_CntrlDetach(&comm, (HitTile*)&resultx);
    CAL_CntrlDetach(&comm, (HitTile*)&resulty);

    CAL_CntrlDestroy(&comm);


    /* 4. CLOCK RESULTS */
    mainClock = omp_get_wtime() - mainClock;

    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );



    double resultado=0,suma=0;
    for ( i=0; i<xsize; i++ ) {
         for ( j=0; j<ysize; j++ ) {
              suma += pow(  hit_tileElemAt( resultx, 2, i, j )+ hit_tileElemAt( resulty, 2, i, j )  ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");

    hit_tileFree(resultx);
    hit_tileFree(resulty);

    CAL_CntrlFinish();  

  return 0;
}
