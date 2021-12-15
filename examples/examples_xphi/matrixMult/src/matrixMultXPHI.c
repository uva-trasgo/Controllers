#ifndef MIC_DEV
#define MIC_DEV 0
#endif
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <mkl.h>
#include <math.h>


void doMultiply(int size, float (*restrict Matrix_A), \
          float (*restrict Matrix_B), \
          float (*restrict Matrix_C))
{
   #pragma offload target(mic:MIC_DEV), in(Matrix_A:length(size*size)) \
      in(Matrix_B:length(size*size)), inout(Matrix_C:length(size*size))
	{
	int i,j,k; 
	#pragma omp parallel for default(none) \
      		shared(Matrix_A,Matrix_B,Matrix_C) firstprivate(size) private(i,j,k)
	 for ( i = 0; i < size; ++i){
     	     for ( k = 0; k < size; ++k){
	   //#pragma ivdep
     	   for ( j = 0; j < size; ++j){
     		    Matrix_C[i*size+j] += Matrix_A[i*size+k] * Matrix_B[k*size+j];
      	}}}}
}


double  mainClock;

int main(int argc, char* argv[])
{

   if(argc < 2)
   {
         fprintf(stderr,"Use: %s <size> \n",argv[0]);
        return -1;
   }

   int i, j, k;
   int size = atoi(argv[1]);


   float (*restrict Matrix_A) = malloc(sizeof(float)*size*size);
   float (*restrict Matrix_B) = malloc(sizeof(float)*size*size);
   float (*restrict Matrix_C) = malloc(sizeof(float)*size*size);


   for ( i = 0; i < size; i++) {
      for ( j = 0; j < size; j++) {
         Matrix_A[i*size+j] = (float)((i + j)%100);
         Matrix_B[i*size+j] = (float)((i - j)%100);
         Matrix_C[i*size+j] = 0;
      }
   }

   mainClock = omp_get_wtime();

   doMultiply (size, Matrix_A, Matrix_B, Matrix_C);

   mainClock = omp_get_wtime() - mainClock;

   // TIMES
   printf("\n ----------------------- TIME ----------------------- \n");
   printf("Clock main: %lf\n", mainClock );

    // Calculate NORM
    double resultado=0,suma=0;
    for ( i=0; i<size; i++ ) {
            for ( j=0; j<size; j++ ) {
              suma += pow(  Matrix_C[ i*size+ j ] ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");


   free(Matrix_A); free(Matrix_B); free(Matrix_C);
   return 0;
} 
