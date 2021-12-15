#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"
#include "CAL_KernelGPU.h"
#include "CAL_KernelCPU.h"
#include "CAL_CntrlDev.h"

/* TODO CPU and GPU precision are not the same, problem when the number of iterations is high */



Timer tInner;
 
void
dumpShape (HitShape sh)
{

  int i;
  printf ("[%d:%d:%d", hit_shapeSig (sh, 0).begin, hit_shapeSig (sh, 0).end,
          hit_shapeSig (sh, 0).stride);
  for (i = 1; i < hit_shapeDims (sh); i++)
    printf (",%d:%d:%d", hit_shapeSig (sh, i).begin, hit_shapeSig (sh, i).end,
            hit_shapeSig (sh, i).stride);
  printf ("] \t cards: [%d", hit_sigCard (hit_shapeSig (sh, 0)));
  for (i = 1; i < hit_shapeDims (sh); i++)
    printf (",%d", hit_sigCard (hit_shapeSig (sh, i)));
  printf ("]\n");
}

CAL_KERNEL_GPU_CHAR_STATIC(Mandelbrot, 2, square32, low, low);
CAL_KERNEL_GPU(Mandelbrot, 8, OUT, HitTile_float*, resultx,  IVAL, int, xsize,  
		IVAL, int, ysize, IVAL, float, begin_x, IVAL, float, begin_y, IVAL, float, end_x, IVAL, float, end_y, IVAL, int, offset ){
/* Version KTiles */
        //int i = blockDim.y * blockIdx.y + threadIdx.y;
        //int j = blockDim.x * blockIdx.x + threadIdx.x;
	int i = threadId.y;
	int j = threadId.x;
	int maxiter=60000;

        if ( i >= xsize || j >= ysize ) return;

        float x0 = begin_x + (end_x - begin_x)/xsize * (i+offset);
        float y0 = begin_y + (end_y - begin_y)/ysize * j;
        float x = 0;
        float y = 0;

#ifdef DEBUG
        if ( i == 0 && j == 0 )
                printf("[0,0] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
        if ( i == 31 && j == 8 )
                printf("[31,8] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
        if ( i == 39 && j == 0 )
                printf("[39,0] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
        if ( i == 0 && j == 39 )
                printf("[0,39] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
        if ( i == 38 && j == 2 )
                printf("[38,2] Valores: (%20.18lf , %20.18lf)\n", x0, y0 );
#endif

        int iteration = 0;
        while (x*x + y*y < 2*2 && iteration < maxiter ) {
                float xtemp = x*x - y*y + x0;
                y = 2*x*y + y0;
                x = xtemp;
                iteration = iteration + 1;
        }

#ifdef DEBUG
        if ( i == 0 && j == 0 )
                printf("[0,0] Valores: (%20.18lf , %20.18lf)\n", x, y );
        if ( i == 31 && j == 8 )
                printf("[31,8] Valores: (%20.18lf , %20.18lf)\n", x, y );
        if ( i == 39 && j == 0 )
                printf("[39,0] Valores: (%20.18lf , %20.18lf)\n", x, y );
        if ( i == 0 && j == 39 )
                printf("[0,39] Valores: (%20.18lf , %20.18lf)\n", x, y );
        if ( i == 38 && j == 2 )
                printf("[38,2] Valores: (%20.18lf , %20.18lf)\n", x, y );
	
	printf("%d, %d -- %lf  %lf \n", i+offset, j , x, y);
#endif
        hit_ktileElemAt(resultx, 2, i, j) = (float)iteration;

}

CAL_KERNEL_CPU(Mandelbrot_cpu, 8, OUT, HitTile_float*, resultx, IVAL, int*, xsize,
                IVAL, int*, ysize, IVAL,  float*, begin_x, IVAL,  float*, begin_y, IVAL, float*, end_x, IVAL, float*, end_y, IVAL, int*, offset ){
 //    int sizex = (int)hit_tileElemAtNoStride( *xsize, 1, 0 );
 //	printf("XSIZE %d \n", *xsize);
//      int sizey = (int)hit_tileElemAtNoStride( ysize, 1, 0 );


      float x0 =  *begin_x + ( *end_x - *begin_x)/(* xsize ) * (threadId.x + (*offset) );

      float y0 = *begin_y + ( *end_y - *begin_y)/( *ysize ) * threadId.y;

       float x = 0;
      float y = 0;
      int maxiter2=60000;
      int iteration = 0;
      while (x*x + y*y < 2*2 && iteration < maxiter2 ) {
             float xtemp = x*x - y*y + x0;
             y = 2*x*y + y0;
             x = xtemp;
            iteration = iteration + 1;
       }
//	printf("%d, %d -- %lf  %lf \n", threadId.x+ *offset, threadId.y , x, y);
	
       hit_tileElemAt(*resultx,2, threadId.x, threadId.y) = (float)iteration;
	
}


HitClock commClock;
HitClock mainClock;
HitClock seqClock;

/**
 MAIN PROGRAM 
*/
int main(int argc, char *argv[]) {
 setbuf(stdout, NULL);

omp_set_nested(1);
  #pragma omp parallel num_threads(3)
{
#pragma omp single nowait
{

	if ( argc < 9 ) {
                fprintf(stderr, "Usage: %s <maxiter> <xsize> <ysize> <begin_x> <end_x> <begin_y> <end_y> <perCPU> <perGPU>\n", argv[0]);
                exit (EXIT_FAILURE);
        }

        int maxiter = atoi( argv[1] );
        int xsize = atoi( argv[2] );
        int ysize = atoi( argv[3] );
        float begin_x = (float)atof( argv[4] );
        float end_x = (float)atof( argv[5] );
        float begin_y = (float)atof( argv[6] );
        float end_y = (float)atof( argv[7] );
        float perCPU = (float) atof( argv[8] );
        float perGPU = (float)atof( argv[9] );

	// Needed internally by the controllers

        hit_clockReset(mainClock);
        hit_clockReset(seqClock);
        hit_clockReset(commClock);



	cudaSetDevice(0);
        cudaDeviceSynchronize();
	hit_clockStart( mainClock );
	
	// 1. Part the domain in two 
	HitShape domain= hit_shapeStd2(xsize, ysize);
	HitShape domains_cntrl[2];
        hit_Cntrlpartf(domains_cntrl, domain, perCPU,perGPU);

#ifdef DEBUG            
        dumpShape(domains_cntrl[0]);
        dumpShape(domains_cntrl[1]);
#endif

        // 2. Determine the threads to launch. TODO with a layout
        CALThreadDev threads;
	int b1 = hit_shapeSigCard(domains_cntrl[0],0); 
	int zero =0; 
        CALThreadInitDev(2,threads, domains_cntrl);
        CALThreadReverse2(threads, 1 );
        //CALThreadReverse2(threads, 0 );

	// 3. Create the multiple-device controller
	CALMCntrl cntrlMult;
	CAL_MCntrlCreate2(cntrlMult, CAL_CNTRL_CPU, 10,  CAL_CNTRL_GPU, 0 );	

	// 4. Define data structures shared between several devices in a node
	CHitTile_float resultx; 
	// Data structures, number of devices, type, how part the structure
	hit_CtileDomainAlloc(resultx, 2, float, domains_cntrl);	

		
	//CUDA_CHECK();
	// 5. Attach the data structures to a multiple-device controller

	CAL_MCntrlAttach(resultx, cntrlMult);

        cudaSetDevice(0);
        cudaDeviceSynchronize();
        hit_clockStart( seqClock );

	// Compute
        CAL_MCntrlLaunch(cntrlMult, threads,0, Mandelbrot_cpu, 8, &hit_Ctile(resultx,0), &xsize, &ysize, &begin_x, &begin_y, &end_x, &end_y, &zero);
        CAL_MCntrlLaunch(cntrlMult, threads,1, Mandelbrot, 8, &hit_Ctile(resultx,1), xsize,ysize, begin_x, begin_y, end_x, end_y, b1 );

	CAL_CntrlDestroy(&cntrlMult.controller[0]);

        cudaSetDevice(0);
        cudaDeviceSynchronize();
	hit_clockStop( seqClock );

	// 7. Copy result from device memory to host memory
	CAL_MCntrlDetach(resultx, cntrlMult);
	        
	// 8. Destroy multiple-device controller        
	CAL_CntrlDestroy(&cntrlMult.controller[1]);

        cudaSetDevice(0);
        cudaDeviceSynchronize();
	hit_clockStop( mainClock );


	printf("\nMandelbrot\n" );
	printf("\nCPU-0: %lf%\tGPU-1: %lf%\n", perCPU, perGPU );
        printf("Clock main: %.8lf\n", mainClock.seconds);
        printf("Clock comm: %.8lf\n", commClock.seconds);
        printf("Clock seq: %.8lf\n", seqClock.seconds);


        double resultado=0,suma=0;
	int i,j ;
	int dim_0_CPU= hit_shapeSigCard(domains_cntrl[0],0);
        int dim_1_CPU= hit_shapeSigCard(domains_cntrl[0],1);
	int dim_0= hit_shapeSigCard(domains_cntrl[1],0);
        int dim_1= hit_shapeSigCard(domains_cntrl[1],1);

        for ( i=0; i<dim_0_CPU; i++ ) {
          for ( j=0; j<dim_1_CPU; j++ ) {
              suma += pow( (hit_tileElemAt(resultx.DS_dev[0], 2, i, j) ), 2 );
            }
        }
        for ( i=0; i<dim_0; i++ ) {
          for ( j=0; j<dim_1; j++ ) {
              suma += pow( (hit_tileElemAt(resultx.DS_dev[1], 2, i, j) ), 2 );
            }
        }
        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");
	// 9. Free resources
	hit_CtileFree(resultx, cntrlMult);

}}

	return 0;
}
