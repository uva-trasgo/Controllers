/**
 * @file epsilod.c
 * @brief Epsilod: Stencil code: Any dimensions, stencil as a pattern of weights. Data type: float
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include <epsilod.h>
#include "../../examples/Utils/ctrl_print_info.h"

#ifndef _CTRL_EXAMPLES_EXP_MODE_
#define _PSS_SHP_INFO_
// #define _PSS_SHP_INFO_DEBUG_
#define _PSS_TOPO_INFO_
#define _PSS_WEIGHTS_INFO_
// #define _PSS_MANUAL_REDISTRIBUTION_
#endif // _CTRL_EXAMPLES_EXP_MODE_

// GLOBAL VARIABLE: USE OF CUDA/HIP-AWARE MPI
int mpi_dev_aware;

/* A. KERNEL CHARACTERIZATIONS */
Ctrl_Thread INNER_CHAR[3] = {
	{.dims = 1, .i = 256, .j = 1, .k = 1},
	{.dims = 2, .i = 4, .j = 64, .k = 1},
	{.dims = 3, .i = 1, .j = 4, .k = 64}};

Ctrl_Thread CPU_INNER_CHAR[3] = {
	{.dims = 1, .i = 1024, .j = 1, .k = 1},
	{.dims = 2, .i = 4, .j = 1024, .k = 1},
	{.dims = 3, .i = 2, .j = 2, .k = 1024}};

Ctrl_Thread BORDER_CHAR[3][3] = {
	// 1D
	{{.dims = 1, .i = 256, .j = 1, .k = 1},
	 CTRL_THREAD_NULL,
	 CTRL_THREAD_NULL},
	// 2D
	{{.dims = 2, .i = 1, .j = 256, .k = 1},
	 {.dims = 2, .i = 256, .j = 1, .k = 1},
	 CTRL_THREAD_NULL},
	// 3D
	{{.dims = 3, .i = 1, .j = 4, .k = 64},
	 {.dims = 3, .i = 4, .j = 1, .k = 64},
	 {.dims = 3, .i = 4, .j = 64, .k = 1}}};

Ctrl_Thread CPU_BORDER_CHAR[3][3] = {
	// 1D
	{{.dims = 1, .i = 1024, .j = 1, .k = 1},
	 CTRL_THREAD_NULL,
	 CTRL_THREAD_NULL},
	// 2D
	{{.dims = 2, .i = 1, .j = 1024, .k = 1},
	 {.dims = 2, .i = 1024, .j = 1, .k = 1},
	 CTRL_THREAD_NULL},
	// 3D
	{{.dims = 3, .i = 1, .j = 1, .k = 1024},
	 {.dims = 3, .i = 1, .j = 1, .k = 1024},
	 {.dims = 3, .i = 1, .j = 1024, .k = 1}}};

// #define INNER_CHAR ((Ctrl_Thread){.dims = 2, .x = 64, .y = 4, .z = 1})
// #define CHAR_1D    ((Ctrl_Thread){.dims = 1, .x = 256, .y = 1, .z = 1})
// #define CHAR_CPU   ((Ctrl_Thread){.dims = 2, .x = 32, .y = 32, .z = 1})
// Ctrl_Thread BORDER_CHAR[3] = {
// 	{.dims = 1, .x = 256, .y = 1, .z = 1},
// 	{.dims = 2, .x = 1, .y = 256, .z = 1},
// 	{.dims = 3, .x = 1, .y = 256, .z = 1}};

/* B. GENERIC KERNEL PROTOTYPE AND WRAPPER LAUNCHERS */
#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
CTRL_KERNEL_CHAR(updateCell_default_1D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_2D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_3D, MANUAL, 0, 0, 0);

CTRL_KERNEL_PROTO(updateCell_default_1D,
				  1, GENERIC, DEFAULT, 6,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,
				  INVAL, EpsilodCoords, global_coords,
				  IN, HitTile(float), weight,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_1D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_1D, threads, blockSize, stream, mat, copy, global, weight, factor, *ext_params);
}

CTRL_KERNEL_PROTO(updateCell_default_2D,
				  1, GENERIC, DEFAULT, 6,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,
				  INVAL, EpsilodCoords, global_coords,
				  IN, HitTile(float), weight,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_2D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_2D, threads, blockSize, stream, mat, copy, global, weight, factor, *ext_params);
}

CTRL_KERNEL_PROTO(updateCell_default_3D,
				  1, GENERIC, DEFAULT, 6,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,
				  INVAL, EpsilodCoords, global_coords,
				  IN, HitTile(float), weight,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_3D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_3D, threads, blockSize, stream, mat, copy, global, weight, factor, *ext_params);
}
#endif // EPSILOD_BASE_TYPE IS NOT float

CTRL_KERNEL_CHAR(epsilod_dev_copy, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_copy,
				  1, GENERIC, DEFAULT, 2,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrix,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix_out);

/* D. FALSE INITIALIZATION OF SELECTIONS TO AVOID NON-INITIALIZED WARNINGS */
CTRL_KERNEL_CHAR(epsilod_dev_touch, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_touch,
				  1, GENERIC, DEFAULT,
				  1, OUT, HitTile(EPSILOD_BASE_TYPE), matrix);

CTRL_HOST_TASK(epsilod_host_touch, HitTile(EPSILOD_BASE_TYPE) matrix) { ; }
CTRL_HOST_TASK_PROTO(epsilod_host_touch, 1, OUT, HitTile(EPSILOD_BASE_TYPE), matrix);

/* E. OUTPUT HOST-TASK WRAPPER */
CTRL_HOST_TASK(Ctrl_Write_Output, outputDataFunction f_output, HitTile(EPSILOD_BASE_TYPE) matrix, Epsilod_ext *ext_params) {
	f_output(matrix, ext_params);
}

CTRL_HOST_TASK_PROTO(Ctrl_Write_Output, 3,
					 INVAL, outputDataFunction, f_output,
					 IN, HitTile(EPSILOD_BASE_TYPE), matrix,
					 INVAL, Epsilod_ext *, ext_params);

/* F. DEBUG: WRITE OUTPUT ON STDOUT */
/*
CTRL_HOST_TASK(print_matrix, HitTile( EPSILOD_BASE_TYPE ) mat, Epsilod_ext *ext_params) {
	printf("Matrix for [%d]:\n", hit_Rank);
	int i, j, k;
	switch (mat.shape.info.sig.numDims) {
		case 1:
			for (i = 0; i < hit_tileDimCard(mat, 0); i++) {
				printf("%g ", hit(mat, i));
			}
			break;
		case 2:
			for (i = 0; i < hit_tileDimCard(mat, 0); i++) {
				for (j = 0; j < hit_tileDimCard(mat, 1); j++) {
					printf("%g ", hit(mat, i, j));
				}
				printf("\n");
			}
			break;
		case 3:
			for (i = 0; i < hit_tileDimCard(mat, 0); i++) {
				for (j = 0; j < hit_tileDimCard(mat, 1); j++) {
					for (k = 0; k < hit_tileDimCard(mat, 2); k++) {
						printf("%g ", hit(mat, i, j, k));
						// printf("\n");
					}
					printf("\n");
				}
				printf("\n");
			}
			break;
	}
	printf("\n");
	fflush(stdout);
}

CTRL_HOST_TASK_PROTO(print_matrix, 2, IN, HitTile( EPSILOD_BASE_TYPE ), mat, INVAL, Epsilod_ext *, ext_params);
*/

/* G. STENCIL PATTERN TRANSFERENCE HOST-TASK */
CTRL_HOST_TASK(Ctrl_Copy_Stencil, HitTile_float stencil, float *stencil_data) {
	int i;
	int j;
	int k;
	int dataind = 0;
	int dims    = hit_tileDims(stencil);
	switch (dims) {
		case 1:
			for (i = 0; i < hit_tileDimCard(stencil, 0); i++)
				hit(stencil, i) = stencil_data[i];
			break;
		case 2:
			for (i = 0; i < hit_tileDimCard(stencil, 0); i++)
				for (j = 0; j < hit_tileDimCard(stencil, 1); j++)
					hit(stencil, i, j) = stencil_data[dataind++];

			break;
		case 3:
			for (i = 0; i < hit_tileDimCard(stencil, 0); i++)
				for (j = 0; j < hit_tileDimCard(stencil, 1); j++)
					for (k = 0; k < hit_tileDimCard(stencil, 2); k++)
						hit(stencil, i, j, k) = stencil_data[dataind++];
			break;
		default:
			fprintf(stderr, "[Parallel Stencil Skeleton] %d dims are not supported in the stencil definition, max. 3 dims\n", dims);
			exit(EXIT_FAILURE);
	}
}

CTRL_HOST_TASK_PROTO(Ctrl_Copy_Stencil, 2,
					 OUT, HitTile_float, stencil,
					 INVAL, float *, stencil_data);

#define validShape(s) (hit_shapeDims((s)) != (-1))

typedef struct
{
	int index;
	HitTile(EPSILOD_BASE_TYPE) tile;
} CommCompIndex;

/**
 * Compare tiles to order communications.
 * Compares by number of elements.
 * This doesn't take into account that memory transfers for tiles with non-consecutive elements in memory
 * may take longer even if the number of elements is smaller.
 */
int compare_comm_tiles(const void *a, const void *b) {
	// qsort sorts in ascending order
	// a less than b -> return negative
	CommCompIndex index_a = *(CommCompIndex *)a;
	CommCompIndex index_b = *(CommCompIndex *)b;
	if (index_a.tile.acumCard > index_b.tile.acumCard)
		return -1;
	if (index_a.tile.acumCard < index_b.tile.acumCard)
		return 1;
	return 0;
}

/* H. EXPERIMENTATION: GLOBAL clocks */
HitClock mainClock;
HitClock initClock;
HitClock loopClock;
/// HitClock	redistributeClock;
HitClock commClock;

/* HELP. PRINT EXTRA OPTIONS OF EPSILOD USING ENVIRONMENT VARIABLES */
void epsilod_print_usage() {
	if (hit_Rank == 0) {
		fprintf(stderr, "\nEPSILOD environment variables:\n");
		fprintf(stderr, "\tEPSILOD_MPI_DEV_AWARE=y|n   Activate the use of CUDA/HIP aware MPI communications\n");
		fprintf(stderr, "\tEPSILOD_PARTITION=r         Regular blocks of similar sizes on a multidimensional grid topology\n");
		fprintf(stderr, "\tEPSILOD_PARTITION=r<n_dims> Regular blocks of similar sizes on the first <n_dims> dimensions\n");
		fprintf(stderr, "\tEPSILOD_PARTITION=s<dim>    Regular blocks of similar sizes on a single dimension topology\n");
		fprintf(stderr, "\tEPSILOD_PARTITION=w<dim>    Weigthed block distribution in the single choosen dimension. Processes weigths are specified in the device selection configuratuion file.\n");
		fprintf(stderr, "\tThe default behaviour corresponds to s0.\n");
		fprintf(stderr, "\n");
	}
}

void printAll(const char *format, ...) {
	#ifndef _CTRL_EXAMPLES_EXP_MODE_
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	#endif // _CTRL_EXAMPLES_EXP_MODE_
}

void printOnce(const char *format, ...) {
	#ifndef _CTRL_EXAMPLES_EXP_MODE_
	if (hit_Rank == 0) {
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		va_end(args);
	}
	#endif // _CTRL_EXAMPLES_EXP_MODE_
}

void printClockInfo() {
	#ifdef _CTRL_EXAMPLES_EXP_MODE_
	if (hit_Rank == 0)
		printf("%lf, %lf, %lf, %lf\n", mainClock.max, initClock.max, loopClock.max, commClock.max);
	#else  // _CTRL_EXAMPLES_EXP_MODE_
	hit_clockPrintMax(mainClock);
	hit_clockPrintMax(initClock);
	hit_clockPrintMax(loopClock);
	/// hit_clockPrintMax( redistributeClock );
	hit_clockPrintMax(commClock);
	#endif // _CTRL_EXAMPLES_EXP_MODE_
	fflush(stdout);
}

void reduceClocks(HitLayout layout) {
	hit_clockReduce(layout, mainClock);
	hit_clockReduce(layout, initClock);
	hit_clockReduce(layout, loopClock);
	/// hit_clockReduce(layout, redistributeClock);
	hit_clockReduce(layout, commClock);
}

void markTiles(PCtrl comm, EpsilodProperties props, Ctrl_Thread threads_touch, Ctrl_Thread blocksize_touch, EpsilodTiles *tiles, EpsilodTiles *copy_tiles, EpsilodCommArgs *comm_args) {

	for (int i = 0; i < props.dims; i++) {
		if (validShape(tiles->border_out_dev[i][0].shape)) {
			Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, tiles->border_out_dev[i][0]);
			Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, copy_tiles->border_out_dev[i][0]);
		}
		if (validShape(tiles->border_out_dev[i][1].shape)) {
			Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, tiles->border_out_dev[i][1]);
			Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, copy_tiles->border_out_dev[i][1]);
		}
	}
	if (validShape((*tiles->inner_local).shape)) {
		Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, *tiles->inner_local);
		Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, *copy_tiles->inner_local);
	}
	for (int i = 0; i < comm_args->border_count; i++) {
		// SKIP EMPTY BORDERS
		if (!comm_args->border_in_active[i]) continue;
		Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, comm_args->tile_border_in[i]);
		// Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, tileCopyBorderIn[i]);
	}
}

Ctrl_Thread initCtrlThreadFromTile(int dims, HitTile(EPSILOD_BASE_TYPE) * p_tile) {
	Ctrl_Thread tile_threads;
	switch (dims) {
		case 1:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 0));
			break;
		case 2:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 0),
							hit_tileDimCard(*p_tile, 1));
			break;
		case 3:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 0),
							hit_tileDimCard(*p_tile, 1),
							hit_tileDimCard(*p_tile, 2));
			break;
		default:
			fprintf(stderr, "[Epsilod ERROR] Controller thread initialization not implemented for %d dimensions.\n", dims);
			exit(EXIT_FAILURE);
	}
	return tile_threads;
}

void doComms(PCtrl comm, EpsilodProperties *props, EpsilodTiles *tiles, EpsilodCommArgs *args) {

	if (!mpi_dev_aware) {
		for (int i = 0; i < props->dims; i++) {
			for (int j = 0; j < 2; j++) {
				if (validShape(tiles->border_out_dev[i][j].shape)) {
					Ctrl_MoveFrom(comm, tiles->border_out_dev[i][j]);
				}
			}
			if (props->topology.type == HIT_TOPOLOGY_PLAIN) break;
		}
		for (int i = 0; i < props->dims; i++) {
			for (int j = 0; j < 2; j++) {
				if (validShape(tiles->border_out_dev[i][j].shape)) {
					Ctrl_WaitTile(comm, tiles->border_out_dev[i][j]);
				}
			}
		}
	}

#define EPSILOD_COMM_ANY
	// #define EPSILOD_COMM_ANY_RECV_FIRST

	#ifdef EPSILOD_COMM_ANY_RECV_FIRST
	hit_clockStart(commClock);
	if (mpi_dev_aware) {
		hit_patternDo((*args->comm_pattern));
	} else {
		hit_patternStartAsync(*args->comm_pattern);
		int endComm;
		for (endComm = hit_patternStepAsyncRecv(*args->comm_pattern);
			 endComm != HIT_PAT_END;
			 endComm = hit_patternStepAsyncRecv(*args->comm_pattern)) {

			// START MOVE-TO FOR RECV
			int border = args->index_comm_border[endComm];

			Ctrl_HostTask(epsilod_host_touch, args->tile_border_in[border]);
			Ctrl_MoveTo(comm, args->tile_border_in[border]);
		}
		hit_patternEndAsync(*args->comm_pattern);

		for (int i = 0; i < args->border_count; i++) {
			// SKIP EMPTY BORDERS
			if (!args->border_in_active[i]) continue;
			Ctrl_WaitTile(comm, args->tile_border_in[i]);
		}
	}
	hit_clockStop(commClock);
	#else // EPSILOD_COMM_ANY_RECV_FIRST
	#ifdef EPSILOD_COMM_ANY
	hit_clockStart(commClock);
	if (mpi_dev_aware) {
		hit_patternDo((*args->comm_pattern));
	} else {
		hit_patternStartAsync(*args->comm_pattern);
		int endComm;
		for (endComm = hit_patternStepAsync(*args->comm_pattern);
			 endComm != HIT_PAT_END;
			 endComm = hit_patternStepAsync(*args->comm_pattern)) {

			// SKIP SENDS
			if (endComm % 2 == 0) continue;

			// START MOVE-TO FOR RECV
			int border = args->index_comm_border[endComm / 2];

			Ctrl_HostTask(epsilod_host_touch, args->tile_border_in[border]);
			Ctrl_MoveTo(comm, args->tile_border_in[border]);
		}

		for (int i = 0; i < args->border_count; i++) {
			// SKIP EMPTY BORDERS
			if (!args->border_in_active[i]) continue;
			Ctrl_WaitTile(comm, args->tile_border_in[i]);
		}
	}
	hit_clockStop(commClock);

	#else // EPSILOD_COMM_ANY
	hit_clockStart(commClock);
	hit_patternDo((*args->comm_pattern));

	if (!mpi_dev_aware) {
		for (int i = 0; i < args->border_count; i++) {
			// SKIP EMPTY BORDERS
			if (!args->border_in_active[i]) continue;
			Ctrl_HostTask(epsilod_host_touch, args->tile_border_in[i]);
			Ctrl_MoveTo(comm, args->tile_border_in[i]);
		}
		for (int i = 0; i < args->border_count; i++) {
			// SKIP EMPTY BORDERS
			if (!args->border_in_active[i]) continue;
			Ctrl_WaitTile(comm, args->tile_border_in[i]);
		}
	}
	hit_clockStop(commClock);
	#endif // EPSILOD_COMM_ANY
	#endif // EPSILOD_COMM_ANY_RECV_FIRST
}

void swap(EpsilodSwapStructs *a, EpsilodSwapStructs *b) {

	for (int i = 0; i < a->tile_list_count; i++) {
		HitTile(EPSILOD_BASE_TYPE) *p_tile_a = a->tile_list[i];
		HitTile(EPSILOD_BASE_TYPE) *p_tile_b = b->tile_list[i];
		HitTile(EPSILOD_BASE_TYPE) tmp       = *p_tile_a;
		*p_tile_a                            = *p_tile_b;
		p_tile_a->ref                        = tmp.ref;
		tmp.ref                              = p_tile_b->ref;
		*p_tile_b                            = tmp;
	}
	HitPattern temp    = *(a->comm_pattern);
	*(a->comm_pattern) = *(b->comm_pattern);
	*(b->comm_pattern) = temp;
}

void compute(PCtrl comm, EpsilodProperties *props, EpsilodComputationArgs *computation_args) {

	stencilDeviceFunction f_updateCell = computation_args->f_updateCell;
	EpsilodTiles         *tiles        = computation_args->tiles;
	EpsilodTiles         *copy_tiles   = computation_args->copy_tiles;
	EpsilodThreads       *threads      = computation_args->threads;
	EpsilodChars         *chars        = computation_args->chars;
	EpsilodGlobalCoords  *coords       = computation_args->coords;
	HitTile_float        *stencil      = computation_args->stencil;
	float                 factor       = computation_args->factor;
	Epsilod_ext          *ext_params   = computation_args->ext_params;

	/* 4.8.2. COMPUTE BORDERS */
	for (int i = 0; i < props->dims; i++) {
		if (validShape(tiles->border_out_dev[i][0].shape) && validShape(copy_tiles->border_out_dev[i][0].shape)) {
			f_updateCell(comm, threads->border_out_dev[i][0], chars->border_out_dev[i], 2 * i, tiles->border_out_dev[i][0], copy_tiles->border_out_dev[i][0], coords->border_out_dev[i][0], *stencil, factor, ext_params);
		}
		if (validShape(tiles->border_out_dev[i][1].shape) && validShape(copy_tiles->border_out_dev[i][1].shape)) {
			f_updateCell(comm, threads->border_out_dev[i][1], chars->border_out_dev[i], 2 * i + 1, tiles->border_out_dev[i][1], copy_tiles->border_out_dev[i][1], coords->border_out_dev[i][1], *stencil, factor, ext_params);
		}
	}

	/* SYNC BORDERS BEFORE INNER */
	if (mpi_dev_aware) {
		for (int i = 0; i < props->dims; i++) {
			if (validShape(tiles->border_out_dev[i][0].shape) && validShape(copy_tiles->border_out_dev[i][0].shape)) {
				Ctrl_WaitTile(comm, tiles->border_out_dev[i][0]);
			}
			if (validShape(tiles->border_out_dev[i][1].shape) && validShape(copy_tiles->border_out_dev[i][1].shape)) {
				Ctrl_WaitTile(comm, tiles->border_out_dev[i][1]);
			}
		}
	}

	/* COMPUTE INNER */
	if (validShape(tiles->inner_local->shape) && validShape(copy_tiles->inner_local->shape)) {
		f_updateCell(comm, threads->inner, chars->inner, 0, *tiles->inner_local, *copy_tiles->inner_local, coords->inner, *stencil, factor, ext_params);
	}
}

void doStep(
	PCtrl comm, EpsilodProperties *props,
	EpsilodComputationArgs *computation_args, EpsilodSwapStructs *swap_structs, EpsilodSwapStructs *swap_structs_copy, EpsilodCommArgs *comm_args) {

	EpsilodTiles *tiles      = computation_args->tiles;
	EpsilodTiles *copy_tiles = computation_args->copy_tiles;

	/* UPDATE TILE COPY */
	if (validShape(copy_tiles->mat->shape) && validShape(tiles->mat->shape)) {
		swap(swap_structs, swap_structs_copy);
	}

	compute(comm, props, computation_args);

	doComms(comm, props, tiles, comm_args);

	Ctrl_WaitTile(comm, *tiles->inner_local);
}

void doStepNoComms(
	PCtrl comm, EpsilodProperties *props,
	EpsilodComputationArgs *computation_args, EpsilodSwapStructs *swap_structs, EpsilodSwapStructs *swap_structs_copy) {

	EpsilodTiles *tiles      = computation_args->tiles;
	EpsilodTiles *copy_tiles = computation_args->copy_tiles;

	if (validShape(copy_tiles->mat->shape) && validShape(tiles->mat->shape)) {
		swap(swap_structs, swap_structs_copy);
	}

	compute(comm, props, computation_args);

	// Wait for kernels to finish:
	for (int i = 0; i < props->dims; i++)
		for (int j = 0; j < 2; j++)
			if (validShape(tiles->border_out_dev[i][j].shape))
				Ctrl_WaitTile(comm, tiles->border_out_dev[i][j]);
	Ctrl_WaitTile(comm, *tiles->inner_local);
}

/* I. STENCIL COMPUTATION FUNCTION: */
void stencilComputation(
	int                    sizes[],
	HitShape               stencilShape,
	float                  stencilData[],
	float                  factor,
	int                    numIterations,
	initDataFunction       f_init,
	initDataDeviceFunction f_dev_init,
	stencilDeviceFunction  f_init_copy,
	stencilDeviceFunction  f_updateCell,
	outputDataFunction     f_output,
	Epsilod_ext           *ext_params_arg,
	char                  *device_selection_file) {

	/* HITMAP COMM. TYPE */
	HitType HIT_CELL;
	#if HIT2_COUNTPARAM(EPSILOD_BASE_TYPE_COMPOUND) < 2
	HIT_CELL = hit_comTranslateType(EPSILOD_BASE_TYPE);
	#else
	#if HIT2_COUNTPARAM(EPSILOD_BASE_TYPE_COMPOUND) != 2
#error "EPSILOD_TYPE_COMPOUND_<type> should contain two arguments separated by comma: <type>, <count>"
	#else
	{
		HitType type  = hit_comTranslateType(EPSILOD_GET_COMPOUND_TYPE(EPSILOD_BASE_TYPE_COMPOUND));
		int     count = EPSILOD_GET_COMPOUND_COUNT(EPSILOD_BASE_TYPE_COMPOUND);
		hit_comTypeArray(&HIT_CELL, type, count);
	}
	#endif
	#endif

	/* EXTERNAL/EXTRA PARAMETERS: USE FOO VARIABLE FOR NULL ARGUMENT */
	Epsilod_ext  foo;
	Epsilod_ext *ext_params = (ext_params_arg == NULL) ? &foo : ext_params_arg;

	int dims = hit_shapeDims(stencilShape);

	/* CHECK IF GENERIC KERNEL HAS BEEN CHOSEN */
	if (f_updateCell == NULL) {
		#if !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
		fprintf(stderr, "[Parallel Stencil Skeleton ERROR] Generic update kernel is only defined for float base type.\n\tFor other types a custom update kernel should be provided as argument.\n");
		fflush(stderr);
		exit(EXIT_FAILURE);
		#else  // float
		switch (dims) {
			case 1:
				f_updateCell = updateCell_default_1D;
				break;
			case 2:
				f_updateCell = updateCell_default_2D;
				break;
			case 3:
				f_updateCell = updateCell_default_3D;
				break;
			default:
				fprintf(stderr, "[Parallel Stencil Skeleton ERROR] Stencil with invalid number of dimensions. The skeleton only supports 1D, 2D or 3D stencils.\n");
				fflush(stderr);
				exit(EXIT_FAILURE);
		}
		#endif // float
	}

	/* READ ENVIRONMENT: USE CUDA/HIP MPI AWARE */
	mpi_dev_aware = hit_envNoYes("EPSILOD_MPI_DEV_AWARE");
	printOnce("Epsilod Using Device-Aware MPI: %c\n", (mpi_dev_aware) ? 'y' : 'n');
	if (mpi_dev_aware)
		printOnce("Note: Device-Aware MPI only works if it is suported and activated in the MPI layer\n");

	/* INITIALIZE DEVICE Controllers */
	__ctrl_block__(device_selection_file) {
		int i, j, k;

		/* INIT CLOCKS */
		hit_clockSynchronizeAll();
		hit_clockStart(mainClock);
		hit_clockStart(initClock);

		PCtrl comm = Ctrl_Get(0);

		printf("MPI_Rank[%d]", hit_Rank);
		Ctrl_PrintInfo();
		printf("\n");
		fflush(stdout);

		Ctrl_SetDependanceMode(comm, CTRL_MODE_EXPLICIT);

		/* 0. BUILD THE STENCIL TILE */
		HitTile_float stencil = Ctrl_DomainAlloc(comm, float, stencilShape);
		Ctrl_HostTask(Ctrl_Copy_Stencil, stencil, stencilData);
		Ctrl_MoveTo(comm, stencil);
		Ctrl_WaitTile(comm, stencil);

		/* 1. BUILD ORIGINAL SHAPE */
		HitShape shp = HIT_SHAPE_NULL;
		hit_shapeDimsSet(shp, dims);
		for (i = 0; i < dims; i++) {
			hit_shapeSig(shp, i) = hit_sig(0, sizes[i] - 1, 1);
		}

		/* 2. SHORTCUTS FOR BORDER SIZES */
		// v1.2 @Arturo, new predefined size type for borders
		// int *borderLow  = (int *)malloc(dims * sizeof(int));
		// int *borderHigh = (int *)malloc(dims * sizeof(int));
		EpsilodBorders borders;
		// printf("Borders:\n");
		for (i = 0; i < dims; i++) {
			borders.low[i]  = -hit_tileDimBegin(stencil, i);
			borders.high[i] = hit_tileDimEnd(stencil, i);
			// printf("\tDim %d: %d %d\n", i, borders.low[i], borders.high[i]);
		}

		/* 3.1. SHAPE TO DISTRIBUTE COMPUTATION (WITHOUT BORDERS) */
		HitShape shpInner = shp;
		for (i = 0; i < dims; i++) {
			shpInner = hit_shapeTransform(shpInner, i, HIT_SHAPE_BEGIN, +borders.low[i]);
			shpInner = hit_shapeTransform(shpInner, i, HIT_SHAPE_END, -borders.high[i]);
		}

		/* 3.2. SELECT AND BUILD PARTITION/DISTRIBUTION */
		int   partition_dim       = 0;
		int   partition_dim_count = -1;
		bool  use_weights         = false;
		char *partition_str       = getenv("EPSILOD_PARTITION");
		if (partition_str != NULL) {
			if (strlen(partition_str) > 2) {
				fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: More than two characters. String: %s\n\n", partition_str);
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
				exit(EXIT_FAILURE);
			}
			char *err;
			char *part_arg = &partition_str[1];
			switch (partition_str[0]) {
				case 'r':
				case 'R':
					if (*part_arg == '\0') {
						partition_dim_count = dims;
					} else {
						partition_dim_count = (int)strtol(part_arg, &err, 10);
						if (err == part_arg || partition_dim_count < 1 || partition_dim_count > dims) {
							fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Optional number of dimensions should be in the range [1:%d]. String: %s \n\n", dims, partition_str);
							MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
							exit(EXIT_FAILURE);
						}
					}
					break;
				case 'w':
				case 'W':
					use_weights = true;
				case 's':
				case 'S':
					partition_dim = (int)strtol(part_arg, &err, 10);
					if (err == part_arg || partition_dim < 0 || partition_dim > dims - 1) {
						fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Dimension should be in the range [0:%d]. String: %s \n\n", dims - 1, partition_str);
						MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
						exit(EXIT_FAILURE);
					}
					break;
				default:
					fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Unknown partition type. String: %s\n\n", partition_str);
					MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
					exit(EXIT_FAILURE);
			}
		}
		// if (partition_dim >= dims) {
		// 	fprintf(stderr, "\nError in EPSILOD_PARTITION environment string: Dimension %d, should be in the range of [0:stencil_dimensions-1]. String: %s\n\n", partition_dim, partition_str);
		// 	MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
		// 	exit(EXIT_FAILURE);
		// }

		// printf("Partition dim count: %d\n", partition_dim_count);

		/* 3.2. BUILD DISTRIBUTED SHAPE */
		int         topoDims;
		HitTopology topo;
		HitLayout   lay;
		HitWeights  weights = Ctrl_ConfigWeights();

		// TOPOLOGY
		if (partition_dim_count == -1) {
			topoDims = 1;
			topo     = hit_topology(plug_topPlain);

			// WEIGHTED DISTRIBUTION
			if (use_weights) {
				lay = hit_layout(plug_layDimWeighted_Blocks, topo, shpInner, partition_dim, weights);
			}
			// REGULAR DISTRIBUTION
			else {
				// TODO: use the proper layout
				lay = hit_layout(plug_layDimBlocks, topo, shpInner, partition_dim);
			}
		} else {
			if (partition_dim_count < 1 || partition_dim_count > 3) {
				fprintf(stderr, "\nError EPSILOD: Number of dimensions for processes topology should be in the range [1:3]. Current value:%d \n\n", dims);
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
				exit(EXIT_FAILURE);
			}
			topoDims = partition_dim_count;
			topo     = hit_topology(plug_topArray, partition_dim_count);
			lay      = hit_layout(plug_layBlocks, topo, shpInner);
		}

		HitShape shpLayout = hit_layShape(lay);
		#ifdef _PSS_SHP_INFO_
		printf("shpLayout: ");
		dumpShape(shpLayout);
		fflush(stdout);
		#endif // _SHP_DEV_INFO_

		#ifdef _PSS_TOPO_INFO_
		if (hit_Rank == 0) {
			printf("\nTOPOLOGY: topoDims(%d,%d,%d)\n",
				   hit_topDimCard(topo, 0),
				   hit_topDimCard(topo, 1),
				   hit_topDimCard(topo, 2));
			fflush(stdout);
		}
		#endif // _SHP_DEV_INFO_

		#ifdef _PSS_WEIGHTS_INFO_
		if (hit_Rank == 0) {
			printf("\nPartition weights = {");
			for (int i = 0; i < weights.num_procs; i++)
				printf(" %f,", weights.ratios[i]);
			printf("\b }\n");
			fflush(stdout);
		}
		#endif

		/* 4. ACTIVE PROCESSES */
		if (hit_layImActive(lay)) {

			// 4.1. STOP IF THERE IS NOT ENOUGH INNER DATA FOR A BORDER
			for (i = 0; i < dims; i++) {
				if (hit_shapeSigCard(shpLayout, i) < borders.low[i] ||
					hit_shapeSigCard(shpLayout, i) < borders.high[i]) {
					if (hit_Rank == 0) {
						fprintf(stderr, "\nError: Not enough data after partition, too many processes in a topology axis\n\n");
						MPI_Abort(MPI_COMM_WORLD, MPI_ERR_TOPOLOGY);
						exit(EXIT_FAILURE);
					}
				}
			}

			/* 4.2. EXPANDED SHAPE */
			HitShape shpExpanded = shpLayout;
			for (i = 0; i < dims; i++) {
				shpExpanded = hit_shapeTransform(shpExpanded, i, HIT_SHAPE_BEGIN, -borders.low[i]);
				shpExpanded = hit_shapeTransform(shpExpanded, i, HIT_SHAPE_END, borders.high[i]);
			}

			/* 4.3. BORDER SHAPES: IN/OUT */
			int      numBorders = (int)pow(3, dims);
			HitShape shpBorderIn[numBorders];
			HitShape shpBorderOut[numBorders];
			HitRanks shiftsIn[numBorders];
			HitRanks shiftsOut[numBorders];
			int      borderInActive[numBorders];
			int      borderOutActive[numBorders];
			int      indexCommBorder[numBorders];
			int      indexCommBorderCount = 0;
			for (i = 0; i < numBorders; i++)
				borderInActive[i] = 0;

			// TRAVERSE THE STENCIL TO DETECT ACTIVE AND INACTIVE BORDERS DUE TO WEIGHTS
			int indeces[dims];
			int displacement = 0;
			for (j = 0; j < dims; j++)
				indeces[j] = 0;
			int endAnalysis = 0;
			while (!endAnalysis) {
				// CHECK IF THERE IS A WEIGHT IN THE STENCIL POSITION
				if (stencilData[displacement] != 0) {
					// ACTIVE BORDER, COMPUTE ITS NUMBER TO RAISE THE FLAG
					int acum   = 1;
					int border = 0;
					for (j = dims - 1; j >= 0; j--) {
						if (indeces[j] > borders.low[j])
							border += 2 * acum;
						else if (indeces[j] == borders.low[j])
							border += acum;
						acum *= 3;
					}
					borderInActive[border] = 1;
				}
				// ADVANCE TO THE NEXT STENCIL POSITION
				displacement++;
				for (k = dims - 1; k >= 0; k--) {
					indeces[k]++;
					if (k == 0 && indeces[0] == hit_tileDimCard(stencil, 0)) endAnalysis = 1;
					if (indeces[k] >= hit_tileDimCard(stencil, k))
						indeces[k] = 0;
					else
						break;
				}
			}
			// ALWAYS SKIP FALSE BORDER: TILE INNER
			borderInActive[numBorders / 2] = 0;

			// BUILD BORDER SHAPES AND THE SHORTCUTS OF THE NEIGHBOR SHIFTS
			for (i = 0; i < numBorders; i++) {
				shiftsIn[i]  = HIT_RANKS_NULL;
				shiftsOut[i] = HIT_RANKS_NULL;

				// NON-ACTIVE BORDERS, NULL SHAPES, NULL RANKS
				if (!borderInActive[i]) {
					shpBorderIn[i]  = HIT_SHAPE_NULL;
					shpBorderOut[i] = HIT_SHAPE_NULL;
					continue;
				}
				shpBorderIn[i]  = shpLayout;
				shpBorderOut[i] = shpLayout;

				// EXTRACT RANKS FOR THIS BORDER
				int digits = i;
				for (j = 0; j < dims; j++) {
					shiftsIn[i].rank[j]  = digits % 3 - 1;
					shiftsOut[i].rank[j] = -shiftsIn[i].rank[j];
					digits /= 3;

					// SHAPE IN
					if (shiftsIn[i].rank[j] == -1) {
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_FIRST, borders.low[j]);
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, -borders.low[j]);
					} else if (shiftsIn[i].rank[j] == 1) {
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_LAST, borders.high[j]);
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, borders.high[j]);
					}

					// SHAPE OUT (REVERSERD TARGET)
					if (shiftsIn[i].rank[j] == -1) {
						shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_LAST, borders.low[j]);
					} else if (shiftsIn[i].rank[j] == 1) {
						shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_FIRST, borders.high[j]);
					}
				}
			}

			/* 4.4. NON-OVERLAPED BORDERS IN THE INNER PART
				a) TO EXTRACT DATA FROM DEVICE WITHOUT REPLICATION
				b) TO DETERMINE THE THREADS-GRID CARDINALITIES FOR THE COMPUTING KERNELS
				@arturo Aug 2024: Skip dimensions not selected in the topology
			*/
			HitShape shpInnerLocal = shpLayout;
			HitShape shpBorderOutDev[dims][2];
			for (j = 0; j < dims; j++) {
				if (j < topoDims) {
					shpInnerLocal = hit_shapeTransform(shpInnerLocal, j, HIT_SHAPE_BEGIN, borders.high[j]);
					shpInnerLocal = hit_shapeTransform(shpInnerLocal, j, HIT_SHAPE_END, -borders.low[j]);

					shpBorderOutDev[j][0] = hit_shapeTransform(shpLayout, j, HIT_SHAPE_FIRST, borders.high[j]);
					shpBorderOutDev[j][1] = hit_shapeTransform(shpLayout, j, HIT_SHAPE_LAST, borders.low[j]);
					// TAKE OUT THE PARTS WHICH ARE OVERLAPPED WITH PREVIOUS DIMS
					for (k = 0; k < j; k++) {
						shpBorderOutDev[j][0] = hit_shapeTransform(shpBorderOutDev[j][0], k, HIT_SHAPE_BEGIN, borders.high[k]);
						shpBorderOutDev[j][0] = hit_shapeTransform(shpBorderOutDev[j][0], k, HIT_SHAPE_END, -borders.low[k]);
						shpBorderOutDev[j][1] = hit_shapeTransform(shpBorderOutDev[j][1], k, HIT_SHAPE_BEGIN, borders.high[k]);
						shpBorderOutDev[j][1] = hit_shapeTransform(shpBorderOutDev[j][1], k, HIT_SHAPE_END, -borders.low[k]);
					}
				} else {
					// NULL SIGNATURE
					shpBorderOutDev[j][0] = HIT_SIG_SHAPE_NULL;
					shpBorderOutDev[j][1] = HIT_SIG_SHAPE_NULL;
				}
			}

			for (j = 0; j < dims; j++) {
				for (k = 0; k < dims; k++) {
					if (hit_shapeSig(shpBorderOutDev[j][0], k).begin > hit_shapeSig(shpBorderOutDev[j][0], k).end) {
						shpBorderOutDev[j][0] = HIT_SIG_SHAPE_NULL;
						break;
					}
				}
			}
			for (j = 0; j < dims; j++) {
				for (k = 0; k < dims; k++) {
					if (hit_shapeSig(shpBorderOutDev[j][1], k).begin > hit_shapeSig(shpBorderOutDev[j][1], k).end) {
						shpBorderOutDev[j][1] = HIT_SIG_SHAPE_NULL;
						break;
					}
				}
			}

			/* 4.5. BUILD TILES */
			int num_tiles_total = 1 + 1 + 2 * numBorders + dims * 2;
			HitTile(EPSILOD_BASE_TYPE) * tileList[num_tiles_total];
			HitTile(EPSILOD_BASE_TYPE) * copyList[num_tiles_total];

			HitTile(EPSILOD_BASE_TYPE) tileBorderIn[numBorders];
			HitTile(EPSILOD_BASE_TYPE) tileCopyBorderIn[numBorders];
			HitTile(EPSILOD_BASE_TYPE) tileBorderOut[numBorders];
			HitTile(EPSILOD_BASE_TYPE) tileCopyBorderOut[numBorders];
			HitTile(EPSILOD_BASE_TYPE) tileBorderOutDev[dims][2];
			HitTile(EPSILOD_BASE_TYPE) tileCopyBorderOutDev[dims][2];

			HitTile(EPSILOD_BASE_TYPE) globalMat = Ctrl_Domain(EPSILOD_BASE_TYPE, shp);

			HitTile(EPSILOD_BASE_TYPE) tileMat, tileCopy;

			tileMat  = Ctrl_Select(EPSILOD_BASE_TYPE, globalMat, shpExpanded, CTRL_SELECT_ARR_COORD); //-------------//select tileMat
			tileCopy = Ctrl_Select(EPSILOD_BASE_TYPE, globalMat, shpExpanded, CTRL_SELECT_ARR_COORD); //-------------//select tileCopy

			tileList[0] = &tileMat;
			copyList[0] = &tileCopy;

			Ctrl_Alloc(comm, tileMat);  //---------------------//alloc tileMat
			Ctrl_Alloc(comm, tileCopy); //---------------------//alloc tileCopy

			HitTile(EPSILOD_BASE_TYPE) tileInnerLocal     = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpInnerLocal, CTRL_SELECT_ARR_COORD);
			HitTile(EPSILOD_BASE_TYPE) tileCopyInnerLocal = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpInnerLocal, CTRL_SELECT_ARR_COORD);

			tileList[1] = &tileInnerLocal;
			copyList[1] = &tileCopyInnerLocal;

			for (i = 0; i < numBorders; i++) {
				tileBorderIn[i]      = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpBorderIn[i], CTRL_SELECT_ARR_COORD);   //-------------------//select
				tileCopyBorderIn[i]  = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpBorderIn[i], CTRL_SELECT_ARR_COORD);  //-------------------//select
				tileBorderOut[i]     = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpBorderOut[i], CTRL_SELECT_ARR_COORD);  //-------------------//select
				tileCopyBorderOut[i] = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpBorderOut[i], CTRL_SELECT_ARR_COORD); //-------------------//select

				tileList[2 + 2 * i]     = &tileBorderIn[i];
				copyList[2 + 2 * i]     = &tileCopyBorderIn[i];
				tileList[2 + 2 * i + 1] = &tileBorderOut[i];
				copyList[2 + 2 * i + 1] = &tileCopyBorderOut[i];
			}

			/* ELIMINATE BORDERS EXCEPT IF THEY ARE GLOBAL FOR IO SELECTION */
			HitTile(EPSILOD_BASE_TYPE) io_tile, io_copy;
			HitShape io_shape = hit_tileShape(tileMat);
			for (i = 0; i < dims; i++) {
				/* DIM i FIRST BORDER IS NOT MINE */
				if (!hit_sigIn(hit_shapeSig(io_shape, i), hit_tileDimBegin(globalMat, i))) {
					io_shape = hit_shapeTransform(io_shape, i, HIT_SHAPE_BEGIN, borders.low[i]);
				}
				/* DIM i LAST BORDER IS NOT MINE */
				if (!hit_sigIn(hit_shapeSig(io_shape, i), hit_tileDimEnd(globalMat, i))) {
					io_shape = hit_shapeTransform(io_shape, i, HIT_SHAPE_END, -borders.high[i]);
				}
			}
			// hit_tileSelectArrayCoords(&io_tile, &tileMat, io_shape);
			// hit_tileSelectArrayCoords(&io_copy, &tileCopy, io_shape);
			io_tile = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, io_shape, CTRL_SELECT_ARR_COORD);
			io_copy = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, io_shape, CTRL_SELECT_ARR_COORD);

			for (i = 0; i < dims; i++) {
				if (validShape(shpBorderOutDev[i][0])) {
					tileBorderOutDev[i][0]     = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpBorderOutDev[i][0], CTRL_SELECT_ARR_COORD);  //-------------------//select
					tileCopyBorderOutDev[i][0] = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpBorderOutDev[i][0], CTRL_SELECT_ARR_COORD); //-------------------//select
				} else {
					tileBorderOutDev[i][0]     = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
					tileCopyBorderOutDev[i][0] = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
				}
				if (validShape(shpBorderOutDev[i][1])) {
					tileBorderOutDev[i][1]     = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpBorderOutDev[i][1], CTRL_SELECT_ARR_COORD);  //-------------------//select
					tileCopyBorderOutDev[i][1] = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpBorderOutDev[i][1], CTRL_SELECT_ARR_COORD); //-------------------//select
				} else {
					tileBorderOutDev[i][1]     = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
					tileCopyBorderOutDev[i][1] = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
				}

				tileList[2 + 2 * numBorders + 2 * i]     = &tileBorderOutDev[i][0];
				copyList[2 + 2 * numBorders + 2 * i]     = &tileCopyBorderOutDev[i][0];
				tileList[2 + 2 * numBorders + 2 * i + 1] = &tileBorderOutDev[i][1];
				copyList[2 + 2 * numBorders + 2 * i + 1] = &tileCopyBorderOutDev[i][1];
			}

			// Display extra shape information to help debugging
			#ifdef _PSS_SHP_INFO_DEBUG_
			HitShape temp_shp;
			for (i = 0; i < dims; i++) {
				temp_shp = shpBorderOutDev[i][0];
				printf("[%d] shpBorderOutDev low dim %d: ", hit_Rank, i);
				if (validShape(temp_shp)) {
					dumpShape(temp_shp);
				} else {
					printf("None\n");
				}
				fflush(stdout);
				temp_shp = shpBorderOutDev[i][1];
				printf("[%d] shpBorderOutDev high dim %d: ", hit_Rank, i);
				if (validShape(temp_shp)) {
					dumpShape(temp_shp);
				} else {
					printf("None\n");
				}
				fflush(stdout);
			}
			for (i = 0; i < numBorders; i++) {
				if (!borderInActive[i])
					continue;
				printf("[%d] Border IN (%d): ", hit_Rank, i);
				dumpShape(shpBorderIn[i]);
				fflush(stdout);
			}
			printf("[%d] shpInnerLocal: ", hit_Rank);
			dumpShape(shpInnerLocal);
			fflush(stdout);
			printf("[%d] shpExpanded: ", hit_Rank);
			dumpShape(shpExpanded);
			fflush(stdout);
			printf("[%d] io_shape: ", hit_Rank);
			dumpShape(io_shape);
			fflush(stdout);
			#endif // _PSS_SHP_INFO_DEBUG_

			/* 4.6. BUILD DISTRIBUTED-MEMORY COMMUNICATION PATTERN */
			HitPattern    neighSync     = hit_pattern(HIT_PAT_UNORDERED);
			HitPattern    neighSyncCopy = hit_pattern(HIT_PAT_UNORDERED);
			CommCompIndex sorted_comm_indices[numBorders];
			for (i = 0; i < numBorders; i++) {
				CommCompIndex comm_index = {i, tileBorderIn[i]};
				sorted_comm_indices[i]   = comm_index;
			}
#define EPSILOD_COMM_SORT
			#ifdef EPSILOD_COMM_SORT
			qsort(sorted_comm_indices, numBorders, sizeof(CommCompIndex), compare_comm_tiles);
			#endif
			for (i = sorted_comm_indices[j = 0].index; j < numBorders; i = sorted_comm_indices[++j].index) {
				printOnce("Border comm: index=%d size=%d\n", i, tileBorderIn[i].acumCard);

				// KEEP TRACK OF ACTIVE INPUT/OUTPUT COMMs SEPARATELY
				borderOutActive[i] = borderInActive[i];

				// SKIP EMPTY BORDERS
				if (!borderInActive[i]) continue;

				// LOCATE NEIGHBORS IN THE LAYOUT GRID
				HitRanks neighIn  = hit_layNeighborN(lay, shiftsIn[i]);
				HitRanks neighOut = hit_layNeighborN(lay, shiftsOut[i]);

				// IF NEIGHBOR DOES NOT EXIST, DEACTIVATE BORDER COMMs.
				if (neighIn.rank[0] == HIT_RANK_NULL) {
					// Free tiles before overwriting them
					Ctrl_Free(comm, tileBorderIn[i], tileCopyBorderIn[i]);
					borderInActive[i]   = 0;
					tileBorderIn[i]     = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
					tileCopyBorderIn[i] = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
				}
				if (neighOut.rank[0] == HIT_RANK_NULL) {
					// Free tiles before overwriting them
					Ctrl_Free(comm, tileBorderOut[i], tileCopyBorderOut[i]);
					borderOutActive[i]   = 0;
					tileBorderOut[i]     = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
					tileCopyBorderOut[i] = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
				}

				// IF BOTH NEIGHBORS DO NOT EXIST, SKIP ADDING COMMs
				if (!borderInActive[i] && !borderOutActive[i])
					continue;

				// USE CUDA/HIP MPI AWARE
				if (mpi_dev_aware) {
					tileBorderIn[i].data      = Ctrl_GetDevPtr(comm, tileBorderIn[i]);
					tileBorderOut[i].data     = Ctrl_GetDevPtr(comm, tileBorderOut[i]);
					tileCopyBorderIn[i].data  = Ctrl_GetDevPtr(comm, tileCopyBorderIn[i]);
					tileCopyBorderOut[i].data = Ctrl_GetDevPtr(comm, tileCopyBorderOut[i]);
				}

				// ADD COMMs TO THE PATTERNS
				// printf("CTRL[%d] Border[%d] Adding comm. to the pattern\n", hit_Rank, i);
				hit_patternAdd(&neighSync, hit_comSendRecv(lay, neighOut, &tileBorderOut[i], neighIn, &tileBorderIn[i], HIT_CELL));
				hit_patternAdd(&neighSyncCopy, hit_comSendRecv(lay, neighOut, &tileCopyBorderOut[i], neighIn, &tileCopyBorderIn[i], HIT_CELL));

				// ANNOTATE THE INDEX OF THE BORDER IN THE PATTERN
				indexCommBorder[indexCommBorderCount] = i;
				indexCommBorderCount++;
			}

			/* 4.7. INITIALIZE REDISTRIBUTION STRUCTURES */
			/// HitTile *redistributedTiles[2];
			/// HitPattern *redistributedPatterns[2];

			/// HitAvg avgs = hit_avgSimple(49); // TODO: change windows from 49 (50 - 1) to something that makes more sense.

			/* Kernels characterization */
			/* Inner tile characterization CPUs/GPUs */
			Ctrl_Thread inner_char = comm->type == CTRL_TYPE_CPU ? CPU_INNER_CHAR[dims - 1] : INNER_CHAR[dims - 1];
			/* Borders characterization CPUs/GPUs */
			Ctrl_Thread *border_char = comm->type == CTRL_TYPE_CPU ? &CPU_BORDER_CHAR[dims - 1][0] : &BORDER_CHAR[dims - 1][0];

			EpsilodThreads computation_threads;
			computation_threads.inner = initCtrlThreadFromTile(dims, &tileInnerLocal);
			Ctrl_Thread thr_border_out_dev[dims][2];
			computation_threads.border_out_dev = thr_border_out_dev;
			for (int i = 0; i < dims; i++) {
				for (int j = 0; j < 2; j++) {
					thr_border_out_dev[i][j] = initCtrlThreadFromTile(dims, &tileBorderOutDev[i][j]);
				}
			}

			/* 4.8. INITIALIZE ARRAY */
			printOnce("Init stage\n");
			fflush(stdout);

			EpsilodCoords global;
			global.dims      = dims;
			global.size[0]   = sizes[0];
			global.size[1]   = (dims > 1) ? sizes[1] : 0;
			global.size[2]   = (dims > 2) ? sizes[2] : 0;
			global.offset[0] = hit_shapeSig(shpExpanded, 0).begin;
			global.offset[1] = hit_shapeSig(shpExpanded, 1).begin;
			global.offset[2] = hit_shapeSig(shpExpanded, 2).begin;
			global.borders   = borders;

			EpsilodCoords global_inner_local = global;
			global_inner_local.offset[0]     = hit_shapeSig(shpInnerLocal, 0).begin;
			global_inner_local.offset[1]     = hit_shapeSig(shpInnerLocal, 1).begin;
			global_inner_local.offset[2]     = hit_shapeSig(shpInnerLocal, 2).begin;

			EpsilodCoords global_border_out_dev[dims][2];
			for (int i = 0; i < dims; i++) {
				for (int j = 0; j < 2; j++) {
					global_border_out_dev[i][j]           = global;
					global_border_out_dev[i][j].offset[0] = hit_shapeSig(shpBorderOutDev[i][j], 0).begin;
					global_border_out_dev[i][j].offset[1] = hit_shapeSig(shpBorderOutDev[i][j], 1).begin;
					global_border_out_dev[i][j].offset[2] = hit_shapeSig(shpBorderOutDev[i][j], 2).begin;
				}
			}

			// printf(
			// 	"Global coords:\n"
			// 	"\toffset io: %d %d %d\n"
			// 	"\toffset inner: %d %d %d\n"
			// 	"\toffset border out dev low: %d %d %d\n"
			// 	"\toffset border out dev high: %d %d %d\n",
			// 	global.offset[0],
			// 	global.offset[1],
			// 	global.offset[2],
			// 	global_inner_local.offset[0],
			// 	global_inner_local.offset[1],
			// 	global_inner_local.offset[2],
			// 	global_border_out_dev_low[0].offset[0],
			// 	global_border_out_dev_low[0].offset[1],
			// 	global_border_out_dev_low[0].offset[2],
			// 	global_border_out_dev_high[0].offset[0],
			// 	global_border_out_dev_high[0].offset[1],
			// 	global_border_out_dev_high[0].offset[2]);

			/* BUILD COMPUTATION STRUCTS */
			EpsilodTiles           tiles;
			EpsilodTiles           copy_tiles;
			EpsilodProperties      props;
			EpsilodComputationArgs computation_args;
			EpsilodCommArgs        comm_args;
			EpsilodSwapStructs     swap_structs;
			EpsilodSwapStructs     swap_structs_copy;

			props.dims     = dims;
			props.topology = topo;

			computation_args.tiles                      = &tiles;
			computation_args.tiles->mat                 = &tileMat;
			computation_args.tiles->inner_local         = &tileInnerLocal;
			computation_args.tiles->border_out_dev      = tileBorderOutDev;
			computation_args.copy_tiles                 = &copy_tiles;
			computation_args.copy_tiles->mat            = &tileCopy;
			computation_args.copy_tiles->inner_local    = &tileCopyInnerLocal;
			computation_args.copy_tiles->border_out_dev = tileCopyBorderOutDev;

			computation_args.threads = &computation_threads;

			computation_args.chars                 = &(EpsilodChars){};
			computation_args.chars->inner          = inner_char;
			computation_args.chars->border_out_dev = border_char;

			computation_args.coords                 = &(EpsilodGlobalCoords){};
			computation_args.coords->inner          = global_inner_local;
			computation_args.coords->border_out_dev = global_border_out_dev;

			computation_args.stencil    = &stencil;
			computation_args.factor     = factor;
			computation_args.ext_params = ext_params;

			swap_structs.comm_pattern         = &neighSync;
			swap_structs.tile_list            = tileList;
			swap_structs.tile_list_count      = num_tiles_total;
			swap_structs_copy.comm_pattern    = &neighSyncCopy;
			swap_structs_copy.tile_list       = copyList;
			swap_structs_copy.tile_list_count = num_tiles_total;

			comm_args.comm_pattern      = &neighSync;
			comm_args.tile_border_in    = tileBorderIn;
			comm_args.border_count      = numBorders;
			comm_args.border_in_active  = borderInActive;
			comm_args.index_comm_border = indexCommBorder;

			/* 4.8.1. FIRST STAGE (Optional): INITIALIZATION IN HOST */
			if (f_init != NULL) {
				printOnce("\tInitializing in host.\n");
				fflush(stdout);
				f_init(tileMat, global, ext_params);
				Ctrl_HostTask(epsilod_host_touch, tileMat);
				Ctrl_MoveTo(comm, tileMat);
				Ctrl_WaitTile(comm, tileMat);
			}

			/* 4.8.2. SECOND STAGE (Optional): INITIALIZATION IN DEVICE */
			Ctrl_Thread threads_touch   = {.dims = 1, .i = 1, .j = 0, .k = 0};
			Ctrl_Thread blocksize_touch = {.dims = 1, .i = 1, .j = 1, .k = 1};
			if (f_dev_init != NULL) {
				printOnce("\tInitializing in device.\n");
				fflush(stdout);
				Ctrl_Thread exp_threads = initCtrlThreadFromTile(dims, &tileMat);
				f_dev_init(comm, exp_threads, inner_char, 0, tileMat, global, ext_params);
			}

			bool tiles_marked = false;
			/* 4.8.3. INITIALIZE COPY */
			if (f_init_copy == NULL) {
				// #define EPSILOD_INITIALIZE_COPY_IN_HOST
				#ifdef EPSILOD_INITIALIZE_COPY_IN_HOST
				printOnce("\tInitializing copy in host.\n");
				fflush(stdout);
				// Host: Initialize data in the copy
				char *omp_env     = getenv("OMP_NUM_THREADS");
				int   omp_threads = (omp_env != NULL) ? atoi(omp_env) : 1;
				#pragma omp parallel for num_threads(omp_threads)
				for (int i = 0; i < tileMat.acumCard; i++)
					hit(tileCopy, i) = hit(tileMat, i);
				// Send tileCopy to the device
				Ctrl_MoveTo(comm, tileCopy);
				Ctrl_WaitTile(comm, tileCopy);
				#else // INITIALIZE COPY IN DEVICE
				printOnce("\tInitializing copy in device.\n");
				fflush(stdout);
				Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, tileMat);
				Ctrl_Thread threads_flat = {.dims = 1, .i = tileMat.acumCard, .j = 1, .k = 1};
				// TODO: the number of threads was reduced from 512 to 256 to support OpenMP.
				// It should be queried
				Ctrl_Thread blocksize_flat = {.dims = 1, .i = 256, .j = 1, .k = 1};
				Ctrl_Launch(comm, epsilod_dev_copy, threads_flat, blocksize_flat, tileMat, tileCopy);
				// TODO: remove when Ctrl_MoveTo's of tileBorderIn are properly optimised
				// The Ctrl_MoveTo's are currently overwriting border data in the device
				Ctrl_MoveFrom(comm, tileMat);
				Ctrl_WaitTile(comm, tileMat);
				Ctrl_MoveFrom(comm, tileCopy);
				Ctrl_WaitTile(comm, tileCopy);
				#endif
			} else {
				markTiles(comm, props, threads_touch, blocksize_touch, &tiles, &copy_tiles, &comm_args);
				tiles_marked                  = true;
				computation_args.f_updateCell = f_init_copy;
				doStep(comm, &props, &computation_args, &swap_structs, &swap_structs_copy, &comm_args);
				swap(&swap_structs, &swap_structs_copy);
			}

			/* 4.8.4. MARK INITIALIZATION OF SUBSELECTIONS AND COPIES, ELIMINATE WARNINGS */
			if (!tiles_marked)
				markTiles(comm, props, threads_touch, blocksize_touch, &tiles, &copy_tiles, &comm_args);

			hit_clockStop(initClock);

			Ctrl_Synchronize();
			hit_comBarrier(lay);

			/* 4.9. COMPUTATION LOOP */
			printOnce("Computation stage\n");
			fflush(stdout);
			hit_clockStart(loopClock);

			computation_args.f_updateCell = f_updateCell;

			for (int loopIndex = 0; loopIndex < numIterations - 1; loopIndex++) {
				doStep(comm, &props, &computation_args, &swap_structs, &swap_structs_copy, &comm_args);
			}

			/* 4.10. LAST ITERATION UPDATE: NO COMMUNICATION AFTER */
			if (numIterations > 0) {
				doStepNoComms(comm, &props, &computation_args, &swap_structs, &swap_structs_copy);
			}

			printOnce(" End\n");
			fflush(stdout);

			// Move matrix to the host:
			Ctrl_MoveFrom(comm, tileMat);
			Ctrl_WaitTile(comm, tileMat);

			// Ctrl_HostTask(comm, print_matrix, tileMat, ext_params);

			/* 4.11. CLOCK RESULTS */
			hit_clockStop(mainClock);
			hit_clockStop(loopClock);

			reduceClocks(lay);
			printClockInfo();

			/* 4.12. WRITE RESULT MATRIX */
			f_output(numIterations % 2 == 0 ? io_tile : io_copy, ext_params);
			hit_patternFree(&neighSync);
			hit_patternFree(&neighSyncCopy);

			printOnce("Output finished\n");
			fflush(stdout);

			/* FREE TILES */
			printOnce("Free tiles\n");
			fflush(stdout);
			for (i = 0; i < num_tiles_total; i++) {
				Ctrl_Free(comm, *tileList[i]);
				Ctrl_Free(comm, *copyList[i]);
			}
			Ctrl_Free(comm, io_tile);
			Ctrl_Free(comm, io_copy);
		} // layactive

		/* 5. INACTIVE PROCESSES: ONLY COLLECTIVE CLOCK OPERATIONS */
		else {
			fprintf(stderr, "[%d] Warning, process not active\n", hit_Rank);

			hit_clockStop(initClock);
			hit_clockStop(mainClock);
			hit_clockStop(loopClock);

			reduceClocks(lay);
			printClockInfo();
		}

		/* 6. FREE OTHER RESOURCES */
		printOnce("Free data structures\n");
		fflush(stdout);
		Ctrl_Free(comm, stencil);
		hit_layFree(lay);
		hit_topFree(topo);
		// v1.2 @Arturo, new predefined size type for borders
		// free(borderLow);
		// free(borderHigh);

		printOnce("Stop distributed Controllers\n");
		fflush(stdout);
		Ctrl_EndBlock();

		printOnce("Epsilod End\n");
		fflush(stdout);
	}
	#if HIT2_COUNTPARAM(EPSILOD_BASE_TYPE_COMPOUND) == 2
	hit_comFreeType(HIT_CELL);
	#endif
}
