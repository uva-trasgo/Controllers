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
#endif // _CTRL_EXAMPLES_EXP_MODE_

#define EPSILOD_COMM_SORT
#define EPSILOD_COMM_ANY
// #define EPSILOD_COMM_ANY_RECV_FIRST
// #define EPSILOD_INITIALIZE_COPY_IN_HOST

#define validShape(s) (hit_shapeDims((s)) != (-1))

// use of cuda/hip-aware mpi
int mpi_dev_aware;

/* A. Kernel characterizations */
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

/* B. Generic kernel prototype and wrapper launchers */
#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
CTRL_KERNEL_CHAR(updateCell_default_1D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_2D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_3D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_4D, MANUAL, 0, 0, 0);

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

CTRL_KERNEL_PROTO(updateCell_default_4D,
				  1, GENERIC, DEFAULT, 6,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,
				  INVAL, EpsilodCoords, global_coords,
				  IN, HitTile(float), weight,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_4D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_4D, threads, blockSize, stream, mat, copy, global, weight, factor, *ext_params);
}
#endif // EPSILOD_BASE_TYPE != float

CTRL_KERNEL_CHAR(epsilod_dev_copy_1d, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_copy_1d,
				  1, GENERIC, DEFAULT, 2,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrix,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix_out);

CTRL_KERNEL_CHAR(epsilod_dev_copy_2d, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_copy_2d,
				  1, GENERIC, DEFAULT, 2,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrix,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix_out);

CTRL_KERNEL_CHAR(epsilod_dev_copy_3d, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_copy_3d,
				  1, GENERIC, DEFAULT, 2,
				  IN, HitTile(EPSILOD_BASE_TYPE), matrix,
				  OUT, HitTile(EPSILOD_BASE_TYPE), matrix_out);

// TODO epsilod_dev_copy_4d

/* D. False initialization of selections to avoid non-initialized warnings */
CTRL_KERNEL_CHAR(epsilod_dev_touch, MANUAL, 0, 0, 0);
CTRL_KERNEL_PROTO(epsilod_dev_touch,
				  1, GENERIC, DEFAULT,
				  1, OUT, HitTile(EPSILOD_BASE_TYPE), matrix);

CTRL_HOST_TASK(epsilod_host_touch, HitTile(EPSILOD_BASE_TYPE) matrix) { ; }
CTRL_HOST_TASK_PROTO(epsilod_host_touch, 1, OUT, HitTile(EPSILOD_BASE_TYPE), matrix);

/* E. Output host-task wrapper */
CTRL_HOST_TASK(Ctrl_Write_Output, outputDataFunction f_output, HitTile(EPSILOD_BASE_TYPE) matrix, Epsilod_ext *ext_params) {
	f_output(matrix, ext_params);
}

CTRL_HOST_TASK_PROTO(Ctrl_Write_Output, 3,
					 INVAL, outputDataFunction, f_output,
					 IN, HitTile(EPSILOD_BASE_TYPE), matrix,
					 INVAL, Epsilod_ext *, ext_params);

/* F. Stencil pattern transference host-task */
CTRL_HOST_TASK(Ctrl_Copy_Stencil, HitTile_float stencil, float *stencil_data) {
	for (int i = 0; i < hit_tileCard(stencil); i++)
		hit(stencil, i) = stencil_data[i];
}

CTRL_HOST_TASK_PROTO(Ctrl_Copy_Stencil, 2,
					 OUT, HitTile_float, stencil,
					 INVAL, float *, stencil_data);

typedef struct {
	int index;
	HitTile(EPSILOD_BASE_TYPE) tile;
} CommCompIndex;

/**
 * Compare tiles to order communications by number of elements.
 * qsort sorts in ascending order a less than b -> return negative
 */
int compare_comm_tiles(const void *a, const void *b) {
	CommCompIndex index_a = *(CommCompIndex *)a;
	CommCompIndex index_b = *(CommCompIndex *)b;
	if (index_a.tile.acumCard > index_b.tile.acumCard)
		return -1;
	if (index_a.tile.acumCard < index_b.tile.acumCard)
		return 1;
	return 0;
}

/* H. Experimentation: global clocks */
HitClock mainClock;
HitClock initClock;
HitClock loopClock;
HitClock commClock;

/* Print extra options of epsilod using environment variables */
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
	hit_clockPrintMax(commClock);
	#endif // _CTRL_EXAMPLES_EXP_MODE_
	fflush(stdout);
}

void reduceClocks(HitLayout layout) {
	hit_clockReduce(layout, mainClock);
	hit_clockReduce(layout, initClock);
	hit_clockReduce(layout, loopClock);
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
		// Skip empty borders
		if (!comm_args->border_in_active[i]) continue;
		Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, comm_args->tile_border_in[i]);
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
		default:
			Ctrl_ThreadInit(tile_threads,
							hit_tileDimCard(*p_tile, 0),
							hit_tileDimCard(*p_tile, 1),
							hit_tileDimCard(*p_tile, 2));
			break;
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

			// Start move-to for recv
			int border = args->index_comm_border[endComm];

			Ctrl_HostTask(epsilod_host_touch, args->tile_border_in[border]);
			Ctrl_MoveTo(comm, args->tile_border_in[border]);
		}
		hit_patternEndAsync(*args->comm_pattern);

		for (int i = 0; i < args->border_count; i++) {
			// Skip empty borders
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

			// Skip sends
			if (endComm % 2 == 0) continue;

			// Start move-to for recv
			int border = args->index_comm_border[endComm / 2];

			Ctrl_HostTask(epsilod_host_touch, args->tile_border_in[border]);
			Ctrl_MoveTo(comm, args->tile_border_in[border]);
		}

		for (int i = 0; i < args->border_count; i++) {
			// Skip empty borders
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
			// Skip empty borders
			if (!args->border_in_active[i]) continue;
			Ctrl_HostTask(epsilod_host_touch, args->tile_border_in[i]);
			Ctrl_MoveTo(comm, args->tile_border_in[i]);
		}
		for (int i = 0; i < args->border_count; i++) {
			// Skip empty borders
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

	// Compute borders
	for (int i = 0; i < props->dims; i++) {
		if (validShape(tiles->border_out_dev[i][0].shape) && validShape(copy_tiles->border_out_dev[i][0].shape)) {
			f_updateCell(comm, threads->border_out_dev[i][0], chars->border_out_dev[i], 2 * i, tiles->border_out_dev[i][0], copy_tiles->border_out_dev[i][0], coords->border_out_dev[i][0], *stencil, factor, ext_params);
		}
		if (validShape(tiles->border_out_dev[i][1].shape) && validShape(copy_tiles->border_out_dev[i][1].shape)) {
			f_updateCell(comm, threads->border_out_dev[i][1], chars->border_out_dev[i], 2 * i + 1, tiles->border_out_dev[i][1], copy_tiles->border_out_dev[i][1], coords->border_out_dev[i][1], *stencil, factor, ext_params);
		}
	}

	// Sync borders before inner
	for (int i = 0; i < props->dims; i++) {
		if (validShape(tiles->border_out_dev[i][0].shape) && validShape(copy_tiles->border_out_dev[i][0].shape)) {
			Ctrl_WaitTile(comm, tiles->border_out_dev[i][0]);
		}
		if (validShape(tiles->border_out_dev[i][1].shape) && validShape(copy_tiles->border_out_dev[i][1].shape)) {
			Ctrl_WaitTile(comm, tiles->border_out_dev[i][1]);
		}
	}

	// Compute inner
	if (validShape(tiles->inner_local->shape) && validShape(copy_tiles->inner_local->shape)) {
		f_updateCell(comm, threads->inner, chars->inner, 0, *tiles->mat, *copy_tiles->mat, coords->inner, *stencil, factor, ext_params);
	}
}

void doStep(
	PCtrl comm, EpsilodProperties *props,
	EpsilodComputationArgs *computation_args, EpsilodSwapStructs *swap_structs, EpsilodSwapStructs *swap_structs_copy, EpsilodCommArgs *comm_args) {

	EpsilodTiles *tiles      = computation_args->tiles;
	EpsilodTiles *copy_tiles = computation_args->copy_tiles;

	// Update tile copy
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

	// Wait for kernels to finish
	for (int i = 0; i < props->dims; i++)
		for (int j = 0; j < 2; j++)
			if (validShape(tiles->border_out_dev[i][j].shape))
				Ctrl_WaitTile(comm, tiles->border_out_dev[i][j]);
	Ctrl_WaitTile(comm, *tiles->inner_local);
}

void stencilComputation(
	HitInd                 sizes[],
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

	// Hitmap comm. type
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

	// External/extra parameters
	Epsilod_ext *ext_params = (ext_params_arg == NULL) ? &(Epsilod_ext){0} : ext_params_arg;

	int dims = hit_shapeDims(stencilShape);

	// Check if generic kernel has been chosen
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
			case 4:
				f_updateCell = updateCell_default_4D;
				break;
			default:
				fprintf(stderr, "[Parallel Stencil Skeleton ERROR] Stencil with invalid number of dimensions. The skeleton only supports 1D, 2D or 3D stencils.\n");
				fflush(stderr);
				exit(EXIT_FAILURE);
		}
		#endif // float
	}

	// Read env: use CUDA/HIP MPI aware
	mpi_dev_aware = hit_envNoYes("EPSILOD_MPI_DEV_AWARE");
	printOnce("Epsilod Using Device-Aware MPI: %c\n", (mpi_dev_aware) ? 'y' : 'n');
	if (mpi_dev_aware)
		printOnce("Note: Device-Aware MPI only works if it is suported and activated in the MPI layer\n");

	// Initialize device Controllers
	__ctrl_block__(device_selection_file) {
		// init clocks
		hit_clockSynchronizeAll();
		hit_clockStart(mainClock);
		hit_clockStart(initClock);

		PCtrl comm = Ctrl_Get(0);

		printf("MPI_Rank[%d]", hit_Rank);
		Ctrl_PrintInfo();
		printf("\n");
		fflush(stdout);

		Ctrl_SetDependanceMode(comm, CTRL_MODE_EXPLICIT);

		/* 0. Build the stencil tile */
		HitTile_float stencil = Ctrl_DomainAlloc(comm, float, stencilShape);
		Ctrl_HostTask(Ctrl_Copy_Stencil, stencil, stencilData);
		Ctrl_MoveTo(comm, stencil);
		Ctrl_WaitTile(comm, stencil);

		/* 1. Build original shape */
		HitShape shp = HIT_SHAPE_NULL;
		hit_shapeDimsSet(shp, dims);
		for (int i = 0; i < dims; i++) {
			hit_shapeSig(shp, i) = hit_sig(0, sizes[i] - 1, 1);
		}

		/* 2. Shortcuts for border sizes */
		EpsilodBorders borders;
		for (int i = 0; i < dims; i++) {
			borders.low[i]  = -hit_tileDimBegin(stencil, i);
			borders.high[i] = hit_tileDimEnd(stencil, i);
		}

		/* 3.1. Shape to distribute computation (without borders) */
		HitShape shpInner = shp;
		for (int i = 0; i < dims; i++) {
			shpInner = hit_shapeTransform(shpInner, i, HIT_SHAPE_BEGIN, +borders.low[i]);
			shpInner = hit_shapeTransform(shpInner, i, HIT_SHAPE_END, -borders.high[i]);
		}

		/* 3.2. Select and build partition/distribution */
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

		/* 3.2. Build distributed shape */
		int         topoDims;
		HitTopology topo;
		HitLayout   lay;
		HitWeights  weights = Ctrl_ConfigWeights();

		// Topology
		if (partition_dim_count == -1) {
			topoDims = 1;
			topo     = hit_topology(plug_topPlain);

			// Weighted distribution
			if (use_weights) {
				lay = hit_layout(plug_layDimWeighted_Blocks, topo, shpInner, partition_dim, weights);
			}
			// Regular distribution
			else {
				// TODO: use the proper layout
				lay = hit_layout(plug_layDimBlocks, topo, shpInner, partition_dim);
			}
		} else {
			if (partition_dim_count < 1 || partition_dim_count > dims) {
				fprintf(stderr, "\nError EPSILOD: Number of dimensions for processes topology should be in the range [1:dims]. "
								"Current dims: %d Current topology dims: %d\n\n",
						dims, partition_dim_count);
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

			// 4.1. Stop if there is not enough inner data for a border
			for (int i = 0; i < dims; i++) {
				if (hit_shapeSigCard(shpLayout, i) < borders.low[i] ||
					hit_shapeSigCard(shpLayout, i) < borders.high[i]) {
					if (hit_Rank == 0) {
						fprintf(stderr, "\nError: Not enough data after partition, too many processes in a topology axis\n\n");
						MPI_Abort(MPI_COMM_WORLD, MPI_ERR_TOPOLOGY);
						exit(EXIT_FAILURE);
					}
				}
			}

			/* 4.2. Expanded shape */
			HitShape shpExpanded = shpLayout;
			for (int i = 0; i < dims; i++) {
				shpExpanded = hit_shapeTransform(shpExpanded, i, HIT_SHAPE_BEGIN, -borders.low[i]);
				shpExpanded = hit_shapeTransform(shpExpanded, i, HIT_SHAPE_END, borders.high[i]);
			}

			/* 4.3. Border shapes: in/out */
			int      numBorders = (int)pow(3, dims);
			HitShape shpBorderIn[numBorders];
			HitShape shpBorderOut[numBorders];
			HitRanks shiftsIn[numBorders];
			HitRanks shiftsOut[numBorders];
			int      borderInActive[numBorders];
			int      borderOutActive[numBorders];
			int      indexCommBorder[numBorders];
			int      indexCommBorderCount = 0;
			for (int i = 0; i < numBorders; i++)
				borderInActive[i] = 0;

			// Traverse the stencil to detect active and inactive borders due to weights
			int indeces[dims];
			int displacement = 0;
			for (int j = 0; j < dims; j++)
				indeces[j] = 0;
			int endAnalysis = 0;
			while (!endAnalysis) {
				// Check if there is a weight in the stencil position
				if (stencilData[displacement] != 0) {
					// Active border, compute its number to raise the flag
					int acum   = 1;
					int border = 0;
					for (int j = dims - 1; j >= 0; j--) {
						if (indeces[j] > borders.low[j])
							border += 2 * acum;
						else if (indeces[j] == borders.low[j])
							border += acum;
						acum *= 3;
					}
					borderInActive[border] = 1;
				}
				// Advance to the next stencil position
				displacement++;
				for (int k = dims - 1; k >= 0; k--) {
					indeces[k]++;
					if (k == 0 && indeces[0] == hit_tileDimCard(stencil, 0)) endAnalysis = 1;
					if (indeces[k] >= hit_tileDimCard(stencil, k))
						indeces[k] = 0;
					else
						break;
				}
			}
			// Always skip false border: tile inner
			borderInActive[numBorders / 2] = 0;

			// Build border shapes and the shortcuts of the neighbor shifts
			for (int i = 0; i < numBorders; i++) {
				shiftsIn[i]  = HIT_RANKS_NULL;
				shiftsOut[i] = HIT_RANKS_NULL;

				// Non-active borders, null shapes, null ranks
				if (!borderInActive[i]) {
					shpBorderIn[i]  = HIT_SHAPE_NULL;
					shpBorderOut[i] = HIT_SHAPE_NULL;
					continue;
				}
				shpBorderIn[i]  = shpLayout;
				shpBorderOut[i] = shpLayout;

				// Extract ranks for this border
				int digits = i;
				for (int j = 0; j < dims; j++) {
					shiftsIn[i].rank[j]  = digits % 3 - 1;
					shiftsOut[i].rank[j] = -shiftsIn[i].rank[j];
					digits /= 3;

					// Shape in
					if (shiftsIn[i].rank[j] == -1) {
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_FIRST, borders.low[j]);
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, -borders.low[j]);
					} else if (shiftsIn[i].rank[j] == 1) {
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_LAST, borders.high[j]);
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, borders.high[j]);
					}

					// Shape out (reverserd target)
					if (shiftsIn[i].rank[j] == -1) {
						shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_LAST, borders.low[j]);
					} else if (shiftsIn[i].rank[j] == 1) {
						shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_FIRST, borders.high[j]);
					}
				}
			}

			/* 4.4. Non-overlaped borders in the inner part
				a) To extract data from device without replication
				b) To determine the threads-grid cardinalities for the computing kernels
				@arturo Aug 2024: Skip dimensions not selected in the topology
			*/
			HitShape shpInnerLocal = shpLayout;
			HitShape shpBorderOutDev[dims][2];
			for (int j = 0; j < dims; j++) {
				if (j < topoDims) {
					shpInnerLocal = hit_shapeTransform(shpInnerLocal, j, HIT_SHAPE_BEGIN, borders.high[j]);
					shpInnerLocal = hit_shapeTransform(shpInnerLocal, j, HIT_SHAPE_END, -borders.low[j]);

					shpBorderOutDev[j][0] = hit_shapeTransform(shpLayout, j, HIT_SHAPE_FIRST, borders.high[j]);
					shpBorderOutDev[j][1] = hit_shapeTransform(shpLayout, j, HIT_SHAPE_LAST, borders.low[j]);
					// Take out the parts which are overlapped with previous dims
					for (int k = 0; k < j; k++) {
						shpBorderOutDev[j][0] = hit_shapeTransform(shpBorderOutDev[j][0], k, HIT_SHAPE_BEGIN, borders.high[k]);
						shpBorderOutDev[j][0] = hit_shapeTransform(shpBorderOutDev[j][0], k, HIT_SHAPE_END, -borders.low[k]);
						shpBorderOutDev[j][1] = hit_shapeTransform(shpBorderOutDev[j][1], k, HIT_SHAPE_BEGIN, borders.high[k]);
						shpBorderOutDev[j][1] = hit_shapeTransform(shpBorderOutDev[j][1], k, HIT_SHAPE_END, -borders.low[k]);
					}
				} else {
					// NULL signature
					shpBorderOutDev[j][0] = HIT_SIG_SHAPE_NULL;
					shpBorderOutDev[j][1] = HIT_SIG_SHAPE_NULL;
				}
			}

			for (int j = 0; j < dims; j++) {
				for (int k = 0; k < dims; k++) {
					if (hit_shapeSig(shpBorderOutDev[j][0], k).begin > hit_shapeSig(shpBorderOutDev[j][0], k).end) {
						shpBorderOutDev[j][0] = HIT_SIG_SHAPE_NULL;
						break;
					}
				}
			}
			for (int j = 0; j < dims; j++) {
				for (int k = 0; k < dims; k++) {
					if (hit_shapeSig(shpBorderOutDev[j][1], k).begin > hit_shapeSig(shpBorderOutDev[j][1], k).end) {
						shpBorderOutDev[j][1] = HIT_SIG_SHAPE_NULL;
						break;
					}
				}
			}

			/* 4.5. Build tiles */
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

			tileMat  = Ctrl_Select(EPSILOD_BASE_TYPE, globalMat, shpExpanded, CTRL_SELECT_ARR_COORD);
			tileCopy = Ctrl_Select(EPSILOD_BASE_TYPE, globalMat, shpExpanded, CTRL_SELECT_ARR_COORD);

			tileList[0] = &tileMat;
			copyList[0] = &tileCopy;

			Ctrl_Alloc(comm, tileMat);
			Ctrl_Alloc(comm, tileCopy);

			HitTile(EPSILOD_BASE_TYPE) tileInnerLocal     = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpInnerLocal, CTRL_SELECT_ARR_COORD);
			HitTile(EPSILOD_BASE_TYPE) tileCopyInnerLocal = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpInnerLocal, CTRL_SELECT_ARR_COORD);

			tileList[1] = &tileInnerLocal;
			copyList[1] = &tileCopyInnerLocal;

			for (int i = 0; i < numBorders; i++) {
				tileBorderIn[i]      = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpBorderIn[i], CTRL_SELECT_ARR_COORD);
				tileCopyBorderIn[i]  = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpBorderIn[i], CTRL_SELECT_ARR_COORD);
				tileBorderOut[i]     = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpBorderOut[i], CTRL_SELECT_ARR_COORD);
				tileCopyBorderOut[i] = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpBorderOut[i], CTRL_SELECT_ARR_COORD);

				tileList[2 + 2 * i]     = &tileBorderIn[i];
				copyList[2 + 2 * i]     = &tileCopyBorderIn[i];
				tileList[2 + 2 * i + 1] = &tileBorderOut[i];
				copyList[2 + 2 * i + 1] = &tileCopyBorderOut[i];
			}

			// Eliminate borders except if they are global for io selection
			HitTile(EPSILOD_BASE_TYPE) io_tile, io_copy;
			HitShape io_shape = hit_tileShape(tileMat);
			for (int i = 0; i < dims; i++) {
				// Dim i first border is not mine
				if (!hit_sigIn(hit_shapeSig(io_shape, i), hit_tileDimBegin(globalMat, i))) {
					io_shape = hit_shapeTransform(io_shape, i, HIT_SHAPE_BEGIN, borders.low[i]);
				}
				// Dim i last border is not mine
				if (!hit_sigIn(hit_shapeSig(io_shape, i), hit_tileDimEnd(globalMat, i))) {
					io_shape = hit_shapeTransform(io_shape, i, HIT_SHAPE_END, -borders.high[i]);
				}
			}
			io_tile = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, io_shape, CTRL_SELECT_ARR_COORD);
			io_copy = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, io_shape, CTRL_SELECT_ARR_COORD);

			for (int i = 0; i < dims; i++) {
				if (validShape(shpBorderOutDev[i][0])) {
					tileBorderOutDev[i][0]     = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpBorderOutDev[i][0], CTRL_SELECT_ARR_COORD);
					tileCopyBorderOutDev[i][0] = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpBorderOutDev[i][0], CTRL_SELECT_ARR_COORD);
				} else {
					tileBorderOutDev[i][0]     = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
					tileCopyBorderOutDev[i][0] = *(HitTile(EPSILOD_BASE_TYPE) *)&HIT_TILE_NULL;
				}
				if (validShape(shpBorderOutDev[i][1])) {
					tileBorderOutDev[i][1]     = Ctrl_Select(EPSILOD_BASE_TYPE, tileMat, shpBorderOutDev[i][1], CTRL_SELECT_ARR_COORD);
					tileCopyBorderOutDev[i][1] = Ctrl_Select(EPSILOD_BASE_TYPE, tileCopy, shpBorderOutDev[i][1], CTRL_SELECT_ARR_COORD);
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
			for (int i = 0; i < dims; i++) {
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
			for (int i = 0; i < numBorders; i++) {
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

			/* 4.6. Build distributed-memory communication pattern */
			HitPattern    neighSync     = hit_pattern(HIT_PAT_UNORDERED);
			HitPattern    neighSyncCopy = hit_pattern(HIT_PAT_UNORDERED);
			CommCompIndex sorted_comm_indices[numBorders];
			for (int i = 0; i < numBorders; i++) {
				CommCompIndex comm_index = {i, tileBorderIn[i]};
				sorted_comm_indices[i]   = comm_index;
			}
			#ifdef EPSILOD_COMM_SORT
			qsort(sorted_comm_indices, numBorders, sizeof(CommCompIndex), compare_comm_tiles);
			#endif
			for (int j = 0, i = sorted_comm_indices[j].index; j < numBorders; i = sorted_comm_indices[++j].index) {
				printOnce("Border comm: index=%d size=%ld\n", i, tileBorderIn[i].acumCard);

				// Keep track of active input/output comms separately
				borderOutActive[i] = borderInActive[i];

				// Skip empty borders
				if (!borderInActive[i]) continue;

				// Locate neighbors in the layout grid
				HitRanks neighIn  = hit_layNeighborN(lay, shiftsIn[i]);
				HitRanks neighOut = hit_layNeighborN(lay, shiftsOut[i]);

				// If neighbor does not exist, deactivate border comms
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

				// If both neighbors do not exist, skip adding comms
				if (!borderInActive[i] && !borderOutActive[i])
					continue;

				// Use CUDA/HIP MPI aware
				if (mpi_dev_aware) {
					tileBorderIn[i].data      = Ctrl_GetDevPtr(comm, tileBorderIn[i]);
					tileBorderOut[i].data     = Ctrl_GetDevPtr(comm, tileBorderOut[i]);
					tileCopyBorderIn[i].data  = Ctrl_GetDevPtr(comm, tileCopyBorderIn[i]);
					tileCopyBorderOut[i].data = Ctrl_GetDevPtr(comm, tileCopyBorderOut[i]);
				}

				// Add comms to the patterns
				hit_patternAdd(&neighSync, hit_comSendRecv(lay, neighOut, &tileBorderOut[i], neighIn, &tileBorderIn[i], HIT_CELL));
				hit_patternAdd(&neighSyncCopy, hit_comSendRecv(lay, neighOut, &tileCopyBorderOut[i], neighIn, &tileCopyBorderIn[i], HIT_CELL));

				// Annotate the index of the border in the pattern
				indexCommBorder[indexCommBorderCount] = i;
				indexCommBorderCount++;
			}

			// Kernel characterizations
			int          char_dims   = (dims > 3) ? 3 : dims;
			Ctrl_Thread  inner_char  = comm->type == CTRL_TYPE_CPU ? CPU_INNER_CHAR[char_dims - 1] : INNER_CHAR[char_dims - 1];
			Ctrl_Thread *border_char = comm->type == CTRL_TYPE_CPU ? &CPU_BORDER_CHAR[char_dims - 1][0] : &BORDER_CHAR[char_dims - 1][0];

			EpsilodThreads computation_threads;
			computation_threads.inner = initCtrlThreadFromTile(dims, &tileMat);
			Ctrl_Thread thr_border_out_dev[dims][2];
			computation_threads.border_out_dev = thr_border_out_dev;
			for (int i = 0; i < dims; i++) {
				for (int j = 0; j < 2; j++) {
					thr_border_out_dev[i][j] = initCtrlThreadFromTile(dims, &tileBorderOutDev[i][j]);
				}
			}

			/* 4.8. Initialize array */
			printOnce("Init stage\n");
			fflush(stdout);

			EpsilodCoords global = {0};
			global.dims          = dims;
			for (int i = 0; i < dims; i++) {
				global.size[i]   = sizes[i];
				global.offset[i] = hit_shapeSig(shpExpanded, i).begin;
			}
			global.borders = borders;

			EpsilodCoords global_inner_local = global;
			EpsilodCoords global_border_out_dev[dims][2];
			for (int i = 0; i < dims; i++) {
				global_inner_local.offset[i] = hit_shapeSig(shpInnerLocal, i).begin;
				for (int j = 0; j < 2; j++) {
					global_border_out_dev[i][j]           = global;
					global_border_out_dev[i][j].offset[i] = hit_shapeSig(shpBorderOutDev[i][j], i).begin;
				}
			}

			// Build computation structs
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

			/* 4.8.1. First stage (Optional): initialization in host */
			if (f_init != NULL) {
				printOnce("\tInitializing in host.\n");
				fflush(stdout);
				f_init(tileMat, global, ext_params);
				Ctrl_HostTask(epsilod_host_touch, tileMat);
				Ctrl_MoveTo(comm, tileMat);
				Ctrl_WaitTile(comm, tileMat);
			}

			/* 4.8.2. Second stage (Optional): initialization in device */
			Ctrl_Thread threads_touch   = {.dims = 1, .i = 1, .j = 0, .k = 0};
			Ctrl_Thread blocksize_touch = {.dims = 1, .i = 1, .j = 1, .k = 1};
			if (f_dev_init != NULL) {
				printOnce("\tInitializing in device.\n");
				fflush(stdout);
				Ctrl_Thread exp_threads = initCtrlThreadFromTile(dims, &tileMat);
				f_dev_init(comm, exp_threads, inner_char, 0, tileMat, global, ext_params);
			}

			bool tiles_marked = false;
			/* 4.8.3. Initialize copy */
			if (f_init_copy == NULL) {
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
				#else // EPSILOD_INITIALIZE_COPY_IN_HOST
				printOnce("\tInitializing copy in device.\n");
				fflush(stdout);
				Ctrl_Launch(comm, epsilod_dev_touch, threads_touch, blocksize_touch, tileMat);
				Ctrl_Thread threads_flat, blocksize_flat;
				// This is limited by Controllers kernel thread id type, not by Ctrl_Thread
				if (tileMat.acumCard <= INT_MAX) {
					Ctrl_ThreadInit(threads_flat, tileMat.acumCard);
					// TODO: the number of threads was reduced from 512 to 256 to support OpenCL. It should be queried instead.
					Ctrl_ThreadInit(blocksize_flat, 256);
					Ctrl_Launch(comm, epsilod_dev_copy_1d, threads_flat, blocksize_flat, tileMat, tileCopy);
				} else {
					threads_flat   = initCtrlThreadFromTile(dims, &tileMat);
					blocksize_flat = INNER_CHAR[dims - 1];
					switch (dims) {
						case 1: Ctrl_Launch(comm, epsilod_dev_copy_1d, threads_flat, blocksize_flat, tileMat, tileCopy); break;
						case 2: Ctrl_Launch(comm, epsilod_dev_copy_2d, threads_flat, blocksize_flat, tileMat, tileCopy); break;
						case 3: Ctrl_Launch(comm, epsilod_dev_copy_3d, threads_flat, blocksize_flat, tileMat, tileCopy); break;
						default:
							fprintf(stderr, "\nError: Matrix copy not implemented for more than 3 dimensions when total cardinality is greater than UINT_MAX.\n\n");
							MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
							exit(EXIT_FAILURE);
							break;
					}
				}
				#endif // EPSILOD_INITIALIZE_COPY_IN_HOST
			} else {
				markTiles(comm, props, threads_touch, blocksize_touch, &tiles, &copy_tiles, &comm_args);
				tiles_marked                  = true;
				computation_args.f_updateCell = f_init_copy;
				doStep(comm, &props, &computation_args, &swap_structs, &swap_structs_copy, &comm_args);
				swap(&swap_structs, &swap_structs_copy);
			}

			/* 4.8.4. Mark initialization of subselections and copies, eliminate warnings */
			if (!tiles_marked)
				markTiles(comm, props, threads_touch, blocksize_touch, &tiles, &copy_tiles, &comm_args);

			hit_clockStop(initClock);

			Ctrl_Synchronize();
			hit_comBarrier(lay);

			/* 4.9. Computation loop */
			printOnce("Computation stage\n");
			fflush(stdout);
			hit_clockStart(loopClock);

			computation_args.f_updateCell = f_updateCell;

			for (int loopIndex = 0; loopIndex < numIterations - 1; loopIndex++) {
				doStep(comm, &props, &computation_args, &swap_structs, &swap_structs_copy, &comm_args);
			}

			/* 4.10. Last iteration update: no communication after */
			if (numIterations > 0) {
				doStepNoComms(comm, &props, &computation_args, &swap_structs, &swap_structs_copy);
			}

			Ctrl_Synchronize();
			hit_clockStop(loopClock);

			printOnce(" End\n");
			fflush(stdout);

			// Move matrix to the host:
			Ctrl_MoveFrom(comm, tileMat);
			Ctrl_WaitTile(comm, tileMat);

			/* 4.11. Clock results */
			hit_clockStop(mainClock);

			reduceClocks(lay);
			printClockInfo();

			/* 4.12. Write result matrix */
			f_output(numIterations % 2 == 0 ? io_tile : io_copy, ext_params);
			hit_patternFree(&neighSync);
			hit_patternFree(&neighSyncCopy);

			printOnce("Output finished\n");
			fflush(stdout);

			// Free tiles
			printOnce("Free tiles\n");
			fflush(stdout);
			for (int i = 0; i < num_tiles_total; i++) {
				Ctrl_Free(comm, *tileList[i]);
				Ctrl_Free(comm, *copyList[i]);
			}
			Ctrl_Free(comm, io_tile);
			Ctrl_Free(comm, io_copy);
		} else {
			/* 5. Inactive processes: only collective clock operations */
			fprintf(stderr, "[%d] Warning, process not active\n", hit_Rank);

			hit_clockStop(initClock);
			hit_clockStop(mainClock);
			hit_clockStop(loopClock);

			reduceClocks(lay);
			printClockInfo();
		}

		/* 6. Free other resources */
		printOnce("Free data structures\n");
		fflush(stdout);
		Ctrl_Free(comm, stencil);
		hit_layFree(lay);
		hit_topFree(topo);

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
