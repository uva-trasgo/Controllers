#ifndef CAL_KERNELGPUCHAR_H
#define CAL_KERNELGPUCHAR_H

#define CAL_NARCHS 4 // Numero de arquitecturas que existen
/* Arquitecturas soportadas por la biblioteca */
#define CAL_ARCH_FERMI 2
#define CAL_ARCH_KEPLER 3

/* 
 * Hilos por bloque para cada una de los tipos de kernels.
 * Cada uno del los valores corresponde con una arquitectura.
 */
#if defined(__cplusplus)
	#define CAL_GPU_kchar_1defdefdef { dim3(0), dim3(0), dim3(256), dim3(256) }
	#define CAL_GPU_kchar_2defdefdef { dim3(0,0), dim3(0,0), dim3(256,1), dim3(256,1) }

	#define CAL_GPU_kchar_1mediumlowlow { dim3(128), dim3(128), dim3(128), dim3(128) }
	#define CAL_GPU_kchar_1fulllowlow { dim3(256), dim3(256), dim3(256), dim3(256) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(16,16), dim3(16,16), dim3(16,16), dim3(16,16) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(128,2), dim3(128,2), dim3(128,2), dim3(128,2) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(2,128), dim3(2,128), dim3(2,128), dim3(2,128) }
	#define CAL_GPU_kchar_2fulllowlow { dim3(128,2), dim3(128,2), dim3(128,2), dim3(128,2) }
	#define CAL_GPU_kchar_2square32lowlow { dim3(32,32), dim3(32,32), dim3(32,32), dim3(32,32) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(3,7), dim3(3,7), dim3(3,7), dim3(3,7) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(4,64), dim3(4,64), dim3(4,64), dim3(4,64) }

	//#define CAL_GPU_kchar_2fullmediummedium { dim3(16,16), dim3(16,16), dim3(16,16), dim3(16,16) }
	#define CAL_GPU_kchar_2fullmediummedium { dim3(128,2), dim3(128,2), dim3(128,2), dim3(128,2) }

	#define CAL_GPU_kchar_2fulllowhigh { dim3(0,0), dim3(0,0), dim3(64,3), dim3(32,4) }


	//#define CAL_GPU_kchar_2mediummediummedium { dim3(0,0), dim3(0,0), dim3(64,1), dim3(32,4) }
	//#define CAL_GPU_kchar_2mediummediummedium { dim3(3,7), dim3(3,7), dim3(3,7), dim3(3,7) }
	//#define CAL_GPU_kchar_2mediummediummedium { dim3(4,32), dim3(4,32), dim3(4,32), dim3(4,32) }
	//#define CAL_GPU_kchar_2mediummediummedium { dim3(4,32), dim3(4,32), dim3(2,128), dim3(2,128) }
	#define CAL_GPU_kchar_2mediummediummedium { dim3(32,4), dim3(32,4), dim3(128,2), dim3(128,2) }

	#define CAL_GPU_kchar_2fixedsquare32 { dim3(16,16), dim3(16,16), dim3(32,32), dim3(32,32) }
	#define CAL_GPU_kchar_2fixedsquare16 { dim3(16,16), dim3(16,16), dim3(16,16), dim3(16,16) }
	#define CAL_GPU_kchar_2fixedsquare4 { dim3(4,4), dim3(4,4), dim3(4,4), dim3(4,4) }
	#define CAL_GPU_kchar_2fixedsquare2 { dim3(2,2), dim3(2,2), dim3(2,2), dim3(2,2) }
#else
		
	/*#define CAL_GPU_kchar_1defdefdef { dim3(0), dim3(0), dim3(256), dim3(256) }
	#define CAL_GPU_kchar_2defdefdef { dim3(0,0), dim3(0,0), dim3(256,1), dim3(256,1) }

	#define CAL_GPU_kchar_1mediumlowlow { dim3(128), dim3(128), dim3(128), dim3(128) }
	#define CAL_GPU_kchar_1fulllowlow { dim3(256), dim3(256), dim3(256), dim3(256) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(16,16), dim3(16,16), dim3(16,16), dim3(16,16) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(128,2), dim3(128,2), dim3(128,2), dim3(128,2) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(2,128), dim3(2,128), dim3(2,128), dim3(2,128) }
	#define CAL_GPU_kchar_2fulllowlow { dim3(128,2), dim3(128,2), dim3(128,2), dim3(128,2) }
	#define CAL_GPU_kchar_2square32lowlow { dim3(32,32), dim3(32,32), dim3(32,32), dim3(32,32) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(3,7), dim3(3,7), dim3(3,7), dim3(3,7) }
	//#define CAL_GPU_kchar_2fulllowlow { dim3(4,64), dim3(4,64), dim3(4,64), dim3(4,64) }

	//#define CAL_GPU_kchar_2fullmediummedium { dim3(16,16), dim3(16,16), dim3(16,16), dim3(16,16) }
	#define CAL_GPU_kchar_2fullmediummedium { dim3(128,2), dim3(128,2), dim3(128,2), dim3(128,2) }

	#define CAL_GPU_kchar_2fulllowhigh { dim3(0,0), dim3(0,0), dim3(64,3), dim3(32,4) }


	//#define CAL_GPU_kchar_2mediummediummedium { dim3(0,0), dim3(0,0), dim3(64,1), dim3(32,4) }
	//#define CAL_GPU_kchar_2mediummediummedium { dim3(3,7), dim3(3,7), dim3(3,7), dim3(3,7) }
	//#define CAL_GPU_kchar_2mediummediummedium { dim3(4,32), dim3(4,32), dim3(4,32), dim3(4,32) }
	//#define CAL_GPU_kchar_2mediummediummedium { dim3(4,32), dim3(4,32), dim3(2,128), dim3(2,128) }
	#define CAL_GPU_kchar_2mediummediummedium { dim3(32,4), dim3(32,4), dim3(128,2), dim3(128,2) }

	#define CAL_GPU_kchar_2fixedsquare32 { dim3(16,16), dim3(16,16), dim3(32,32), dim3(32,32) }
	#define CAL_GPU_kchar_2fixedsquare16 { dim3(16,16), dim3(16,16), dim3(16,16), dim3(16,16) }
	#define CAL_GPU_kchar_2fixedsquare4 { dim3(4,4), dim3(4,4), dim3(4,4), dim3(4,4) }
	#define CAL_GPU_kchar_2fixedsquare2 { dim3(2,2), dim3(2,2), dim3(2,2), dim3(2,2) }*/
#endif /* __cplusplus */

#endif
