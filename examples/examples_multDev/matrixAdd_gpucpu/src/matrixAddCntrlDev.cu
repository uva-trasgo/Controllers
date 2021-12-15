#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"
#include "CAL_KernelGPU.h"
#include "CAL_KernelCPU.h"
#include "CAL_CntrlDev.h"



//hit_tileNewType( float );



/* GPU KERNELS */

/*CAL_KERNEL_GPU_CHAR_STATIC(Init_GPU, 2, full, low, low);
CAL_KERNEL_GPU(Init_GPU, 3, OUT, HitTile_float*, A, OUT, HitTile_float*, B, IO, HitTile_float*, C){
        int i = threadId.x;
        int j = threadId.y;
        hit_ktileElemAt(A, 2, i, j)=2.12;
        hit_ktileElemAt(B, 2, i, j)=0.2;
        hit_ktileElemAt(C, 2, i, j)=0;
}
*/


CAL_KERNEL_GPU_CHAR_STATIC(MatAdd, 2, full, low, low);
CAL_KERNEL_GPU(MatAdd, 4, IN, HitTile_float*, A, IN, HitTile_float*, B, IO, HitTile_float*, C, IVAL, int, NITER){
/* Version KTiles */
/* Same best performance using ktile notation. Register indexes i,j obtain the last littel
	performance improvement, beyond the reference code */
	int i = threadId.y;
	int j = threadId.x;
	for (int iter=0; iter<NITER; iter++) 
		hit_ktileElemAt(C, 2, i, j) = 
			hit_ktileElemAt(C, 2, i, j) +
			hit_ktileElemAt(A, 2, i, j) + 
			hit_ktileElemAt(B, 2, i, j);
}




/* CPU KERNELS */
CAL_KERNEL_CPU(MatAdd_CPU, 4, IN, HitTile_float*, A, IN, HitTile_float*, B, OUT, HitTile_float*, C, IN, int*, niter){
  int k;
  
  for(k=0;k< *niter;k++){
        hit_tileElemAt(*C, 2, threadId.x, threadId.y) = hit_tileElemAt(*C, 2, threadId.x, threadId.y) + hit_tileElemAt(*A, 2, threadId.x, threadId.y) + hit_tileElemAt(*B, 2, threadId.x, threadId.y) ;
  }
}
/*
CAL_KERNEL_CPU(Init_CPU, 3, OUT, HitTile_float*, A, OUT, HitTile_float*, B, OUT, HitTile_float*, C){
  int k;
  hit_tileElemAt(*A, 2, threadId.x, threadId.y)=2.12;
  hit_tileElemAt(*B, 2, threadId.x, threadId.y)=0.2;
  hit_tileElemAt(*C, 2, threadId.x, threadId.y)=0;
  
}
*/


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

	if ( argc!=6 ) {
		fprintf(stderr, "Usage: %s <matrixSize> <numIter> <num_cpus> <per CPU> <per GPU>\n\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int SIZE = atoi( argv[1] );
	int NITER = atoi( argv[2] );
	int num_cpus = atoi( argv[3] );
	float perCPU = atof( argv[4] );
	float perGPU = atof( argv[5] );

        hit_clockReset(mainClock);
        hit_clockReset(seqClock);
        hit_clockReset(commClock);


	// 1. Part the domain in two 
	HitShape domain= hit_shapeStd2(SIZE, SIZE);
    	HitShape domains_cntrl[2];
    	hit_Cntrlpartf(domains_cntrl, domain, perCPU,perGPU);

#ifdef DEBUG    	
	dumpShape(domains_cntrl[0]);
    	dumpShape(domains_cntrl[1]);
	printf("\n Cards: %d %d\n", hit_shapeSigCard(domains_cntrl[1],0), hit_shapeSigCard(domains_cntrl[1],1)); 
#endif



	// 2. Determine the threads to launch. TODO with a layout
	CALThreadDev threads; 
	CALThreadInitDev(2,threads, domains_cntrl);
	CALThreadReverse2(threads, 1 );

	// 3. Create the multiple-device controller
	CALMCntrl cntrlMult;
	CAL_MCntrlCreate2(cntrlMult, CAL_CNTRL_CPU, num_cpus, CAL_CNTRL_GPU, 0 );	

	// 4. Define data structures shared between several devices in a node
	CHitTile_float A, B, C; 
	// Data structures, number of devices, type, how part the structure
	hit_CtileDomainAlloc(A, 2, float, domains_cntrl);	
	hit_CtileDomainAlloc(B, 2, float, domains_cntrl);	
	hit_CtileDomainAlloc(C, 2, float, domains_cntrl);	
	
	
	//CUDA_CHECK();

	// 6. Invoke kernels
	// Initialize
	//CAL_MCntrlLaunch(cntrlMult, threads,0, Init_CPU, 3, &(A.DS_dev[0]), &(B.DS_dev[0]), &(C.DS_dev[0])); 
	//CAL_MCntrlLaunch(cntrlMult, threads,1, Init_GPU, 3, &(A.DS_dev[1]), &(B.DS_dev[1]), &(C.DS_dev[1])); 
	int dim_0= hit_shapeSigCard(domains_cntrl[1],0);
	int dim_1= hit_shapeSigCard(domains_cntrl[1],1);

	int dim_0_CPU= hit_shapeSigCard(domains_cntrl[0],0);
	int dim_1_CPU= hit_shapeSigCard(domains_cntrl[0],1);
        for (int i=0; i<dim_0; i++){
                for (int j=0; j<dim_1; j++) {
                     hit_tileElemAt(A.DS_dev[1], 2, i, j) = 0.12;
                     hit_tileElemAt(B.DS_dev[1], 2, i, j) = 0.2;
                     hit_tileElemAt(C.DS_dev[1], 2, i, j) = 0;
		}
        }

        for (int i=0; i<dim_0_CPU; i++){
                for (int j=0; j<dim_1_CPU; j++) {
                     hit_tileElemAt(A.DS_dev[0], 2, i, j) = 0.12;
                     hit_tileElemAt(C.DS_dev[0], 2, i, j) = 0;
                     hit_tileElemAt(B.DS_dev[0], 2, i, j) = 0.2;
		}
	}

	cudaDeviceSynchronize();
        hit_clockStart( mainClock );
	
	// 5. Attach the data structures to a multiple-device controller
	CAL_MCntrlAttach(A, cntrlMult);
	CAL_MCntrlAttach(B, cntrlMult);
	CAL_MCntrlAttach(C, cntrlMult);

	cudaDeviceSynchronize();
        hit_clockStart( seqClock );
	// Compute
        CAL_MCntrlLaunch(cntrlMult, threads,0, MatAdd_CPU, 4, &hit_Ctile(A,0), &hit_Ctile(B,0), &hit_Ctile(C,0), &NITER);
        CAL_MCntrlLaunch(cntrlMult, threads,1, MatAdd, 4, &hit_Ctile(A,1), &hit_Ctile(B,1), &hit_Ctile(C,1), NITER);
	
	CAL_CntrlDestroy(&cntrlMult.controller[0]);

	cudaDeviceSynchronize();
        hit_clockStop( seqClock );
	// 7. Copy result from device memory to host memory

	CAL_MCntrlDetach(A, cntrlMult);
	CAL_MCntrlDetach(B, cntrlMult);
	CAL_MCntrlDetach(C, cntrlMult);

	// 8. Destroy multiple-device controller	
	//CAL_MDestroy2(cntrlMult);
	CAL_CntrlDestroy(&cntrlMult.controller[1]);

	cudaDeviceSynchronize();
        hit_clockStop( mainClock );

	printf("\nmatrixAdd SIZE: %d %d\n", SIZE, NITER );
	printf("\n%d-CPU: %f%\tGPU %f%\n", num_cpus, perCPU, perGPU );
        printf("Clock main: %.8lf\n", mainClock.seconds);
        printf("Clock comm: %.8lf\n", commClock.seconds);
        printf("Clock seq: %.8lf\n", seqClock.seconds);

#ifndef NDEBUG
	cudaDeviceSynchronize();
	printf("\n Checking\n");


        for (int i=0; i<dim_0; i++)
                for (int j=0; j<dim_1; j++) 
                        if ( fabs( hit_tileElemAt((C.DS_dev[1]), 2, i, j) - 0.32 * NITER ) > 0.01 )
                                fprintf(stderr, "ERROR: Resultado C[%d,%d] = %f\n", i, j, 
							hit_tileElemAt((C.DS_dev[1]), 2, i, j) );
		
        for (int i=0; i<dim_0_CPU; i++)
                for (int j=0; j<dim_1_CPU; j++) 
                        if ( fabs( hit_tileElemAt((C.DS_dev[0]), 2, i, j) - 0.32 * NITER ) > 0.01 )
                                fprintf(stderr, "ERROR: Resultado C_CPU[%d,%d] = %f\n", i, j, 
							hit_tileElemAt((C.DS_dev[0]), 2, i, j) );
	
	

#endif
	
	// 9. Free resources
	hit_CtileFree(A, cntrlMult);
	hit_CtileFree(B, cntrlMult);
	hit_CtileFree(C, cntrlMult);
	

}}

	return 0;
}
