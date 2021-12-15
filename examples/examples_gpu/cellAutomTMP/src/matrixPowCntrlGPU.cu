// System includes
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"

#ifndef BLOCK_SIZE
#define BLOCK_SIZE 32
#endif

// #ifndef DEBUG
// #define DEBUG
// #endif // DEBUG

hit_tileNewType(float);
hit_ktileNewType(float);
int GPU = 0;

/* A. GPU Kernel charazterizations */
CAL_KERNEL_GPU_CHAR_STATIC(Copy, 2, full, low, low);
CAL_KERNEL_GPU_CHAR_STATIC(MatMult, 2, fixed, square, 32);

/* B. GPU Kernel implementation */
CAL_KERNEL(Copy, dGPU, KHitTile_float dst, KHitTile_float src)
{
	int row = blockDim.y * blockIdx.y + threadIdx.y;
	int col = blockDim.x * blockIdx.x + threadIdx.x;
	if (row < dst.origAcumCard[1] && col < dst.origAcumCard[1])
		hit_tileElemAtNoStride(dst, 2, row, col) = hit_tileElemAtNoStride(src, 2, row, col);
}

/* B. CPU Kernel implementation */
CAL_KERNEL(MatMult, dGPU, KHitTile_float A, KHitTile_float B, KHitTile_float C)
{
	__shared__ float tile_a[BLOCK_SIZE][BLOCK_SIZE];
	__shared__ float tile_b[BLOCK_SIZE][BLOCK_SIZE];

	int row = blockIdx.y * BLOCK_SIZE + threadIdx.y;
	int col = blockIdx.x * BLOCK_SIZE + threadIdx.x;
	float tmp = 0.0;
	int idx;
	int n = A.origAcumCard[1]; // Number of columns

	for (int sub = 0; sub < gridDim.x; ++sub)
	{
		idx = row * n + sub * BLOCK_SIZE + threadIdx.x;
		if (idx >= n * n)
		{
			// n may not divisible by BLOCK_SIZE
			tile_a[threadIdx.y][threadIdx.x] = 0;
		}
		else
		{
			tile_a[threadIdx.y][threadIdx.x] = hit_tileElemAtNoStride(A, 1, idx);
		}

		idx = (sub * BLOCK_SIZE + threadIdx.y) * n + col;
		if (idx >= n * n)
		{
			tile_b[threadIdx.y][threadIdx.x] = 0;
		}
		else
		{
			tile_b[threadIdx.y][threadIdx.x] = hit_tileElemAtNoStride(B, 1, idx);
		}
		__syncthreads();

		for (int k = 0; k < BLOCK_SIZE; ++k)
		{
			tmp += tile_a[threadIdx.y][k] * tile_b[k][threadIdx.x];
		}
		__syncthreads();
	}
	if (row < n && col < n)
	{
		hit_tileElemAtNoStride(C, 2, row, col) = tmp;
	}
	#ifdef DEBUG
	if (!row && !col)
		printf("matrixMultCuda(%d, %d) C: %g <- Ci %g X A %g\n", n, n,
			hit_tileElemAtNoStride(C, 2, row, col),
			hit_tileElemAtNoStride(A, 2, row, col),
			hit_tileElemAtNoStride(B, 2, row, col)
		);
	#endif // DEBUG
}

CAL_KERNEL_PROTO(MatMult, // Name
				 1, dGPU, // Implementations
				 3, IN, HitTile_float, A, IN, HitTile_float, B, OUT, HitTile_float, C);

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO(Copy,	// Name
				 1, dGPU, // Implementations
				 2, OUT, HitTile_float, dst, IN, HitTile_float, src);

/**
 * Program main
 */
int main(int argc, char **argv)
{

	// 1. Init Controllers library
	omp_set_nested(1);
	omp_set_num_threads(3);
#pragma omp parallel
	{
#pragma omp single
		{

			// ARTURO: Simpler interface for square matrices (only one parameter)
			// 2. Taking arguments
			if (argc != 4)
			{
				fprintf(stderr, "Usage: %s <matrixSize> <n_power> <GPU>\n\n", argv[0]);
				exit(EXIT_FAILURE);
			}
			int SIZE = atoi(argv[1]);
			int POWER = atoi(argv[2]);
			GPU = atoi(argv[3]);
			int MATRIX_SIZE = sizeof(float) * SIZE * SIZE;

			/*Timer*/
			Timer tTotal, tTotalA;
			Timer tComp;
			Timer tComm;
			TimerCreate(tTotal);
			// TimerCreate(tTotalA);
			// TimerCreate(tComp);
			// TimerCreate(tComm);

			// Use a larger block size for Fermi and above
			int block_size = 32;

			/*Variables*/
			HitTile_float A;
			HitTile_float Ci;
			HitTile_float C1, C2;
			HitTile_float C;

			// 3. Declare and initialize full matrices and domains
			hit_tileDomain(&A, float, 2, SIZE, SIZE);
			hit_tileDomain(&Ci, float, 2, SIZE, SIZE);
			hit_tileDomain(&C1, float, 2, SIZE, SIZE);
			hit_tileDomain(&C2, float, 2, SIZE, SIZE);
			hit_tileAlloc(&A);
			hit_tileAlloc(&C1);
			hit_tileAlloc(&C2);

			for (int i = 0; i < SIZE; i++)
				for (int j = 0; j < SIZE; j++)
				{
					hit_tileElemAt(A, 2, i, j) = 2.0;
					// hit_tileElemAt(C1, 2, i, j) = 0.0;
				}

			// 4. Init computation threads
			CALThread threads;
			CALThreadInit(threads, 2, SIZE, SIZE);

			// 5. Create controller object
			CALCntrl comm;
			CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU);

			// TimerStart(tComm);
			// TimerStart(tComp);
			// TimerStop(tComp);

			// 6. Attach data structures to the controllers
			CAL_CntrlAttach(&comm, (HitTile *)&A);
			CAL_CntrlAttach(&comm, (HitTile *)&C1);
			CAL_CntrlAttach(&comm, (HitTile *)&C2);
			CAL_CntrlInternal(&comm, (HitTile *)&Ci);

			/* First iteration */
			if(POWER > 1){
				/* Copy A into C(0) */
				CAL_CntrlLaunch(comm, Copy, threads, Ci, A);
				/* Calculate C <- C_i x A */
				CAL_CntrlLaunch(comm, MatMult, threads, Ci, A, C1);
				/* Copy result C_i <- C  */
				CAL_CntrlLaunch(comm, Copy, threads, Ci, C1);

				CAL_CntrlGlobalSync(&comm);
				TimerStart(tTotal);
			}
			// 7. Launch the task: invoking the kerneli
			for (int i = 1; i < POWER; i++)
			{
				if(i % 2){ /* Odd */
					/* Move result to host */
					CAL_CntrlMoveFrom(&comm, (HitTile *)&C1);
					/* Calculate C <- C_i x A */
					CAL_CntrlLaunch(comm, MatMult, threads, Ci, A, C2);
					/* Copy result C_i <- C  */
					CAL_CntrlLaunch(comm, Copy, threads, Ci, C2);
				} else { /* Even */
					/* Move result to host */
					CAL_CntrlMoveFrom(&comm, (HitTile *)&C2);
					/* Calculate C <- C_i x A */
					CAL_CntrlLaunch(comm, MatMult, threads, Ci, A, C1);
					/* Copy result C_i <- C  */
					CAL_CntrlLaunch(comm, Copy, threads, Ci, C1);
				}
			}
			// 8. Times
			CAL_CntrlGlobalSync(&comm);
			TimerStop(tTotal);

			// 9. Copy result from device memory to host memory
			CAL_CntrlDetach(&comm, (HitTile *)&C1);
			CAL_CntrlDetach(&comm, (HitTile *)&C2);
			CAL_CntrlDetach(&comm, (HitTile *)&A);

			// C = (POWER % 2) ? C1 : C2;

			// 10. Destroy the controller
			CAL_CntrlDestroy(&comm);

			printf("\nVERSION SINCRONA************\n");
			printf("SIZE %d %d\n", SIZE, POWER);
			printf("Clock Main %.8lf <--\n", TimerGetTime(tTotal));

			// 11. Calculate NORM
			double resultado = 0, suma = 0;
			for (int i = 0; i < SIZE; i++)
			{
				for (int j = 0; j < SIZE; j++)
				{
// #ifdef DEBUG
// 					printf("%lf ", hit_tileElemAt(C, 2, i, j));
// #endif
					// suma += pow(hit_tileElemAt(C, 2, i, j), 2);
				}
// #ifdef DEBUG
// 				printf("\n");
// #endif // DEBUG
			}
			printf("\n ----------------------- NORM ----------------------- \n");
			printf("\n Acumulated sum: %lf", suma);

			resultado = sqrt(suma);
			printf("\n Result: %lf \n", resultado);

			printf("\n ---------------------------------------------------- \n");

			// 12. Free data structures
			hit_tileFree(A);
			hit_tileFree(C1);
			hit_tileFree(C2);

			// 13. Finish the controller library
			CAL_CntrlFinish();

			return (0);
		}
