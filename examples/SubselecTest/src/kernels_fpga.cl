/**
 * @file kernels_fpga.cl
 * @brief SubselecTest: FPGA kernels for Ctrl version
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(long);

CTRL_KERNEL_FN(some_kernel, FPGA, NDRANGE, KHitTileR_arg(long, tile)) {
	hit(tile, thr_i, thr_j) *= hit(tile, thr_i, thr_j);
	// hitStrided(tile, thr_i, thr_j) *= hitStrided(tile, thr_i, thr_j);

	CTRL_KERNEL_END();
}

CTRL_KERNEL_FN(big_kernel, FPGA, NDRANGE, KHitTile_arg(long, tile1), KHitTile_arg(long, tile2), KHitTile_arg(long, tile3), KHitTile_arg(long, tile4), KHitTile_arg(long, tile5), KHitTile_arg(long, tile6), KHitTile_arg(long, dummy1), KHitTile_arg(long, dummy2), KHitTile_arg(long, dummy3), KHitTile_arg(long, dummy4), KHitTile_arg(long, dummy5), KHitTile_arg(long, dummy6)) {
	hit(tile1, thr_i, thr_j) *= hit(tile1, thr_i, thr_j);
	hit(tile2, thr_j, thr_i) = -hit(tile2, thr_j, thr_i);
	hit(tile3, thr_i, thr_j) = 0;
	hit(tile4, thr_j, thr_i) %= 2;
	hit(tile5, thr_i, thr_j) = hit(tile6, thr_j, thr_i);

	CTRL_KERNEL_END();
}
