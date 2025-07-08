/**
 * @file epsilod.h
 * @brief Epsilod: Stencil code: Any dimensions, stencil as a pattern of weights. Data type: float
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#ifndef _EPSILOD_H_
#define _EPSILOD_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef _CTRL_ARCH_CUDA_
#include <cuda_runtime_api.h>
#endif // _CTRL_ARCH_CUDA_

#include <epsilod_ext_type.h>

#include "Ctrl.h"

Ctrl_NewType(float);

/* Print usage: Extra options with environment variables */
void epsilod_print_usage();

/* Special functions definition. */
typedef void (*stencilFunction)(PCtrl, Ctrl_Thread, Ctrl_Thread, int, HitTile_float, HitTile_float, HitTile_float, float, Epsilod_ext *);
typedef void (*initDataFunction)(HitTile_float, int, int[], int[], Epsilod_ext);
typedef void (*outputDataFunction)(HitTile_float, Epsilod_ext);

/**
 * @brief Parform a stencil computation.
 *
 * @param sizes Sizes of the matrix to compute. in elements row major order.
 * @param stencilShape Shape of the stencil operator.
 * @param stencilData Weights of neighbours for the stencil operator. If \p f_updateCell is not null the value of non zero elements doesn't matter.
 * @param factor Divisor for neighbouring elements. Only used if \p f_updateCell is null.
 * @param numIterations Number of iterations to compute.
 * @param f_updateCell Optional. Custom kernel to perform the stencil operation..
 * @param f_init Initialization function.
 * @param f_output Output function.
 * @param ext_params Extra arguments to pass to the kernel.
 * @param device_selection_file Controller device configuration file containing the information on which devices to use.
 */
void stencilComputation(int                sizes[],
						HitShape           stencilShape,
						float              stencilData[],
						float              factor,
						int                numIterations,
						stencilFunction    f_updateCell,
						initDataFunction   f_init,
						outputDataFunction f_output,
						Epsilod_ext       *ext_params,
						char              *device_selection_file);

/**
 * @brief Register a stencil kernel.
 *
 * Creates a function named \p stencilName which may be passed to \e stencilComputation.
 *
 * @hideinitializer
 * @param stencilName Name of the Kernel for the stencil.
 * @param ... List of implementations available for the kernel. In the form: ARCH, SUBARCH.
 */
#define REGISTER_STENCIL(stencilName, ...)                                                          \
	REGISTER_STENCIL_N(stencilName, CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)                      \
	CTRL_KERNEL_CHAR(stencilName, MANUAL, 64, 8, 1);                                                \
	void stencilName(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream,            \
					 HitTile_float mat, HitTile_float copy, HitTile_float weight, float factor,     \
					 Epsilod_ext *ext_params) {                                                     \
		Ctrl_LaunchToStream(ctrl, stencilName, threads, blockSize, stream, mat, copy, *ext_params); \
	}

#define REGISTER_STENCIL_N(stencilName, n_archs_times2, ...)  REGISTER_STENCIL_N2(stencilName, n_archs_times2, __VA_ARGS__)
#define REGISTER_STENCIL_N2(stencilName, n_archs_times2, ...) STENCIL_PROTO(stencilName, n_archs_times2, n_archs_##n_archs_times2, __VA_ARGS__)
#define STENCIL_PROTO(stencilName, n_archs_times2, n_archs, ...)           \
	CTRL_KERNEL_PROTO(stencilName,                                         \
					  n_archs,                                             \
					  STENCIL_EXTRACT_ARCHS_##n_archs_times2(__VA_ARGS__), \
					  3,                                                   \
					  OUT, HitTile_float, matrix,                          \
					  IN, HitTile_float, matrixCopy,                       \
					  INVAL, Epsilod_ext, ext_params);

#define STENCIL_EXTRACT_ARCHS_2(arch, subarch)       arch, subarch
#define STENCIL_EXTRACT_ARCHS_4(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_2(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_6(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_4(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_8(arch, subarch, ...)  arch, subarch, STENCIL_EXTRACT_ARCHS_6(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_10(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_8(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_12(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_10(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_14(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_12(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_16(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_14(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_18(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_16(__VA_ARGS__)
#define STENCIL_EXTRACT_ARCHS_20(arch, subarch, ...) arch, subarch, STENCIL_EXTRACT_ARCHS_18(__VA_ARGS__)

#define n_archs_2  1
#define n_archs_4  2
#define n_archs_6  3
#define n_archs_8  4
#define n_archs_10 5
#define n_archs_12 6
#define n_archs_14 7
#define n_archs_16 8
#define n_archs_18 9
#define n_archs_20 10

/**
 * @brief Register a border detector for a stencil.
 *
 * This allows to specify a different kernel for the inner part of the matrix and the border.
 * @note Currently this only works for 2D stencils and FPGA kernel implementations.
 *
 * @hideinitializer
 * @param inner_kernel Name of the Kernel for the internal part of the matrix.
 * @param hborder_kernel Name of the Kernel for the horizontal borders of the matrix.
 * @param vborder_kernel Name of the Kernel for the vertical borders of the matrix.
 */
#define REGISTER_BORDER_DETECTOR(inner_kernel, hborder_kernel, vborder_kernel)                      \
	void inner_kernel##_multikernel(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize,         \
									int stream, HitTile_float mat, HitTile_float copy,              \
									HitTile_float stencil, float factor, Epsilod_ext *ext_params) { \
		Ctrl_Info info = Ctrl_GetInfo(ctrl);                                                        \
		if (!strcmp(info.type, "FPGA")) {                                                           \
			switch (hit_tileDims(mat)) {                                                            \
				case 2: {                                                                           \
					int x_size     = hit_tileDimCard(mat, 1);                                       \
					int y_size     = hit_tileDimCard(mat, 0);                                       \
					int up_size    = -hit_tileDimBegin(stencil, 0);                                 \
					int down_size  = hit_tileDimEnd(stencil, 0);                                    \
					int left_size  = -hit_tileDimBegin(stencil, 1);                                 \
					int right_size = hit_tileDimEnd(stencil, 1);                                    \
					if (x_size <= left_size || x_size <= right_size) {                              \
						Ctrl_LaunchToStream(ctrl, vborder_kernel, threads, blockSize, stream,       \
											mat, copy, *ext_params);                                \
					} else if (y_size <= up_size || y_size <= down_size) {                          \
						Ctrl_LaunchToStream(ctrl, hborder_kernel, threads, blockSize, stream,       \
											mat, copy, *ext_params);                                \
					} else {                                                                        \
						Ctrl_LaunchToStream(ctrl, inner_kernel, threads, blockSize, stream,         \
											mat, copy, *ext_params);                                \
					}                                                                               \
					break;                                                                          \
				}                                                                                   \
				/* TODO: Other dimensions */                                                        \
				default:                                                                            \
					Ctrl_LaunchToStream(ctrl, inner_kernel, threads, blockSize, stream,             \
										mat, copy, *ext_params);                                    \
			}                                                                                       \
		} else {                                                                                    \
			Ctrl_LaunchToStream(ctrl, inner_kernel, threads, blockSize, stream,                     \
								mat, copy, *ext_params);                                            \
		}                                                                                           \
	}
#endif
