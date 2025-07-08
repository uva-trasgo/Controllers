/**
 * @file kernels_fpga (.cl)
 * @brief MatrixAdd: Specific kernels for the FPGA architecture
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Ctrl_FPGA_Kernel.h"
#include "kernels_params.h"

#define madd_params 4, INVAL, int, n_iter, IN, HitTile_float, A, IN, HitTile_float, B, IO, HitTile_float, C

CTRL_KERNEL_FN(Add, FPGA, NDRANGE, K_arg(int, n_iter), KHitTileR_arg(float, A), KHitTileR_arg(float, B), KHitTileR_arg(float, C)) {
	for (int k = 0; k < n_iter; k++) {
		hit(C, thr_i, thr_j) =
			hit(A, thr_i, thr_j) +
			hit(B, thr_i, thr_j) +
			hit(C, thr_i, thr_j);
	}

	CTRL_KERNEL_END();
};

CTRL_KERNEL_FN(Add, FPGA, TASK, K_arg(int, n_iter), KHitTileR_arg(float, A), KHitTileR_arg(float, B), KHitTileR_arg(float, C)) {
	for (int k = 0; k < n_iter; k++) {
		// Can't unroll because the sizes are not known at compile time
		for (int i = 0; i < hit_tileDimCard(C, 0); i++) {
			for (int j = 0; j < hit_tileDimCard(C, 1); j++) {
				hit(C, i, j) =
					hit(A, i, j) +
					hit(B, i, j) +
					hit(C, i, j);
			}
		}
	}

	CTRL_KERNEL_END();
};
