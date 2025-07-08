/**
 * @file test_parallelStencilSkeleton_kernels_2d4.cl
 * @brief Epsilod: Example with several key stencils. FPGA 2d4 kernel code.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "../epsilod_ext_type.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* B.1. 2D COMPACT, RADIUS 1: 4-POINT STAR, NO CORNERS */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2)
CTRL_KERNEL_FN(updateCell_4, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = (hit(matrixCopy, x - 1, y) +
						 hit(matrixCopy, x + 1, y) +
						 hit(matrixCopy, x, y - 1) +
						 hit(matrixCopy, x, y + 1)) /
						4;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 1, 256, 1)
__CTRL_FPGA_KERNEL_REPLICATE(1, 4)
CTRL_KERNEL_FN(updateCell_4_verticalBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = (hit(matrixCopy, x - 1, y) +
						 hit(matrixCopy, x + 1, y) +
						 hit(matrixCopy, x, y - 1) +
						 hit(matrixCopy, x, y + 1)) /
						4;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 256, 1, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_4_horizontalBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = (hit(matrixCopy, x - 1, y) +
						 hit(matrixCopy, x + 1, y) +
						 hit(matrixCopy, x, y - 1) +
						 hit(matrixCopy, x, y + 1)) /
						4;

	CTRL_KERNEL_END();
}
