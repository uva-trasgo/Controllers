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

#include EPSILOD_TYPES_INCLUDE
#include <epsilod_types.h>
#include "Ctrl.h"

/* HitTile types of base type and float for stencil weights */
#if !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
Ctrl_NewType(float);
#endif
Ctrl_NewType(EPSILOD_BASE_TYPE);

/* Special functions definition. */
typedef void (*stencilDeviceFunction)(PCtrl, Ctrl_Thread, Ctrl_Thread, int, HitTile(EPSILOD_BASE_TYPE), HitTile(EPSILOD_BASE_TYPE), EpsilodCoords, HitTile(float), float, Epsilod_ext *);
typedef void (*initDataDeviceFunction)(PCtrl, Ctrl_Thread, Ctrl_Thread, int, HitTile(EPSILOD_BASE_TYPE), EpsilodCoords, Epsilod_ext *);
typedef void (*initDataFunction)(HitTile(EPSILOD_BASE_TYPE), EpsilodCoords, Epsilod_ext *);
typedef void (*outputDataFunction)(HitTile(EPSILOD_BASE_TYPE), Epsilod_ext *);

typedef struct EpsilodProperties {
	int         dims;
	HitTopology topology;
} EpsilodProperties;

typedef struct EpsilodTiles {
	HitTile(EPSILOD_BASE_TYPE) * mat;
	HitTile(EPSILOD_BASE_TYPE) * inner_local;
	HitTile(EPSILOD_BASE_TYPE) (*border_out_dev)[2];
} EpsilodTiles;

typedef struct EpsilodThreads {
	Ctrl_Thread inner;
	Ctrl_Thread (*border_out_dev)[2];
} EpsilodThreads;

typedef struct EpsilodChars {
	Ctrl_Thread  inner;
	Ctrl_Thread *border_out_dev;
} EpsilodChars;

typedef struct EpsilodGlobalCoords {
	EpsilodCoords inner;
	EpsilodCoords (*border_out_dev)[2];
} EpsilodGlobalCoords;

typedef struct EpsilodSwapStructs {
	HitTile(EPSILOD_BASE_TYPE) * *tile_list;
	int         tile_list_count;
	HitPattern *comm_pattern;
} EpsilodSwapStructs;

typedef struct EpsilodComputationArgs {
	stencilDeviceFunction f_updateCell;
	EpsilodTiles         *tiles;
	EpsilodTiles         *copy_tiles;
	EpsilodThreads       *threads;
	EpsilodChars         *chars;
	EpsilodGlobalCoords  *coords;
	HitTile_float        *stencil;
	float                 factor;
	Epsilod_ext          *ext_params;
} EpsilodComputationArgs;

typedef struct EpsilodCommArgs {
	HitPattern *comm_pattern;
	HitTile(EPSILOD_BASE_TYPE) * tile_border_in;
	int  border_count;
	int *border_in_active;
	int *index_comm_border;
} EpsilodCommArgs;

/* Print usage: Extra options with environment variables */
void epsilod_print_usage();

/* Parallel stencil skeleton launcher prototype (public API). */
void stencilComputation(int                    sizes[],
						HitShape               stencilShape,
						float                  stencilData[],
						float                  factor,
						int                    numIterations,
						initDataFunction       f_init,
						initDataDeviceFunction f_dev_init,
						stencilDeviceFunction  f_init_copy,
						stencilDeviceFunction  f_dev_updateCell,
						outputDataFunction     f_output,
						Epsilod_ext           *ext_params,
						char                  *device_selection_file);

/* Definitions to declare kernels for the initialization stage */
#define REGISTER_INIT(init_name, ...)                                                                                                                                   \
	REGISTER_INIT_N(init_name, CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)                                                                                               \
	CTRL_KERNEL_CHAR(init_name, MANUAL, 64, 8, 1);                                                                                                                      \
	void init_name(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, EpsilodCoords global, Epsilod_ext *ext_params) { \
		Ctrl_LaunchToStream(ctrl, init_name, threads, blockSize, stream, mat, global, *ext_params);                                                                     \
	}

#define REGISTER_INIT_N(init_name, n_archs_times2, ...)  REGISTER_INIT_N2(init_name, n_archs_times2, __VA_ARGS__)
#define REGISTER_INIT_N2(init_name, n_archs_times2, ...) INIT_PROTO(init_name, n_archs_times2, n_archs_##n_archs_times2, __VA_ARGS__)
#define INIT_PROTO(init_name, n_archs_times2, n_archs, ...)                \
	CTRL_KERNEL_PROTO(init_name,                                           \
					  n_archs,                                             \
					  STENCIL_EXTRACT_ARCHS_##n_archs_times2(__VA_ARGS__), \
					  3,                                                   \
					  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,             \
					  INVAL, EpsilodCoords, global_coords,                 \
					  INVAL, Epsilod_ext, ext_params);

/* Definitions to declare optimized stencil kernel prototypes and wrapper launcher */
#define REGISTER_STENCIL(stencilname, ...)                                                                                                                                                                                                      \
	REGISTER_STENCIL_N(stencilname, CTRL_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)                                                                                                                                                                  \
	CTRL_KERNEL_CHAR(stencilname, MANUAL, 64, 8, 1);                                                                                                                                                                                            \
	void stencilname(PCtrl ctrl, Ctrl_Thread threads, Ctrl_Thread blockSize, int stream, HitTile(EPSILOD_BASE_TYPE) mat, HitTile(EPSILOD_BASE_TYPE) copy, EpsilodCoords global, HitTile_float stencil, float factor, Epsilod_ext *ext_params) { \
		Ctrl_LaunchToStream(ctrl, stencilname, threads, blockSize, stream, mat, copy, global, stencil, factor, *ext_params);                                                                                                                    \
	}

#define REGISTER_STENCIL_N(stencilname, n_archs_times2, ...)  REGISTER_STENCIL_N2(stencilname, n_archs_times2, __VA_ARGS__)
#define REGISTER_STENCIL_N2(stencilname, n_archs_times2, ...) STENCIL_PROTO(stencilname, n_archs_times2, n_archs_##n_archs_times2, __VA_ARGS__)
#define STENCIL_PROTO(stencilname, n_archs_times2, n_archs, ...)           \
	CTRL_KERNEL_PROTO(stencilname,                                         \
					  n_archs,                                             \
					  STENCIL_EXTRACT_ARCHS_##n_archs_times2(__VA_ARGS__), \
					  6,                                                   \
					  OUT, HitTile(EPSILOD_BASE_TYPE), matrix,             \
					  IN, HitTile(EPSILOD_BASE_TYPE), matrixCopy,          \
					  INVAL, EpsilodCoords, global_coords,                 \
					  IN, HitTile(float), stencil,                         \
					  INVAL, float, factor,                                \
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

#endif
