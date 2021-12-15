#include <stdio.h>
#include <stdlib.h>
//#include <mkl.h>
#include <cuda_runtime.h>
#include "cublas_v2.h"

//#include "decls.h"
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

int main(int argc, char* argv[])
{
	int final_power = 0;
	int rows = 0;
	int columns = 0;

	if (argc < 2){
		printf("Error. Numero de argumentos insuficiente\n");
		exit(-1);
		}
	else if( argc < 3 ) {
		final_power = atoi( argv[1] );
		rows = 250;
		columns = 250;
		}
	else if( argc < 4 ) {
		final_power = atoi( argv[1] );
		rows = atoi( argv[2] );
		columns = rows;
		}
	else if( argc >= 4 ) {
		final_power = atoi( argv[1] );
		rows = atoi( argv[2] );
	columns = atoi( argv[3] );
		}


    double t_start, t_end;
    int i, j;
    //int M=N;
    float *a;//, *b;
    float *dMatrixA; //, *dMatrixB;

    //init_array();

    /*1. Declaration of host matrices*/
    a = (float *) malloc(sizeof(float)*rows*columns);
    //b = (float *) malloc(sizeof(float)*M*M);
    float cu_one  = 1.0;
    float cu_zero = 0.0;
    float cu_alpha = cu_one; 
    float cu_beta = cu_zero;
    //float* alpha_p = &alpha; float* beta_p = &beta;

    /*for( i=0; i<M; i++){
	for( j=0; j<N; j++){
	    a[i*M+j]= ((float) i*j)/rows; 
	}
    }*/
    for( i=0; i<rows; i++){
	for( j=0; j<columns; j++){
	    a[i*columns+j]= ((float) i*j)/rows; 
	}
    }

    cudaError_t cudaStat = cudaSuccess;
    cublasStatus_t stat;
    cublasHandle_t handle;

    cudaStat = cudaMalloc((void**)&dMatrixA, sizeof(float)*rows*columns);
    //cudaStat = cudaMalloc((void**)&dMatrixB, sizeof(float)*M*M);

    stat = cublasCreate(&handle);

    stat = cublasSetMatrix(rows, columns, sizeof(float), a, rows, dMatrixA, rows);
    if (stat != CUBLAS_STATUS_SUCCESS)
    {
       printf ("Data download of a to dMatrixA (GPU) failed!\n");
       cudaFree (dMatrixA);
       cublasDestroy(handle);
       return EXIT_FAILURE;
    }

    IF_TIME(t_start = rtclock());

    //cublasOperation_t transa = CUBLAS_OP_T (CblasNoTrans) or CUBLAS_OP_N (CblasNoTrans)
    //Since cuBLAS is column-major, transpose the matrix A.
    //stat = cublasDgeam(handle, CUBLAS_OP_T, CUBLAS_OP_N, M, M, &cu_alpha, dMatrixA, M, &cu_beta, dMatrixA, M, dMatrixA, M);
    /*if (stat != CUBLAS_STATUS_SUCCESS)
    {
	printf ("cublasDgeam[1] failed! (error %i = ", stat); printMessageFromCuBLASError(stat); printf(")\n");
	return EXIT_FAILURE;
    }*/

    int power;
    for (power = 1; power >= final_power; power++)
        {
        stat = cublasSgemm(handle, CUBLAS_OP_N, CUBLAS_OP_N, rows, columns, columns, &cu_alpha, dMatrixA, columns, dMatrixA, columns, &cu_beta, dMatrixA, rows); //C = αAB + βC
        if (stat != CUBLAS_STATUS_SUCCESS)
            {
	    printf ("cublasSgemm[%i] failed! (error %i = ", power, stat); printMessageFromCuBLASError(stat); printf(")\n");
	    return EXIT_FAILURE;
            }
        }
    stat = cublasGetMatrix(rows, columns, sizeof(float), dMatrixA, rows, a, rows);

    cublasDestroy(handle);
    cudaFree(dMatrixA);
//printf("Ha llegado!!\n");

    IF_TIME(t_end = rtclock());
    IF_TIME(fprintf(stderr, "%0.6lfs\n", t_end - t_start));

#ifdef TEST
    print_array();
#endif

    free(a);

    return 0;
}
