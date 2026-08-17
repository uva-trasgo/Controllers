#ifndef _CTRL_CORE_TILE_H_
#define _CTRL_CORE_TILE_H_
///@cond INTERNAL
/**
 * @file Ctrl_Tile.h
 * @brief Tile struct and memory states.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#ifdef _CTRL_ARCH_CPU_
#include "Architectures/Cpu/Ctrl_Cpu_Tile.h"
#endif // _CTRL_ARCH_CPU_

#ifdef _CTRL_ARCH_CUDA_
#include "Architectures/Cuda/Ctrl_Cuda_Tile.h"
#endif // _CTRL_ARCH_CUDA_

#ifdef _CTRL_ARCH_HIP_
#include "Architectures/Hip/Ctrl_Hip_Tile.h"
#endif // _CTRL_ARCH_HIP_

#ifdef _CTRL_ARCH_OPENCL_GPU_
#include "Architectures/OpenCL/Ctrl_OpenCL_Tile.h"
#endif // _CTRL_ARCH_OPENCL_GPU_

#ifdef _CTRL_ARCH_FPGA_
#include "Architectures/FPGA/Ctrl_FPGA_Tile.h"
#endif // _CTRL_ARCH_FPGA_

#include "Core/Ctrl_Type.h"

/**
 * @brief Abstract tile implementation.
 * Represents an attachment of a tile to a single Controller object.
 */
typedef struct Ctrl_Tile_Impl {
	Ctrl_Type type;                          /**< Type of the ctrl this tile corresponds to */
	int       device_status;                 /**< Status of this tile memory image on this device, can be CTRL_TILE_UNALLOC, CTRL_TILE_INVALID, CTRL_TILE_VALID */
	int       device_memowner;               /**< Ownership status of the memory of this tile on this device */
	HitInd    origAcumCard[HIT_MAXDIMS + 1]; /**< Original accumulated cardinalities of this tile on this device. Used to compute dimensional accesses */
	HitInd    offset;                        /**< Offset with respect to the original allocated memory in elements, needed for subselections on OpenCL and FPGA */
	union {
		#ifdef _CTRL_ARCH_CPU_
		Ctrl_Cpu_Tile *p_cpu;
		#endif // _CTRL_ARCH_CPU_

		#ifdef _CTRL_ARCH_CUDA_
		Ctrl_Cuda_Tile *p_cuda;
		#endif // _CTRL_ARCH_CUDA_

		#ifdef _CTRL_ARCH_HIP_
		Ctrl_Hip_Tile *p_hip;
		#endif // _CTRL_ARCH_HIP_

		#ifdef _CTRL_ARCH_OPENCL_GPU_
		Ctrl_OpenCL_Tile *p_opencl;
		#endif // _CTRL_ARCH_OPENCL_GPU_

		#ifdef _CTRL_ARCH_FPGA_
		Ctrl_FPGA_Tile *p_fpga;
		#endif // _CTRL_ARCH_FPGA_
	} tile; /**<Specific tile fields for the corresponding backend */
} Ctrl_Tile_Impl;

// TODO @sergioalo pinned should probably use a different enum type
/**
 * @brief Controller extension for Hitmap tiles.
 */
typedef struct Ctrl_Tile {
	Ctrl_GenericEvent last_host_read_event;  /**< Event to sincronyze tasks, represents last write operation on host*/
	Ctrl_GenericEvent last_host_write_event; /**< Event to sincronyze tasks, represents last read operation on host*/
	int               host_status;           /**< Information about the status of the host memory of this tile (unallocated, invalid or valid) */
	int               valid_impls;           /**< Number of valid entris in p_impls list */
	Ctrl_Type         pinned;                /**< Type of pinning of the host image, to know how to free it. CTRL_TYPE_NULL-> not pinned */
	void             *p_pin_queue;           /**< OpenCL queue used to map/unmap host image of this tile to try to make it pinned (only used when host mem is pinned via OpenCL) */
	void             *p_pinned_data;         /**< OpenCL buffer to map host image of this tile to try to make it pinned (only used when host mem is pinned via OpenCL) */
	Ctrl_Tile_Impl   *p_impls;               /**< list of arch specific tiles, of size n_ctrls */
} Ctrl_Tile;

/**
 * \brief List af all tiles associated to a ctrl
 */
typedef struct Ctrl_Tile_List {
	Ctrl_Tile             *p_tile_ext;
	struct Ctrl_Tile_List *p_prev;
	struct Ctrl_Tile_List *p_next;
} Ctrl_Tile_List;

// TODO this could now be a bool
#define CTRL_TILE_INVALID 0
#define CTRL_TILE_VALID   1

/**
 * Default fallback alignment for ctrl tiles, precedence order is: explicit, driver (eg. cudaMallocPitched), this.
 * In bytes.
 */
#define CTRL_DEFAULT_TILE_ALIGNMENT 128

/**
 * Adjust \p p_origAcumCards to cardinalities to transform on memory owner, as in hitmap's \e hit_tileUpdateAcumCards
 * but allowing padding in the last dimension
 *
 * @note 1D tiles ignore alignment.
 *
 * @param[out] p_origAcumCard Original accumulated cardinalities to update. Should have size \e HIT_MAX_DIMS+1
 * @param dims Dimensions of the tile. (0< \p dims <= \e HIT_MAX_DIMS)
 * @param p_card Cardinalities of the tile. Should have size \e HIT_MAX_DIMS
 * @param alignment Alignment for the last dimension. In elements. 0 or 1 mean no alignment.
 */
static inline void Ctrl_Tile_UpdateOrigAcumCards(HitInd *p_origAcumCard, int dims, HitInd *p_card, size_t alignment) {
	if (alignment == 0)
		alignment = 1;

	size_t pitch = ((p_card[dims - 1] + alignment - 1) / alignment) * alignment;

	// ignore alignment on 1D tiles
	if (dims == 1)
		pitch = p_card[0];

	p_origAcumCard[dims]     = 1;
	p_origAcumCard[dims - 1] = pitch;
	for (int i = dims - 2; i >= 0; i--) {
		p_origAcumCard[i] = p_origAcumCard[i + 1] * p_card[i];
	}
}

/**
 * Get the offset of a tile with respect to its parent on a device
 *
 * @param p_tile Tile to get the offset of
 * @param p_tile_data_impl Handle of \p p_tile on the device
 * @return The offset of \p p_tile device image with respect to its parent
 */
static inline HitInd Ctrl_Tile_ParentDeviceOffset(HitTile *p_tile, Ctrl_Tile_Impl *p_tile_data_impl) {
	// shape of p_tile in tile coordinates with respect to the parent
	HitShape tile_shape = hit_tileShapeArray2Tile(p_tile->ref, p_tile->shape);
	HitInd   offset     = 0;
	for (int i = 0; i < hit_tileDims(*p_tile); i++) {
		offset += hit_shapeSig(tile_shape, i).begin * p_tile_data_impl->origAcumCard[i + 1];
	}
	return offset;
}

///@endcond
#endif // _CTRL_CORE_TILE_H_
