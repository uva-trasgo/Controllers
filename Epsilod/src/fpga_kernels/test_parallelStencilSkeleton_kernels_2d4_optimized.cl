
/**
 * @file test_parallelStencilSkeleton_kernels_2d4_optimized.cl
 * @brief Epsilod: Example with several key stencils. Optimized FPGA 2d4 kernel code.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

// TODO: Stencils that read corners are not processed correctly right now.
// The shifting and read pattern of the shift register must be adjusted (at least 2 points in the code).
// (SR_SIZE += 2 * RAD; each item always reads its last element)

// #define DEBUG
// #define PRINT_MATRIX

#include "../epsilod_ext_type.h"
#include "test_parallelStencilSkeleton_kernels_FPGA_optimized_common.h"

#include "Ctrl_FPGA_Kernel.h"

Ctrl_NewType(float);

/* Constant sizes for stencil kernels */

#define RAD 1 // Stencil radius

#define HALO_BLOCK_WIDTH DEFAULT_BLOCK_WIDTH          // With halos
#define BLOCK_WIDTH      (HALO_BLOCK_WIDTH - 2 * RAD) // Without halos

// HALO_BLOCK_WIDTH must be multiple of VEC_SIZE
#ifndef VEC_SIZE
#define VEC_SIZE DEFAULT_VEC_SIZE
#endif // VEC_SIZE

// y_size must be multiple of CU_PARALLELISM
#ifndef CU_PARALLELISM
#define CU_PARALLELISM DEFAULT_CU_PARALLELISM
#endif // CU_PARALLELISM

#if HALO_BLOCK_WIDTH % VEC_SIZE != 0
#error HALO_BLOCK_WIDTH is not multiple of VEC_SIZE. This would cause incorrect hardware with out-of-bounds memory accesses.
#endif // HALO_BLOCK_WIDTH % VEC_SIZE != 0

#define SR_BASE (2 * RAD * HALO_BLOCK_WIDTH) // Elements whose input is a posterior shift register element
// SR dims:      yyyyyyy   xxxxxxxxxxxxxxxx  (does not count last row)
#define SR_LAST_ROW_SIZE (VEC_SIZE)                   // Elements whose input is a global memory datum
#define SR_SIZE          (SR_BASE + SR_LAST_ROW_SIZE) // Shift register size

#if SR_SIZE % VEC_SIZE != 0
#error SR_SIZE is not multiple of VEC_SIZE. This would cause incorrect hardware with out-of-bounds memory accesses.
#endif // SR_SIZE % VEC_SIZE != 0

/* B.1. 2D COMPACT, RADIUS 1: 4-POINT STAR, NO CORNERS */
//__CTRL_FPGA_KERNEL_REPLICATE(CU_PARALLELISM)
CTRL_KERNEL_FN(updateCell_4, FPGA, TASK,
			   KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(Epsilod_ext, ext_params)) {
	int x_size = hit_tileDimCard(matrix, 1); // Elements computed in x dimension
	int y_size = hit_tileDimCard(matrix, 0); // Elements computed in y dimension

	// Number of spatial blocks that will be computed:
	int blocks = (x_size + BLOCK_WIDTH - 1) / BLOCK_WIDTH;

	// Shift register:
	// Stores a partial matrix to reuse data for neighbor accesses
	float shift_register[SR_SIZE];

	// Pre-initialize the shift-register with zeroes:
	#pragma unroll
	for (int idx = 0; idx < SR_SIZE; idx++)
		#ifndef DEBUG
		shift_register[idx] = 0.0f;
	#else // DEBUG
	shift_register[idx] = -1.0f;
	#endif // DEBUG

	// Compute every spatial block:
	int block_shift = HALO_BLOCK_WIDTH * (2 * RAD + y_size); // Amount of shifts to perform per block
	// SR_SIZE to initialize shift register
	// HALO_BLOCK_WIDTH * y size to process all of the elements of the block
	// -RAD because west halo is not read on the first line of each block (when initializing the shift register)
	int total_to_shift = blocks * block_shift; // Total amount of shifts to perform
	int shift_idx_base = 0;                    // Index of current shift

	// Indeces:
	int x  = -RAD;     // x in the block; range: [-RAD, BLOCK_WIDTH + RAD)
	int gx = -RAD;     // x in the global matrix
	int gy = -2 * RAD; // y in the global matrix (offset to initialize the shift register each block)
					   // range: [-2 * RAD, y_size)
					   //#pragma ivdep safelen(HALO_BLOCK_WIDTH)
	while (shift_idx_base < total_to_shift) {
		// Shift the shift register:
		#pragma unroll
		for (int i = 0; i < SR_BASE; i++)
			shift_register[i] = shift_register[i + VEC_SIZE];

		// Read input data from global matrix (vectorized, VEC_SIZE width):
		#pragma unroll
		for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
			int vgx_to_read = gx + vec_idx;
			int vgy_to_read = gy + RAD; // The element read is exactly the one south of the current element

			#ifndef DEBUG
			shift_register[SR_BASE + vec_idx] = hit(matrixCopy, vgy_to_read, vgx_to_read);
			// Out of bounds accesses on last block will not be processed later
			#else // DEBUG
			if (vgx_to_read < x_size + RAD)
				shift_register[SR_BASE + vec_idx] = hit(matrixCopy, vgy_to_read, vgx_to_read);
			else
				shift_register[SR_BASE + vec_idx] = -1.0f;
			#endif // DEBUG
		}

		// Compute the stencil (vectorized, VEC_SIZE width):
		#pragma unroll
		for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
			int vx  = x + vec_idx;
			int vgx = gx + vec_idx;

			bool shift_register_initialized = gy >= 0;

			// Compute and store stencil result of inner part (skip halos and out of bounds data):
			// (Out of bounds data can only happen in the last, non-full, block,
			// as HALO_BLOCK_WIDTH is multiple of VEC_SIZE)
			bool in_bounds = vx >= 0 && vx < BLOCK_WIDTH && vgx < x_size;
			int  sr_center = vec_idx + HALO_BLOCK_WIDTH;
			// Shift register must be fully shifted here
			if (shift_register_initialized && in_bounds) {
				hit(matrix, gy, vgx) = (shift_register[sr_center - 1] +
										shift_register[sr_center + 1] +
										shift_register[sr_center - HALO_BLOCK_WIDTH] +
										shift_register[sr_center + HALO_BLOCK_WIDTH]) /
									   4;
			}
		}

		// Advance indeces:
		shift_idx_base += VEC_SIZE;

		gx += VEC_SIZE;
		x = ((x + VEC_SIZE + RAD) & (HALO_BLOCK_WIDTH - 1)) - RAD;
		if (x == -RAD) { // End of row, move to next line
			gy++;
			if (gy == y_size) { // End of block, move to next block
				gy = -2 * RAD;  // Offset to initialize the shift register
				gx += BLOCK_WIDTH;
			}
			// Reset global x coord:
			gx -= HALO_BLOCK_WIDTH;
		}
	}

	#ifdef PRINT_MATRIX
	if (y_size > 3 * RAD)
		for (int y = -2 * RAD; y < y_size + 2 * RAD; y++) {
			for (int x = -2 * RAD; x < x_size + 2 * RAD; x++)
				printf("%g ", hit(matrix, y, x));
			printf("\n");
		}
	printf("\n");
	#endif // PRINT_MATRIX

	CTRL_KERNEL_END();
}

/* Constant sizes for vertical border kernel */
// Basically, the shift register now is smaller/narrower, as the vertical borders
// are very narrow (radius size + halos). Using the inner's kernel's size is a
// waste of cycles and resources.

#define BLOCK_WIDTH_VBORDER      (RAD)
#define HALO_BLOCK_WIDTH_VBORDER (BLOCK_WIDTH_VBORDER + 2 * RAD) // Account for overlapping elements in different blocks

#undef VEC_SIZE
#define VEC_SIZE 1

#define SR_BASE_VBORDER (2 * RAD * HALO_BLOCK_WIDTH_VBORDER) // Elements whose input is a posterior shift register element
#define SR_SIZE_VBORDER (SR_BASE_VBORDER + SR_LAST_ROW_SIZE) // Shift register size

// Reuse previous names:
#undef HALO_BLOCK_WIDTH
#define HALO_BLOCK_WIDTH HALO_BLOCK_WIDTH_VBORDER

#undef BLOCK_WIDTH
#define BLOCK_WIDTH BLOCK_WIDTH_VBORDER

#undef SR_BASE
#define SR_BASE SR_BASE_VBORDER

#undef SR_SIZE
#define SR_SIZE SR_SIZE_VBORDER

//__CTRL_FPGA_KERNEL_REPLICATE(CU_PARALLELISM)
CTRL_KERNEL_FN(updateCell_4_verticalBorder, FPGA, TASK,
			   KHitTileR_arg(float, matrix), KHitTileR_arg(float, matrixCopy),
			   K_arg(Epsilod_ext, ext_params)) {
	// We know some of these sizes at compile time:
	int x_size = BLOCK_WIDTH;                // Elements computed in x dimension
	int y_size = hit_tileDimCard(matrix, 0); // Elements computed in y dimension

	// This kernel assumes only one block will be computed
	// Stores a partial matrix to reuse data for neighbor accesses
	float shift_register[SR_SIZE];

	// Pre-initialize the shift register with zeroes:
	#pragma unroll
	for (int idx = 0; idx < SR_SIZE; idx++)
		#ifndef DEBUG
		shift_register[idx] = 0.0f;
	#else // DEBUG
	shift_register[idx] = -1.0f;
	#endif // DEBUG

	// Process (only one block):
	int to_shift       = SR_SIZE + HALO_BLOCK_WIDTH * y_size;
	int shift_idx_base = 0;

	// Indeces:
	int x  = -RAD;     // x in the block; range: [-RAD, BLOCK_WIDTH + RAD)
	int gy = -2 * RAD; // y in the global matrix (offset to initialize the shift register each block)
					   // range: [-2 * RAD, y_size)
					   //#pragma ivdep safelen(HALO_BLOCK_WIDTH)
	while (shift_idx_base < to_shift) {
		int gx = x; // x in the global matrix

		// Shift the shift register:
		#pragma unroll
		for (int i = 0; i < SR_BASE; i++)
			shift_register[i] = shift_register[i + VEC_SIZE];

		// Read input data from global matrix (vectorized, VEC_SIZE width):
		#pragma unroll
		for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
			int vgx_to_read = gx + vec_idx;
			int vgy_to_read = gy + RAD; // The element read is exactly the one south of the current element

			#ifndef DEBUG
			shift_register[SR_BASE + vec_idx] = hit(matrixCopy, vgy_to_read, vgx_to_read);
			// Out of bounds accesses on last block will not be processed later
			#else // DEBUG
			if (vgx_to_read < x_size + RAD)
				shift_register[SR_BASE + vec_idx] = hit(matrixCopy, vgy_to_read, vgx_to_read);
			else
				shift_register[SR_BASE + vec_idx] = -1.0f;
			#endif // DEBUG
		}

		// Compute the stencil (vectorized, VEC_SIZE width):
		#pragma unroll
		for (int vec_idx = 0; vec_idx < VEC_SIZE; vec_idx++) {
			int vx  = x + vec_idx;
			int vgx = gx + vec_idx;

			bool shift_register_initialized = gy >= 0;

			// Compute and store stencil result of inner part (skip halos and out of bounds data):
			// (Out of bounds data can only happen in the last, non-full, block,
			// as HALO_BLOCK_WIDTH is multiple of VEC_SIZE)
			bool in_bounds = vx >= 0 && vx < BLOCK_WIDTH && gy < y_size;
			int  sr_center = vec_idx + HALO_BLOCK_WIDTH;
			// Shift register must be fully shifted here
			if (shift_register_initialized && in_bounds)
				hit(matrix, gy, vgx) = (shift_register[sr_center - 1] +
										shift_register[sr_center + 1] +
										shift_register[sr_center - HALO_BLOCK_WIDTH] +
										shift_register[sr_center + HALO_BLOCK_WIDTH]) /
									   4;
		}

		// Advance indeces:
		shift_idx_base += VEC_SIZE;

		x += VEC_SIZE;
		if (x + RAD >= HALO_BLOCK_WIDTH) { // End of row, move to next line
			gy++;
			x = -RAD;
		}
	}

	CTRL_KERNEL_END();
}
