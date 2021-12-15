#ifndef MIC_DEV
#define MIC_DEV 0
#endif
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mkl.h>
#include <math.h>

double  mainClock;


void matAdd(int size, int nIter, float (*restrict Matrix_A)[size], \
          float (*restrict Matrix_B)[size], \
          float (*restrict Matrix_C)[size])
{
   #pragma offload target(mic:MIC_DEV) in(nIter), in(Matrix_A:length(size*size)) \
      in(Matrix_B:length(size*size)), inout(Matrix_C:length(size*size)) 
	{
	int i,j,k;
	omp_set_num_threads(256);

	mainClock = omp_get_wtime();
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
	
	}
}



int main(int argc, char* argv[])
{

   if(argc != 4)
   {
         fprintf(stderr,"Use: %s size nThreads nIter \n",argv[0]);
        return -1;
   }

   int i, j, k;
   int size = atoi(argv[1]);
   int nIter = atoi(argv[2]);
   int nThreads = atoi(argv[3]);

   omp_set_num_threads(nThreads);

   float (*restrict Matrix_A)[size] = malloc(sizeof(float)*size*size);
   float (*restrict Matrix_B)[size] = malloc(sizeof(float)*size*size);
   float (*restrict Matrix_C)[size] = malloc(sizeof(float)*size*size);


   for ( i = 0; i < size; i++) {
      for ( j = 0; j < size; j++) {
         Matrix_A[i][j] = 2.12; //(float)((i + j)%100);
         Matrix_B[i][j] =0.2; // (float)((i * j)%100);
         Matrix_C[i][j] = 0;
      }
   }

   //#pragma offload_transfer target(mic:MIC_DEV) mandatory, in(Matrix_A:length(size*size)) \
      in(Matrix_B:length(size*size)), in(Matrix_C:length(size*size)) \
      signal(Matrix_C)
   

   matAdd (size, nIter, Matrix_A, Matrix_B, Matrix_C);
/*
   #pragma offload_wait target(mic:MIC_DEV) wait(Matrix_C)
	{
	int i,j,k;
	omp_set_num_threads(256);

	#pragma omp parallel
	 for ( i = 0; i < size; ++i)
       	   for ( j = 0; j < size; ++j){
	     #pragma vector aligned
	     #pragma ivdep
     	     for ( k = 0; k < nIter; ++k)
     		    Matrix_C[i][j] += Matrix_A[i][j] + Matrix_B[i][j];
      		}
	}
*/

 //  #pragma offload_transfer target(mic:MIC_DEV) mandatory, out(Matrix_C:length(size*size))


   // TIMES
   printf("\n ----------------------- TIME ----------------------- \n");
   printf("Clock main: %lf\n", mainClock );

    // Calculate NORM
    double resultado=0,suma=0;
    for ( i=0; i<size; i++ ) {
            for ( j=0; j<size; j++ ) {
              suma += pow(  Matrix_C[ i][ j ] ,2);
#ifdef DEBUG2
	      printf("%lf  ", Matrix_C[i][j]);
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


   free(Matrix_A); free(Matrix_B); free(Matrix_C);
   return 0;
} 
