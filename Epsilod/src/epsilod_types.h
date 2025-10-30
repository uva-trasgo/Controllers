#ifndef _EPSILOD_TYPES_H_
#define _EPSILOD_TYPES_H_
/**
 * @file epsilod_types.h
 * @brief Default type declarations for epsilod
 * 	- Default stencil base type: float
 * 	- Default external/extra parametres: int
 * 	- Other internal or interface types
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

/* Default stencil base type */
#ifndef EPSILOD_BASE_TYPE
#define EPSILOD_BASE_TYPE float
#endif

/* Definitions to declare compound types */
#define EPSILOD_TYPE_COMPOUND(type)       EPSILOD_TYPE_COMPOUND2(type)
#define EPSILOD_TYPE_COMPOUND2(type)      EPSILOD_TYPE_COMPOUND_##type
#define EPSILOD_BASE_TYPE_COMPOUND        EPSILOD_TYPE_COMPOUND(EPSILOD_BASE_TYPE)
#define EPSILOD_GET_COMPOUND_TYPE(...)    EPSILOD_GET_COMPOUND_TYPE2(__VA_ARGS__)
#define EPSILOD_GET_COMPOUND_TYPE2(a, b)  a
#define EPSILOD_GET_COMPOUND_COUNT(...)   EPSILOD_GET_COMPOUND_COUNT2(__VA_ARGS__)
#define EPSILOD_GET_COMPOUND_COUNT2(a, b) b

/* Template like types for Tiles and KTiles */
#define HitTile(type)   HitTile2(type)
#define HitTile2(type)  HitTile_##type
#define KHitTile(type)  KHitTile2(type)
#define KHitTile2(type) KHitTile_##type

/* For conditional compilation based on the value of EPSILOD_BASE_TYPE, float or any other one */
#define EPSILOD_BASE_TYPE_float 1
#define EPSILOD_IS_FLOAT(type)  EPSILOD_IS_FLOAT2(type)
#define EPSILOD_IS_FLOAT2(type) EPSILOD_BASE_TYPE_##type

/* Default extra parameters type */
#ifndef EPSILOD_USER_TYPES
#define EPSILOD_USER_TYPES \
	typedef struct {       \
		int foo;           \
	} Epsilod_ext;
#endif

/* Other types used in user declared functions */
#define EPSILOD_MAX_DIMS 4
#ifndef CTRL_USER_TYPES
#define CTRL_USER_TYPES                          \
	EPSILOD_USER_TYPES                           \
	typedef struct {                             \
		int low[EPSILOD_MAX_DIMS];               \
		int high[EPSILOD_MAX_DIMS];              \
	} EpsilodBorders;                            \
	typedef struct {                             \
		int            dims;                     \
		HitInd         size[EPSILOD_MAX_DIMS];   \
		HitInd         offset[EPSILOD_MAX_DIMS]; \
		EpsilodBorders borders;                  \
	} EpsilodCoords;
#endif

#endif // _EPSILOD_TYPES_H_
