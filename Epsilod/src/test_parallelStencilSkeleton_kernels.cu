/**
 * @file test_parallelStencilSkeleton_kernels.cu
 * @brief Epsilod: Example with several key stencils. Kernels code.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "epsilod_types.h"
#include "Ctrl.h"

Ctrl_NewType(float);

// 1D non-compact radius 2
CTRL_KERNEL(updateCell_1dNC4, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, const EpsilodCoords global_coords, const KHitTile_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i) = (0.5 * (hit(matrixCopy, thr_i - 2) + hit(matrixCopy, thr_i + 2)) +
						  hit(matrixCopy, thr_i - 1) + hit(matrixCopy, thr_i + 1)) /
						 3;
});

// 1D compact radius 1
CTRL_KERNEL(updateCell_1dC2, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, const EpsilodCoords global_coords, const KHitTile_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i) = (hit(matrixCopy, thr_i - 1) + hit(matrixCopy, thr_i + 1)) / 2;
});

// 2D compact, radius 1: 4-point star, no corners
CTRL_KERNEL(updateCell_2d4, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, const EpsilodCoords global_coords, const KHitTile_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) = (hit(matrixCopy, thr_i - 1, thr_j) +
								 hit(matrixCopy, thr_i + 1, thr_j) +
								 hit(matrixCopy, thr_i, thr_j - 1) +
								 hit(matrixCopy, thr_i, thr_j + 1)) /
								4;
});

// 2D compact, radius 1: 8-point star, corners included
CTRL_KERNEL(updateCell_2d8, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, const EpsilodCoords global_coords, const KHitTile_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) = (4 * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i + 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1) + hit(matrixCopy, thr_i, thr_j + 1)) +
								 (hit(matrixCopy, thr_i - 1, thr_j - 1) + hit(matrixCopy, thr_i + 1, thr_j - 1) + hit(matrixCopy, thr_i - 1, thr_j + 1) + hit(matrixCopy, thr_i + 1, thr_j + 1))) /
								20;
});

// 2D non-compact, radius 2: 8-point star, no corners
CTRL_KERNEL(updateCell_2dNC8, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, const EpsilodCoords global_coords, const KHitTile_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) = ((hit(matrixCopy, thr_i - 2, thr_j) + hit(matrixCopy, thr_i + 2, thr_j) + hit(matrixCopy, thr_i, thr_j - 2) + hit(matrixCopy, thr_i, thr_j + 2)) +
								 4 * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i + 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1) + hit(matrixCopy, thr_i, thr_j + 1))) /
								20;
});

// 2D non-compact, non-symmetric. radius 2: 5-point star forward-down with one corner element
CTRL_KERNEL(updateCell_2dF5, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, const EpsilodCoords global_coords, const KHitTile_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j) = (2.0f * (hit(matrixCopy, thr_i - 1, thr_j) + hit(matrixCopy, thr_i, thr_j - 1)) +
								 (hit(matrixCopy, thr_i - 2, thr_j) + hit(matrixCopy, thr_i, thr_j - 2)) +
								 .5f * hit(matrixCopy, thr_i - 1, thr_j - 1)) /
								6.5f;
});

// 3D compact. radius 1: 27-point star
CTRL_KERNEL(updateCell_3d27, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, const EpsilodCoords global_coords, const KHitTile_float stencil, const float factor, const Epsilod_ext ext_params, {
	hit(matrix, thr_i, thr_j, thr_k) = (hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k - 1) +
										hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k) +
										hit(matrixCopy, thr_i - 1, thr_j - 1, thr_k + 1) +
										hit(matrixCopy, thr_i - 1, thr_j, thr_k - 1) +
										hit(matrixCopy, thr_i - 1, thr_j, thr_k) +
										hit(matrixCopy, thr_i - 1, thr_j, thr_k + 1) +
										hit(matrixCopy, thr_i - 1, thr_j + 1, thr_k - 1) +
										hit(matrixCopy, thr_i - 1, thr_j + 1, thr_k) +
										hit(matrixCopy, thr_i - 1, thr_j + 1, thr_k + 1) +
										hit(matrixCopy, thr_i, thr_j - 1, thr_k - 1) +
										hit(matrixCopy, thr_i, thr_j - 1, thr_k) +
										hit(matrixCopy, thr_i, thr_j - 1, thr_k + 1) +
										hit(matrixCopy, thr_i, thr_j, thr_k - 1) +
										hit(matrixCopy, thr_i, thr_j, thr_k) +
										hit(matrixCopy, thr_i, thr_j, thr_k + 1) +
										hit(matrixCopy, thr_i, thr_j + 1, thr_k - 1) +
										hit(matrixCopy, thr_i, thr_j + 1, thr_k) +
										hit(matrixCopy, thr_i, thr_j + 1, thr_k + 1) +
										hit(matrixCopy, thr_i + 1, thr_j - 1, thr_k - 1) +
										hit(matrixCopy, thr_i + 1, thr_j - 1, thr_k) +
										hit(matrixCopy, thr_i + 1, thr_j - 1, thr_k + 1) +
										hit(matrixCopy, thr_i + 1, thr_j, thr_k - 1) +
										hit(matrixCopy, thr_i + 1, thr_j, thr_k) +
										hit(matrixCopy, thr_i + 1, thr_j, thr_k + 1) +
										hit(matrixCopy, thr_i + 1, thr_j + 1, thr_k - 1) +
										hit(matrixCopy, thr_i + 1, thr_j + 1, thr_k) +
										hit(matrixCopy, thr_i + 1, thr_j + 1, thr_k + 1)) /
									   27;
});

// 4D compact. radius 1: 8-point star
CTRL_KERNEL(updateCell_4d8, GENERIC, DEFAULT, KHitTile_float matrix, const KHitTile_float matrixCopy, const EpsilodCoords global_coords, const KHitTile_float stencil, const float factor, const Epsilod_ext ext_params, {
	for (int l = 0; l < hit_tileDimCard(matrix, 3); l++)
		hit(matrix, thr_i, thr_j, thr_k, l) =
			(hit(matrixCopy, thr_i - 1, thr_j, thr_k, l) +
			 hit(matrixCopy, thr_i + 1, thr_j, thr_k, l) +
			 hit(matrixCopy, thr_i, thr_j - 1, thr_k, l) +
			 hit(matrixCopy, thr_i, thr_j + 1, thr_k, l) +
			 hit(matrixCopy, thr_i, thr_j, thr_k - 1, l) +
			 hit(matrixCopy, thr_i, thr_j, thr_k + 1, l) +
			 hit(matrixCopy, thr_i, thr_j, thr_k, l - 1) +
			 hit(matrixCopy, thr_i, thr_j, thr_k, l + 1)) /
			8;
});
