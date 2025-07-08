/**
 * @file epsilod_kernels_2d.cl
 * @brief Epsilod: Generic FPGA kernel for fully described 2D stencils.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "../epsilod_ext_type.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* 2D CELL UPDATE DEFAULT STENCIL */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2)
CTRL_KERNEL_FN(updateCell_default_2D, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), KHitTileR_arg(float, weight), K_arg(int, begin_x), K_arg(int, begin_y), K_arg(int, end_x), K_arg(int, end_y), K_arg(float, factor), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;

	float sum = 0;

	int i;
	int j;
	for (i = begin_x; i <= end_x; i++)
		for (j = begin_y; j <= end_y; j++) {
			// if (!hit(weight, i - begin_x, j - begin_y)) continue;
			sum += hit(matrixCopy, x + i, y + j) * hit(weight, i - begin_x, j - begin_y);
		}
	hit(matrix, x, y) = sum / factor;

	CTRL_KERNEL_END();
}
