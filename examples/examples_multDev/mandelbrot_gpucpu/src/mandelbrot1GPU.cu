#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"
#include "CAL_KernelGPU.h"
#include "CAL_KernelCPU.h"
#include "CAL_CntrlDev.h"

Timer tInner;


CAL_KERNEL_GPU_CHAR_STATIC(Mandelbrot, 2, full, low, low);
CAL_KERNEL_GPU(Mandelbrot, 8, OUT, HitTile_float*, resultx, OUT, HitTile_float*, resulty, IVAL, int, xsize,  
		IVAL, int, ysize, IVAL, float, begin_x, IVAL, float, begin_y, IVAL, float, end_x, IVAL, float, end_y ){
/* Version KTiles */
        //int i = blockDim.y * blockIdx.y + threadIdx.y;
        //int j = blockDim.x * blockIdx.x + threadIdx.x;
	int i = threadId.y;
	int j = threadId.x;
	int maxiter=20000;

        if ( i >= xsize || j >= ysize ) return;

        float x0 = begin_x + (end_x - begin_x)/xsize * i;
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
#endif

        hit_ktileElemAt(resultx, 2, i, j) = x;
        hit_ktileElemAt(resulty, 2, i, j) = y;

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

	// Needed internally by the controllers

        hit_clockReset(mainClock);
        hit_clockReset(seqClock);
        hit_clockReset(commClock);


        cudaDeviceSynchronize();
        hit_clockStart( mainClock );
	
	// 1. Part the domain in two 
	HitShape domain= hit_shapeStd2(xsize, ysize);
    	HitShape domains_cntrl[1];
    	//hit_Cntrlpart(domains_cntrl, domain, perCPU,perGPU);
	domains_cntrl[0]=domain;

#ifdef DEBUG    	
	dumpShape(domains_cntrl[0]);
#endif



	// 2. Determine the threads to launch. TODO with a layout
	CALThreadDev threads; 
	CALThreadInitDev(1,threads, domains_cntrl);
	//CALThreadReverse2(threads, 1 );

	// 3. Create the multiple-device controller
	CALMCntrl cntrlMult;
	CAL_MCntrlCreate1(cntrlMult, CAL_CNTRL_GPU, 0 );	

	// 4. Define data structures shared between several devices in a node
	CHitTile_float resultx, resulty; 
	// Data structures, number of devices, type, how part the structure
	hit_CtileDomainAlloc(resultx, 1, float, domains_cntrl);	
	hit_CtileDomainAlloc(resulty, 1, float, domains_cntrl);	
	
	//CUDA_CHECK();

	// 5. Attach the data structures to a multiple-device controller
	CAL_MCntrlAttach(resultx, cntrlMult);
	CAL_MCntrlAttach(resulty, cntrlMult);


	cudaDeviceSynchronize();
        hit_clockStart( seqClock );

	// Compute
        CAL_MCntrlLaunch(cntrlMult, threads,0, Mandelbrot, 8, &hit_Ctile(resultx,0), &hit_Ctile(resulty,0), xsize,ysize, begin_x, begin_y, end_x, end_y);

	CAL_CntrlDestroy(&cntrlMult.controller[0]);

        cudaDeviceSynchronize();
        hit_clockStop( seqClock );

	// 7. Copy result from device memory to host memory
	CAL_MCntrlDetach(resultx, cntrlMult);
	CAL_MCntrlDetach(resulty, cntrlMult);
	        

        cudaDeviceSynchronize();
        hit_clockStop( mainClock );


	printf("\nMandelbrot\n" );
        printf("Clock main: %.8lf\n", mainClock.seconds);
        printf("Clock comm: %.8lf\n", commClock.seconds);
        printf("Clock seq: %.8lf\n", seqClock.seconds);


        double resultado=0,suma=0;
	int i,j ;
        for ( i=0; i<xsize; i++ ) {
          for ( j=0; j<ysize; j++ ) {
              suma += pow( (hit_tileElemAt(resultx.DS_dev[0], 2, i, j) + hit_tileElemAt(resulty.DS_dev[0], 2, i, j)), 2 );
            }
        }
        printf("\n ----------------------- NORM ----------------------- \n");
        printf("\n Acumulated sum: %lf",suma);

        resultado=sqrt( suma );
        printf("\n Result: %lf \n",resultado);

        printf("\n ---------------------------------------------------- \n");
	// 9. Free resources
	hit_CtileFree(resultx, cntrlMult);
	hit_CtileFree(resulty, cntrlMult);	

}}

	return 0;
}
