/**
 * @file test_parallelStencilSkeleton_kernels_2dNC9.cl
 * @brief Epsilod: Example with several key stencils. FPGA 2dNC9 kernel code.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "../epsilod_ext_type.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* B.3. 2D NON-COMPACT, RADIUS 2: 9-POINT STAR, NO CORNERS */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2)
CTRL_KERNEL_FN(updateCell_NC9, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = ((hit(matrixCopy, x - 2, y) + hit(matrixCopy, x + 2, y) + hit(matrixCopy, x, y - 2) + hit(matrixCopy, x, y + 2)) +
						 4 * (hit(matrixCopy, x - 1, y) + hit(matrixCopy, x + 1, y) + hit(matrixCopy, x, y - 1) + hit(matrixCopy, x, y + 1))) /
						20;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 2, 128, 1)
__CTRL_FPGA_KERNEL_REPLICATE(1, 4)
CTRL_KERNEL_FN(updateCell_NC9_verticalBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = ((hit(matrixCopy, x - 2, y) + hit(matrixCopy, x + 2, y) + hit(matrixCopy, x, y - 2) + hit(matrixCopy, x, y + 2)) +
						 4 * (hit(matrixCopy, x - 1, y) + hit(matrixCopy, x + 1, y) + hit(matrixCopy, x, y - 1) + hit(matrixCopy, x, y + 1))) /
						20;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 128, 2, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_NC9_horizontalBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = ((hit(matrixCopy, x - 2, y) + hit(matrixCopy, x + 2, y) + hit(matrixCopy, x, y - 2) + hit(matrixCopy, x, y + 2)) +
						 4 * (hit(matrixCopy, x - 1, y) + hit(matrixCopy, x + 1, y) + hit(matrixCopy, x, y - 1) + hit(matrixCopy, x, y + 1))) /
						20;

	CTRL_KERNEL_END();
}
