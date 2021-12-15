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
#include "CAL_CntrlXPHI.h"
#include "CAL_KernelXPHI.h"
//#include "CAL_AttachXPHI.h"
//#include "../../../cal/xphi/CAL_AttachXPHI.h"
#include "CAL_KernelLibGPU.h"
#include <mkl.h>

hit_tileNewType( float );


double  mainClock;
double  initClock;
double  sequentialClock;
double alpha = 1.0;
double beta = 1.0;

#define SIZE 16

CAL_KERNEL_XPHI(MatSum, 3, IN, HitTile_float*, A, IN, HitTile_float*, B, OUT, HitTile_float*, C){

  int k;
  int nIter=1;

  #pragma vector aligned
  #pragma ivdep
  for(k=0;k< nIter;k++){
	hit_tileElemAt(C, 2, threadId.x, threadId.y) = hit_tileElemAt(C, 2, threadId.x, threadId.y) + hit_tileElemAt(A, 2, threadId.x, threadId.y) + hit_tileElemAt(B, 2, threadId.x, threadId.y) ;
  }

}

CAL_KERNEL_XPHI(scopy, 5, IVAL, int, n, IN, HitTile_float*, x, IVAL, int, incx, IO, HitTile_float*, y, IVAL, int, incy)
{
        #pragma vector aligned
        #pragma ivdep
        cblas_scopy(n, x.data, incx, y.data, incy);
}

CAL_KERNEL_XPHI(sgemv, 11, IVAL, int, trans, IVAL, int, m, IVAL, int, n, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, x, IVAL, int, incx, IVAL, float, beta, IO, HitTile_float*, y, IVAL, int, incy)
{
        #pragma vector aligned
        #pragma ivdep
#ifdef __cplusplus
        cblas_sgemv(CblasRowMajor, CBLAS_TRANSPOSE(trans), m, n, alpha, a.data, lda, x.data, incx, beta, y.data, incy); //y = αA'x + βy (matrix A' * vector x)
#else
        cblas_sgemv(CblasRowMajor, (CBLAS_TRANSPOSE)trans, m, n, alpha, a.data, lda, x.data, incx, beta, y.data, incy); //y = αA'x + βy (matrix A' * vector x)
#endif
}

CAL_KERNEL_XPHI(sger, 9, IVAL, int, m, IVAL, int, n, IVAL, float, alpha, IN, HitTile_float*, x, IVAL, int, incx, IN, HitTile_float*, y, IVAL, int, incy, IO, HitTile_float*, a, IVAL, int, lda)
{
        #pragma vector aligned
        #pragma ivdep
        cblas_sger(CblasRowMajor, m, n, alpha, x.data, incx, y.data, incy, a.data, lda); //A = α*x*y' + A
}

CAL_KERNEL_XPHI(sgemm, 13, IVAL, int, transa, IVAL, int, transb, IVAL, int, m, IVAL, int, n, IVAL, int, k, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, b, IVAL, int, ldb, IVAL, float, beta, IO, HitTile_float*, c, IVAL, int, ldc)
{
        #pragma vector aligned
        #pragma ivdep
#ifdef __cplusplus
        cblas_sgemm(CblasRowMajor, CBLAS_TRANSPOSE(transa), CBLAS_TRANSPOSE(transb), m, n, k, alpha, a.data, lda, b.data, ldb, beta, c.data, ldc); //C = αA'*B' + βC,
#else
        cblas_sgemm(CblasRowMajor, (CBLAS_TRANSPOSE)transa, (CBLAS_TRANSPOSE)transb, m, n, k, alpha, a.data, lda, b.data, ldb, beta, c.data, ldc); //C = αA'*B' + βC,
#endif
}

//CAL_KERNEL_GPU_CHAR_STATIC(scopy, 2, full, low, low);
CAL_KERNEL_LIBGPU(scopy, 5, IVAL, int, n, IN, HitTile_float*, x, IVAL, int, incx, IO, HitTile_float*, y, IVAL, int, incy)
{
        cublasScopy(comm->handleCUBLAS, n, x.kdata, incx, y.kdata, incy);
}

//CAL_KERNEL_GPU_CHAR_STATIC(sgemv, 2, full, low, low);
CAL_KERNEL_LIBGPU(sgemv, 11, IVAL, int, trans, IVAL, int, m, IVAL, int, n, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, x, IVAL, int, incx, IVAL, float, beta, IO, HitTile_float*, y, IVAL, int, incy)
{
#ifdef __cplusplus
        cublasSgemv(comm->handleCUBLAS, cublasOperation_t(trans), m, n, &alpha, a.kdata, lda, x.kdata, incx, &beta, y.kdata, incy); //y = αA'x + βy (matrix A' * vector x)
#else
        cublasSgemv(comm->handleCUBLAS, (cublasOperation_t)trans, m, n, &alpha, a.kdata, lda, x.kdata, incx, &beta, y.kdata, incy); //y = αA'x + βy (matrix A' * vector x)
#endif
}

//CAL_KERNEL_GPU_CHAR_STATIC(sger, 2, full, low, low);
CAL_KERNEL_LIBGPU(sger, 9, IVAL, int, m, IVAL, int, n, IVAL, float, alpha, IN, HitTile_float*, x, IVAL, int, incx, IN, HitTile_float*, y, IVAL, int, incy, IO, HitTile_float*, a, IVAL, int, lda)
{
        cublasSger(comm->handleCUBLAS, m, n, &alpha, x.kdata, incx, y.kdata, incy, a.kdata, lda); //A = α*x*y' + A
}

//CAL_KERNEL_GPU_CHAR_STATIC(sgemm, 2, full, low, low);
CAL_KERNEL_LIBGPU(sgemm, 13, IVAL, int, transa, IVAL, int, transb, IVAL, int, m, IVAL, int, n, IVAL, int, k, IVAL, float, alpha, IN, HitTile_float*, a, IVAL, int, lda, IN, HitTile_float*, b, IVAL, int, ldb, IVAL, float, beta, IO, HitTile_float*, c, IVAL, int, ldc)
{
#ifdef __cplusplus
        cublasSgemm(comm->handleCUBLAS, cublasOperation_t(transa), cublasOperation_t(transb), m, n, k, &alpha, a.kdata, lda, b.kdata, ldb, &beta, c.kdata, ldc); //C = αA'*B' + βC,
#else
        cublasSgemm(comm->handleCUBLAS, (cublasOperation_t)transa, (cublasOperation_t)transb, m, n, k, &alpha, a.kdata, lda, b.kdata, ldb, &beta, c.kdata, ldc); //C = αA'*B' + βC,
#endif
}


/* A. INITIALIZE MATRIX */
void init_arrays(HitTile_float u1, HitTile_float u2, HitTile_float v1, HitTile_float v2,
                HitTile_float w, HitTile_float x, HitTile_float y, HitTile_float z,
                HitTile_float A, HitTile_float B, int size)
{
    int i, j;

    //for (i=0; i<N; i++) {
    for (i = 0; i < size; i++) {
        hit_tileElemAt(u1, 1, i) = i;
        hit_tileElemAt(u2, 1, i) = ((i+1)/size)/2.0;
        hit_tileElemAt(v1, 1, i) = ((i+1)/size)/4.0;
        hit_tileElemAt(v2, 1, i) = ((i+1)/size)/6.0;
        hit_tileElemAt(y, 1, i)  = ((i+1)/size)/8.0;
        hit_tileElemAt(z, 1, i)  = ((i+1)/size)/9.0;
        hit_tileElemAt(x, 1, i)  = 0.0;
        hit_tileElemAt(w, 1, i)  = 0.0;
        for (j = 0; j < size; j++) {
            hit_tileElemAt(A, 2, i, j) = ((float) i*j)/size;
            hit_tileElemAt(B, 2, i, j) = hit_tileElemAt(A, 2, i, j);
        }
    }
}


/* A. INITIALIZE MATRIX */
void initMatrix( HitTile_float matrixA, HitTile_float matrixB, HitTile_float matrixC, int rows, int columns ) {

        /* 1. INIT Mat = 0 */
        int i,j;
   //     #pragma omp for private(i,j) collapse(2)
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
			hit_tileElemAt( matrixA, 2, i, j ) =0.12; // (i+j)%100;
		}
	}

     //   #pragma omp for private(i,j) collapse(2)
        for (i=0; i<columns; i++){
                for (j=0; j<rows; j++){
			hit_tileElemAt( matrixB, 2, i, j ) = 0.2; //(i*j)%100;
		}
	}

       // #pragma omp for private(i,j) collapse(2)
        for (i=0; i<rows; i++){
                for (j=0; j<rows; j++){
                        //matrixC[ i*rows + j ] = 0;
			hit_tileElemAt( matrixC, 2, i, j ) = 0;
		}
	}
}


double t_start, t_end;

int main(int argc, char *argv[])
{

//    #ifdef DEBUG
     setbuf(stdout, NULL);
     setbuf(stderr, NULL);
  //  #endif

    CAL_CntrlInit(1);

    if ( argc < 2 ) {
                fprintf(stderr, "\nUsage: %s <numRows> \n", argv[0]);
                exit(EXIT_FAILURE);
    }
    int rows = atoi( argv[1] );
    int columns = rows;
  //  int nIter = atoi( argv[2] );
//    int num_threads=atoi( argv[3]);

    int i, j, k;

#ifdef DEBUG
printf("\n Rows: %d Columns: %d \n", rows,columns);
#endif

#pragma offload target(mic:0)
{
//printf("\n Venga vamos\n");
}
    
    /* 2. DECLARE FULL MATRIX AND COPY */
    HitTile_float  matrixC;
    hit_tileDomain( &matrixC, float, 2, rows, columns );

    hit_tileAlloc( &matrixC );

   printf("Sembrando 1\n"); fflush(stdout);
    /* 2. DECLARE FULL MATRIX AND COPY */
    HitTile_float matrixA, matrixB;
    HitTile_float vectorU1, vectorU2;
    HitTile_float vectorV1, vectorV2;
    HitTile_float vectorW, vectorX, vectorY, vectorZ;

   printf("Sembrando 1\n"); fflush(stdout);
    hit_tileDomain(&matrixA, float, 2, rows, columns);
    hit_tileDomain(&matrixB, float, 2, rows, columns);
    hit_tileDomain(&vectorU1, float, 1, columns);
    hit_tileDomain(&vectorU2, float, 1, columns);
    hit_tileDomain(&vectorV1, float, 1, columns);
    hit_tileDomain(&vectorV2, float, 1, columns);
    hit_tileDomain(&vectorW, float, 1, columns);
    hit_tileDomain(&vectorX, float, 1, columns);
    hit_tileDomain(&vectorY, float, 1, columns);
    hit_tileDomain(&vectorZ, float, 1, columns);
   printf("Sembrando 1\n"); fflush(stdout);
    hit_tileAlloc(&matrixA);
    /*HitShape bshape = hit_tileShape(matrixB);
    printf("hit_shapeDims(bshape) = %i\n", hit_shapeDims(bshape));
    HitSig bsig = hit_shapeSig(bshape, 0);
    printf("hit_shapeSig = {.begin = %i, .end = %i, .stride=%i}\n", bsig.begin, bsig.end, bsig.stride);
    bsig = hit_shapeSig(bshape, 1);
    printf("hit_shapeSig = {.begin = %i, .end = %i, .stride=%i}\n", bsig.begin, bsig.end, bsig.stride);*/
    hit_tileAlloc(&matrixB); //Quitar
    hit_tileAlloc(&vectorU1);
    hit_tileAlloc(&vectorU2);
    hit_tileAlloc(&vectorV1);
    hit_tileAlloc(&vectorV2);
    hit_tileAlloc(&vectorW);
    hit_tileAlloc(&vectorX);
    hit_tileAlloc(&vectorY);
    hit_tileAlloc(&vectorZ);



    CALCntrl comm;
    CALThread threads;

    CALThreadInit(threads, 2, rows, columns);


   // initMatrix( matrixA, matrixB, matrixC, rows, columns );
    init_arrays(vectorU1, vectorU2, vectorV1, vectorV2, vectorW, vectorX, vectorY, vectorZ, matrixA, matrixB, columns);


#ifdef DEBUG2 
	printf("\n Matrix A\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixA, 2, i, j ));
		}
		printf("\n");
	}

	printf("\n Matrix B\n");
        for (i=0; i<rows; i++){
                for (j=0; j<columns; j++){
                        printf("%lf ", hit_tileElemAt( matrixB, 2, i, j ));
		}
		printf("\n");
	}

#endif


    /* CREATE CNTRLS */
    mainClock = omp_get_wtime();
    
    CAL_CntrlCreate(&comm, CAL_CNTRL_XPHI, 228);
    CAL_CntrlAttach(&comm, (HitTile*)&matrixC);
    CAL_CntrlAttach  (&comm, (HitTile*)&matrixA);
    CAL_CntrlAttach  (&comm, (HitTile*)&matrixB);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorU1);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorU2);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorV1);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorV2);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorW);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorX);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorY);
    CAL_CntrlAttach  (&comm, (HitTile*)&vectorZ);
 
   // CAL_CntrlLaunch(comm, MatSum, threads, 3, &matrixA, &matrixB, &matrixC);	

    CAL_CntrlDetach(&comm, (HitTile*)&vectorW);

    CAL_CntrlDestroy(&comm);


    /* 4. CLOCK RESULTS */
    mainClock = omp_get_wtime() - mainClock;

    // TIMES
    printf("\n ----------------------- TIME ----------------------- \n");
    printf("Clock main: %lf\n", mainClock );



#ifdef DEBUG2 
	printf("\n Matrix C\n");
        for (i=0; i<rows; i++){
                        printf("%lf ", hit_tileElemAt( vectorW, 1, i ));
		}
		printf("\n");
	}
#endif


    // Calculate NORM
    double resultado=0,suma=0;
    for ( i=0; i<rows; i++ ) {
         for ( j=0; j<columns; j++ ) {
              suma += pow(  hit_tileElemAt( matrixC, 2, i, j ) ,2);
            }
    }
    printf("\n ----------------------- NORM ----------------------- \n");
    printf("\n Acumulated sum: %lf",suma);

    resultado=sqrt( suma );
    printf("\n Result: %lf \n",resultado);

    printf("\n ---------------------------------------------------- \n");

    hit_tileFree(matrixC);
    hit_tileFree(matrixA);
    hit_tileFree(matrixB); //Quitar
    hit_tileFree(vectorU1);
    hit_tileFree(vectorU2);
    hit_tileFree(vectorV1);
    hit_tileFree(vectorV2);
    hit_tileFree(vectorW);
    hit_tileFree(vectorX);
    hit_tileFree(vectorY);
    hit_tileFree(vectorZ);

    CAL_CntrlFinish();  

  return 0;
}
