/**
 * @file Matrix_Increment_Ctrl_FPGA_Kernels.cl
 * @brief Matrix Increment: Ctrl FPGA kernel file
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

CTRL_KERNEL_FN(Increment, FPGA, NDRANGE, K_arg(int, n), KHitTileR_arg(float, input), KHitTileR_arg(float, output)) {
	hit(output, thr_i, thr_j) = hit(input, thr_i, thr_j) + n;

	CTRL_KERNEL_END();
}
