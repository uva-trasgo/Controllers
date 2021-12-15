#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "CAL.h"

#define SEED 6834723

hit_tileNewType(float);
hit_ktileNewType(float);

/* A. GPU Kernel charazterizations */
CAL_KERNEL_GPU_CHAR_STATIC(Copy, 2, full, low, low);
CAL_KERNEL_GPU_CHAR_STATIC(Update, 2, medium, medium, medium);

/* B.1 GPU Kernel implementation */
CAL_KERNEL(Copy, dGPU, KHitTile_float dst, KHitTile_float src)
{
	int row = threadId.y;
	int col = threadId.x;
	hit_tileElemAtNoStride(dst, 2, row, col) = hit_tileElemAtNoStride(src, 2, row, col);
}

/* B.2 GPU Kernel implementation */
CAL_KERNEL(Update, dGPU, KHitTile_float dst, KHitTile_float src)
{
	int row = threadId.y + 1;
	int col = threadId.x + 1;

	hit_tileElemAtNoStride(dst, 2, row, col) = (hit_tileElemAtNoStride(src, 2, row - 1, col) +
												hit_tileElemAtNoStride(src, 2, row + 1, col) +
												hit_tileElemAtNoStride(src, 2, row, col - 1) +
												hit_tileElemAtNoStride(src, 2, row, col + 1)) /
											   4;
}

/* C. Defining kernel prototypes */
CAL_KERNEL_PROTO(Copy,	// Name
				 1, dGPU, // Implementations
				 2, OUT, HitTile_float, dst, IN, HitTile_float, src);

CAL_KERNEL_PROTO(Update,  // Name
				 1, dGPU, // Implementations
				 2, OUT, HitTile_float, dst, IN, HitTile_float, src);

/*
 * Main program to perform cellular automata 
 */
int main(int argc, char *argv[])
{

#ifdef DEBUG
	setbuf(stderr, NULL);
	setbuf(stdout, NULL);
#endif

	// 1. Init Controllers library
	//CAL_CntrlInit(2);
	omp_set_nested(1);
	omp_set_num_threads(3);
#pragma omp parallel
	{
#pragma omp single
		{

			// 2. Taking arguments
			if (argc != 5)
			{
				fprintf(stderr, "Usage: %s <matrixSize> <matrixSize> <numStages> <GPU>\n", argv[0]);
				exit(EXIT_FAILURE);
			}
			int SIZE = atoi(argv[1]);
			int STAGES = atoi(argv[3]);
			int GPU = atoi(argv[4]);

			HitTile_float mat;
			HitTile_float copy;
			Timer tTotal, tTotalA;
			Timer tComp, tComm;

			// 3. Declare and initialize full matrices and domains
			hit_tileDomain(&mat, float, 2, SIZE, SIZE);
			hit_tileDomain(&copy, float, 2, SIZE, SIZE);
			hit_tileAlloc(&mat);
			//hit_tileAlloc( &copy );

			srand(SEED);
			TimerCreate(tTotal);
			TimerCreate(tTotalA);
			TimerCreate(tComm);
			TimerCreate(tComm);

			for (int i = 0; i < SIZE; i++)
				for (int j = 0; j < SIZE; j++)
					hit_tileElemAt(mat, 2, i, j) = 0.0;

			for (int j = 0; j < SIZE; j++)
			{
				hit_tileElemAt(mat, 2, 0, j) = 1;
				hit_tileElemAt(mat, 2, SIZE - 1, j) = 2;
			}
			for (int i = 0; i < SIZE; i++)
			{
				hit_tileElemAt(mat, 2, i, 0) = 3;
				hit_tileElemAt(mat, 2, i, SIZE - 1) = 4;
			}

			cudaDeviceSynchronize();

			// 4. Init computation threads
			CALThread threads;
			CALThreadInit(threads, 2, SIZE, SIZE);
			CALThread update;
			CALThreadInit(update, 2, SIZE - 2, SIZE - 2)

				// 5. Create controller object
				CALCntrl comm;
			CAL_CntrlCreate(&comm, CAL_CNTRL_GPU, GPU);

			TimerStart(tTotal);
			TimerStart(tComm);
			TimerStart(tComp);
			TimerStop(tComp);
			// 6. Attach data structures to the controllers
			CAL_CntrlAttach(&comm, (HitTile *)&mat);
			// CAL_CntrlAttach(&comm, (HitTile*)&copy);
			CAL_CntrlInternal(&comm, (HitTile *)&copy);
			CAL_CntrlSync(comm);
			TimerStop(tComm);

			// 7. Launch the task: invoking the kerneli
			for (int i = 0; i < STAGES; i++)
			{
				TimerContinue(tComp);
				// Update copy
				CAL_CntrlLaunch(comm, Copy, threads, copy, mat);

				// Compute iteration
				CAL_CntrlLaunch(comm, Update, update, mat, copy);
				CAL_CntrlSync(comm);
				TimerStop(tComp);

				TimerContinue(tComm);
				CAL_CntrlMoveFrom(&comm, (HitTile *)&mat);
				CAL_CntrlSync(comm);
				TimerStop(tComm);
			}
			TimerContinue(tComm);

			// 8. Copy result from device memory to host memory
			CAL_CntrlDetach(&comm, (HitTile *)&mat);
			CAL_CntrlSync(comm);

			// 9. Destroy the controller
			CAL_CntrlDestroy(&comm);
			TimerStop(tTotal);
			TimerStop(tComm);

			// 10. Times
			cudaDeviceSynchronize();

			printf("\nVERSION SINCRONA************\n");
			printf("SIZE %d %d\n", SIZE, STAGES);
			printf("Clock Main %.8lf <--\n", TimerGetTime(tTotal));
			printf("Clock Comm %.8lf\n", TimerGetTime(tComm));
			printf("Clock Comp %.8lf\n", TimerGetTime(tComp));

			// 11. Calculate NORM
			double resultado = 0, suma = 0;
			for (int i = 0; i < SIZE; i++)
			{
				for (int j = 0; j < SIZE; j++)
				{
					// #ifdef DEBUG
					//printf("%lf \n", hit_tileElemAt( mat, 2, i, j ));
					//#endif
					suma += pow(hit_tileElemAt(mat, 2, i, j), 2);
				}
			}
			//printf("\n ----------------------- NORM ----------------------- \n");
			//printf("\n Acumulated sum: %lf",suma);

			//resultado=sqrt( suma );
			//printf("\n Result: %lf \n",resultado);

			//printf("\n ---------------------------------------------------- \n");

			// 12. Free data structures
			hit_tileFree(mat);
			hit_tileFree(copy);
			TimerDestroy(tTotal);

			/*Version asincrona*/ ///////////////////////////////////

			/*Declaración e inicialización*/
			HitTile_float matA;
			HitTile_float copyA;
			hit_tileDomain(&matA, float, 2, SIZE, SIZE);
			hit_tileDomain(&copyA, float, 2, SIZE, SIZE);
			hit_tileAlloc(&matA);

			for (int i = 0; i < SIZE; i++)
				for (int j = 0; j < SIZE; j++)
					hit_tileElemAt(matA, 2, i, j) = 0.0;

			for (int j = 0; j < SIZE; j++)
			{
				hit_tileElemAt(matA, 2, 0, j) = 1;
				hit_tileElemAt(matA, 2, SIZE - 1, j) = 2;
			}
			for (int i = 0; i < SIZE; i++)
			{
				hit_tileElemAt(matA, 2, i, 0) = 3;
				hit_tileElemAt(matA, 2, i, SIZE - 1) = 4;
			}

			/*Controller*/
			CALCntrl commA;
			CAL_CntrlCreate(&commA, CAL_CNTRL_GPU, GPU);

			cudaStream_t stream1, stream2;
			cudaStreamCreate(&stream1);
			cudaStreamCreate(&stream2);

			CAL_CntrlAttach(&commA, (HitTile *)&matA);
			CAL_CntrlAttach(&commA, (HitTile *)&copyA);
			CAL_CntrlSync(commA);

			TimerStart(tTotalA);

			/*Copia a device*/
			CAL_CntrlMoveTo(&commA, (HitTile *)&matA);
			CAL_CntrlLaunch(commA, Copy, threads, copyA, matA);
			CAL_CntrlLaunch(commA, Update, update, matA, copyA);
			CAL_CntrlSync(commA);

			// 7. Launch the task: invoking the kerneli
			for (int i = 0; i < STAGES - 1; i++)
			{

				CAL_CntrlMoveFrom(&commA, (HitTile *)&matA);

				CAL_CntrlLaunch(commA, Copy, threads, copyA, matA);
				CAL_CntrlLaunch(commA, Update, update, matA, copyA);
				CAL_CntrlSync(commA);
			}
			CAL_CntrlDetach(&commA, (HitTile *)&matA);
			CAL_CntrlDestroy(&commA);
			TimerStop(tTotalA);

			printf("\nVERSION A_SINCRONA************\n");
			printf("SIZE %d %d\n", SIZE, STAGES);
			printf("Clock Main %.8lf <--\n", TimerGetTime(tTotalA));
			printf("\nPorcentaje de mejora (%d x %d; ier %d) %.3lf%\n", SIZE, SIZE, STAGES, (100.0 - (TimerGetTime(tTotalA) * 100.0) / TimerGetTime(tTotal)));

			// 11. Calculate NORM
			resultado = 0, suma = 0;
			for (int i = 0; i < SIZE; i++)
			{
				for (int j = 0; j < SIZE; j++)
				{
					suma += pow(hit_tileElemAt(matA, 2, i, j), 2);
				}
			}
			//printf("\n ----------------------- NORM ----------------------- \n");
			//printf("\n Acumulated sum: %lf",suma);

			//resultado=sqrt( suma );
			//printf("\n Result: %lf \n",resultado);

			//printf("\n ---------------------------------------------------- \n");

			/*Free*/
			hit_tileFree(matA);
			hit_tileFree(copyA);
			cudaStreamDestroy(stream1);
			cudaStreamDestroy(stream2);

			// 13. Finish the controller library
			CAL_CntrlFinish();

			return 0;
		}
