#ifndef MIC_DEV
#define MIC_DEV 0
#endif
#define ALLOC   alloc_if(1)
#define FREE    free_if(1)
#define RETAIN  free_if(0)
#define REUSE   alloc_if(0)

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>
#include <mkl.h>
#include <math.h>

double  mainClock, totalClock;


void matAdd(int size, int nIter, float* ( Matrix_A), \
          float *( Matrix_B), \
          float *( Matrix_C))
{

	//Attach
	// #pragma offload target(mic:MIC_DEV) in(Matrix_A:length(size*size) ALLOC RETAIN) \
	      in(Matrix_B:length(size*size) ALLOC RETAIN), in(Matrix_C:length(size*size) ALLOC RETAIN) 
//	{}

	// COmputation
	#pragma offload target(mic:MIC_DEV) in(Matrix_A:length(size*size) ), \
                                     in(Matrix_B:length(size*size) ), inout(Matrix_C:length(size*size) ) 
	//#pragma offload target(mic:MIC_DEV) nocopy(Matrix_A ), \
                                      nocopy(Matrix_B), nocopy(Matrix_C) 
	{
	int i,j,k;

       // mainClock = omp_get_wtime();

	#pragma omp parallel for private(i,j,k) //firstprivate(nIter) 
	 for ( i = 0; i < size; ++i)
       	   for ( j = 0; j < size; ++j){
	     #pragma vector aligned
	     #pragma ivdep
     	     for ( k = 0; k < nIter; ++k)
     		    //Matrix_C[i][j] += Matrix_A[i][j] + Matrix_B[i][j];
     		    Matrix_C[i*size+j] += Matrix_A[i*size+j] + Matrix_B[i*size+j];
      		}

	
	}
	

//	mainClock = omp_get_wtime() - mainClock;


	//Detach
//	#pragma offload target(mic:MIC_DEV) out(Matrix_C:length(size*size) FREE REUSE)
	{} 

}



int main(int argc, char* argv[])
{

   if(argc < 2)
   {
         fprintf(stderr,"Use: %s <size>  \n",argv[0]);
        return -1;
   }

   int i, j, k;
   int size = atoi(argv[1]);
   int nIter = 1; //atoi(argv[2]);
//   int nThreads = atoi(argv[3]);

  // omp_set_num_threads(nThreads);

   float *( Matrix_A) = malloc(sizeof(float)*size*size);
   float *( Matrix_B) = malloc(sizeof(float)*size*size);
   float *( Matrix_C) = malloc(sizeof(float)*size*size);


   for ( i = 0; i < size; i++) {
      for ( j = 0; j < size; j++) {
         Matrix_A[i*size+j] = 0.12; //(float)((i + j)%100);
         Matrix_B[i*size+j] =0.2; // (float)((i * j)%100);
         Matrix_C[i*size+j] = 0;
      }
   }

   
#pragma offload target(mic:0)
{
//printf("\n Venga vamos\n");
}
 

   
   totalClock = omp_get_wtime();
   
 //  matAdd (size, nIter, Matrix_A, Matrix_B, Matrix_C);
          #pragma offload target(mic:0) in(Matrix_A:length(size*size) ), \
                                     in(Matrix_B:length(size*size) ), inout(Matrix_C:length(size*size) ) 
        {
        int i,j,k;

        #pragma omp parallel for private(i,j,k) //firstprivate(nIter) 
         for ( i = 0; i < size; ++i)
           for ( j = 0; j < size; ++j){
             #pragma vector aligned
             #pragma ivdep
           //  for ( k = 0; k < nIter; ++k)
                    //Matrix_C[i][j] += Matrix_A[i][j] + Matrix_B[i][j];
                    Matrix_C[i*size+j] += Matrix_A[i*size+j] + Matrix_B[i*size+j];
           }


        }
 
   totalClock = omp_get_wtime() - totalClock;
/*

   mainClock = omp_get_wtime();
   #pragma offload target(mic:MIC_DEV) wait(transfer) \
              nocopy(Matrix_A), nocopy(Matrix_C), nocopy(Matrix_B) //signal(transferOut)
	{
	int i,j,k;
	//omp_set_num_threads(256);
	char michostname[100];
	gethostname(michostname, sizeof(michostname));
	printf("MIC: Hello world from MIC. I am %s and I have %ld logical cores. I was called from host: %s \n", michostname, sysconf(_SC_NPROCESSORS_ONLN), hostname);


	#pragma omp parallel for default(none) \
                shared(Matrix_A,Matrix_B,Matrix_C,size) firstprivate(nIter) private(i,j,k)
	 for ( i = 0; i < size; ++i)
       	   for ( j = 0; j < size; ++j){
	     #pragma vector aligned
	     #pragma ivdep
     	     for ( k = 0; k < nIter; ++k)
     		    Matrix_C[i][j] += Matrix_A[i][j] + Matrix_B[i][j];
      		} 

   	mainClock = omp_get_wtime() - mainClock;
*/

//   #pragma offload_transfer target(mic:MIC_DEV)  wait(transferOut) signal(transferOut) out(Matrix_C:length(size*size)) 
  // #pragma offload_wait target(mic:MIC_DEV) wait(transferOut)

    	// Calculate NORM
    	double resultado=0,suma=0;
    	for ( i=0; i<size; i++ ) {
            for ( j=0; j<size; j++ ) {
              suma += pow(  Matrix_C[ i*size+ j ] ,2);
		#ifdef DEBUG2
	      	printf("%lf  ", Matrix_C[i*size+j]);
		#endif
            }
	#ifdef DEBUG2
	printf("\n");
	#endif
    	}
   	 printf("\n ----------------------- NORM ----------------------- \n");
   	 printf("\n Acumulated sum: %lf",suma);

    	resultado=sqrt( suma );
    	printf("\n Result: %lf \n",resultado);

    	printf("\n ---------------------------------------------------- \n");

   // TIMES
   printf("\n ----------------------- TIME ----------------------- \n");
   printf("Clock seq: %lf\n", mainClock );
   printf("Clock main: %lf\n", totalClock );



   free(Matrix_A); free(Matrix_B); free(Matrix_C);
   return 0;
} 
