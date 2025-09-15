/**
 * @file Jacobi_Multidim_Ctrl_Kernels.c
 * @brief Multidimensional Jacobi: Ctrl version kernels.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Ctrl.h"
#include "Jacobi_Multidim_Ctrl.h"

Ctrl_NewType(double);

CTRL_KERNEL(Jacobi_1D, GENERIC, DEFAULT, CTRL_KPARAMS(jacobi_params), {
	int i = thr_i + 1;

	hit(tileMat, i) = (hit(tileCopy, i - 1) + hit(tileCopy, i + 1)) / 2;
});

CTRL_KERNEL(Jacobi_2D, GENERIC, DEFAULT, CTRL_KPARAMS(jacobi_params), {
	int i = thr_i + 1;
	int j = thr_j + 1;

	hit(tileMat, i, j) =
		(hit(tileCopy, i - 1, j) +
		 hit(tileCopy, i + 1, j) +
		 hit(tileCopy, i, j - 1) +
		 hit(tileCopy, i, j + 1)) /
		4;
});

CTRL_KERNEL(Jacobi_3D, GENERIC, DEFAULT, CTRL_KPARAMS(jacobi_params), {
	int i = thr_i + 1;
	int j = thr_j + 1;
	int k = thr_k + 1;

	hit(tileMat, i, j, k) =
		(hit(tileCopy, i - 1, j, k) +
		 hit(tileCopy, i + 1, j, k) +
		 hit(tileCopy, i, j - 1, k) +
		 hit(tileCopy, i, j + 1, k) +
		 hit(tileCopy, i, j, k - 1) +
		 hit(tileCopy, i, j, k + 1)) /
		6;
});

CTRL_KERNEL(Jacobi_4D, GENERIC, DEFAULT, CTRL_KPARAMS(jacobi_params), {
	int i = thr_i + 1;
	int j = thr_j + 1;
	int k = thr_k + 1;

	for (int l = 1; l < hit_tileDimCard(tileMat, 3) - 1; l++)
		hit(tileMat, i, j, k, l) =
			(hit(tileCopy, i - 1, j, k, l) +
			 hit(tileCopy, i + 1, j, k, l) +
			 hit(tileCopy, i, j - 1, k, l) +
			 hit(tileCopy, i, j + 1, k, l) +
			 hit(tileCopy, i, j, k - 1, l) +
			 hit(tileCopy, i, j, k + 1, l) +
			 hit(tileCopy, i, j, k, l - 1) +
			 hit(tileCopy, i, j, k, l + 1)) /
			8;
});