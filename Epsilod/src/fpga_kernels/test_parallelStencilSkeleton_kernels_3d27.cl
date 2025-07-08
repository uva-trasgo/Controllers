/**
 * @file test_parallelStencilSkeleton_kernels_3d27.cl
 * @brief Epsilod: Example with several key stencils. FPGA 3d27 kernel code.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "../epsilod_ext_type.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* C.1. 3D COMPACT. RADIUS 1: 27-POINT STAR */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 2)
CTRL_KERNEL_FN(updateCell_3d27, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;
	int z = thr_k;

	hit(matrix, x, y, z) = (hit(matrixCopy, x - 1, y - 1, z - 1) +
							hit(matrixCopy, x - 1, y - 1, z) +
							hit(matrixCopy, x - 1, y - 1, z + 1) +
							hit(matrixCopy, x - 1, y, z - 1) +
							hit(matrixCopy, x - 1, y, z) +
							hit(matrixCopy, x - 1, y, z + 1) +
							hit(matrixCopy, x - 1, y + 1, z - 1) +
							hit(matrixCopy, x - 1, y + 1, z) +
							hit(matrixCopy, x - 1, y + 1, z + 1) +
							hit(matrixCopy, x, y - 1, z - 1) +
							hit(matrixCopy, x, y - 1, z) +
							hit(matrixCopy, x, y - 1, z + 1) +
							hit(matrixCopy, x, y, z - 1) +
							hit(matrixCopy, x, y, z) +
							hit(matrixCopy, x, y, z + 1) +
							hit(matrixCopy, x, y + 1, z - 1) +
							hit(matrixCopy, x, y + 1, z) +
							hit(matrixCopy, x, y + 1, z + 1) +
							hit(matrixCopy, x + 1, y - 1, z - 1) +
							hit(matrixCopy, x + 1, y - 1, z) +
							hit(matrixCopy, x + 1, y - 1, z + 1) +
							hit(matrixCopy, x + 1, y, z - 1) +
							hit(matrixCopy, x + 1, y, z) +
							hit(matrixCopy, x + 1, y, z + 1) +
							hit(matrixCopy, x + 1, y + 1, z - 1) +
							hit(matrixCopy, x + 1, y + 1, z) +
							hit(matrixCopy, x + 1, y + 1, z + 1)) /
						   27;

	CTRL_KERNEL_END();
}

// TODO: specific optimized kernels for each border type
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 2)
CTRL_KERNEL_FN(updateCell_3d27_border, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;
	int z = thr_k;

	hit(matrix, x, y, z) = (hit(matrixCopy, x - 1, y - 1, z - 1) +
							hit(matrixCopy, x - 1, y - 1, z) +
							hit(matrixCopy, x - 1, y - 1, z + 1) +
							hit(matrixCopy, x - 1, y, z - 1) +
							hit(matrixCopy, x - 1, y, z) +
							hit(matrixCopy, x - 1, y, z + 1) +
							hit(matrixCopy, x - 1, y + 1, z - 1) +
							hit(matrixCopy, x - 1, y + 1, z) +
							hit(matrixCopy, x - 1, y + 1, z + 1) +
							hit(matrixCopy, x, y - 1, z - 1) +
							hit(matrixCopy, x, y - 1, z) +
							hit(matrixCopy, x, y - 1, z + 1) +
							hit(matrixCopy, x, y, z - 1) +
							hit(matrixCopy, x, y, z) +
							hit(matrixCopy, x, y, z + 1) +
							hit(matrixCopy, x, y + 1, z - 1) +
							hit(matrixCopy, x, y + 1, z) +
							hit(matrixCopy, x, y + 1, z + 1) +
							hit(matrixCopy, x + 1, y - 1, z - 1) +
							hit(matrixCopy, x + 1, y - 1, z) +
							hit(matrixCopy, x + 1, y - 1, z + 1) +
							hit(matrixCopy, x + 1, y, z - 1) +
							hit(matrixCopy, x + 1, y, z) +
							hit(matrixCopy, x + 1, y, z + 1) +
							hit(matrixCopy, x + 1, y + 1, z - 1) +
							hit(matrixCopy, x + 1, y + 1, z) +
							hit(matrixCopy, x + 1, y + 1, z + 1)) /
						   27;

	CTRL_KERNEL_END();
}
