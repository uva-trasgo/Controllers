#ifndef CAL_THREAD_H
#define CAL_THREAD_H

/**
 * @brief Almacena el identificador absoluto del hilo en cada dimensión. 
 */
typedef struct {
	int dims;
	int x;
	int y;
	int z;
} CALThread;

#define CALThreadInit(threads, ndims, params...) CALThread##ndims(threads, ndims, params)

#define CALThread1(threads, ndims, param1) CALThread2(threads, ndims, param1, 1)
#define CALThread2(threads, ndims, param1, param2) CALThread3(threads, ndims, param1, param2, 1)
#define CALThread3(threads, ndims, param1, param2, param3) threads.dims = ndims, threads.y = param1; threads.x = param2; threads.z = param3;

#endif
