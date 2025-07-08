/**
 * @file test_parallelStencilSkeleton_kernels_1dNC4.cl
 * @brief Epsilod: Example with several key stencils. FPGA 1dNC4 kernel code.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "../epsilod_ext_type.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* A.1. 1D NON-COMPACT RADIUS 2 */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 256, 1, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_1dNC4, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;

	hit(matrix, x) = (0.5 * (hit(matrixCopy, x - 2) + hit(matrixCopy, x + 2)) +
					  hit(matrixCopy, x - 1) + hit(matrixCopy, x + 1)) /
					 3;

	CTRL_KERNEL_END();
}

__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_1dNC4_border, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;

	hit(matrix, x) = (0.5 * (hit(matrixCopy, x - 2) + hit(matrixCopy, x + 2)) +
					  hit(matrixCopy, x - 1) + hit(matrixCopy, x + 1)) /
					 3;

	CTRL_KERNEL_END();
}
