#include <stdio.h>
#include <stdlib.h>
//#include <mkl.h>
#include <cuda_runtime.h>
#include "cublas_v2.h"

#define DECLS_FLOAT
#include "decls.h"
#include "util.h"

void printMessageFromCuBLASError(cublasStatus_t error_code)
{
    switch(error_code)
	{
	case 0:
	    printf("CUBLAS_STATUS_SUCCESS"); break;
	case 1:
	    printf("CUBLAS_STATUS_NOT_INITIALIZED"); break;
	case 3:
    	    printf("CUBLAS_STATUS_ALLOC_FAILED"); break;
	case 7:
            printf("CUBLAS_STATUS_INVALID_VALUE"); break;
	case 8:
            printf("CUBLAS_STATUS_ARCH_MISMATCH"); break;
	case 11:
            printf("CUBLAS_STATUS_MAPPING_ERROR"); break;
	case 13:
            printf("CUBLAS_STATUS_EXECUTION_FAILED"); break;
	case 14:
            printf("CUBLAS_STATUS_INTERNAL_ERROR"); break;
	case 15:
            printf("CUBLAS_STATUS_NOT_SUPPORTED"); break;
	case 16:
            printf("CUBLAS_STATUS_LICENSE_ERROR"); break;
	}
}

main()
{
    double t_start, t_end;
    int i, j;
    int M=N;
    float *a;//, *b;
    float *dMatrixA, *dMatrixB;
    float *dVectorU1, *dVectorU2;
    float *dVectorV1, *dVectorV2;
    float *dVectorW, *dVectorX, *dVectorY, *dVectorZ;

    init_array();

    /*1. Declaration of host matrices*/
    a = (float *) malloc(sizeof(float)*M*M);
    //b = (float *) malloc(sizeof(float)*M*M);
    float cu_alpha = alpha; 
    float cu_beta = beta;
    float cu_one  = 1.0;
    float cu_zero = 0.0;
    //float* alpha_p = &alpha; float* beta_p = &beta;

    for( i=0; i<M; i++){
        for( j=0; j<N; j++){
            a[i*M+j]= A[i][j]; 
        }
    }

    cudaError_t cudaStat = cudaSuccess;
    cublasStatus_t stat;
    cublasHandle_t handle;

    cudaStat = cudaMalloc((void**)&dMatrixA, sizeof(float)*M*M);
    cudaStat = cudaMalloc((void**)&dMatrixB, sizeof(float)*M*M);
    cudaStat = cudaMalloc((void**)&dVectorU1, sizeof(float)*M);
    cudaStat = cudaMalloc((void**)&dVectorU2, sizeof(float)*M);
    cudaStat = cudaMalloc((void**)&dVectorV1, sizeof(float)*M);
    cudaStat = cudaMalloc((void**)&dVectorV2, sizeof(float)*M);
    cudaStat = cudaMalloc((void**)&dVectorW, sizeof(float)*M); //Init to zeros, algorithm output
    cudaStat = cudaMalloc((void**)&dVectorX, sizeof(float)*M); //Init to zeros
    cudaStat = cudaMalloc((void**)&dVectorY, sizeof(float)*M);
    cudaStat = cudaMalloc((void**)&dVectorZ, sizeof(float)*M);

    stat = cublasCreate(&handle);

#ifdef DECLS_FLOAT
    stat = cublasSetMatrix(M, M, sizeof(float), a, M, dMatrixA, M);
#else
    stat = cublasSetMatrix(M, M, sizeof(double), a, M, dMatrixA, M);
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
       printf ("Data download of a to dMatrixA (GPU) failed!\n");
       cudaFree (dMatrixA);
       cublasDestroy(handle);
       return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasSetVector(M, sizeof(float), u1, 1, dVectorU1, 1);
#else
    stat = cublasSetVector(M, sizeof(double), u1, 1, dVectorU1, 1);
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
       printf ("Data download of a to dVectorU1 (GPU) failed!\n");
       cudaFree (dVectorU1);
       cublasDestroy(handle);
       return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasSetVector(M, sizeof(float), u2, 1, dVectorU2, 1);
#else
    stat = cublasSetVector(M, sizeof(double), u1, 1, dVectorU1, 1);
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
       printf ("Data download of a to dVectorU2 (GPU) failed! (error %i)\n", stat);
       cudaFree (dVectorU2);
       cublasDestroy(handle);
       return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasSetVector(M, sizeof(float), v1, 1, dVectorV1, 1);
    stat = cublasSetVector(M, sizeof(float), v2, 1, dVectorV2, 1);
    stat = cublasSetVector(M, sizeof(float), y, 1, dVectorY, 1);
    stat = cublasSetVector(M, sizeof(float), z, 1, dVectorZ, 1);
#else
    stat = cublasSetVector(M, sizeof(double), v1, 1, dVectorV1, 1);
    stat = cublasSetVector(M, sizeof(double), v2, 1, dVectorV2, 1);
    stat = cublasSetVector(M, sizeof(double), y, 1, dVectorY, 1);
    stat = cublasSetVector(M, sizeof(double), z, 1, dVectorZ, 1);
#endif

    IF_TIME(t_start = rtclock());

    //cublasOperation_t transa = CUBLAS_OP_T (CblasNoTrans) or CUBLAS_OP_N (CblasNoTrans)
    //Since cuBLAS is column-major, transpose the matrix A.
#ifdef DECLS_FLOAT
    //stat = cublasSgeam(handle, CUBLAS_OP_T, CUBLAS_OP_N, M, M, &cu_alpha, dMatrixA, M, &cu_beta, dMatrixA, M, dMatrixA, M);
#else
    //stat = cublasDgeam(handle, CUBLAS_OP_T, CUBLAS_OP_N, M, M, &cu_alpha, dMatrixA, M, &cu_beta, dMatrixA, M, dMatrixA, M);
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
        printf ("cublasDgeam[1] failed! (error %i = ", stat); printMessageFromCuBLASError(stat); printf(")\n");
        return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasScopy(handle, M*N, dMatrixA, 1, dMatrixB, 1); //Copy matrix a to matrix b
#else
    stat = cublasDcopy(handle, M*N, dMatrixA, 1, dMatrixB, 1); //Copy matrix a to matrix b
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
        printf ("cublasDcopy[2] failed! (error %i = ", stat); printMessageFromCuBLASError(stat); printf(")\n");
        return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasSger(handle, M, N, &cu_one, dVectorU1, 1, dVectorV1, 1, dMatrixB, N); //B = u1*v1' + B
#else
    stat = cublasDger(handle, M, N, &cu_one, dVectorU1, 1, dVectorV1, 1, dMatrixB, N); //B = u1*v1' + B
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
        printf ("cublasDcopy[3] failed! (error %i = ", stat); printMessageFromCuBLASError(stat); printf(")\n");
        return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasSger(handle, M, N, &cu_one, dVectorU2, 1, dVectorV2, 1, dMatrixB, M); //B = u2*v2' + B
#else
    stat = cublasDger(handle, M, N, &cu_one, dVectorU2, 1, dVectorV2, 1, dMatrixB, M); //B = u2*v2' + B
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
        printf ("cublasDger[4] failed! (error %i = ", stat); printMessageFromCuBLASError(stat); printf(")\n");
        return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasScopy(handle, N, dVectorZ, 1, dVectorX, 1); //Copy vector z to vector x
#else
    stat = cublasDcopy(handle, N, dVectorZ, 1, dVectorX, 1); //Copy vector z to vector x
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
        printf ("cublasDcopy[5] failed! (error %i = ", stat); printMessageFromCuBLASError(stat); printf(")\n");
        return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasSgemv(handle, CUBLAS_OP_T, M, N, &cu_beta, dMatrixB, M, dVectorY, 1, &cu_one, dVectorX, 1); //x = βB'y + x (matrix B' * vector y)
#else
    stat = cublasDgemv(handle, CUBLAS_OP_T, M, N, &cu_beta, dMatrixB, M, dVectorY, 1, &cu_one, dVectorX, 1); //x = βB'y + x (matrix B' * vector y)
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
        printf ("cublasDgemv[6] failed! (error %i = ", stat); printMessageFromCuBLASError(stat); printf(")\n");
        return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasSgemv(handle, CUBLAS_OP_N, M, N, &cu_alpha, dMatrixB, M, dVectorX, 1, &cu_zero, dVectorW, 1);//w = αBx (matrix B * vector x)
#else
    stat = cublasDgemv(handle, CUBLAS_OP_N, M, N, &cu_alpha, dMatrixB, M, dVectorX, 1, &cu_zero, dVectorW, 1);//w = αBx (matrix B * vector x)
#endif
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
        printf ("cublasDgemv[7] failed! (error %i = ", stat); printMessageFromCuBLASError(stat); printf(")\n");
        return EXIT_FAILURE;
    }
#ifdef DECLS_FLOAT
    stat = cublasGetVector(M, sizeof(float), dVectorW, 1, w, 1);
#else
    stat = cublasGetVector(M, sizeof(double), dVectorW, 1, w, 1);
#endif

    cublasDestroy(handle);
    cudaFree(dMatrixA);
    cudaFree(dMatrixB);
    cudaFree(dVectorU1);
    cudaFree(dVectorU2);
    cudaFree(dVectorV1);
    cudaFree(dVectorV2);
    cudaFree(dVectorW);
    cudaFree(dVectorX);
    cudaFree(dVectorY);
    cudaFree(dVectorZ);
//printf("Ha llegado!!\n");

    IF_TIME(t_end = rtclock());
    IF_TIME(fprintf(stderr, "%0.6lfs\n", t_end - t_start));

#ifdef TEST
    print_array();
#endif

    free(a);

    return 0;
}
