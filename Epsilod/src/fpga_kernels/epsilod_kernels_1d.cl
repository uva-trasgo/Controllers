/**
 * @file epsilod_kernels_1d.cl
 * @brief Epsilod: Generic FPGA kernel for fully described 1D stencils.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "../epsilod_ext_type.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* 1D CELL UPDATE DEFAULT STENCIL */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 256, 1, 1)
__CTRL_FPGA_KERNEL_REPLICATE(4)
CTRL_KERNEL_FN(updateCell_default_1D, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), KHitTileR_arg(float, weight), K_arg(int, begin_x), K_arg(int, end_x), K_arg(float, factor), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;

	float sum = 0;

	int i;
	for (i = begin_x; i <= end_x; i++) {
		// if (!hit(weight, i - begin_x)) continue;
		sum += hit(matrixCopy, x + i) * hit(weight, i - begin_x);
	}
	hit(matrix, x) = sum / factor;

	CTRL_KERNEL_END();
}
