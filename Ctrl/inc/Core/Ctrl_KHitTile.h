#ifndef _CTRL_CORE_KHITTILE_H_
#define _CTRL_CORE_KHITTILE_H_
/**
 * @file Ctrl_KHitTile.h
 * @brief KhitTile structure definitions.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

///@cond INTERNAL
// TODO @sergioalo probably we should avoid dependencies from here to cuda/ocl headers
#ifdef _CTRL_ARCH_CUDA_
#include <cuda_runtime_api.h>
#endif // _CTRL_ARCH_CUDA_

#ifdef _CTRL_ARCH_HIP_
#include <hip/hip_runtime_api.h>
#endif // _CTRL_ARCH_HIP_

#ifdef _CTRL_ARCH_OPENCL_GPU_
#include <CL/cl.h>
#endif // _CTRL_ARCH_OPENCL_GPU_

#ifdef __cplusplus
#if defined(__CUDACC__) || defined(__HIPCC__)
#define restrict __restrict__
#else // !(__CUDACC__ || __HIPCC__)
#define restrict
#endif // (__CUDACC__ || __HIPCC__)
#endif // __cplusplus

//  Global memory specifier for different backend kernels
#ifdef _CTRL_FPGA_KERNEL_FILE_
#define GLOBAL __global
#else // !_CTRL_FPGA_KERNEL_FILE_
#define GLOBAL
#endif // _CTRL_FPGA_KERNEL_FILE_

/**
 * Holds backend specific extra fields
 */
typedef union Ctrl_KHitTile_Ext {
	#ifdef _CTRL_ARCH_CUDA_
	struct {
		cudaTextureObject_t tex; /**< For texture use */
	} cuda;
	#endif // _CTRL_ARCH_CUDA_
	#ifdef _CTRL_ARCH_HIP_
	struct {
		hipTextureObject_t tex; /**< For texture use */
	} hip;
	#endif // _CTRL_ARCH_HIP_
	#ifdef _CTRL_ARCH_OPENCL_GPU_
	struct {
		cl_mem     tex; /**< For texture use */
		cl_sampler smp; /**< For texture use */
	} ocl;
	#endif // _CTRL_ARCH_OPENCL_GPU_
	#ifdef _CTRL_ARCH_FPGA_
	char dummy; // Needed so that the struct is not zero-sized in FPGA compilation

	#endif // _CTRL_ARCH_FPGA_
} Ctrl_KHitTile_Ext;

/**
 * Stripped down version of an abstract \e HitTile so it`s more suitable for use in kernels
 */
typedef struct {
	void             *data;                          /**< Pointer to the data held by the tile. */
	HitInd            origAcumCard[HIT_MAXDIMS + 1]; /**< Dimension accumulated cardinalities. */
	HitInd            card[HIT_MAXDIMS];             /**< Dimension cardinalities. */
	HitInd            offset;                        /**< Offset to original data. For hierarchical subselections. */
	Ctrl_KHitTile_Ext ext;                           /**< Backend specific extra fields  */
} KHitTile;

typedef struct {
	void *restrict data;                             /**< Pointer to the data held by the tile, with a restrict qualifier to improve performance. */
	HitInd            origAcumCard[HIT_MAXDIMS + 1]; /**< Dimension accumulated cardinalities. */
	HitInd            card[HIT_MAXDIMS];             /**< Dimension cardinalities. */
	HitInd            offset;                        /**< Offset to original data. For hierarchical subselections. */
	Ctrl_KHitTile_Ext ext;                           /**< Backend specific extra fields  */
} KHitTileR;

/* @author: Gabriel Rodriguez-Canal
   @brief: KHitTile wrapper to isolate tile coordinates and offset from data, as structures with pointers cannot be passed as arguments */
#ifdef _CTRL_ARCH_FPGA_
typedef struct {
	HitInd origAcumCard[HIT_MAXDIMS + 1];
	HitInd card[HIT_MAXDIMS];
	HitInd offset;
} KHitTile_fpga_wrapper;
#endif // _CTRL_ARCH_FPGA_

#ifdef _CTRL_ARCH_OPENCL_GPU_
typedef struct {
	HitInd origAcumCard[HIT_MAXDIMS + 1];
	HitInd card[HIT_MAXDIMS];
	HitInd offset;
} KHitTile_opencl_wrapper;
#endif // _CTRL_ARCH_OPENCL_GPU_

/**
 * Generate particular polymorphic types of KHitTile as well as appropiate wrappers for OpenCL GPU and FPGA.
 * @hideinitializer
 *
 * @param type Name of a valid native or derived C type.
 *
 * @see Ctrl_NewType, KHitTile
 */
#define hit_ktileNewType(type)                           \
	typedef struct {                                     \
		GLOBAL type      *data;                          \
		HitInd            origAcumCard[HIT_MAXDIMS + 1]; \
		HitInd            card[HIT_MAXDIMS];             \
		HitInd            offset;                        \
		Ctrl_KHitTile_Ext ext;                           \
	} KHitTile_##type;                                   \
	CTRL_KTILE_VARS(type);                               \
                                                         \
	typedef struct {                                     \
		GLOBAL type *restrict data;                      \
		HitInd            origAcumCard[HIT_MAXDIMS + 1]; \
		HitInd            card[HIT_MAXDIMS];             \
		HitInd            offset;                        \
		Ctrl_KHitTile_Ext ext;                           \
	} KHitTileR_##type;                                  \
                                                         \
	typedef GLOBAL type *data_KHitTile_##type;           \
	typedef GLOBAL type *restrict data_KHitTileR_##type;

#ifndef _CTRL_FPGA_KERNEL_FILE_
#define CTRL_KTILE_VARS(type)                                                                              \
	static const char *raw_ktile_KHitTile_##type __attribute__((unused))     = CTRL_MACRO_STRINGIFY(type); \
	static const char *raw_def_ktile_KHitTile_##type __attribute__((unused)) = CTRL_MACRO_STRINGIFY(       \
		typedef struct {                                                                                   \
			__global type *data;                                                                           \
			HitInd         origAcumCard[HIT_MAXDIMS + 1];                                                  \
			HitInd         card[HIT_MAXDIMS];                                                              \
		} KHitTile_##type##_write;                                                                         \
		typedef struct {                                                                                   \
			__global const type *data;                                                                     \
			HitInd               origAcumCard[HIT_MAXDIMS + 1];                                            \
			HitInd               card[HIT_MAXDIMS];                                                        \
		} KHitTile_##type##_read;);                                                                        \
	static bool raw_added_ktile_KHitTile_##type __attribute__((unused)) = false;
#else // _CTRL_FPGA_KERNEL_FILE_
#define CTRL_KTILE_VARS(type)
#endif // !_CTRL_FPGA_KERNEL_FILE_

///@endcond
#endif // _CTRL_CORE_KHITTILE_H_
