/**
 * @file Jacobi_Multidim_Ctrl.c
 * @brief Multidimensional Jacobi: Ctrl version host code.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Ctrl.h"
#include "../../examples/Utils/ctrl_print_info.h"
#include "Jacobi_Multidim_Ctrl.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define hit_tileSwap(a, b)               \
	{                                    \
		HitTile tmp    = *(HitTile *)&a; \
		a              = b;              \
		*(HitTile *)&b = tmp;            \
	}

#define hit_pattern_swap(a, b) \
	{                          \
		HitPattern tmp = a;    \
		a              = b;    \
		b              = tmp;  \
	}

/* Declare type for tiles */
Ctrl_NewType(double);

/* A. Characterization for the kernels */
CTRL_KERNEL_CHAR(Jacobi_1D, MANUAL, 256);
CTRL_KERNEL_CHAR(Jacobi_2D, MANUAL, 4, 64);
CTRL_KERNEL_CHAR(Jacobi_3D, MANUAL, 1, 4, 64);
CTRL_KERNEL_CHAR(Jacobi_4D, MANUAL, 1, 4, 64);

/* B. Defining kernel prototypes */
CTRL_KERNEL_PROTO(Jacobi_1D, 1, GENERIC, DEFAULT, jacobi_params);
CTRL_KERNEL_PROTO(Jacobi_2D, 1, GENERIC, DEFAULT, jacobi_params);
CTRL_KERNEL_PROTO(Jacobi_3D, 1, GENERIC, DEFAULT, jacobi_params);
CTRL_KERNEL_PROTO(Jacobi_4D, 1, GENERIC, DEFAULT, jacobi_params);

HitClock main_clock;
HitClock init_clock;
HitClock sequentialClock;

int io_read_input;
int io_write_output;
int io_write_input;

/* C. Host task to initialize the matrix */
CTRL_HOST_TASK(Init_Matrix, HitTile_double tileMat, HitTile_double tileCopy) {
	/* 0. Read the input matrix from a file */
	if (io_read_input) {
		hit_tileFileReadOptions(&tileMat, "Matrix.in", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_read_input - 1, HIT_FILE_RUNTIME, HIT_FILE_DOUBLE, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
		hit_tileUpdateFromAncestor(&tileCopy);
		return;
	}

	/* 1. Init mat = 0 */
	double zero = 0;
	hit_tileFill(&tileMat, &zero);
	hit_tileFill(&tileCopy, &zero);

	/* 2. Init borders  up(i)=1, down(i)=2, left(i)=3, right(i)=4, ... */
	HitTile root = *hit_tileRoot(&tileMat);

	int i, j, k, l;
	int numDims = hit_tileDims(tileMat);
	switch (numDims) {
		case 1:
			if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0))) {
				hit(tileMat, 0)  = 1;
				hit(tileCopy, 0) = 1;
			}
			if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0))) {
				hit(tileMat, hit_tileDimCard(tileMat, 0) - 1)  = 2;
				hit(tileCopy, hit_tileDimCard(tileMat, 0) - 1) = 2;
			}
			break;
		case 2:
			if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit(tileMat, i, 0)  = 3;
					hit(tileCopy, i, 0) = 3;
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit(tileMat, i, hit_tileDimCard(tileMat, 1) - 1)  = 4;
					hit(tileCopy, i, hit_tileDimCard(tileMat, 1) - 1) = 4;
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
				hit_tileForDimDomain(tileMat, 1, j) {
					hit(tileMat, 0, j)  = 1;
					hit(tileCopy, 0, j) = 1;
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0))) {
				hit_tileForDimDomain(tileMat, 1, j) {
					hit(tileMat, hit_tileDimCard(tileMat, 0) - 1, j)  = 2;
					hit(tileCopy, hit_tileDimCard(tileMat, 0) - 1, j) = 2;
				}
			}
			break;
		case 3:
			if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimBegin(root, 2)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 1, j) {
						hit(tileMat, i, j, 0)  = 5;
						hit(tileCopy, i, j, 0) = 5;
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimEnd(root, 2)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 1, j) {
						hit(tileMat, i, j, hit_tileDimCard(tileMat, 2) - 1)  = 6;
						hit(tileCopy, i, j, hit_tileDimCard(tileMat, 2) - 1) = 6;
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 2, k) {
						hit(tileMat, i, 0, k)  = 3;
						hit(tileCopy, i, 0, k) = 3;
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 2, k) {
						hit(tileMat, i, hit_tileDimCard(tileMat, 1) - 1, k)  = 4;
						hit(tileCopy, i, hit_tileDimCard(tileMat, 1) - 1, k) = 4;
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
				hit_tileForDimDomain(tileMat, 1, j) {
					hit_tileForDimDomain(tileMat, 2, k) {
						hit(tileMat, 0, j, k)  = 1;
						hit(tileCopy, 0, j, k) = 1;
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0)))
				hit_tileForDimDomain(tileMat, 1, j) {
					hit_tileForDimDomain(tileMat, 2, k) {
						hit(tileMat, hit_tileDimCard(tileMat, 0) - 1, j, k)  = 2;
						hit(tileCopy, hit_tileDimCard(tileMat, 0) - 1, j, k) = 2;
					}
				}
			break;
		case 4:
			if (hit_sigIn(hit_tileDimSig(tileMat, 3), hit_tileDimBegin(root, 3)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 1, j) {
						hit_tileForDimDomain(tileMat, 2, k) {
							hit(tileMat, i, j, k, 0)  = 7;
							hit(tileCopy, i, j, k, 0) = 7;
						}
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 3), hit_tileDimEnd(root, 3)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 1, j) {
						hit_tileForDimDomain(tileMat, 2, k) {
							hit(tileMat, i, j, k, hit_tileDimCard(tileMat, 3) - 1)  = 8;
							hit(tileCopy, i, j, k, hit_tileDimCard(tileMat, 3) - 1) = 8;
						}
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimBegin(root, 2)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 1, j) {
						hit_tileForDimDomain(tileMat, 3, l) {
							hit(tileMat, i, j, 0, l)  = 5;
							hit(tileCopy, i, j, 0, l) = 5;
						}
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimEnd(root, 2)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 1, j) {
						hit_tileForDimDomain(tileMat, 3, l) {
							hit(tileMat, i, j, hit_tileDimCard(tileMat, 2) - 1, l)  = 6;
							hit(tileCopy, i, j, hit_tileDimCard(tileMat, 2) - 1, l) = 6;
						}
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 2, k) {
						hit_tileForDimDomain(tileMat, 3, l) {
							hit(tileMat, i, 0, k, l)  = 3;
							hit(tileCopy, i, 0, k, l) = 3;
						}
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
				hit_tileForDimDomain(tileMat, 0, i) {
					hit_tileForDimDomain(tileMat, 2, k) {
						hit_tileForDimDomain(tileMat, 3, l) {
							hit(tileMat, i, hit_tileDimCard(tileMat, 1) - 1, k, l)  = 4;
							hit(tileCopy, i, hit_tileDimCard(tileMat, 1) - 1, k, l) = 4;
						}
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
				hit_tileForDimDomain(tileMat, 1, j) {
					hit_tileForDimDomain(tileMat, 2, k) {
						hit_tileForDimDomain(tileMat, 3, l) {
							hit(tileMat, 0, j, k, l)  = 1;
							hit(tileCopy, 0, j, k, l) = 1;
						}
					}
				}
			if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0)))
				hit_tileForDimDomain(tileMat, 1, j) {
					hit_tileForDimDomain(tileMat, 2, k) {
						hit_tileForDimDomain(tileMat, 3, l) {
							hit(tileMat, hit_tileDimCard(tileMat, 0) - 1, j, k, l)  = 2;
							hit(tileCopy, hit_tileDimCard(tileMat, 0) - 1, j, k, l) = 2;
						}
					}
				}
			break;
	}

	/* 4. Write mat on a file */
	if (io_write_input)
		hit_tileFileWriteOptions(&tileMat, "Matrix.in", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_write_input - 1, HIT_FILE_RUNTIME, HIT_FILE_DOUBLE, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
}

/* D. Defining host task prototypes */
CTRL_HOST_TASK_PROTO(Init_Matrix, 2, OUT, HitTile_double, tileMat, OUT, HitTile_double, tileCopy);

void printClockInfo() {
	#ifdef _CTRL_EXAMPLES_EXP_MODE_
	if (hit_Rank == 0)
		printf("%lf, %lf, %lf\n", main_clock.max, init_clock.max, sequentialClock.max);
	#else  // _CTRL_EXAMPLES_EXP_MODE_
	hit_clockPrintMax(main_clock);
	hit_clockPrintMax(init_clock);
	hit_clockPrintMax(sequentialClock);
	#endif // _CTRL_EXAMPLES_EXP_MODE_
	fflush(stdout);
}

HitPattern create_comm_pattern(int numDims, HitTile_double *p_tile, HitLayout matLayout) {
	hit_comTagSet(TAG_BACK, TAG_FORWARD);
	HitPattern neighSync = hit_pattern(HIT_PAT_UNORDERED);
	for (int i = 0; i < numDims; i++) {
		HitShape sendShp = HIT_SHAPE_NULL;
		HitShape recvShp = HIT_SHAPE_NULL;
		hit_shapeDimsSet(sendShp, numDims);
		hit_shapeDimsSet(recvShp, numDims);
		for (int j = 0; j < numDims; j++) {
			hit_shapeSig(sendShp, j) = hit_sig(1, hit_tileDimCard(*p_tile, j) - 2, 1);
			hit_shapeSig(recvShp, j) = hit_sig(1, hit_tileDimCard(*p_tile, j) - 2, 1);
		}

		// Send back
		hit_shapeSig(sendShp, i) = hit_sigIndex(1);
		hit_shapeSig(recvShp, i) = hit_sigIndex(hit_tileDimCard(*p_tile, i) - 1);

		hit_patternAdd(&neighSync,
					   hit_comSendRecvSelectTag(matLayout,
												hit_layNeighbor(matLayout, i, -1), p_tile,
												sendShp,
												HIT_COM_TILECOORDS,
												hit_layNeighbor(matLayout, i, +1), p_tile,
												recvShp,
												HIT_COM_TILECOORDS,
												HIT_DOUBLE, TAG_BACK));

		// Send forward
		hit_shapeSig(sendShp, i) = hit_sigIndex(hit_tileDimCard(*p_tile, i) - 2);
		hit_shapeSig(recvShp, i) = hit_sigIndex(0);

		hit_patternAdd(&neighSync,
					   hit_comSendRecvSelectTag(matLayout,
												hit_layNeighbor(matLayout, i, +1), p_tile,
												sendShp,
												HIT_COM_TILECOORDS,
												hit_layNeighbor(matLayout, i, -1), p_tile,
												recvShp,
												HIT_COM_TILECOORDS,
												HIT_DOUBLE, TAG_FORWARD));
	}
	return neighSync;
}

HitTile_double create_global_mat(int numDims, int dims[numDims]) {
	HitShape matrixShp = HIT_SHAPE_NULL;
	hit_shapeDimsSet(matrixShp, numDims);
	for (int i = 0; i < numDims; i++)
		hit_shapeSig(matrixShp, i) = hit_sigStd(dims[i]);
	return Ctrl_Domain(double, matrixShp);
}

HitLayout compute_partition(HitTopology topo, HitTile_double matrix) {
	HitShape parallelShape = hit_tileShape(matrix);
	parallelShape          = hit_shapeExpand(parallelShape, hit_tileDims(matrix), -1);
	return hit_layout(plug_layBlocks, topo, parallelShape);
}

void init_thread_spaces(PCtrl ctrl, HitTile_double tileMat, Ctrl_Thread *p_thr_space, Ctrl_Thread *p_blk_size) {
	// CPU spcific blocksizes
	Ctrl_Thread cpu_char[3] = {
		{.dims = 1, .i = 1024, .j = 1, .k = 1},
		{.dims = 2, .i = 4, .j = 1024, .k = 1},
		{.dims = 3, .i = 2, .j = 2, .k = 1024}};

	switch (hit_tileDims(tileMat)) {
		case 1:
			Ctrl_ThreadInit((*p_thr_space), hit_tileDimCard(tileMat, 0) - 2);
			*p_blk_size = ctrl->type == CTRL_TYPE_CPU ? cpu_char[0] : CTRL_THREAD_NULL;
			break;
		case 2:
			Ctrl_ThreadInit((*p_thr_space),
							hit_tileDimCard(tileMat, 0) - 2,
							hit_tileDimCard(tileMat, 1) - 2);
			*p_blk_size = ctrl->type == CTRL_TYPE_CPU ? cpu_char[1] : CTRL_THREAD_NULL;
			break;
		case 3:
		case 4:
			Ctrl_ThreadInit((*p_thr_space),
							hit_tileDimCard(tileMat, 0) - 2,
							hit_tileDimCard(tileMat, 1) - 2,
							hit_tileDimCard(tileMat, 2) - 2);
			*p_blk_size = ctrl->type == CTRL_TYPE_CPU ? cpu_char[2] : CTRL_THREAD_NULL;
			break;
	}
}

void write_output(HitTile_double tileMat) {
	HitTile        root = *hit_tileRoot(&tileMat);
	HitTile_double outputTile;
	HitShape       outputShape = hit_tileShape(tileMat);
	// First row is not mine
	if (!hit_sigIn(hit_shapeSig(outputShape, 0), hit_tileDimBegin(root, 0)))
		outputShape = hit_shapeTransform(outputShape, 0, HIT_SHAPE_BEGIN, +1);
	// Last row is not mine
	if (!hit_sigIn(hit_shapeSig(outputShape, 0), hit_tileDimEnd(root, 0)))
		outputShape = hit_shapeTransform(outputShape, 0, HIT_SHAPE_END, -1);
	if (hit_tileDims(tileMat) > 1) {
		// First column is not mine
		if (!hit_sigIn(hit_shapeSig(outputShape, 1), hit_tileDimBegin(root, 1)))
			outputShape = hit_shapeTransform(outputShape, 1, HIT_SHAPE_BEGIN, +1);
		// Last column is not mine
		if (!hit_sigIn(hit_shapeSig(outputShape, 1), hit_tileDimEnd(root, 1)))
			outputShape = hit_shapeTransform(outputShape, 1, HIT_SHAPE_END, -1);
	}
	if (hit_tileDims(tileMat) > 2) {
		// First depth is not mine
		if (!hit_sigIn(hit_shapeSig(outputShape, 2), hit_tileDimBegin(root, 2)))
			outputShape = hit_shapeTransform(outputShape, 2, HIT_SHAPE_BEGIN, +1);
		// Last depth is not mine
		if (!hit_sigIn(hit_shapeSig(outputShape, 2), hit_tileDimEnd(root, 2)))
			outputShape = hit_shapeTransform(outputShape, 2, HIT_SHAPE_END, -1);
	}
	if (hit_tileDims(tileMat) > 3) {
		// First layer is not mine
		if (!hit_sigIn(hit_shapeSig(outputShape, 3), hit_tileDimBegin(root, 3)))
			outputShape = hit_shapeTransform(outputShape, 3, HIT_SHAPE_BEGIN, +1);
		// Last layer is not mine
		if (!hit_sigIn(hit_shapeSig(outputShape, 3), hit_tileDimEnd(root, 3)))
			outputShape = hit_shapeTransform(outputShape, 3, HIT_SHAPE_END, -1);
	}
	hit_tileSelectArrayCoords(&outputTile, &tileMat, outputShape);
	hit_tileFileWriteOptions(&outputTile, "Result.out", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_write_output - 1, HIT_FILE_RUNTIME, HIT_FILE_DOUBLE, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
}

/* E. Main program */
int main(int argc, char *argv[]) {
	Ctrl_Init(&argc, &argv);

	// Process arguments
	if (argc < 4) {
		fprintf(stderr, "\nUsage: %s <sizeDim0> [ <sizeDim_i> ... ] <numIterations> <config_file>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int numDims = argc - 3;
	int dims[numDims];
	for (int i = 0; i < numDims; i++)
		dims[i] = atoi(argv[i + 1]);
	int   numIter   = atoi(argv[argc - 2]);
	char *conf_file = argv[argc - 1];

	// Read env variables
	const char *io_options[] = {"none", "array", "tile", NULL};
	io_read_input            = hit_envOptions("TEST_READ_INPUT", io_options);
	io_write_output          = hit_envOptions("TEST_WRITE_OUTPUT", io_options);
	io_write_input           = hit_envOptions("TEST_WRITE_INPUT", io_options);

	__ctrl_block__(conf_file) {

		/* 0. Init clocks */
		hit_clockSynchronizeAll();
		hit_clockStart(main_clock);
		hit_clockStart(init_clock);
		hit_clockReset(sequentialClock);

		/* 1. Setup topology global matrix and partition */
		HitTopology    topo      = hit_topology(plug_topArray, numDims);
		HitTile_double matrix    = create_global_mat(numDims, dims);
		HitLayout      matLayout = compute_partition(topo, matrix);

		/* 2. Active processes */
		if (hit_layImActive(matLayout)) {
			PCtrl ctrl = Ctrl_Get(0);

			printf("MPI_Rank[%d]", hit_Rank);
			Ctrl_PrintInfo();
			printf("\n");
			fflush(stdout);

			/* 2.1. Create and allocate local tiles with space for foreign data */
			HitShape expandedShape = hit_shapeExpand(hit_layShape(matLayout), numDims, 1);

			HitTile_double tileMat = Ctrl_Select(double, matrix, expandedShape, CTRL_SELECT_DEFAULT);
			Ctrl_Alloc(ctrl, tileMat);
			HitTile_double tileCopy = Ctrl_Select(double, matrix, expandedShape, CTRL_SELECT_DEFAULT);
			Ctrl_Alloc(ctrl, tileCopy);

			#ifdef DEBUG
			hit_dumpTileFile(tileMat, "Tile", "Matrix tile");
			#endif

			/* 2.1.3. Thread spaces for kernels */
			Ctrl_Thread thr_space, blk_size;
			init_thread_spaces(ctrl, tileMat, &thr_space, &blk_size);

			/* 2.2. Communication patterns */
			HitPattern neighSync     = create_comm_pattern(numDims, &tileMat, matLayout);
			HitPattern neighSyncCopy = create_comm_pattern(numDims, &tileCopy, matLayout);

			// TODO try to use implicit and avoid explicit mem moves
			Ctrl_SetDependanceMode(ctrl, CTRL_MODE_EXPLICIT);

			/* 2.3. Initialize matrix */
			Ctrl_HostTask(Init_Matrix, tileMat, tileCopy);
			Ctrl_MoveTo(ctrl, tileCopy);

			/* 2.4. Computation loop */
			for (int loopIndex = 0; loopIndex < numIter; loopIndex++) {
				hit_clockContinue(sequentialClock);

				/* 2.4.1. Update tile copy */
				hit_tileSwap(tileMat, tileCopy);
				hit_pattern_swap(neighSync, neighSyncCopy);
				Ctrl_MoveTo(ctrl, tileCopy);
				Ctrl_WaitTile(ctrl, tileCopy);

				/* 2.4.2. Kernel launch */
				switch (numDims) {
					case 1: Ctrl_Launch(ctrl, Jacobi_1D, thr_space, blk_size, tileMat, tileCopy); break;
					case 2: Ctrl_Launch(ctrl, Jacobi_2D, thr_space, blk_size, tileMat, tileCopy); break;
					case 3: Ctrl_Launch(ctrl, Jacobi_3D, thr_space, blk_size, tileMat, tileCopy); break;
					case 4: Ctrl_Launch(ctrl, Jacobi_4D, thr_space, blk_size, tileMat, tileCopy); break;
				}

				/* 2.4.3. Communicate */
				// Skip in last iteration
				if (loopIndex < numIter - 1) {
					Ctrl_MoveFrom(ctrl, tileMat);
					Ctrl_WaitTile(ctrl, tileMat);
					hit_patternDo(neighSync);
					hit_clockStop(sequentialClock);
				}
			}
			Ctrl_MoveFrom(ctrl, tileMat);
			Ctrl_Synchronize();

			/* 2.6. Clock results */
			hit_clockStop(sequentialClock);
			hit_clockStop(main_clock);
			hit_clockReduce(matLayout, main_clock);
			hit_clockReduce(matLayout, sequentialClock);
			printClockInfo();

			/* 2.7. Write result matrix */
			if (io_write_output) write_output(tileMat);

			/* 2.8. Free resources */
			Ctrl_Free(ctrl, tileMat, tileCopy);
			hit_patternFree(&neighSync);
		} else {
			/* 5. Inactive processes: only collective clock operations */
			printf("Warning -- Non-active process %d\n", hit_Rank);
			hit_clockStop(init_clock);
			hit_clockStop(main_clock);
			hit_clockReduce(matLayout, main_clock);
			hit_clockReduce(matLayout, sequentialClock);
			printClockInfo();
		}

		/* 6. Free other resources */
		hit_layFree(matLayout);
		hit_topFree(topo);
		Ctrl_EndBlock();
	}
	Ctrl_Finalize();
	return 0;
}
