#ifndef _EPSILOD_KERNELS_H_
#define _EPSILOD_KERNELS_H_
/**
 * @file epsilod_kernels.h
 * @brief Epsilod: Decalaration chain for epsilod types for kernels
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include EPSILOD_TYPES_INCLUDE
#include <epsilod_types.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Ctrl.h"

#if !EPSILOD_IS_FLOAT(EPSILOD_BASE_TYPE)
Ctrl_NewType(float);
#endif
Ctrl_NewType(EPSILOD_BASE_TYPE);

#endif // _EPSILOD_KERNELS_H_
