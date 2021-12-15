/*
 * Definitions for kernel implementations, and masks to identify the implementations
 * that fit in a device/launching requirement
 *
 * @version 1.0
 * @author Arturo Gonzalez-Escribano
 * @date Oct 2016
 */

#ifndef _CAL_KImpl_
#define _CAL_KImpl_
typedef enum {
		CPU,
		libCPU,
		dGPU,
		dGPUCK,
		XPhi,
		libXPhi,
		Generic
} CAL_Impl;

#define CAL_IMPL_SpCPU2		2
#define CAL_IMPL_SpCPU		4	
#define CAL_IMPL_libCPU		8	
#define CAL_IMPL_CPU		16

#define CAL_IMPL_SpGPU2		32
#define CAL_IMPL_SpGPU		64	
#define CAL_IMPL_dGPUCK		128
#define CAL_IMPL_dGPU		256

#define CAL_IMPL_SpXphi2	512
#define CAL_IMPL_SpXPhi		1024	
#define CAL_IMPL_libXPhi	2048
#define CAL_IMPL_XPhi		4096

#define CAL_IMPL_Generic	8192

#endif // _CAL_KImpl_
