/**
 * @file test_parallelStencilSkeleton.c
 * @brief Epsilod: Example with several key stencils. Data type: float
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include <epsilod.h>

// Global variables for input/ouput file options
int io_read_input   = 0;
int io_write_input  = 0;
int io_write_output = 0;

/* A. Protoypes */
/* A.1. Array init and output functions */
void initData(HitTile_float io_tile, EpsilodCoords global, Epsilod_ext *ext_params);
void outputData(HitTile_float io_tile, Epsilod_ext *ext_params);

/* A.2. Initialization functions */
void initData1D(HitTile_float tileMat, EpsilodCoords global);
void initData2D(HitTile_float tileMat, EpsilodCoords global);
void initData3D(HitTile_float tileMat, EpsilodCoords global);
void initData4D(HitTile_float tileMat, EpsilodCoords global);

/* B. Initialize arrays */
void initData(HitTile_float io_tile, EpsilodCoords global, Epsilod_ext *ext_params) {
	if (io_read_input) {
		// Optional: reading the input matrix from a file
		hit_tileFileReadOptions(&io_tile, "Matrix.in", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_read_input - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	} else {
		HitTile_float tileMat = *(HitTile_float *)hit_tileMemoryAncestor(&io_tile);

		char *omp_env     = getenv("OMP_NUM_THREADS");
		int   omp_threads = (omp_env != NULL) ? atoi(omp_env) : 1;
		#pragma omp parallel for num_threads(omp_threads)
		for (int i = 0; i < tileMat.acumCard; i++) {
			hit(tileMat, i) = 0;
		}

		// Init borders
		switch (global.dims) {
			case 1: initData1D(tileMat, global); break;
			case 2: initData2D(tileMat, global); break;
			case 3: initData3D(tileMat, global); break;
			case 4: initData4D(tileMat, global); break;
			default:
				fprintf(stderr, "Error: This init function only works for up to 4 dimensions\n");
				MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
				exit(EXIT_FAILURE);
		}
	}

	if (io_write_input) {
		// Write mat to a file (debugging)
		hit_tileFileWriteOptions(&io_tile, "Matrix.copy", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_write_input - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	}
}

/* B.2. Initialize borders array 1D */
void initData1D(HitTile_float tileMat, EpsilodCoords global) {
	HitTile root = *hit_tileRoot(&tileMat);

	// Init borders down(i)=1, up(i)=2.
	// First elements are mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0))) {
		for (int i = 0; i < global.borders.low[0]; i++) {
			hit(tileMat, i) = 1;
		}
	}

	// Last elements are mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0))) {
		for (int i = 0; i < global.borders.high[0]; i++) {
			hit(tileMat, hit_tileDimCard(tileMat, 0) - 1 - i) = 2;
		}
	}
}

/* B.3. Initialize borders matrix 2D */
void initData2D(HitTile_float tileMat, EpsilodCoords global) {
	HitTile root = *hit_tileRoot(&tileMat);
	int     i, j;

	// Init borders up(i)=1, down(i)=2, left(i)=3, right(i)=4
	// First column is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
		for (int j = 0; j < global.borders.low[1]; j++)
			hit_tileForDimDomain(tileMat, 0, i)
				hit(tileMat, i, j) = 3;

	// Last column is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
		for (j = 0; j < global.borders.high[1]; j++)
			hit_tileForDimDomain(tileMat, 0, i)
				hit(tileMat, i, hit_tileDimCard(tileMat, 1) - 1 - j) = 4;

	// First row is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
		for (i = 0; i < global.borders.low[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit(tileMat, i, j) = 1;

	// Last row is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0)))
		for (i = 0; i < global.borders.high[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit(tileMat, hit_tileDimCard(tileMat, 0) - 1 - i, j) = 2;
}

/* B.4. Initialize borders matrix 3D */
void initData3D(HitTile_float tileMat, EpsilodCoords global) {
	HitTile root = *hit_tileRoot(&tileMat);
	int     i, j, k;

	// Init borders
	// First layer of k is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimBegin(root, 2)))
		hit_tileForDimDomain(tileMat, 0, i) {
			hit_tileForDimDomain(tileMat, 1, j) {
				for (k = 0; k < global.borders.low[2]; k++)
					hit(tileMat, i, j, k) = 5;
			}
		}

	// Last layer of k is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimEnd(root, 2)))
		hit_tileForDimDomain(tileMat, 0, i) {
			hit_tileForDimDomain(tileMat, 1, j) {
				for (k = 0; k < global.borders.high[2]; k++)
					hit(tileMat, i, j, hit_tileDimCard(tileMat, 2) - 1 - k) = 6;
			}
		}

	// First layer of j is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
		hit_tileForDimDomain(tileMat, 0, i) {
			for (j = 0; j < global.borders.low[1]; j++)
				hit_tileForDimDomain(tileMat, 2, k)
					hit(tileMat, i, j, k) = 3;
		}

	// Last layer of j is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
		hit_tileForDimDomain(tileMat, 0, i) {
			for (j = 0; j < global.borders.high[1]; j++)
				hit_tileForDimDomain(tileMat, 2, k)
					hit(tileMat, i, hit_tileDimCard(tileMat, 1) - 1 - j, k) = 4;
		}

	// First layer of i is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
		for (i = 0; i < global.borders.low[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileForDimDomain(tileMat, 2, k)
					hit(tileMat, i, j, k) = 1;

	// Last layer of i is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0)))
		for (i = 0; i < global.borders.high[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileForDimDomain(tileMat, 2, k)
					hit(tileMat, hit_tileDimCard(tileMat, 0) - 1 - i, j, k) = 2;
}

/* B.5. Initialize borders matrix 4D */
void initData4D(HitTile_float tileMat, EpsilodCoords global) {
	HitTile root = *hit_tileRoot(&tileMat);
	int     i, j, k, l;

	// Init borders
	// First layer of l is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 3), hit_tileDimBegin(root, 3)))
		hit_tileForDimDomain(tileMat, 0, i) {
			hit_tileForDimDomain(tileMat, 1, j) {
				hit_tileForDimDomain(tileMat, 2, k) {
					for (l = 0; l < global.borders.low[3]; l++)
						hit(tileMat, i, j, k, l) = 7;
				}
			}
		}

	// Last layer of l is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 3), hit_tileDimEnd(root, 3)))
		hit_tileForDimDomain(tileMat, 0, i) {
			hit_tileForDimDomain(tileMat, 1, j) {
				hit_tileForDimDomain(tileMat, 2, k) {
					for (l = 0; l < global.borders.high[3]; l++)
						hit(tileMat, i, j, k, hit_tileDimCard(tileMat, 3) - 1 - l) = 8;
				}
			}
		}

	// First layer of k is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimBegin(root, 2)))
		hit_tileForDimDomain(tileMat, 0, i) {
			hit_tileForDimDomain(tileMat, 1, j) {
				for (k = 0; k < global.borders.low[2]; k++)
					hit_tileForDimDomain(tileMat, 3, l)
						hit(tileMat, i, j, k, l) = 5;
			}
		}

	// Last layer of k is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 2), hit_tileDimEnd(root, 2)))
		hit_tileForDimDomain(tileMat, 0, i) {
			hit_tileForDimDomain(tileMat, 1, j) {
				for (k = 0; k < global.borders.high[2]; k++)
					hit_tileForDimDomain(tileMat, 3, l)
						hit(tileMat, i, j, hit_tileDimCard(tileMat, 2) - 1 - k, l) = 6;
			}
		}

	// First layer of j is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimBegin(root, 1)))
		hit_tileForDimDomain(tileMat, 0, i) {
			for (j = 0; j < global.borders.low[1]; j++)
				hit_tileForDimDomain(tileMat, 2, k)
					hit_tileForDimDomain(tileMat, 3, l)
						hit(tileMat, i, j, k, l) = 3;
		}

	// Last layer of j is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 1), hit_tileDimEnd(root, 1)))
		hit_tileForDimDomain(tileMat, 0, i) {
			for (j = 0; j < global.borders.high[1]; j++)
				hit_tileForDimDomain(tileMat, 2, k)
					hit_tileForDimDomain(tileMat, 3, l)
						hit(tileMat, i, hit_tileDimCard(tileMat, 1) - 1 - j, k, l) = 4;
		}

	// First layer of i is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimBegin(root, 0)))
		for (i = 0; i < global.borders.low[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileForDimDomain(tileMat, 2, k)
					hit_tileForDimDomain(tileMat, 3, l)
						hit(tileMat, i, j, k, l) = 1;

	// Last layer of i is mine
	if (hit_sigIn(hit_tileDimSig(tileMat, 0), hit_tileDimEnd(root, 0)))
		for (i = 0; i < global.borders.high[0]; i++)
			hit_tileForDimDomain(tileMat, 1, j)
				hit_tileForDimDomain(tileMat, 2, k)
					hit_tileForDimDomain(tileMat, 3, l)
						hit(tileMat, hit_tileDimCard(tileMat, 0) - 1 - i, j, k, l) = 2;
}

/* C. Write results */
void outputData(HitTile_float io_tile, Epsilod_ext *ext_params) {
	if (io_write_output) {
		hit_tileFileWriteOptions(&io_tile, "Matrix.out", NULL, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME, io_write_output - 1, HIT_FILE_RUNTIME, HIT_FILE_FLOAT, HIT_FILE_RUNTIME, HIT_FILE_RUNTIME);
	}
}

/* D. Declarations of optimized stencil kernels:
 * see test_parallelStencilSkeleton_kernels.c
 */
REGISTER_STENCIL(updateCell_1dNC4, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_1dC2, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_2d4, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_2d8, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_2dNC8, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_2dF5, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_3d27, GENERIC, DEFAULT);
REGISTER_STENCIL(updateCell_4d8, GENERIC, DEFAULT);

void print_usage(char *argv[]) {
	if (hit_Rank == 0) {
		fprintf(stderr, "\n=== DISTRIBUTED MULTI-GPU STENCIL COMPUTATION EXAMPLE ===\n");
		fprintf(stderr, "\nUsage: %s <stencilId> <size0> [ <size1> [ <size2> [ <size3> ] ] ] <numIterations> <device_selection_file>\n", argv[0]);
		fprintf(stderr, "\t1dnc4\t1D Non-Compact, 4-points\n");
		fprintf(stderr, "\t1dc2\t1D Compact, 2-points\n");
		fprintf(stderr, "\t2d4\t2D Compact 4-star\n");
		fprintf(stderr, "\t2d8\t2D Compact 8-star\n");
		fprintf(stderr, "\t2dnc8\t2D Non-Compact, higher-order, 8-star\n");
		fprintf(stderr, "\t2df5\t2D Non-Compact, Forward (right-down), 5-points\n");
		fprintf(stderr, "\t3d27\t3D Compact, 27-star\n");
		fprintf(stderr, "\t4d8\t4D Compact, 8-star\n");
		fprintf(stderr, "\tAppend \"_\" before the stencilId (e.g: _2d4) to use an optimized implementation instead of a generic one.\n");
		fprintf(stderr, "\nEnvironment variable:\n");
		fprintf(stderr, "\tTEST_EPSILOD_READ_INPUT=none|array|tile Read input from file Matrix.in\n");
		fprintf(stderr, "\tTEST_EPSILOD_WRITE_OUTPUT=none|array|tile Write output to file Matrix.out\n");
		fprintf(stderr, "\tTEST_EPSILOD_WRITE_INPUT=none|array|tile Wite input to file Matrix.copy\n");
		epsilod_print_usage();
		hit_filePrintUsage();
		fprintf(stderr, "\n");
	}
}

int main(int argc, char *argv[]) {
	// Stencil declarations
	// Stencil shapes
	HitShape shpSt_1dNC      = hitShape((-2, 2));
	HitShape shpSt_1dC       = hitShape((-1, 1));
	HitShape shpSt_2dCompact = hitShape((-1, 1), (-1, 1));
	HitShape shpSt_2dNC      = hitShape((-2, 2), (-2, 2));
	HitShape shpSt_2dF5      = hitShape((-2, 0), (-2, 0));
	HitShape shpSt_3dCompact = hitShape((-1, 1), (-1, 1), (-1, 1));
	HitShape shpSt_4d8       = hitShape((-1, 1), (-1, 1), (-1, 1), (-1, 1));

	// Stencil weights
	float stencilData_1dNC4[] = {0.5, 1, 0, 1, 0.5};

	float stencilData_1dC2[] = {1, 0, 1};

	float stencilData_4[] = {
		0, 1, 0,
		1, 0, 1,
		0, 1, 0};

	float stencilData_8[] = {
		1, 4, 1,
		4, 0, 4,
		1, 4, 1};

	float stencilData_NC8[] = {
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

	float stencilData_4d8[] = {
		0, 0, 0,   0, 0, 0,   0, 0, 0, 
		0, 0, 0,   0, 1, 0,   0, 0, 0, 
		0, 0, 0,   0, 0, 0,   0, 0, 0, 

		0, 0, 0,   0, 1, 0,   0, 0, 0, 
		0, 1, 0,   1, 0, 1,   0, 1, 0, 
		0, 0, 0,   0, 1, 0,   0, 0, 0, 
		
		0, 0, 0,   0, 0, 0,   0, 0, 0, 
		0, 0, 0,   0, 1, 0,   0, 0, 0,
		0, 0, 0,   0, 0, 0,   0, 0, 0};
	// clang-format on

	// Init communication system
	Ctrl_Init(&argc, &argv);

	if (argc < 5 || argc > 8) {
		print_usage(argv);
		exit(EXIT_FAILURE);
	}

	// Read arguments
	int dims = argv[1][0] == '_' ? argv[1][1] - '0' : argv[1][0] - '0';
	if (dims < 0 || dims > 4) {
		fprintf(stderr, "Error: Non-supported number of dimensions: %d, should be in the range [1:4]\n\n", dims);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}

	// Argument to select stencil example
	char *stencilType = argv[1][0] == '_' ? &argv[1][1] : argv[1];

	// Arguments for each dimension size
	int sizes[4] = {0};
	for (int d = 0; d < dims; d++)
		sizes[d] = atoi(argv[2 + d]);

	// Argument for iterations
	int numIter = atoi(argv[2 + dims]);

	// Argument for config file
	char *device_selection_file = argv[3 + dims];

	// Stencil selection
	HitShape              shpStencil;
	float                *stencilData;
	float                 factor = 0;
	stencilDeviceFunction f_stencil;

	if (!strcmp(stencilType, "1dnc4")) {
		shpStencil  = shpSt_1dNC;
		stencilData = stencilData_1dNC4;
		factor      = 3;
		f_stencil   = updateCell_1dNC4;
	} else if (!strcmp(stencilType, "1dc2")) {
		shpStencil  = shpSt_1dC;
		stencilData = stencilData_1dC2;
		factor      = 2;
		f_stencil   = updateCell_1dC2;
	} else if (!strcmp(stencilType, "2d4")) {
		shpStencil  = shpSt_2dCompact;
		stencilData = stencilData_4;
		factor      = 4;
		f_stencil   = updateCell_2d4;
	} else if (!strcmp(stencilType, "2d8")) {
		shpStencil  = shpSt_2dCompact;
		stencilData = stencilData_8;
		factor      = 20;
		f_stencil   = updateCell_2d8;
	} else if (!strcmp(stencilType, "2dnc8")) {
		shpStencil  = shpSt_2dNC;
		stencilData = stencilData_NC8;
		factor      = 20;
		f_stencil   = updateCell_2dNC8;
	} else if (!strcmp(stencilType, "2df5")) {
		shpStencil  = shpSt_2dF5;
		stencilData = stencilData_F5;
		factor      = 6.5;
		f_stencil   = updateCell_2dF5;
	} else if (!strcmp(stencilType, "3d27")) {
		shpStencil  = shpSt_3dCompact;
		stencilData = stencilData_27;
		factor      = 27;
		f_stencil   = updateCell_3d27;
	} else if (!strcmp(stencilType, "4d8")) {
		shpStencil  = shpSt_4d8;
		stencilData = stencilData_4d8;
		factor      = 8;
		f_stencil   = updateCell_4d8;
	} else {
		fprintf(stderr, "\nError: Unknown stencil type %s\n", stencilType);
		print_usage(argv);
		Ctrl_Finalize();
		exit(EXIT_FAILURE);
	}

	// Skip specific kernel when the user requests the use of generic kernel
	if (argv[1][0] != '_') f_stencil = NULL;

	// Read env variables
	const char *io_options[] = {"none", "array", "tile", NULL};
	io_read_input            = hit_envOptions("TEST_EPSILOD_READ_INPUT", io_options);
	io_write_output          = hit_envOptions("TEST_EPSILOD_WRITE_OUTPUT", io_options);
	io_write_input           = hit_envOptions("TEST_EPSILOD_WRITE_INPUT", io_options);

	// Launch stencil computation
	stencilComputation(sizes, shpStencil, stencilData, factor, numIter, initData, NULL, NULL, f_stencil, outputData, NULL, device_selection_file);

	Ctrl_Finalize();
	return 0;
}
