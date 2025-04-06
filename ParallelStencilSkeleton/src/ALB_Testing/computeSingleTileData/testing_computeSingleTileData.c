
#ifndef _EPSILOD_H_
#define _EPSILOD_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _CTRL_ARCH_CUDA_
#include <cuda_runtime_api.h>
#endif // _CTRL_ARCH_CUDA_

#include <epsilod_ext_type.h>

#include "Ctrl.h"

/* Definitions to declare optimized stencil kernel prototypes and wrapper launcher */
#define REGISTER_STENCIL(stencilname, ...)                                                                                                                                                     \
	REGISTER_STENCIL_N(stencilname, CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)                                                                                                                 \
	CTRL_KERNEL_CHAR(stencilname, MANUAL, 64, 8, 1);                                                                                                                                           \
	void stencilname(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile_float mat, HitTile_float copy, HitTile_float weight, float factor, Epsilod_ext *ext_params) { \
		Ctrl_LaunchToStream(ctrl, stencilname, threads, blockSize, stream, mat, copy, *ext_params);                                                                                            \
	}

#define REGISTER_STENCIL_N(stencilname, n_archs_times2, ...)  REGISTER_STENCIL_N2(stencilname, n_archs_times2, __VA_ARGS__)
#define REGISTER_STENCIL_N2(stencilname, n_archs_times2, ...) STENCIL_PROTO(stencilname, n_archs_times2, n_archs_##n_archs_times2, __VA_ARGS__)
#define STENCIL_PROTO(stencilname, n_archs_times2, n_archs, ...)           \
	CTRL_KERNEL_PROTO(stencilname,                                         \
					  n_archs,                                             \
					  STENCIL_EXTRACT_ARCHS_##n_archs_times2(__VA_ARGS__), \
					  3,                                                   \
					  OUT, HitTile_float, matrix,                          \
					  IN, HitTile_float, matrixCopy,                       \
					  INVAL, Epsilod_ext, ext_params);

#define STENCIL_EXTRACT_ARCHS_2(arch, subarch)       arch, subarch
#define STENCIL_EXTRACT_ARCHS_4(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_2(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_6(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_4(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_8(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_6(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_10(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_8(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_12(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_10(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_14(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_12(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_16(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_14(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_18(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_16(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_20(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_18(__VA_ARGS__)

#define n_archs_2  1
#define n_archs_4  2
#define n_archs_6  3
#define n_archs_8  4
#define n_archs_10 5
#define n_archs_12 6
#define n_archs_14 7
#define n_archs_16 8
#define n_archs_18 9
#define n_archs_20 10

#endif


#define _PSS_SHP_INFO_
#define _PSS_TOPO_INFO_
#define _PSS_WEIGHTS_INFO_

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

/* B. GENERIC KERNEL PROTOTYPE AND WRAPPER LAUNCHERS */
CTRL_KERNEL_CHAR(updateCell_default_1D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_2D, MANUAL, 0, 0, 0);
CTRL_KERNEL_CHAR(updateCell_default_3D, MANUAL, 0, 0, 0);

CTRL_KERNEL_PROTO(updateCell_default_1D,
				  1, GENERIC, DEFAULT, 7,
				  OUT, HitTile_float, matrix,
				  IN, HitTile_float, matrixCopy,
				  IN, HitTile_float, weight,
				  INVAL, int, begin_x,
				  INVAL, int, end_x,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_1D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile_float mat, HitTile_float copy, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_1D, threads, blockSize, stream, mat, copy, weight,
						hit_tileDimBegin(weight, 0), hit_tileDimEnd(weight, 0),
						factor, *ext_params);
}

CTRL_KERNEL_PROTO(updateCell_default_2D,
				  1, GENERIC, DEFAULT, 9,
				  OUT, HitTile_float, matrix,
				  IN, HitTile_float, matrixCopy,
				  IN, HitTile_float, weight,
				  INVAL, int, begin_x,
				  INVAL, int, begin_y,
				  INVAL, int, end_x,
				  INVAL, int, end_y,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_2D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile_float mat, HitTile_float copy, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_2D, threads, blockSize, stream, mat, copy, weight,
						hit_tileDimBegin(weight, 0), hit_tileDimBegin(weight, 1),
						hit_tileDimEnd(weight, 0), hit_tileDimEnd(weight, 1),
						factor, *ext_params);
}

CTRL_KERNEL_PROTO(updateCell_default_3D,
				  1, GENERIC, DEFAULT, 11,
				  OUT, HitTile_float, matrix,
				  IN, HitTile_float, matrixCopy,
				  IN, HitTile_float, weight,
				  INVAL, int, begin_x,
				  INVAL, int, begin_y,
				  INVAL, int, begin_z,
				  INVAL, int, end_x,
				  INVAL, int, end_y,
				  INVAL, int, end_z,
				  INVAL, float, factor,
				  INVAL, Epsilod_ext, ext_params);

void updateCell_default_3D(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile_float mat, HitTile_float copy, HitTile_float weight, float factor, Epsilod_ext *ext_params) {
	Ctrl_LaunchToStream(ctrl, updateCell_default_3D, threads, blockSize, stream, mat, copy, weight,
						hit_tileDimBegin(weight, 0), hit_tileDimBegin(weight, 1), hit_tileDimBegin(weight, 2),
						hit_tileDimEnd(weight, 0), hit_tileDimEnd(weight, 1), hit_tileDimEnd(weight, 2),
						factor, *ext_params);
}

/* D. FALSE INITIALIZATION OF SELECTIONS TO AVOID NON-INITIALIZED WARNINGS */
CTRL_HOST_TASK(Ctrl_Sub_Select_Init, HitTile_float matrix) { return; }
CTRL_HOST_TASK_PROTO(Ctrl_Sub_Select_Init, 1, OUT, HitTile_float, matrix);

/* F. DEBUG: WRITE OUTPUT ON STDOUT */
CTRL_HOST_TASK(print_matrix, HitTile_float mat, Epsilod_ext *ext_params) {
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

CTRL_HOST_TASK_PROTO(print_matrix, 2, IN, HitTile_float, mat, INVAL, Epsilod_ext *, ext_params);

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

/* H. EXPERIMENTATION: GLOBAL clocks */
HitClock mainClock;
HitClock initClock;
HitClock loopClock;
HitClock iterClock;
HitClock redistributeClock;
HitClock commClock;

int iterALB = 0;

/* HELP. PRINT EXTRA OPTIONS OF EPSILOD USING ENVIRONMENT VARIABLES */
void epsilod_print_usage() {
	fprintf(stderr, "\nEPSILOD environment variable:\n");
	fprintf(stderr, "\tEPSILOD_PARTITION=r       Regular blocks of similar sizes on a multidimensional grid topology\n");
	fprintf(stderr, "\tEPSILOD_PARTITION=r<dim>  Regular blocks of similar sizes on a single dimension topology\n");
	fprintf(stderr, "\tEPSILOD_PARTITION=w<dim>  Weigthed block distribution in the single choosen dimension. Processes weigths are specified in the device selection configuratuion file.\n");
	fprintf(stderr, "\n");
}

/* Special functions definition. */
typedef void (*stencilFunction)(PCtrl, Ctrl_Thread, Ctrl_Thread, int, HitTile_float, HitTile_float, HitTile_float, float, Epsilod_ext *);
typedef void (*initDataFunction)(HitTile_float, int, int[], int[], Epsilod_ext);
typedef void (*outputDataFunction)(HitTile_float, Epsilod_ext);

// GLOBAL VARIABLES FOR INPUT/OUPUT FILE OPTIONS
int io_read_input   = 0;
int io_write_input  = 0;
int io_write_output = 0;

/* A. PROTOYPES */
/* A.1. ARRAY INIT AND OUTPUT FUNCTIONS */
void initData(HitTile_float io_tile, int dims, int borderLow[], int borderHigh[], Epsilod_ext ext_params);
void outputData(HitTile_float io_tile, Epsilod_ext ext_params);

/* A.2. INITIALIZATION FUNCTIONS */
void initData1D(HitTile_float tileMat, int dims, int borderLow[], int borderHigh[]);
void initData2D(HitTile_float tileMat, int dims, int borderLow[], int borderHigh[]);
void initData3D(HitTile_float tileMat, int dims, int borderLow[], int borderHigh[]);

/* B. INITIALIZE ARRAYs */
/* B.1. INIT ARRAY: FILL WITH 0s, CALL FUNCTION TO INIT BORDERS 1, 2 or 3 DIMENSIONS */
void initData(HitTile_float io_tile, int dims, int borderLow[], int borderHigh[], Epsilod_ext ext_params) {
	/* 0. OPTIONAL COMPILATION: READING THE INPUT MATRIX FROM A FILE */
	if (io_read_input) {
		hit_tileFileReadOptions(&io_tile, "Matrix.in", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_read_input - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	} else {
		HitTile_float tileMat = *(HitTile_float *)hit_tileMemoryAncestor(&io_tile);

		char *omp_env     = getenv("OMP_NUM_THREADS");
		int   omp_threads = (omp_env != NULL) ? atoi(omp_env) : 1;
		#pragma omp parallel for num_threads(omp_threads)
		for (int i = 0; i < tileMat.acumCard; i++) {
			hit(tileMat, i) = 0;
		}

		/* 2. INIT BORDERS */
		switch (dims) {
			case 1: initData1D(tileMat, dims, borderLow, borderHigh); break;
			case 2: initData2D(tileMat, dims, borderLow, borderHigh); break;
			case 3: initData3D(tileMat, dims, borderLow, borderHigh); break;
			default:
				fprintf(stderr, "Error: This init function only works for 1, 2, or 3 dimensions\n");
				Ctrl_Finalize();
				exit(EXIT_FAILURE);
		}
	}

	/* 4. WRITE MAT TO A FILE (DEBUGGING) */
	if (io_write_input) {
		hit_tileFileWriteOptions(&io_tile, "Matrix.copy", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_write_input - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	}
}

/* B.2. INITIALIZE BORDERS ARRAY 1D */
void initData1D(HitTile_float tileMat, int dims, int borderLow[], int borderHigh[]) {
	/* 2. INIT BORDERS  DOWN(i)=1, UP(i)=2. */
	HitTile root = *hit_tileRoot(&tileMat);
	int     i;

	/* 2.1. FIRST ELEMENTS ARE MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0))) {
		for (i = 0; i < borderLow[0]; i++) {
			hit_tileElemAt(tileMat, 1, i) = 1;
		}
	}

	/* 2.4. LAST ELEMENTS ARE MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0))) {
		for (i = 0; i < borderHigh[0]; i++) {
			hit_tileElemAt(tileMat, 1, hit_tileDimCard(tileMat, 0) - 1 - i) = 2;
		}
	}
}

/* B.3. INITIALIZE BORDERS MATRIX 2D */
void initData2D(HitTile_float tileMat, int dims, int borderLow[], int borderHigh[]) {
	/* 2. INIT BORDERS  UP(i)=1, DOWN(i)=2, LEFT(i)=3, RIGHT(i)=4 */
	HitTile root = *hit_tileRoot(&tileMat);
	int     i, j;

	/* 2.1. FIRST COLUMN IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
		for (j = 0; j < borderLow[1]; j++)
			hit_tileForDimDomain(tileMat, 0, i)
				hit_tileElemAt(tileMat, 2, i, j) = 3;

	/* 2.2. LAST COLUMN IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
		for (j = 0; j < borderHigh[1]; j++)
			hit_tileForDimDomain(tileMat, 0, i)
				hit_tileElemAt(tileMat, 2, i, hit_tileDimCard(tileMat, 1) - 1 - j) = 4;

	/* 2.3. FIRST ROW IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
		for (i = 0; i < borderLow[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileElemAt(tileMat, 2, i, j) = 1;

	/* 2.4. LAST ROW IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0)))
		for (i = 0; i < borderHigh[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileElemAt(tileMat, 2, hit_tileDimCard(tileMat, 0) - 1 - i, j) = 2;
}

/* B.4. INITIALIZE BORDERS MATRIX 3D */
void initData3D(HitTile_float tileMat, int dims, int borderLow[], int borderHigh[]) {
	/* 2. INIT BORDERS */
	HitTile root = *hit_tileRoot(&tileMat);
	int     i, j, k;

	/* FIRST LAYER OF k IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimBegin(root, 2)))
		hit_tileForDimDomain(tileMat, 0, i)
			hit_tileForDimDomain(tileMat, 1, j) for (k = 0; k < borderLow[2]; k++)
				hit_tileElemAt(tileMat, 3, i, j, k) = 5;

	/* LAST LAYER OF k IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimEnd(root, 2)))
		hit_tileForDimDomain(tileMat, 0, i)
			hit_tileForDimDomain(tileMat, 1, j) for (k = 0; k < borderHigh[2]; k++)
				hit_tileElemAt(tileMat, 3, i, j, hit_tileDimCard(tileMat, 2) - 1 - k) = 6;

	/* FIRST LAYER OF j IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
		hit_tileForDimDomain(tileMat, 0, i) for (j = 0; j < borderLow[1]; j++)
			hit_tileForDimDomain(tileMat, 2, k)
				hit_tileElemAt(tileMat, 3, i, j, k) = 3;

	/* LAST LAYER OF j IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
		hit_tileForDimDomain(tileMat, 0, i) for (j = 0; j < borderHigh[1]; j++)
			hit_tileForDimDomain(tileMat, 2, k)
				hit_tileElemAt(tileMat, 3, i, hit_tileDimCard(tileMat, 1) - 1 - j, k) = 4;

	/* FIRST LAYER OF i IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
		for (i = 0; i < borderLow[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileForDimDomain(tileMat, 2, k)
					hit_tileElemAt(tileMat, 3, i, j, k) = 1;

	/* LAST LAYER OF i IS MINE */
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0)))
		for (i = 0; i < borderHigh[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileForDimDomain(tileMat, 2, k)
					hit_tileElemAt(tileMat, 3, hit_tileDimCard(tileMat, 0) - 1 - i, j, k) = 2;
}

/* C. WRITE RESULTS */
void outputData(HitTile_float io_tile, Epsilod_ext ext_params) {
	if (io_write_output) {
		hit_tileFileWriteOptions(&io_tile, "Matrix.out", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_write_output - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	}
}

/* D. DECLARATIONS OF OPTIMIZED STENCIL KERNELS:
 * SEE test_parallelStencilSkeleton_kernels.c FILE */
REGISTER_STENCIL(updateCell_1dNC4, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_1dC2, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_4, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_9, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_NC9, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_F5, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_3d27, GENERIC, DEFAULT);


double clamp(double d, double min, double max) {
  const double t = d < min ? min : d;
  return t > max ? max : t;
}

int clampInt(int d, int min, int max) {
  const int t = d < min ? min : d;
  return t > max ? max : t;
}

int returnValue;

// FUNCTIONS AND STRUCTS RELATED TO ALB
static inline void printHitShape2(HitShape sh){
	HitSig sig;

	printf("HitShape(dims=%d)\n",hit_sshapeDims(sh));
	for(int dim=0; dim<hit_sshapeDims(sh); dim++){
		sig = hit_shapeSig(sh, dim);
		printf("[%d] Dim %d\n", hit_Rank, dim);
		printf("\t[%d] HitSig(begin=%d, end=%d, stride=%d, card=%d)\n", hit_Rank, sig.begin, sig.end, sig.stride, hit_sigCard(sig));
		printf("\t[%d] ", hit_Rank);
		for(int i=sig.begin; i<=sig.end; i+=sig.stride) printf("%d ", i);

		printf("\n");
	}
}


/* I. STENCIL COMPUTATION FUNCTION: */
void testingCase(
	int                sizes[],
	HitShape           stencilShape,
	float              stencilData[],
	float              factor,
	stencilFunction    f_updateCell,
	initDataFunction   f_init,
	char              *device_selection_file) {

	HitClock stencilComputationClock;
	hit_clockStart(stencilComputationClock);

	/* EXTERNAL/EXTRA PARAMETERS: USE FOO VARIABLE FOR NULL ARGUMENT */
	Epsilod_ext  foo;
	Epsilod_ext *ext_params = &foo;

	int dims = hit_shapeDims(stencilShape);

	/* CHECK IF GENERIC KERNEL HAS BEEN CHOSEN */
	if (f_updateCell == NULL) {
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
	}

	/* INITIALIZE DEVICE Controllers */
	__ctrl_block__(device_selection_file) {
		int i, j, k;

		/* INIT CLOCKS */
		hit_clockSynchronizeAll();
		hit_clockStart(mainClock);
		hit_clockStart(initClock);

		PCtrl comm = Ctrl_Get(0);

		#ifndef _CTRL_EXP_MODE_
		Ctrl_Info info = Ctrl_GetInfo(comm);
		printf("MPI_Rank[%d] DEVICE: %s", hit_Rank, info.device_name);
		fflush(stdout);
		#endif // _CTRL_EXAMPLES_EXP_MODE_

		Ctrl_SetDependanceMode(comm, CTRL_MODE_EXPLICIT);

		/* 0. BUILD THE STENCIL TILE */
		HitTile_float stencil = Ctrl_Domain(comm, float, stencilShape);

		Ctrl_Alloc(comm, stencil, CTRL_MEM_ALLOC_BOTH);
		Ctrl_WaitTile(comm, stencil);

		if (f_updateCell == updateCell_default_1D ||
			f_updateCell == updateCell_default_2D ||
			f_updateCell == updateCell_default_3D) {
			Ctrl_HostTask(comm, Ctrl_Copy_Stencil, stencil, stencilData);
			Ctrl_MoveTo(comm, stencil);
		}

		/* 1. BUILD ORIGINAL SHAPE */
		HitShape shp = HIT_SHAPE_NULL;
		hit_shapeDimsSet(shp, dims);
		for (i = 0; i < dims; i++) {
			hit_shapeSig(shp, i) = hit_sig(0, sizes[i] - 1, 1);
		}

		/* 2. SHORTCUTS FOR BORDER SIZES */
		int *borderLow  = (int *)malloc(dims * sizeof(int));
		int *borderHigh = (int *)malloc(dims * sizeof(int));
		for (i = 0; i < dims; i++) {
			borderLow[i]  = -hit_tileDimBegin(stencil, i);
			borderHigh[i] = hit_tileDimEnd(stencil, i);
		}

		/* 3.1. SHAPE TO DISTRIBUTE COMPUTATION (WITHOUT BORDERS) */
		HitShape shpInner = shp;
		for (i = 0; i < dims; i++) {
			shpInner = hit_shapeTransform(shpInner, i, HIT_SHAPE_BEGIN, +borderLow[i]);
			shpInner = hit_shapeTransform(shpInner, i, HIT_SHAPE_END, -borderHigh[i]);
		}

		/* 3.2. SELECT AND BUILD PARTITION/DISTRIBUTION */
		int   partition_dim = 0;
		bool  use_weights   = false;
		char *partition_str = getenv("EPSILOD_PARTITION");
		if (partition_str != NULL) {
			if (strlen(partition_str) > 2) {
				fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: More than two characters. String: %s\n\n", partition_str);
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
				exit(EXIT_FAILURE);
			}
			switch (partition_str[0]) {
				case 'r':
				case 'R':
					break;
				case 'w':
				case 'W':
					use_weights = true;
					break;
				default:
					fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Unknown partition type. String: %s\n\n", partition_str);
					MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
					exit(EXIT_FAILURE);
			}
			switch (partition_str[1]) {
				case '\0':
					partition_dim = -1;
					break;
				case '0':
				case '1':
				case '2':
					partition_dim = partition_str[1] - '0';
					break;
				default:
					fprintf(stderr, "\nError in EPSILOD_PARTITION enviroment string: Optional dimension should be in the range [0:2]. String: %s \n\n", partition_str);
					MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
					exit(EXIT_FAILURE);
			}
		}
		if (partition_dim >= dims) {
			fprintf(stderr, "\nError in EPSILOD_PARTITION environment string: Dimension %d, should be in the range of [0:stencil_dimensions-1]. String: %s\n\n", partition_dim, partition_str);
			MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
			exit(EXIT_FAILURE);
		}

		HitTopology topo;
		HitLayout   lay;
		HitWeights  weights = Ctrl_ConfigWeights();

		// TOPOLOGY
		if (partition_dim != -1)
			topo = hit_topology(plug_topPlain);
		else {
			switch (dims) {
				case 1:
					topo = hit_topology(plug_topPlain);
					break;
				case 2:
					topo = hit_topology(plug_topArray2DComplete);
					break;
				case 3:
					topo = hit_topology(plug_topArray3D);
					break;
				default:
					fprintf(stderr, "\nError EPSILOD: Number of dimensions for processes topology should be in the range [1:3]. Current value:%d \n\n", dims);
					MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
					exit(EXIT_FAILURE);
			}
		}

		// WEIGHTED DISTRIBUTION
		if (use_weights) {
			lay = hit_layout(plug_layDimWeighted_Blocks, topo, shpInner, partition_dim, weights);
		}
		// REGULAR DISTRIBUTION
		else {
			lay = hit_layout(plug_layBlocks, topo, shpInner);
		}

		HitShape shpLayout = hit_layShape(lay);
		#ifdef _PSS_SHP_INFO_
		printf(", shpLayout: ");
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

			TileData tileData;
			TileData tileDataCopy;
			TileCommon common;
			
			common.numBorders = (int)pow(3, dims);
			common.num_tiles_total = 1 + 1 + 2 * common.numBorders + dims * 2;
			common.borderInActive = malloc(common.numBorders * sizeof(int));

			allocateTileData(&tileData, common, dims);
			allocateTileData(&tileDataCopy, common, dims);

			// 4.1. STOP IF THERE IS NOT ENOUGH INNER DATA FOR A BORDER
			for (i = 0; i < dims; i++) {
				if (hit_shapeSigCard(shpLayout, i) < borderLow[i] ||
					hit_shapeSigCard(shpLayout, i) < borderHigh[i]) {
					if (hit_Rank == 0) {
						fprintf(stderr, "\nError: Not enough data after partition, too many processes in a topology axis\n\n");
						MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
						exit(EXIT_FAILURE);
					}
				}
			}

			/* 4.2. EXPANDED SHAPE */
			HitShape shpExpanded = shpLayout;
			for (i = 0; i < dims; i++) {
				shpExpanded = hit_shapeTransform(shpExpanded, i, HIT_SHAPE_BEGIN, -borderLow[i]);
				shpExpanded = hit_shapeTransform(shpExpanded, i, HIT_SHAPE_END, borderHigh[i]);
			}

			/* 4.3. BORDER SHAPES: IN/OUT */
			common.numBorders = (int)pow(3, dims);
			HitShape shpBorderIn[common.numBorders];
			HitShape shpBorderOut[common.numBorders];
			HitRanks shiftsIn[common.numBorders];
			HitRanks shiftsOut[common.numBorders];

			for (i = 0; i < common.numBorders; i++)
				*(common.borderInActive + i) = 0;

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
						if (indeces[j] > borderLow[j])
							border += 2 * acum;
						else if (indeces[j] == borderLow[j])
							border += acum;
						acum *= 3;
					}
					*(common.borderInActive + border) = 1;
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
			*(common.borderInActive + (common.numBorders / 2)) = 0;

			// BUILD BORDER SHAPES AND THE SHORTCUTS OF THE NEIGHBOR SHIFTS
			for (i = 0; i < common.numBorders; i++) {
				shiftsIn[i]  = HIT_RANKS_NULL;
				shiftsOut[i] = HIT_RANKS_NULL;

				// NON-ACTIVE BORDERS, NULL SHAPES, NULL RANKS
				if (!*(common.borderInActive + i)) {
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
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_FIRST, borderLow[j]);
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, -borderLow[j]);
					} else if (shiftsIn[i].rank[j] == 1) {
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_LAST, borderHigh[j]);
						shpBorderIn[i] = hit_shapeTransform(shpBorderIn[i], j, HIT_SHAPE_MOVE, borderHigh[j]);
					}

					// SHAPE OUT (REVERSERD TARGET)
					if (shiftsIn[i].rank[j] == -1) {
						shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_LAST, borderLow[j]);
					} else if (shiftsIn[i].rank[j] == 1) {
						shpBorderOut[i] = hit_shapeTransform(shpBorderOut[i], j, HIT_SHAPE_FIRST, borderHigh[j]);
					}
				}
			}

			// DEACTIVATE BORDER COMMs DUE TO LACK OF NEIGHBOR
			for (i = 0; i < common.numBorders; i++) {
				// SKIP EMPTY BORDERS
				if (!*(common.borderInActive + i)) continue;
				// LOCATE NEIGHBOR IN THE LAYOUT GRID
				HitRanks neighIn  = hit_layNeighborN(lay, shiftsIn[i]);
				HitRanks neighOut = hit_layNeighborN(lay, shiftsOut[i]);

				// IF NEIGHBORS DOES NOT EXIST, DEACTIVATE BORDER COMMs.
				if (neighIn.rank[0] == -1 && neighOut.rank[0] == -1) {
					*(common.borderInActive + i) = 0;
				}
			}

			/* 4.4. NON-OVERLAPED BORDERS IN THE INNER PART
				a) TO EXTRACT DATA FROM DEVICE WITHOUT REPLICATION
				b) TO DETERMINE THE THREADS-GRID CARDINALITIES FOR THE COMPUTING KERNELS
			*/
			HitShape shpInnerLocal = shpLayout;
			HitShape shpBorderOutDev[dims][2];
			for (j = 0; j < dims; j++) {
				shpInnerLocal = hit_shapeTransform(shpInnerLocal, j, HIT_SHAPE_BEGIN, borderHigh[j]);
				shpInnerLocal = hit_shapeTransform(shpInnerLocal, j, HIT_SHAPE_END, -borderLow[j]);

				shpBorderOutDev[j][0] = hit_shapeTransform(shpLayout, j, HIT_SHAPE_FIRST, borderHigh[j]);
				shpBorderOutDev[j][1] = hit_shapeTransform(shpLayout, j, HIT_SHAPE_LAST, borderLow[j]);
				// TAKE OUT THE PARTS WHICH ARE OVERLAPPED WITH PREVIOUS DIMS
				for (k = 0; k < j; k++) {
					shpBorderOutDev[j][0] = hit_shapeTransform(shpBorderOutDev[j][0], k, HIT_SHAPE_BEGIN, borderHigh[k]);
					shpBorderOutDev[j][0] = hit_shapeTransform(shpBorderOutDev[j][0], k, HIT_SHAPE_END, -borderLow[k]);
					shpBorderOutDev[j][1] = hit_shapeTransform(shpBorderOutDev[j][1], k, HIT_SHAPE_BEGIN, borderHigh[k]);
					shpBorderOutDev[j][1] = hit_shapeTransform(shpBorderOutDev[j][1], k, HIT_SHAPE_END, -borderLow[k]);
				}
				// NULL SIGNATURE
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
			common.num_tiles_total = 1 + 1 + 2 * common.numBorders + dims * 2;



			HitTile_float tileBorderOut[common.numBorders];
			HitTile_float tileCopyBorderOut[common.numBorders];

			HitTile_float globalMat = Ctrl_Domain(comm, float, shp);


			tileData.tileMat  = Ctrl_Select(comm, float, globalMat, shpExpanded, CTRL_SELECT_ARR_COORD); //-------------//select tileMat
			tileDataCopy.tileMat = Ctrl_Select(comm, float, globalMat, shpExpanded, CTRL_SELECT_ARR_COORD); //-------------//select tileCopy

			Ctrl_Alloc(comm, tileData.tileMat, CTRL_MEM_ALLOC_BOTH);  //---------------------//alloc tileMat
			Ctrl_Alloc(comm, tileDataCopy.tileMat, CTRL_MEM_ALLOC_BOTH); //---------------------//alloc tileCopy

			tileData.tileInnerLocal    = Ctrl_Select(comm, float, tileData.tileMat, shpInnerLocal, CTRL_SELECT_ARR_COORD);
			tileDataCopy.tileInnerLocal = Ctrl_Select(comm, float, tileDataCopy.tileMat, shpInnerLocal, CTRL_SELECT_ARR_COORD);

			for (i = 0; i < common.numBorders; i++) {
				*(tileData.tileBorderIn + i) = Ctrl_Select(comm, float, tileData.tileMat, shpBorderIn[i], CTRL_SELECT_ARR_COORD);   //-------------------//select
				*(tileDataCopy.tileBorderIn + i)  = Ctrl_Select(comm, float, tileDataCopy.tileMat, shpBorderIn[i], CTRL_SELECT_ARR_COORD);  //-------------------//select
				tileBorderOut[i] = Ctrl_Select(comm, float, tileData.tileMat, shpBorderOut[i], CTRL_SELECT_ARR_COORD);  //-------------------//select
				tileCopyBorderOut[i] = Ctrl_Select(comm, float, tileDataCopy.tileMat, shpBorderOut[i], CTRL_SELECT_ARR_COORD); //-------------------//select
			}

			// clang-format off
			
			#define validShape(s) (hit_shapeDims((s)) != (-1))
			// clang-format on

			/* ELIMINATE BORDERS EXCEPT IF THEY ARE GLOBAL FOR IO SELECTION */
			HitShape io_shape = hit_tileShape(tileData.tileMat);
			for (i = 0; i < dims; i++) {
				/* DIM i FIRST BORDER IS NOT MINE */
				if (!hit_sigIn(hit_shapeSig(io_shape, i), hit_tileDimBegin(globalMat, i))) {
					io_shape = hit_shapeTransform(io_shape, i, HIT_SHAPE_BEGIN, borderLow[i]);
				}
				/* DIM i LAST BORDER IS NOT MINE */
				if (!hit_sigIn(hit_shapeSig(io_shape, i), hit_tileDimEnd(globalMat, i))) {
					io_shape = hit_shapeTransform(io_shape, i, HIT_SHAPE_END, -borderHigh[i]);
				}
			}
			hit_tileSelectArrayCoords(&(tileData.io_tile), &(tileData.tileMat), io_shape);
			hit_tileSelectArrayCoords(&(tileDataCopy.io_tile), &(tileDataCopy.tileMat), io_shape);

			for (i = 0; i < dims; i++) {
				if (validShape(shpBorderOutDev[i][0])) {
					(*((tileData.tileBorderOutDev + i * 2) + 0)) = Ctrl_Select(comm, float, tileData.tileMat, shpBorderOutDev[i][0], CTRL_SELECT_ARR_COORD);  //-------------------//select
					(*((tileDataCopy.tileBorderOutDev + i * 2) + 0)) = Ctrl_Select(comm, float, tileDataCopy.tileMat, shpBorderOutDev[i][0], CTRL_SELECT_ARR_COORD); //-------------------//select
				} else {
					(*((tileData.tileBorderOutDev + i * 2) + 0)) = *(HitTile_float *)&HIT_TILE_NULL;
					(*((tileDataCopy.tileBorderOutDev + i * 2) + 0)) = *(HitTile_float *)&HIT_TILE_NULL;
				}
				if (validShape(shpBorderOutDev[i][1])) {
					(*((tileData.tileBorderOutDev + i * 2) + 1))  = Ctrl_Select(comm, float, tileData.tileMat, shpBorderOutDev[i][1], CTRL_SELECT_ARR_COORD);  //-------------------//select
					(*((tileDataCopy.tileBorderOutDev + i * 2) + 1)) = Ctrl_Select(comm, float, tileDataCopy.tileMat, shpBorderOutDev[i][1], CTRL_SELECT_ARR_COORD); //-------------------//select
				} else {
					(*((tileData.tileBorderOutDev + i * 2) + 1)) = *(HitTile_float *)&HIT_TILE_NULL;
					(*((tileDataCopy.tileBorderOutDev + i * 2) + 1)) = *(HitTile_float *)&HIT_TILE_NULL;
				}


			}

			/* 4.6. BUILD DISTRIBUTED-MEMORY COMMUNICATION PATTERN */
			tileData.neighSync = hit_pattern(HIT_PAT_UNORDERED);
			for (i = 0; i < common.numBorders; i++) {
				// SKIP EMPTY BORDERS
				if (!(*(common.borderInActive + i))) continue;

				// LOCATE NEIGHBORS IN THE LAYOUT GRID

				HitRanks neighIn  = HIT_RANKS_NULL;
				HitRanks neighOut = HIT_RANKS_NULL;

				if (*(common.borderInActive + i)) {
					neighIn  = hit_layNeighborN(lay, shiftsIn[i]);
					neighOut = hit_layNeighborN(lay, shiftsOut[i]);
				}

				hit_patternAdd(&(tileData.neighSync), hit_comSendRecv(lay,
															neighOut, &tileBorderOut[i],
															neighIn, (tileData.tileBorderIn + i),
															HIT_FLOAT));
			}

			tileDataCopy.neighSync = hit_pattern(HIT_PAT_UNORDERED);
			for (i = 0; i < common.numBorders; i++) {
				// SKIP EMPTY BORDERS
				if (!(*(common.borderInActive + i))) continue;

				// LOCATE NEIGHBORS IN THE LAYOUT GRID

				HitRanks neighIn  = HIT_RANKS_NULL;
				HitRanks neighOut = HIT_RANKS_NULL;

				if (*(common.borderInActive + i)) {
					neighIn  = hit_layNeighborN(lay, shiftsIn[i]);
					neighOut = hit_layNeighborN(lay, shiftsOut[i]);
				}

				hit_patternAdd(&(tileDataCopy.neighSync), hit_comSendRecv(lay,
																neighOut, &tileCopyBorderOut[i],
																neighIn, (tileDataCopy.tileBorderIn + i),
																HIT_FLOAT));
			}

			/* 4.7. INITIALIZE REDISTRIBUTION STRUCTURES */

			/// HitAvg avgs = hit_avgSimple(49); // TODO: change windows from 49 (50 - 1) to something that makes more sense.
			
			//HitAvg avgs = hit_avgSimple(49);
			HitAvg avgs = hit_avgSimple(30);

			/* 4.8. INITIALIZE ARRAY */
			if (hit_Rank == 0) {
				printf("Init stage\n");
				fflush(stdout);
			}

			f_init(tileData.io_tile, dims, borderLow, borderHigh, *ext_params);

			char *omp_env     = getenv("OMP_NUM_THREADS");
			int   omp_threads = (omp_env != NULL) ? atoi(omp_env) : 1;
			#pragma omp parallel for num_threads(omp_threads)
			for (int i = 0; i < tileData.tileMat.acumCard; i++)
				hit(tileDataCopy.tileMat, i) = hit(tileData.tileMat, i);
			
			/* 4.8.b. FALSE INITIALIZATION OF SUBSELECTIONS AND COPY, ELIMINATE WARNINGS */
			Ctrl_HostTask(comm, Ctrl_Sub_Select_Init, tileData.tileMat);
			Ctrl_HostTask(comm, Ctrl_Sub_Select_Init, tileDataCopy.tileMat);
			for (i = 0; i < dims; i++) {
				if (validShape((*((tileData.tileBorderOutDev+i*2)+0)).shape)) {
					Ctrl_HostTask( comm, Ctrl_Sub_Select_Init, *((tileData.tileBorderOutDev+i*2)+0));
				}
				if (validShape((*((tileData.tileBorderOutDev+i*2)+1)).shape)) {
					Ctrl_HostTask( comm, Ctrl_Sub_Select_Init, *((tileData.tileBorderOutDev+i*2)+1));
				}
			}
			if (validShape((tileData.tileInnerLocal).shape)) {
				Ctrl_HostTask( comm, Ctrl_Sub_Select_Init, tileData.tileInnerLocal);
			}
			for (i = 0; i < common.numBorders; i++) {
				// SKIP EMPTY BORDERS
				if (!(*(common.borderInActive+i))) continue;
				Ctrl_HostTask(comm, Ctrl_Sub_Select_Init, *(tileData.tileBorderIn + i));
				Ctrl_HostTask(comm, Ctrl_Sub_Select_Init, *(tileDataCopy.tileBorderIn + i));
			}

			/* Send tileMat to the device */
			Ctrl_MoveTo(comm, tileData.tileMat, tileDataCopy.tileMat);
			Ctrl_WaitTile(comm, tileData.tileMat, tileDataCopy.tileMat);
			// clang-format off
			
			#define copy2Threads( th, arr ) {th.i = arr[0]; th.j = arr[1]; th.k = arr[2];}
			// clang-format on
			Ctrl_Thread threadsInner;
			threadsInner.dims = dims;

			int threadsInnerCoords[3] = {1, 1, 1};

			for (i = 0; i < dims && i < 3; i++) {
				threadsInnerCoords[i] = hit_tileDimCard(tileData.tileInnerLocal, i);
			}
			copy2Threads(threadsInner, threadsInnerCoords);

			Ctrl_Thread thrBorderOutDev[dims][2];

			for (i = 0; i < dims; i++) {
				thrBorderOutDev[i][0].dims = dims;
				thrBorderOutDev[i][1].dims = dims;

				int coords[2][3] = {{1, 1, 1}, {1, 1, 1}};
				for (j = 0; j < dims && j < 3; j++) {
					coords[0][j] = hit_tileDimCard(*((tileData.tileBorderOutDev+i*2)+0), j);
					coords[1][j] = hit_tileDimCard(*((tileData.tileBorderOutDev+i*2)+1), j);
				}
				copy2Threads(thrBorderOutDev[i][0], coords[0]);
				copy2Threads(thrBorderOutDev[i][1], coords[1]);
				thrBorderOutDev[i][0].dims = dims;
				thrBorderOutDev[i][1].dims = dims;
			}

			/* Inner tile characterization */
			Ctrl_Thread inner_char = comm->type == CTRL_TYPE_CPU ? CPU_INNER_CHAR[dims - 1] : INNER_CHAR[dims - 1];

			/* Borders characterization for CPUs */
			Ctrl_Thread *border_char = comm->type == CTRL_TYPE_CPU ? &CPU_BORDER_CHAR[dims - 1][0] : &BORDER_CHAR[dims - 1][0];

			hit_clockStop(initClock);

			hit_comBarrier(lay);

			/* 4.9. COMPUTATION LOOP */
			hit_clockStart(loopClock);


			// clang-format off
			
		
			#define swap(a, b, _dims, _numborders)                  									\
			{																							\
				TileData tempA = a;          															\
				TileData tempB = b;																		\
				a = b;                      															\
				b = tempA;																				\
				a.tileMat.ref = tempA.tileMat.ref;														\
				a.tileInnerLocal.ref = tempA.tileInnerLocal.ref;										\
				a.io_tile.ref = tempA.io_tile.ref; 														\
				b.tileMat.ref = tempB.tileMat.ref;														\
				b.tileInnerLocal.ref = tempB.tileInnerLocal.ref;										\
				b.io_tile.ref = tempB.io_tile.ref;														\
				for(int _i = 0; _i<_dims; _i++){														\
					HitTile *x = (*((tempB.tileBorderOutDev+_i*2)+0)).ref;								\
					HitTile *y = (*((tempB.tileBorderOutDev+_i*2)+1)).ref;								\
					(*((a.tileBorderOutDev+_i*2)+0)).ref = (*((tempA.tileBorderOutDev+_i*2)+0)).ref;    \
					(*((a.tileBorderOutDev+_i*2)+1)).ref = (*((tempA.tileBorderOutDev+_i*2)+1)).ref;	\
					(*((b.tileBorderOutDev+_i*2)+0)).ref = x;											\
					(*((b.tileBorderOutDev+_i*2)+1)).ref = y;											\
				}																						\
				for(int _i = 0; _i<_numborders; _i++){													\
					HitTile *x = (*((tempB.tileBorderIn)+_i)).ref;										\
					(*((a.tileBorderIn)+_i)).ref = (*((tempA.tileBorderIn)+_i)).ref;					\
					(*((b.tileBorderIn)+_i)).ref = x;													\
				}																						\
			}

			// clang-format on

			if (hit_Rank == 0) {
				printf("Testing stage\n");
				fflush(stdout);
			}
			//ADD TESTING HERE
			TileData newTiles;
			allocateTileData(&newTiles, common, dims);
			//computeSingleTileData(&newTiles, &common, comm, shp, newShape, newLay, stencil, dims, stencilData, borderLow, borderHigh, &globalMat);
			computeSingleTileData(&newTiles, &common, comm, shp, shpLayout, lay, stencil, dims, stencilData, borderLow, borderHigh, &globalMat);
			for(int i = 0; i < dims; i++){
				if(
					!(shpExpanded.info.sig.sig[i].begin == newTiles.tileMat.shape.info.sig.sig[i].begin) ||
					!(shpExpanded.info.sig.sig[i].end == newTiles.tileMat.shape.info.sig.sig[i].end) ||
					!((shpExpanded.info.sig.sig[i].end - shpExpanded.info.sig.sig[i].begin + 1) == newTiles.tileMat.card[i])
				){
					returnValue = 21;
				}
			}


			//END OF TESTING BLOCK

			/* 4.11. CLOCK RESULTS */
			hit_clockStop(mainClock);
			hit_clockStop(loopClock);
			hit_clockReduce(lay, mainClock);
			hit_clockReduce(lay, initClock);
			hit_clockReduce(lay, loopClock);
			hit_clockReduce( lay, redistributeClock );
			hit_clockReduce(lay, commClock);
			hit_clockPrintMax(mainClock);
			hit_clockPrintMax(initClock);
			hit_clockPrintMax(loopClock);
			hit_clockPrintMax( redistributeClock );
			hit_clockPrintMax(commClock);
			fflush(stdout);
			hit_clockStop(stencilComputationClock);

			hit_patternFree(&(tileData).neighSync);
			hit_patternFree(&(tileDataCopy).neighSync);
			freeTileData(&tileData);
			freeTileData(&tileDataCopy);
		} // layactive

		/* 5. INACTIVE PROCESSES: ONLY COLLECTIVE CLOCK OPERATIONS */
		else {
			printf("[%d] Warning, process not active\n", hit_Rank);
			hit_clockStop(initClock);
			hit_clockStop(mainClock);
			hit_clockStop(loopClock);
			hit_clockReduce(lay, mainClock);
			hit_clockReduce(lay, initClock);
			hit_clockReduce(lay, loopClock);
			hit_clockReduce( lay, redistributeClock );
			hit_clockReduce(lay, commClock);
			hit_clockPrintMax(mainClock);
			hit_clockPrintMax(initClock);
			hit_clockPrintMax(loopClock);
			hit_clockPrintMax( redistributeClock );
			hit_clockPrintMax(commClock);
		}

		/* 6. FREE OTHER RESOURCES */
		if (hit_Rank == 0) {
			printf("Free data structures\n");
			fflush(stdout);
		}
		hit_layFree(lay);
		hit_topFree(topo);
		free(borderLow);
		free(borderHigh);

		if (hit_Rank == 0) {
			printf("Stop distributed Controllers\n");
			fflush(stdout);
		}
		Ctrl_EndBlock();

		if (hit_Rank == 0) {
			printf("Epsilod End\n");
			fflush(stdout);
		}
	}
}

int main(int argc, char *argv[]) {

	//RETURN VALUE SETTING
	returnValue = 20;

	/* EXAMPLE STENCIL DECLARATIONS */
	/* RADIUS OF THE CHOSEN STENCIL EXAMPLES */
	HitShape shpSt_1dNC      = hitShape((-2, 2));
	HitShape shpSt_1dC       = hitShape((-1, 1));
	HitShape shpSt_2dCompact = hitShape((-1, 1), (-1, 1));
	HitShape shpSt_2dNC      = hitShape((-2, 2), (-2, 2));
	HitShape shpSt_2dF5      = hitShape((-2, 0), (-2, 0));
	HitShape shpSt_3dCompact = hitShape((-1, 1), (-1, 1), (-1, 1));

	/* WEIGHTS OF THE CHOSEN STENCIL EXAMPLES */
	float stencilData_1dNC4[] = {0.5, 1, 0, 1, 0.5};

	float stencilData_1dC2[] = {1, 0, 1};

	float stencilData_4[] = {
		0, 1, 0,
		1, 0, 1,
		0, 1, 0};

	float stencilData_9[] = {
		1, 4, 1,
		4, 0, 4,
		1, 4, 1};

	float stencilData_NC9[] = {
		0, 0, 1, 0, 0,
		0, 0, 4, 0, 0,
		1, 4, 0, 4, 1,
		0, 0, 4, 0, 0,
		0, 0, 1, 0, 0};

	float stencilData_F5[] = {
		0, 0, 1,
		0, 0.5, 2,
		1, 2, 0};

	// clang-format off
	float stencilData_27[] = {
		1, 1, 1,   1, 1, 1,   1, 1, 1,
		1, 1, 1,   1, 1, 1,   1, 1, 1,
		1, 1, 1,   1, 1, 1,   1, 1, 1};
	// clang-format on

	/* Init communication system */
	Ctrl_Init(&argc, &argv);

	//TESTING

	HitShape        shpStencil;
	float          *stencilData;
	float           factor = 0;
	stencilFunction f_stencil;

	char *device_selection_file = argv[1];

	//1DIMS
	/* STENCIL SELECTION */

	int sizes_1d[3] = {100, 0, 0};

	shpStencil  = shpSt_1dNC;
	stencilData = stencilData_1dNC4;
	factor      = 3;
	f_stencil   = updateCell_1dNC4;

	/* LAUNCH TESTING */
	testingCase(sizes_1d, shpStencil, stencilData, factor, f_stencil, initData, device_selection_file);

	shpStencil  = shpSt_1dC;
	stencilData = stencilData_1dC2;
	factor      = 2;
	f_stencil   = updateCell_1dC2;

	/* LAUNCH TESTING */
	testingCase(sizes_1d, shpStencil, stencilData, factor, f_stencil, initData, device_selection_file);



	//2DIMS
	/* STENCIL SELECTION */

	int sizes_2d[3] = {100, 100, 0};

	shpStencil  = shpSt_2dCompact;
	stencilData = stencilData_4;
	factor      = 4;
	f_stencil   = updateCell_4;

	/* LAUNCH TESTING */
	testingCase(sizes_2d, shpStencil, stencilData, factor, f_stencil, initData, device_selection_file);

	shpStencil  = shpSt_2dCompact;
	stencilData = stencilData_9;
	factor      = 20;
	f_stencil   = updateCell_9;

	/* LAUNCH TESTING */
	testingCase(sizes_2d, shpStencil, stencilData, factor, f_stencil, initData, device_selection_file);

	shpStencil  = shpSt_2dNC;
	stencilData = stencilData_NC9;
	factor      = 20;
	f_stencil   = updateCell_NC9;

	/* LAUNCH TESTING */
	testingCase(sizes_2d, shpStencil, stencilData, factor, f_stencil, initData, device_selection_file);

	shpStencil  = shpSt_2dF5;
	stencilData = stencilData_F5;
	factor      = 6.5;
	f_stencil   = updateCell_F5;

	/* LAUNCH TESTING */
	testingCase(sizes_2d, shpStencil, stencilData, factor, f_stencil, initData, device_selection_file);



	//3DIMS
	/* STENCIL SELECTION */

	int sizes_3d[3] = {100, 100, 100};

	shpStencil  = shpSt_3dCompact;
	stencilData = stencilData_27;
	factor      = 27;
	f_stencil   = updateCell_3d27;

	/* LAUNCH TESTING */
	testingCase(sizes_3d, shpStencil, stencilData, factor, f_stencil, initData, device_selection_file);

	/* END */
	Ctrl_Finalize();
	return returnValue;
}
