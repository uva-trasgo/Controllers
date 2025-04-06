/**
 * @file epsilod_kernels.cu
 * @brief Epsilod: Generic kernels for fully described 1D, 2D, 3D stencils.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "epsilod_kernels.h"

#if EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
/* 1D CELL UPDATE DEFAULT STENCIL */
CTRL_KERNEL(updateCell_default_1D, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, EpsilodCoords global_coords, const KHitTile_float weight, const float factor, const Epsilod_ext ext_params, {
	int x       = thr_i;
	int begin_x = -global_coords.borders.low[0];
	int end_x   = global_coords.borders.high[0];

	float sum = 0;

	int i;
	for (i = begin_x; i <= end_x; i++) {
		if (!hit(weight, i - begin_x)) continue;
		sum += hit(matrixCopy, x + i) * hit(weight, i - begin_x);
	}
	hit(matrix, x) = sum / factor;
});

/* 2D CELL UPDATE DEFAULT STENCIL */
CTRL_KERNEL(updateCell_default_2D, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, EpsilodCoords global_coords, const KHitTile_float weight, const float factor, const Epsilod_ext ext_params, {
	int x       = thr_i;
	int y       = thr_j;
	int begin_x = -global_coords.borders.low[0];
	int end_x   = global_coords.borders.high[0];
	int begin_y = -global_coords.borders.low[1];
	int end_y   = global_coords.borders.high[1];

	float sum = 0;

	int i;
	int j;
	for (i = begin_x; i <= end_x; i++)
		for (j = begin_y; j <= end_y; j++) {
			if (!hit(weight, i - begin_x, j - begin_y)) continue;
			sum += hit(matrixCopy, x + i, y + j) * hit(weight, i - begin_x, j - begin_y);
		}
	hit(matrix, x, y) = sum / factor;
});

/* 3D CELL UPDATE DEFAULT STENCIL */
CTRL_KERNEL(updateCell_default_3D, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, EpsilodCoords global_coords, const KHitTile_float weight, const float factor, const Epsilod_ext ext_params, {
	int x       = thr_i;
	int y       = thr_j;
	int z       = thr_k;
	int begin_x = -global_coords.borders.low[0];
	int end_x   = global_coords.borders.high[0];
	int begin_y = -global_coords.borders.low[1];
	int end_y   = global_coords.borders.high[1];
	int begin_z = -global_coords.borders.low[2];
	int end_z   = global_coords.borders.high[2];

	float sum = 0;

	int i;
	int j;
	int k;
	for (i = begin_x; i <= end_x; i++)
		for (j = begin_y; j <= end_y; j++)
			for (k = begin_z; k <= end_z; k++) {
				if (!hit(weight, i - begin_x, j - begin_y, k - begin_z)) continue;
				sum += hit(matrixCopy, x + i, y + j, z + k) * hit(weight, i - begin_x, j - begin_y, k - begin_z);
			}
	hit(matrix, x, y, z) = sum / factor;
});
#endif

/* COPY KERNEL FOR DEVICE INITIALIZATION */
CTRL_KERNEL(epsilod_dev_copy, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, const KHitTile(EPSILOD_BASE_TYPE) matrix_out, {
	hit(matrix_out, thr_i) = hit(matrix, thr_i);
});

/* EMPTY KERNEL: TO SIGNAL SUBSELECTION AND ROOT TILES AS MODIFIED TO TRACK DEPENDENCIES */
CTRL_KERNEL(epsilod_dev_touch, GENERIC, DEFAULT, KHitTile(EPSILOD_BASE_TYPE) matrix, { ; });
