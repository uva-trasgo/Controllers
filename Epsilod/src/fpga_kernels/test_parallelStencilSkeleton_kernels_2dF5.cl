/**
 * @file test_parallelStencilSkeleton_kernels_2dF5.cl
 * @brief Epsilod: Example with several key stencils. FPGA 2dF5 kernel code.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "../epsilod_ext_type.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* B.3. 2D NON-COMPACT, NON-SYMMETRIC. RADIUS 2: 5-POINT STAR
 * FORWARD-DOWN WITH ONE CORNER ELEMENT */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2)
CTRL_KERNEL_FN(updateCell_F5, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = (2.0f * (hit(matrixCopy, x - 1, y) + hit(matrixCopy, x, y - 1)) +
						 (hit(matrixCopy, x - 2, y) + hit(matrixCopy, x, y - 2)) +
						 .5f * hit(matrixCopy, x - 1, y - 1)) /
						6.5f;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 2, 128, 1)
__CTRL_FPGA_KERNEL_REPLICATE(1, 4)
CTRL_KERNEL_FN(updateCell_F5_verticalBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = (2.0f * (hit(matrixCopy, x - 1, y) + hit(matrixCopy, x, y - 1)) +
						 (hit(matrixCopy, x - 2, y) + hit(matrixCopy, x, y - 2)) +
						 .5f * hit(matrixCopy, x - 1, y - 1)) /
						6.5f;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 128, 2, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_F5_horizontalBorder, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	hit(matrix, x, y) = (2.0f * (hit(matrixCopy, x - 1, y) + hit(matrixCopy, x, y - 1)) +
						 (hit(matrixCopy, x - 2, y) + hit(matrixCopy, x, y - 2)) +
						 .5f * hit(matrixCopy, x - 1, y - 1)) /
						6.5f;

	CTRL_KERNEL_END();
}
