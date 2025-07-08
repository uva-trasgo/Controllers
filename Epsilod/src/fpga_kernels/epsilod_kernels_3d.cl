/**
 * @file epsilod_kernels_3d.cl
 * @brief Epsilod: Generic FPGA kernel for fully described 3D stencils.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "../epsilod_ext_type.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* 3D CELL UPDATE DEFAULT STENCIL */
__CTRL_FPGA_KERNEL_OPTIMIZE_FOR(8, 64, 4, 1)
__CTRL_FPGA_KERNEL_REPLICATE(2, 2, 2)
CTRL_KERNEL_FN(updateCell_default_3D, FPGA, NDRANGE, KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy), KHitTileR_arg(float, weight), K_arg(int, begin_x), K_arg(int, begin_y), K_arg(int, begin_z), K_arg(int, end_x), K_arg(int, end_y), K_arg(int, end_z), K_arg(float, factor), K_arg(Epsilod_ext, ext_params)) {
	int x = thr_i;
	int y = thr_j;
	int z = thr_k;

	float sum = 0;

	int i;
	int j;
	int k;
	for (i = begin_x; i <= end_x; i++)
		for (j = begin_y; j <= end_y; j++)
			for (k = begin_z; k <= end_z; k++) {
				// if (!hit(weight, i - begin_x, j - begin_y, k - begin_z)) continue;
				sum += hit(matrixCopy, x + i, y + j, z + k) * hit(weight, i - begin_x, j - begin_y, k - begin_z);
			}
	hit(matrix, x, y, z) = sum / factor;

	CTRL_KERNEL_END();
}
