/**
 * @file test_gassimulation_types.h
 * @brief Type definitions for gas simulation EPSILOD example.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */
#ifndef _GASSIMULATION_TYPES_H_
#define _GASSIMULATION_TYPES_H_

#include <stdlib.h>
#include <stdint.h>

// #define GASSIMULATION_USE_FLOAT
#ifdef GASSIMULATION_USE_FLOAT
	#define GASSIMULATION_CELL_TYPE float
#else
	#define GASSIMULATION_CELL_TYPE double
#endif

// Revisar if __CUDACC__
#define POW(a, b) powf(a, b)

#define VEC3_SCALE(v_out, v, scale) \
	v_out.x = v.x * scale;          \
	v_out.y = v.y * scale;          \
	v_out.z = v.z * scale;

#define VEC3_ADD(v1, v2) \
	v1.x += v2.x;        \
	v1.y += v2.y;        \
	v1.z += v2.z;

#define VEC3_DOT(v1, v2) (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z)

// const int FLAG_OBSTACLE      = 1 << 0;
// const int FLAG_KEEP_VELOCITY = 1 << 1;
// #define FLAG_OBSTACLE      (1 << 0)
// #define FLAG_KEEP_VELOCITY (1 << 1)
#define FLAG_OBSTACLE      1
#define FLAG_KEEP_VELOCITY 2

#define Q 19

typedef struct {
	GASSIMULATION_CELL_TYPE data[Q];
} cell_t;

#define EPSILOD_TYPE_COMPOUND_cell_t GASSIMULATION_CELL_TYPE, Q

#define MANTISSA_SIZE 23
#define EXPONENT_SIZE 8
#define MAX_EXPONENT  255
typedef struct {
	unsigned int mantissa : MANTISSA_SIZE;
	unsigned int exponent : EXPONENT_SIZE;
	unsigned int sign : 1;
} floatparts;

typedef struct {
	GASSIMULATION_CELL_TYPE x, y, z;
} vec3f;

typedef struct {
	int x, y, z;
} vec3i;

#endif // _GASSIMULATION_TYPES_H_
