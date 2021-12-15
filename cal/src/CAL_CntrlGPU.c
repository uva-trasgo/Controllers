
#include "CAL_CntrlGPU.h"

#include <cuda.h>
#include <assert.h>
#include <cuda_runtime.h>
#include <hit_ktile.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#define NOTIME

/***************************************
 ******** Auxiliar functions **********
 **************************************/
// #define CUDA_CHECK() { cudaError_t error; if ((error = cudaGetLastError())!=cudaSuccess){ fprintf(stdout,"%s: %s\n", cudaGetErrorName(error),cudaGetErrorString(error)); fflush(stdout);exit(EXIT_FAILURE); } }
#define CUDA_CHECK()                                                                                                                     \
	{                                                                                                                                    \
		cudaError_t error;                                                                                                               \
		if ((error = cudaGetLastError()) != cudaSuccess)                                                                                 \
		{                                                                                                                                \
			fprintf(stdout, "CUDA Error at: %s::%d\n %s: %s\n", __FILE__, __LINE__, cudaGetErrorName(error), cudaGetErrorString(error)); \
			fflush(stdout);                                                                                                              \
			fflush(stderr);                                                                                                              \
			exit(EXIT_FAILURE);                                                                                                          \
		}                                                                                                                                \
	}

/*************************************************************
 ******** Prototypes of tasks' evaluation functions **********
 *************************************************************/
void CAL_CntrlGPUEvalTaskKernelLaunch(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskKernelLaunchAsync(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskAttachTile(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskAttachTileAsync(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskDetachTile(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskDetachTileAsync(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskMoveTo(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskMoveToAsync(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskMoveFrom(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskMoveFromAsync(CALCntrlGPU *, CAL_Task *);
void CAL_CntrlGPUEvalTaskWaitTile(CALCntrlGPU *, CAL_Task *);

/*************************************************************
 ************ Controller callback functions ******************
 *************************************************************/
void CUDART_CB CAL_CntrlGPUCallbackKernelLaunch(cudaStream_t stream, cudaError_t status, void *data);
void CUDART_CB CAL_CntrlGPUCallbackMoveTo(cudaStream_t stream, cudaError_t status, void *data);
void CUDART_CB CAL_CntrlGPUCallbackMoveFrom(cudaStream_t stream, cudaError_t status, void *data);

/********************************************
 ******** GPU Controller functions **********
 ********************************************/

/*
 * Create the controller and its corresponding variables
 * Params:
 *      cntrl: Controller to be created.
 *      device: GPU used by this controller
 */
void CAL_CntrlGPUCreate(CALCntrlGPU *cntrl, int device)
{

	/* Select policy of controller */
	if (getenv("CAL_CNTRL_ASYNC_MODE"))
	{
		cntrl->isAsync = atoi(getenv("CAL_CNTRL_ASYNC_MODE"));
	}
	else
	{
		cntrl->isAsync = 0;
	}
	/* Allocate the controller */
	CAL_TaskQueue_init(&(cntrl->listTask));
	sem_init(&(cntrl->semHost), 0, 0);
	sem_init(&(cntrl->semCntrl), 0, 0);
	CAL_Task task = CAL_TASK_NULL;

	// Assign device
	cntrl->numGPU = device;
	cudaSetDevice(cntrl->numGPU);
	cudaStreamCreate(&(cntrl->streamGPU));
#ifdef DEBUG
	CUDA_CHECK();
#endif

	// USe the determined GPU
	cudaSetDevice(cntrl->numGPU);
	cudaDeviceSynchronize();
#ifdef DEBUG
	CUDA_CHECK();
#endif

	/* Reserva la memoria. */
	cntrl->attachedTiles = (CALTileGPU *)malloc(MAX_TILES * sizeof(CALTileGPU));
	cntrl->internalTiles = (CALTileGPU *)malloc(MAX_TILES * sizeof(CALTileGPU));

	/* Inicializa la estructura de datos. */
	cntrl->nAttachedTiles = 0;
	cntrl->nInternalTiles = 0;

#ifdef TIME
	HitClock Transfer_HToD_Clock;
	HitClock Transfer_DToH_Clock;
	HitClock kernelClock;
	HitClock TTAsyn;

	hit_clockReset(Transfer_HToD_Clock);
	hit_clockReset(Transfer_DToH_Clock);
	hit_clockReset(kernelClock);
	hit_clockReset(TTAsyn);

#endif

#ifdef DEBUG
	printf("Creado comunicador de GPU %d.\n", cntrl->numGPU);
	fflush(stdout);
	fflush(stdout);
#endif
	/* Initialize the data structure */
	cntrl->ntask = 0;

	//omp_init_lock(&(cntrl->destroyCntrl));
	//omp_init_lock(&(cntrl->lockQueue));
	//// Blocking controller
	//omp_set_lock(&(cntrl->destroyCntrl));

	omp_init_lock(&(cntrl->destroyCntrl));
	omp_init_lock(&(cntrl->syncGlobalCntrl));
	omp_init_lock(&(cntrl->lockQueue));
	// Blocking controller
	omp_set_lock(&(cntrl->destroyCntrl));
	//omp_set_lock(&(cntrl->syncGlobalCntrl));

#pragma omp task
	{
		{
			{
				if (cntrl->isAsync)
				{
#ifdef DEBUG
					printf("\n Start Controller at async mode\n");
					fflush(stdout);
#endif
					CAL_CntrlGPUConsumeTaskQueueAsync(cntrl);
				}
				else
				{
#ifdef DEBUG
					printf("\n Start Controller at sync mode\n");
					fflush(stdout);
#endif
					CAL_CntrlGPUConsumeTaskQueueSync(cntrl);
				}
			}
		}
#ifdef DEBUG
		printf("Controller destroyed\n");
		fflush(stdout);
#endif

		// Once the controller is detroyed, clean the queue and unset locks
		omp_set_lock(&(cntrl->lockQueue));
		CAL_TaskQueue_clean(&(cntrl->listTask));
		omp_unset_lock(&(cntrl->lockQueue));
		omp_unset_lock(&(cntrl->destroyCntrl));
		sem_destroy(&(cntrl->semHost));
		sem_destroy(&(cntrl->semCntrl));
	}
}

/*
 * Synchornous policy to consume queue of tasks 
 * Params:
 *      cntrl: instance ofController.
 */
void CAL_CntrlGPUConsumeTaskQueueSync(CALCntrlGPU *cntrl)
{
	int taskDone = 0;
	int FlagFinish = 0;

#ifdef TIME
	hit_clockContinue(TTAsyn);
#endif

	// TODO improve the active wait using semaphores
	// While the controller has not been destroyed
	while (FlagFinish == 0)
	{

#ifdef DEBUG
//       printf("\n Waiting task taskDone:%d. \n", taskDone); fflush(stdout);
#endif

		// If there are tasks in the queue
		// #pragma omp flush(cntrl)
		if (!CAL_TaskQueue_is_empty(&(cntrl->listTask)))
		{

			// Take the first task --> an improvement can be to make a better choice (not the first in the queue)
			omp_set_lock(&(cntrl->lockQueue));
			CAL_Task *run = CAL_TaskQueue_pointer_pop(&(cntrl->listTask));
			omp_unset_lock(&(cntrl->lockQueue));
#ifdef DEBUG
			printf("\n Task chosen: %d \n", (run)->label);
			fflush(stdout);
#endif

			// Evaluation of task
			switch ((run)->label)
			{
			case CAL_TASK_TYPE_KERNEL: // Task executing a Kernel
				CAL_CntrlGPUEvalTaskKernelLaunch(cntrl, run);
				// Counting the number of executed kernelsCAL_TaskQueue_push_CKernel
				taskDone++;
				break;
			case CAL_TASK_TYPE_DESTROYCNTRL: // Last task for destroying the Controller
#ifdef DEBUG
				printf("Destroying Controller.\n");
				fflush(stdout);
#endif
				cudaDeviceSynchronize();
				FlagFinish = 1;
				// Counting the number of executed kernels
				// taskDone++;
				break;
			case CAL_TASK_TYPE_GLOBALSYNC: // Sincronización global con el Host
#ifdef DEBUG
				printf("Global synchronize Controller.\n");
				fflush(stdout);
#endif
				// Counting the number of executed kernels
				CAL_CntrlGPUEvalTaskGlobalSync(cntrl, run);
				taskDone++;
				break;
			case CAL_TASK_TYPE_ATTACH: // Task for attaching a tile
				CAL_CntrlGPUEvalTaskAttachTile(cntrl, run);
				// Counting the number of executed kernels
				// taskDone++;
				break;
			case CAL_TASK_TYPE_DETACH: // Task for detaching a tile
				CAL_CntrlGPUEvalTaskDetachTile(cntrl, run);
				// Counting the number of executed kernels
				taskDone++;
				break;
			case CAL_TASK_TYPE_MOVETO: /*Tarea de movimiento al Host (sin destruir memoria)*/
				CAL_CntrlGPUEvalTaskMoveTo(cntrl, run);
				// Counting the number of executed kernels
				// taskDone++;
				break;
			case CAL_TASK_TYPE_MOVEFROM: /*Tarea de movimiento al Host (sin destruir memoria)*/
				CAL_CntrlGPUEvalTaskMoveFrom(cntrl, run);
				// Counting the number of executed kernels
				// taskDone++;
				break;
			case CAL_TASK_TYPE_WAITTILE: // Waiting for HitTile's stream
				CAL_CntrlGPUEvalTaskWaitTile(cntrl, run);
				break;
			default:
				break;
			}
		}
	} //While
}

/*
 * Asynchornous policy to consume queue of tasks 
 * Params:
 *      cntrl: instance ofController.
 */
void CAL_CntrlGPUConsumeTaskQueueAsync(CALCntrlGPU *cntrl)
{
	int taskDone = 0;
	int FlagFinish = 0;

#ifdef TIME
	hit_clockContinue(TTAsyn);
#endif

	// TODO improve the active wait using semaphores
	// While the controller has not been destroyed
	while (FlagFinish == 0)
	{

#ifdef DEBUG
//       printf("\n Waiting task taskDone:%d. \n", taskDone); fflush(stdout);
#endif

		// If there are tasks in the queue
		// #pragma omp flush(cntrl)
		if (!CAL_TaskQueue_is_empty(&(cntrl->listTask)))
		{

			// Take the first task --> an improvement can be to make a better choice (not the first in the queue)
			omp_set_lock(&(cntrl->lockQueue));
			CAL_Task *run = CAL_TaskQueue_pointer_pop(&(cntrl->listTask));
			omp_unset_lock(&(cntrl->lockQueue));
#ifdef DEBUG
			printf("\n Task chosen: %d \n", (run)->label);
			fflush(stdout);
#endif

			// Evaluation of task
			switch ((run)->label)
			{
			case CAL_TASK_TYPE_KERNEL: // Task executing a Kernel
				CAL_CntrlGPUEvalTaskKernelLaunchAsync(cntrl, run);
				// Counting the number of executed kernelsCAL_TaskQueue_push_CKernel
				taskDone++;
				break;
			case CAL_TASK_TYPE_DESTROYCNTRL: // Last task for destroying the Controller
#ifdef DEBUG
				printf("Destroying Controller.\n");
				fflush(stdout);
#endif
				cudaDeviceSynchronize();
				FlagFinish = 1;
				// Counting the number of executed kernels
				// taskDone++;
				break;
			case CAL_TASK_TYPE_GLOBALSYNC: // Sincronización global con el Host
#ifdef DEBUG
				printf("Global synchronize Controller.\n");
				fflush(stdout);
#endif
				// Counting the number of executed kernels
				CAL_CntrlGPUEvalTaskGlobalSync(cntrl, run);
				// printf("HOLA\n"); fflush(stdout);
				// omp_unset_lock(&(cntrl->syncGlobalCntrl));
				// taskDone++;
				break;
			case CAL_TASK_TYPE_ATTACH: // Task for attaching a tile
				CAL_CntrlGPUEvalTaskAttachTileAsync(cntrl, run);
				// Counting the number of executed kernels
				// taskDone++;
				break;
			case CAL_TASK_TYPE_DETACH: // Task for detaching a tile
				CAL_CntrlGPUEvalTaskDetachTileAsync(cntrl, run);
				// Counting the number of executed kernels
				taskDone++;
				break;
			case CAL_TASK_TYPE_MOVETO: // Tarea de movimiento al Host (sin destruir memoria)
				CAL_CntrlGPUEvalTaskMoveToAsync(cntrl, run);
				// Counting the number of executed kernels
				// taskDone++;
				break;
			case CAL_TASK_TYPE_MOVEFROM: // Tarea de movimiento al Host (sin destruir memoria)
				CAL_CntrlGPUEvalTaskMoveFromAsync(cntrl, run);
				// Counting the number of executed kernels
				// taskDone++;
				break;
			case CAL_TASK_TYPE_WAITTILE: // Waiting for HitTile's stream
				CAL_CntrlGPUEvalTaskWaitTile(cntrl, run);
				break;
			default:
				break;
			}
		}
	} //While
}

/////////////////////////////////
// CONTROLLER EXTERNAL FUNCTIONS

/*
 * Destroy a Controller
 */
void CAL_CntrlGPUDestroy(CALCntrlGPU *cntrl)
{

	// USe the determined GPU
	cudaSetDevice(cntrl->numGPU);
	cudaDeviceSynchronize();

	// This function is executed by the master thread
	// The controller reachs the end, no more task would be added
	// Add a task with a value -2 in the taskEnd field
	// Allocating new task
	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_DESTROYCNTRL;
	newTask.position = (cntrl->ntask);

	//Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));

	//Wait for the syncronization and destroy locks
	omp_set_lock(&(cntrl->destroyCntrl));
	omp_destroy_lock(&(cntrl->destroyCntrl));
	omp_destroy_lock(&(cntrl->lockQueue));

	CAL_TaskQueue_clean(&(cntrl->listTask));

#ifdef DEBUG
	printf("Eliminando Comunicador de GPU %d.\n", cntrl->numGPU);
	fflush(stdout);
	fflush(stdout);
#endif
}

//Yuri punto de sincronización con el host
void CAL_CntrlGPUGlobalSync(CALCntrlGPU *cntrl)
{ //Yuri*************************

	// USe the determined GPU
	// cudaSetDevice(cntrl->numGPU);
	// cudaDeviceSynchronize();

	// This function is executed by the master thread
	// The controller reachs the end, no more task would be added
	// Add a task with a value -2 in the taskEnd field
	// Allocating new task
	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_GLOBALSYNC;
	newTask.position = (cntrl->ntask);

	//Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));

	//Wait for the global syncronization
	// omp_set_lock(&(cntrl->syncGlobalCntrl));
	sem_wait(&(cntrl->semHost));

#ifdef DEBUG
	printf("Esperando a Comunicador de GPU %d.\n", cntrl->numGPU);
	fflush(stdout);
	fflush(stdout);
#endif
} //CAL_CntrlGPUGlobalSync

/*
 * Attach a tile
 */
void CAL_CntrlGPUAttach(CALCntrlGPU *cntrl, HitTile *tile)
{

	if (tile->refCntrl != NULL)
	{
		fprintf(stderr, "CAL Runtime error - Attach: Variable already associated to a Cntrl\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	tile->refCntrl = (void *)cntrl;

	/* Flags */
	tile->inCntrlType = 1;
	tile->transferred = 0;
	tile->recover = 0;

	/* Launching a task in the queue to perform the actual data transfer*/
	// This function is executed by the master thread
	// The controller needs to attach a tile
	// Add a task with a value 3 in the taskEnd field
	// Allocating new task
	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_ATTACH;
	newTask.position = (cntrl->ntask);
	newTask.arguments = (HitTile *)tile;

	//Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));
}

/*
 * Attach a tile (async version)
 */
void CAL_CntrlGPUAttachAsyn(CALCntrlGPU *cntrl, HitTile *tile)
{

	CAL_CntrlAttach(cntrl, tile);
}

/*
 * Move a tile to device
 */
void CAL_CntrlGPUMoveTo(CALCntrlGPU *cntrl, HitTile *tile)
{

	if (tile->refCntrl == NULL)
	{
		fprintf(stderr, "CAL Runtime error - MoveTo: Variable Whitout device memory\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	tile->refCntrl = (void *)cntrl;

	/* Flags */
	tile->inCntrlType = 1;
	tile->transferred = 0;
	tile->recover = 0;

	/* Launching a task in the queue to perform the actual data transfer*/
	// This function is executed by the master thread
	// The controller needs to attach a tile
	// Add a task with a value 3 in the taskEnd field
	// Allocating new task
	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_MOVETO;
	newTask.position = (cntrl->ntask);
	newTask.arguments = (HitTile *)tile;

	//Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));

} // CAL_CntrlGPUMoveTo

/*
 * Move a tile to device (async version)
 */
void CAL_CntrlGPUMoveToAsyn(CALCntrlGPU *cntrl, HitTile *tile)
{

	CAL_CntrlGPUMoveTo(cntrl, tile);

} // CAL_CntrlGPUMoveToAsyn

/*
 * Detach a tile
 */
void CAL_CntrlGPUDetach(CALCntrlGPU *cntrl, HitTile *tile)
{

	if (tile->refCntrl != cntrl)
	{
		fprintf(stderr, "CAL Runtime error - Detach: Variable not attached to this Cntrl\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	if (tile->inCntrlType != 1)
	{
		fprintf(stderr, "CAL Runtime error - Detach: Not proper variable, perhaps internal?\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	/* Launching a task in the queue to perform the actual data transfer*/
	// This function is executed by the master thread
	// The controller needs to attach a tile
	// Add a task with a value 3 in the taskEnd field
	// Allocating new task
	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_DETACH;
	newTask.position = (cntrl->ntask);
	newTask.arguments = (HitTile *)tile;

	// Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));

} // CAL_CntrlGPUDetach

/*
 * Detach a tile (async version)
 */
void CAL_CntrlGPUDetachAsyn(CALCntrlGPU *cntrl, HitTile *tile)
{

	CAL_CntrlGPUDetach(cntrl, tile);

} // CAL_CntrlGPUDetachAsyn

/*
 * Move a tile from device (async version)
 */
void CAL_CntrlGPUMoveFrom(CALCntrlGPU *cntrl, HitTile *tile)
{

	if (tile->refCntrl != cntrl)
	{
		fprintf(stderr, "CAL Runtime error - Detach: Variable not attached to this Cntrl\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	if (tile->inCntrlType != 1)
	{
		fprintf(stderr, "CAL Runtime error - Detach: Not proper variable, perhaps internal?\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	/* Launching a task in the queue to perform the actual data transfer*/
	// This function is executed by the master thread
	// The controller needs to attach a tile
	// Add a task with a value 3 in the taskEnd field
	// Allocating new task
	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_MOVEFROM;
	newTask.position = (cntrl->ntask);
	newTask.arguments = (HitTile *)tile;

	//Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));

} //CAL_CntrlGPUMoveFrom

/*
 * Move a tile from device (async version)
 */
void CAL_CntrlGPUMoveFromAsync(CALCntrlGPU *cntrl, HitTile *tile)
{

	CAL_CntrlGPUMoveFrom(cntrl, tile);

} // CAL_CntrlGPUMoveFromAsync

// TODO: created by ismael @ 2017-12-15 14:06:00
// Implement async version
/*
 * Create internally a data structure
 */
void CAL_CntrlGPUInternal(CALCntrlGPU *cntrl, HitTile *tile)
{

	if (tile->refCntrl != NULL)
	{
		fprintf(stderr, "CAL Runtime error - InternalCreate: Variable already associated to a Cntrl\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	tile->refCntrl = (void *)cntrl;

	/* Crea la memoria en el dispositivo. */
	cudaMalloc((void **)&(tile->devData), (size_t)tile->acumCard * tile->baseExtent);
#ifdef DEBUG
	CUDA_CHECK();
	printf("Create internal: %p\n", tile->devData);
	fflush(stdout);
#endif

	/* Marcas */
	tile->inCntrlType = 2;
	tile->transferred = 1;
	tile->recover = 0;

	/* Launching a task in the queue to perform the actual data transfer*/
	// This function is executed by the master thread
	// The controller needs to attach a tile
	// Add a task with a value 3 in the taskEnd field
	// Allocating new task
	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_ATTACH;
	newTask.position = (cntrl->ntask);
	newTask.arguments = (HitTile *)tile;

	//Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));
}

// TODO: created by ismael @ 2017-12-15 14:06:36
// Implement async version
/*
 * Destroy Internal Function: recover is use dto choice between only free the memory or also take back the data 
 */
void CAL_CntrlGPUDestroyInternal(CALCntrlGPU *cntrl, HitTile *tile, int recover)
{
	if (tile->refCntrl != cntrl)
	{
		fprintf(stderr, "CAL Runtime error - InternalDestroy: Variable not associated to this Cntrl\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	if (tile->inCntrlType != 2)
	{
		fprintf(stderr, "CAL Runtime error - InternalDestroy: Not proper variable, perhaps attached?\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}

	/* Launching a task in the queue to perform the actual data transfer*/
	// This function is executed by the master thread
	// The controller needs to attach a tile
	// Add a task with a value 3 in the taskEnd field
	// Allocating new task
	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_DETACH;
	newTask.position = (cntrl->ntask);
	newTask.arguments = (HitTile *)tile;
	tile->recover = recover;

	//Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));
}

// TODO: created by ismael @ 2017-12-15 14:07:04
// Mask stream's use
// FIXME: noticed by ismael @ 2018-1-11 13:43:39
// Unused
//yuri
/*
 * WaitTile Function: block host thread execution until communications of tiles are finished
 */
void CAL_CntrlGPUWaitTile(CALCntrlGPU *cntrl, HitTile *tile)
{

	CAL_Task newTask = CAL_TASK_NULL;
	newTask.label = CAL_TASK_TYPE_WAITTILE;
	newTask.position = (cntrl->ntask);

	// cudaStream_t stream;
	// CAL_CntrlGPUGetTileHandler(cntrl, tile, &stream);
	// newTask.stream= stream;
	newTask.arguments = (HitTile *)tile;
	//Add task to the cntrl
	omp_set_lock(&(cntrl->lockQueue));
	CAL_TaskQueue_push(&(cntrl->listTask), newTask);
	cntrl->ntask++;
	omp_unset_lock(&(cntrl->lockQueue));

	sem_wait(&(cntrl->semHost));
}

///////////////////////
// HANDLER MANAGEMENT

// TODO: created by ismael @ 2017-12-17 18:44:34
// Use own stream for each tile, instead of using stream of controller
/*
 * Get tile's stream: retrieve stream of tile or create new one otherwise.
 */
void CAL_CntrlGPUGetTileHandler(CALCntrlGPU *cntrl, HitTile *tile, CAL_Request **prequest)
{
	if (tile->handler == NULL)
	{
		fprintf(stderr, "CAL Runtime error - GetOrCreateTileHandler: Tile create handler failed!\n");
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	*prequest = ((CAL_Request *)(tile->handler));
}

/*
 * Initialize tile's handler.
 */
void CAL_CntrlGPUCreateTileHandler(CALCntrlGPU *cntrl, CAL_Request **prequest)
{
	if (*prequest == NULL)
	{
		*prequest = (CAL_Request *)malloc(sizeof(CAL_Request));
		if (*prequest == NULL)
		{
			fprintf(stderr, "CAL Runtime error - RequestCreate: Request malloc failed!\n");
			fflush(stdout);
			exit(EXIT_FAILURE);
		}
		**prequest = (CAL_Request){NULL, NULL};
	}
	else
	{
		fprintf(stderr, "CAL Runtime error - RequestCreate: Request already create (0x%X)!\n", prequest);
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	cudaSetDevice(cntrl->numGPU);
	if ((*prequest)->stream == NULL)
	{
		(*prequest)->stream = (cudaStream_t *)malloc(sizeof(cudaStream_t));
		if ((*prequest)->stream == NULL)
		{
			fprintf(stderr, "CAL Runtime error - RequestCreate: Request stream malloc failed!\n");
			fflush(stdout);
			exit(EXIT_FAILURE);
		}
		cudaStreamCreate((cudaStream_t *)(*prequest)->stream);
#ifdef DEBUG
		CUDA_CHECK();
#endif
	}
	if ((*prequest)->event == NULL)
	{
		(*prequest)->event = (cudaEvent_t *)malloc(sizeof(cudaEvent_t));
		if ((*prequest)->event == NULL)
		{
			fprintf(stderr, "CAL Runtime error - RequestCreate: Request stream malloc failed!\n");
			fflush(stdout);
			exit(EXIT_FAILURE);
		}
		cudaEventCreate((cudaEvent_t *)(*prequest)->event);
#ifdef DEBUG
		CUDA_CHECK();
#endif
	}
}

/*
 * Destroy tile's stream: destroy stream of tile or create new one otherwise.
 */
void CAL_CntrlGPUDestroyTileHandler(CALCntrlGPU *cntrl, HitTile *tile)
{
	cudaSetDevice(cntrl->numGPU);
	if (tile->handler != NULL)
	{
		CAL_CntrlGPURequestDestroy(cntrl, (CAL_Request *)tile->handler);
#ifdef DEBUG
		CUDA_CHECK();
#endif
		free(tile->handler);
	}
	tile->handler = NULL;
}

/* 
 * Destroy request's stream
 */
void CAL_CntrlGPURequestDestroy(CALCntrlGPU *cntrl, CAL_Request *request)
{
	cudaSetDevice(cntrl->numGPU);
	if (request->stream != NULL)
	{
		cudaStreamDestroy(*((cudaStream_t *)request->stream));
#ifdef DEBUG
		CUDA_CHECK();
#endif
	}
	free(request->stream);
	if (request->event != NULL)
	{
		cudaEventDestroy(*((cudaEvent_t *)request->event));
#ifdef DEBUG
		CUDA_CHECK();
#endif
	}
	free(request->event);
}

///////////////////////////////////
// TASKS'S EVALUATION FUNCTIONS

void CAL_CntrlGPUEvalTaskGlobalSync(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("\n Executing global sync. \n");
	fflush(stdout);
#endif

	cudaSetDevice(cntrl->numGPU);

	cudaDeviceSynchronize();

	sem_post(&(cntrl->semHost));

#ifdef DEBUG
	CUDA_CHECK();
	printf("\n Executing global sync. \n");
	fflush(stdout);
#endif
}

/* 
 * Evaluation of kernel launchs
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskKernelLaunch(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("\n Executing work. \n");
	fflush(stdout);
#endif

	cudaSetDevice(cntrl->numGPU);

	for (int i = 0; i < run->narguments; i++)
	{
#ifdef DEBUG
		printf("\t[%d] Role: %d\n",
			   i, run->roles[i]);
		fflush(stdout);
#endif
		if (run->roles[i] != CAL_INVAL)
		{
			HitTile *ptile = run->pointers[i];
			KHitTile *pktile = ((uint8_t *)run->arguments + run->displacements[i]);

			// Fix device memory pointer in case does not match
			if (ptile->devData != pktile->data)
			{
#ifdef DEBUG
				printf("\t[%d]At kernel launch: difference found at %p tile parameter %p data pointer: pointer: %p tile: %p\n",
					   i, pktile, ptile, ptile->devData, pktile->data);
				fflush(stdout);
#endif
				pktile->data = ptile->devData;
			}
		}
	}

#ifdef DEBUG
	for (int i = 0; i < run->narguments; i++)
	{
		if (run->roles[i] != CAL_INVAL)
		{
			HitTile *ptile = run->pointers[i];
			printf("\t[%d]Kernel launch ptile %p: h: %p d: %p\n", i, ptile, ptile->data, ptile->devData);
			KHitTile *pktile = ((uint8_t *)run->arguments + run->displacements[i]);
			printf("\t[%d]Kernel launch tile %p: d: %p\n", i, pktile, pktile->data);
		}
	}
#endif

	// Malloc memory of task to be accesible by device callbacks' driver
	// CAL_Task* ptask = malloc(sizeof(CAL_Task));
	// *ptask = run;

	run->kernelWrapper(cntrl->streamGPU, dGPU, run->deviceId, run->threads, run->arguments);
	// cudaDeviceSynchronize();

#ifdef DEBUG
	CUDA_CHECK();
	printf("\n Executed work. \n");
	fflush(stdout);
#endif
}

/* 
 * Evaluation of kernel launchs
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskKernelLaunchAsync(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("\n Executing work. \n");
	fflush(stdout);
#endif

	cudaSetDevice(cntrl->numGPU);

	for (int i = 0; i < run->narguments; i++)
	{
#ifdef DEBUG
		printf("\t[%d] Role: %d\n",
			   i, run->roles[i]);
		fflush(stdout);
#endif
		if (run->roles[i] != CAL_INVAL)
		{
			HitTile *ptile = run->pointers[i];
			KHitTile *pktile = ((uint8_t *)run->arguments + run->displacements[i]);

			// Fix device memory pointer in case does not match
			if (ptile->devData != pktile->data)
			{
#ifdef DEBUG
				printf("\t[%d]At kernel launch: difference found at %p tile parameter %p data pointer: pointer: %p tile: %p\n",
					   i, pktile, ptile, ptile->devData, pktile->data);
				fflush(stdout);
#endif
				pktile->data = ptile->devData;
			}

			// Block kernel launch in case output tile has pending communication
			if (run->roles[i] != CAL_OUT)
			{

// CRITICAL ZONE BEGIN: kernel dependency on input tile communication
#ifdef DEBUG
				printf("KernelLaunch entering critical zone\n");
				fflush(stdout);
#endif //DEBUG

#pragma omp atomic write
				ptile->isCntrlBlocked = 1;

				if (ptile->hasPendingMoveTo)
				{ // Check if tile has pending communication
// Set flag of controller thread blocked by tile to true
#ifdef DEBUG
					printf("KernelLaunch tile (%p) has pending comm. Locking cntrl thread\n", ptile);
					fflush(stdout);
#endif //DEBUG

					// Unlock critical zone
					sem_wait(&(cntrl->semCntrl));

#ifdef DEBUG
					printf("KernelLaunch tile (%p) comm finished. Cntrl thread unlocked\n", ptile);
					fflush(stdout);
#endif //DEBUG
				}
				else
				{ // Otherwise
#pragma omp atomic write
					ptile->isCntrlBlocked = 0;
				}
#ifdef DEBUG
				printf("KernelLaunch leaving critical zone\n");
				fflush(stdout);
#endif			//DEBUG
				// CRITICAL ZONE END
			}

			if (run->roles[i] != CAL_IN)
			{
#pragma omp atomic update
				(ptile->cntrlKernelCount)++;
			}
		}
	}

#ifdef DEBUG
	for (int i = 0; i < run->narguments; i++)
	{
		if (run->roles[i] != CAL_INVAL)
		{
			HitTile *ptile = run->pointers[i];
			printf("\t[%d]Kernel launch ptile %p: h: %p d: %p\n", i, ptile, ptile->data, ptile->devData);
			KHitTile *pktile = ((uint8_t *)run->arguments + run->displacements[i]);
			printf("\t[%d]Kernel launch tile %p: d: %p\n", i, pktile, pktile->data);
		}
	}
#endif

	// Malloc memory of task to be accesible by device callbacks' driver
	run->kernelWrapper(cntrl->streamGPU, dGPU, run->deviceId, run->threads, run->arguments);
	cudaStreamAddCallback(cntrl->streamGPU, CAL_CntrlGPUCallbackKernelLaunch, (void *)run, 0);

#ifdef DEBUG
	CUDA_CHECK();
	printf("\n Executed work. \n");
	fflush(stdout);
#endif
}

/* 
 * Evaluation of attach of tiles in controller
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskAttachTile(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("Attaching\n");
	fflush(stdout);
#endif
	cudaSetDevice(cntrl->numGPU);

	HitTile *tile = ((HitTile *)run->arguments);

	/* Allocate memory in the target device */
	cudaMalloc((void **)&(tile->devData), (size_t)tile->acumCard * tile->baseExtent);

#ifdef DEBUG
	CUDA_CHECK();
	printf("Data pointers Attach (%p): %p %p \n", tile, tile->data, tile->devData);
	fflush(stdout);
	printf("Data size Attach: %d %d \n", tile->acumCard, tile->baseExtent);
	fflush(stdout);
#endif

	CAL_CntrlGPUEvalTaskMoveTo(cntrl, run);
}

/* 
 * Evaluation of attach of tiles in controller
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskAttachTileAsync(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("Attaching\n");
	fflush(stdout);
#endif
	cudaSetDevice(cntrl->numGPU);

	HitTile *tile = ((HitTile *)run->arguments);

	if (tile->inCntrlType == 1)
	{
		/* Realloc host memory as pinned */
		void *temp_tile_data;
		void *aux_tile_data;
		cudaHostAlloc((void **)&(temp_tile_data), (size_t)tile->acumCard * tile->baseExtent, cudaHostAllocDefault);
		cudaMemcpy(temp_tile_data, tile->data, (size_t)tile->acumCard * tile->baseExtent, cudaMemcpyHostToHost);

		/* TODO: Avoid hardcode of set of tile members */
		hit_tileFree(*tile);
		tile->memPtr = tile->data = temp_tile_data;
		tile->memStatus = HIT_MS_OWNER;
	}

	/* Allocate memory in the target device */
	cudaMalloc((void **)&(tile->devData), (size_t)tile->acumCard * tile->baseExtent);

#ifdef DEBUG
	CUDA_CHECK();
	printf("Data pointers Attach (%p): %p %p \n", tile, tile->data, tile->devData);
	fflush(stdout);
	printf("Data size Attach: %d %d \n", tile->acumCard, tile->baseExtent);
	fflush(stdout);
#endif

	/* Allocate handler structure to tile */
	CAL_CntrlGPUCreateTileHandler(cntrl, (CAL_Request *)(&(tile->handler)));

	CAL_CntrlGPUEvalTaskMoveToAsync(cntrl, run);
}

/* 
 * Evaluation of detach of tiles from controller
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskDetachTile(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	CUDA_CHECK();
	printf("Detaching\n");
	fflush(stdout);
#endif

	// cudaDeviceSynchronize();

#ifdef DEBUG
	CUDA_CHECK();
#endif

	HitTile *tile = ((HitTile *)run->arguments);

	CAL_CntrlGPUEvalTaskMoveFrom(cntrl, run);

	cudaSetDevice(cntrl->numGPU);
	cudaDeviceSynchronize();
	cudaFree(tile->devData);

#ifdef DEBUG
	CUDA_CHECK();
	printf("Data (%p) free: h: %p d: %p\n", tile, tile->data, tile->devData);
	fflush(stdout);
#endif

	// FIXME: noticed by ismael @ 2017-12-21 17:08:24
	// Destroy stream of tile after deallocate tile
	// CAL_CntrlGPUDestroyTileStream(cntrl, tile);

	tile->devData = NULL;
	tile->refCntrl = NULL;
	tile->inCntrlType = 0;
}

/* 
 * Evaluation of detach of tiles from controller
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskDetachTileAsync(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	CUDA_CHECK();
	printf("Detaching\n");
	fflush(stdout);
#endif

	HitTile *tile = ((HitTile *)run->arguments);

	CAL_CntrlGPUEvalTaskMoveFromAsync(cntrl, run);
	cudaStreamSynchronize(*((cudaStream_t *)((CAL_Request *)tile->handler)->stream));

	cudaSetDevice(cntrl->numGPU);
	cudaFree(tile->devData);

#ifdef DEBUG
	CUDA_CHECK();
#endif

	if (tile->inCntrlType == 1)
	{
		/* Realloc host memory as conventional */
		void *temp_tile_data;
		void *aux_tile_data;

		/* TODO: Avoid hardcode of set of tile members */
		temp_tile_data = tile->data;
		tile->memPtr = NULL;
		tile->data = NULL;
		tile->memStatus = HIT_MS_NOMEM;
		hit_tileAlloc(tile);

		cudaMemcpy(tile->data, temp_tile_data, (size_t)tile->acumCard * tile->baseExtent, cudaMemcpyHostToHost);

		/* Free allocate conventional memory in host */
		cudaFreeHost(temp_tile_data);
	}

#ifdef DEBUG
	CUDA_CHECK();
	printf("Data (%p) free: h: %p d: %p\n", tile, tile->data, tile->devData);
	fflush(stdout);
#endif

	// FIXME: noticed by ismael @ 2017-12-21 17:08:24
	// Destroy stream of tile after deallocate tile
	CAL_CntrlGPUDestroyTileHandler(cntrl, tile);

	tile->devData = NULL;
	tile->refCntrl = NULL;
	tile->inCntrlType = 0;
}

/* 
 * Evaluation of comunication of tile from host to device
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskMoveTo(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("Moving to\n");
	fflush(stdout);
#endif
	cudaSetDevice(cntrl->numGPU);

	// cudaDeviceSynchronize();

	HitTile *tile = ((HitTile *)run->arguments);

	/* Communication of HitTile */
	if (tile->transferred != 1)
	{
#ifdef DEBUG
		printf("Move to (%p): %p %p \n", tile, tile->data, tile->devData);
		fflush(stdout);
#endif

		// TODO: Yuri: Hacer la transferencia asincrona.
		// Habra otra tarea que espere a la transferencia.
		cudaMemcpyAsync(tile->devData, tile->data, tile->acumCard * tile->baseExtent, cudaMemcpyHostToDevice, cntrl->streamGPU);
		// cudaDeviceSynchronize();
		tile->transferred = 1;
		tile->recover = 1;
	}
#ifdef DEBUG
	CUDA_CHECK();
	printf("Data (%p) moved to device: h: %p d: %p  \n", tile, tile->data, tile->devData);
	fflush(stdout);
#endif
}

/* 
 * Evaluation of comunication of tile from host to device
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskMoveToAsync(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("Moving to\n");
	fflush(stdout);
#endif
	cudaSetDevice(cntrl->numGPU);

	HitTile *tile = ((HitTile *)run->arguments);

	/* Communication of HitTile */
	CAL_Request *outrequest = CAL_REQUEST_NULL;
	CAL_CntrlGPUGetTileHandler(cntrl, tile, &outrequest);

	if (tile->transferred != 1)
	{
#ifdef DEBUG
		printf("Move to (%p): %p %p \n", tile, tile->data, tile->devData);
		fflush(stdout);
#endif

// CRITICAL ZONE BEGIN: kernel dependency on input tile communication
#ifdef DEGUB
		printf("MoveTo entering critical zone\n");
		fflush(stdout);
#endif //DEGUB

#pragma omp atomic write
		tile->hasPendingMoveTo = 1;

#ifdef DEBUG
		printf("MoveTo leaving critical zone\n");
		fflush(stdout);
#endif //DEBUG
		// CRITICAL ZONE END

		// TODO: Yuri: Hacer la transferencia asincrona.
		// Habra otra tarea que espere a la transferencia.
		cudaMemcpyAsync(tile->devData, tile->data, tile->acumCard * tile->baseExtent, cudaMemcpyHostToDevice, (cudaStream_t) * ((cudaStream_t *)(outrequest->stream)));
		cudaStreamAddCallback((cudaStream_t) * ((cudaStream_t *)(outrequest->stream)), CAL_CntrlGPUCallbackMoveTo, (void *)tile, 0);
		tile->transferred = 1;
		tile->recover = 1;
	}
#ifdef DEBUG
	CUDA_CHECK();
	printf("Data (%p) moved to device: h: %p d: %p  \n", tile, tile->data, tile->devData);
	fflush(stdout);
#endif
}

/* 
 * Evaluation of comunication of tile from device to host
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskMoveFrom(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("Moving from\n");
	fflush(stdout);
#endif

	cudaSetDevice(cntrl->numGPU);

	// cudaDeviceSynchronize();

	HitTile *tile = ((HitTile *)run->arguments);

	if (tile->recover != 0)
	{

#ifdef DEBUG
		printf("Move from (%p): %p %p \n", tile, tile->data, tile->devData);
		fflush(stdout);
#endif

		cudaMemcpyAsync(tile->data, tile->devData, tile->acumCard * tile->baseExtent, cudaMemcpyDeviceToHost, cntrl->streamGPU);
		// cudaDeviceSynchronize();
		// tile->recover = 0;
	}

#ifdef DEBUG
	CUDA_CHECK();
	printf("Data (%p) moved from device: h: %p d: %p  \n", tile, tile->data, tile->devData);
	fflush(stdout);
#endif
}

/* 
 * Evaluation of comunication of tile from device to host
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskMoveFromAsync(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("Moving from\n");
	fflush(stdout);
#endif

	cudaSetDevice(cntrl->numGPU);

	HitTile *tile = ((HitTile *)run->arguments);

	/* Communication of HitTile */
	CAL_Request *outrequest = CAL_REQUEST_NULL;
	CAL_CntrlGPUGetTileHandler(cntrl, tile, &outrequest);

	if (tile->recover != 0)
	{

#ifdef DEBUG
		printf("Move from (%p): %p %p \n", tile, tile->data, tile->devData);
		fflush(stdout);
#endif

#ifdef DEBUG
		printf("MoveFrom entering critical zone (%p): Kernel count: %d isCntrlBlocked: %d\n", tile, tile->cntrlKernelCount, tile->isCntrlBlocked);
		fflush(stdout);
#endif //DEBUG

#pragma omp atomic write
		tile->isCntrlBlocked = 1;

		if (tile->cntrlKernelCount > 0)
		{
#ifdef DEBUG
			printf("MoveFrom tile (%p) has pending kernel. Locking cntrl thread. isCntrlBlocked: %d\n", tile, tile->isCntrlBlocked);
			fflush(stdout);
#endif //DEBUG
			sem_wait(&(cntrl->semCntrl));

#ifdef DEBUG
			printf("MoveFrom tile (%p) kernel finished. Cntrl thread unlocked\n", tile);
			fflush(stdout);
#endif //DEBUG
		}
		else
		{
#pragma omp atomic write
			tile->isCntrlBlocked = 0;
		}
#ifdef DEBUG
		printf("MoveFrom leaving critical zone\n");
		fflush(stdout);
#endif //DEBUG

#pragma omp atomic write
		tile->hasPendingMoveFrom = 1;

		cudaMemcpyAsync(tile->data, tile->devData, tile->acumCard * tile->baseExtent, cudaMemcpyDeviceToHost, (cudaStream_t) * ((cudaStream_t *)outrequest->stream));
		cudaStreamAddCallback((cudaStream_t) * ((cudaStream_t *)(outrequest->stream)), CAL_CntrlGPUCallbackMoveFrom, (void *)tile, 0);
		// tile->recover = 0;
	}

#ifdef DEBUG
	CUDA_CHECK();
	printf("Data (%p) moved from device: h: %p d: %p  \n", tile, tile->data, tile->devData);
	fflush(stdout);
#endif
}

/* 
 * Evaluation of synchronization with tile.
 * Params:
 *      cntrl: Controller in charge of task.
 *      run: task that must be evaluated.
 */
void CAL_CntrlGPUEvalTaskWaitTile(CALCntrlGPU *cntrl, CAL_Task *run)
{

#ifdef DEBUG
	printf("Synchronize with tile\n");
	fflush(stdout);
#endif

	HitTile *tile = (HitTile *)run->arguments;

#ifdef DEBUG
	printf("WaitTile entering critical zone\n");
	fflush(stdout);
#endif //DEBUG

#pragma omp atomic write
	tile->isCntrlBlocked = 1;

	if (tile->hasPendingMoveFrom)
	{
#ifdef DEBUG
		printf("WaitTile tile (%p) has pending comm MoveFrom. Locking cntrl thread\n", tile);
		fflush(stdout);
#endif //DEBUG
		sem_wait(&(cntrl->semCntrl));

#ifdef DEBUG
		printf("WaitTile tile (%p) comm MoveFrom finished. Cntrl thread unlocked\n", tile);
		fflush(stdout);
#endif //DEBUG
	}
	else
	{
#pragma omp atomic write
		tile->isCntrlBlocked = 0;
	}
#ifdef DEBUG
	printf("WaitTile leaving critical zone\n");
	fflush(stdout);
#endif //DEBUG

	sem_post(&(cntrl->semHost));
}

///////////////////////////////////
// TASKS' CALLBACK FUNCTIONS

/* 
 * Kernel callback
 * Params:
 * 		event: CUDA stream of previous execution.
 * 		status: CUDA status of previous execution.
 * 		data: callback data.
 */
void CUDART_CB CAL_CntrlGPUCallbackKernelLaunch(cudaStream_t stream, cudaError_t status, void *data)
{
// Check status of GPU after stream operations are done
#ifdef DEBUG
	CUDA_CHECK();
#endif

	CAL_Task *run = (CAL_Task *)data;
#ifdef DEBUG
	printf("Hi i am a kernel callback.\n");
	fflush(stdout);
#endif

	for (int i = 0; i < run->narguments; i++)
	{
		if (run->roles[i] != CAL_INVAL)
		{
			if (run->roles[i] != CAL_IN)
			{
				HitTile *ptile = run->pointers[i];

#ifdef DEBUG
				printf("Kernel Callback entering critical zone. tile (%p)\n", ptile);
				fflush(stdout);
#endif //DEBUG

#pragma omp atomic update
				ptile->cntrlKernelCount--;

#ifdef DEBUG
				int semCntrlValue;
				sem_getvalue(&(((CALCntrlGPU *)ptile->refCntrl)->semCntrl), &semCntrlValue);
				printf("Kernel Callback tile (%p): Kernel count: %d isCntrlBlocked: %d semCntrl: %d\n", ptile, ptile->cntrlKernelCount, ptile->isCntrlBlocked, semCntrlValue);
				fflush(stdout);
#endif //DEBU

				if (((ptile->cntrlKernelCount) == 0) && (ptile->isCntrlBlocked))
				{
#ifdef DEBUG
					printf("Kernel Callback cntrl is locked. Unlocking\n");
					fflush(stdout);
#endif //DEBUG

// Set flag of controller thread blocked by tile to false
#pragma omp atomic write
					ptile->isCntrlBlocked = 0;
					sem_post(&(((CALCntrlGPU *)ptile->refCntrl)->semCntrl));
				}

#ifdef DEBUG
				printf("Kernel Callback leaving critical zone. tile (%p)\n", ptile);
				fflush(stdout);
#endif //DEBUG
			}
		}
	}

	// Free the executed task
	free(run->roles);
	free(run->pointers);
	free(run->arguments);
	free(run);
}

/* 
 * Communication MoveTo callback
 * Params:
 * 		event: CUDA stream of previous execution.
 * 		status: CUDA status of previous execution.
 * 		data: callback data.
 */
void CUDART_CB CAL_CntrlGPUCallbackMoveTo(cudaStream_t stream, cudaError_t status, void *data)
{
// Check status of GPU after stream operations are done
#ifdef DEBUG
	CUDA_CHECK();
#endif

	HitTile *tile = (HitTile *)data;
#ifdef DEBUG
	printf("Hi i am a communication callback.\n\tUsing tile %p\n", tile);
	fflush(stdout);
#endif

// CRITICAL ZONE BEGIN: kernel dependency on input tile communication
#ifdef DEBUG
	printf("MoveTo Callback entering critical zone\n");
	fflush(stdout);
#endif //DEBUG

// Set flag of tile's pending communication to false
#pragma omp atomic write
	tile->hasPendingMoveTo = 0;

	if (tile->isCntrlBlocked)
	{ // Check if controller is blocked by tile's communication
#ifdef DEBUG
		printf("MoveTo Callback cntrl is locked. Unlocking\n");
		fflush(stdout);
#endif //DEBUG
// Set flag of controller thread blocked by tile to false
#pragma omp atomic write
		tile->isCntrlBlocked = 0;
		sem_post(&(((CALCntrlGPU *)tile->refCntrl)->semCntrl));
	}

#ifdef DEBUG
	int semCntrlValue;
	sem_getvalue(&(((CALCntrlGPU *)tile->refCntrl)->semCntrl), &semCntrlValue);
	printf("MoveTo Callback leaving critical zonetile (%p): Kernel count: %d isCntrlBlocked: %d semCntrl: %d\n", tile, tile->cntrlKernelCount, tile->isCntrlBlocked, semCntrlValue);
	fflush(stdout);
#endif //DEBUG
	   // CRITICAL ZONE END
}

/* 
 * Communication MoveFrom callback
 * Params:
 * 		event: CUDA stream of previous execution.
 * 		status: CUDA status of previous execution.
 * 		data: callback data.
 */
void CUDART_CB CAL_CntrlGPUCallbackMoveFrom(cudaStream_t stream, cudaError_t status, void *data)
{
// Check status of GPU after stream operations are done
#ifdef DEBUG
	CUDA_CHECK();
#endif

	HitTile *tile = (HitTile *)data;
#ifdef DEBUG
	printf("Hi i am a communication callback.\n\tUsing tile %p\n", tile);
	fflush(stdout);
#endif

// CRITICAL ZONE BEGIN: kernel dependency on input tile communication
#ifdef DEBUG
	printf("MoveFrom Callback entering critical zone\n");
	fflush(stdout);
#endif //DEBUG

#pragma omp atomic write
	tile->hasPendingMoveFrom = 0;

	if (tile->isCntrlBlocked)
	{
#pragma omp atomic write
		tile->isCntrlBlocked = 0;
		sem_post(&(((CALCntrlGPU *)tile->refCntrl)->semCntrl));
	}

#ifdef DEBUG
	int semCntrlValue;
	sem_getvalue(&(((CALCntrlGPU *)tile->refCntrl)->semCntrl), &semCntrlValue);
	printf("MoveFrom Callback leaving critical zone(%p): Kernel count: %d isCntrlBlocked: %d semCntrl: %d\n", tile, tile->cntrlKernelCount, tile->isCntrlBlocked, semCntrlValue);
	fflush(stdout);
#endif //DEBUG
	   // CRITICAL ZONE END
}
