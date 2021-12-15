#include "CAL_Cntrl.h"

#ifdef DEBUG
#include <stdio.h>
#endif

void CAL_CntrlCreate(CALCntrl* cntrl, CALCntrlType type, int device, int num_cpus){

#ifdef DEBUG
	printf("Controller created.\n");
	fflush(stdout);
#endif

	cntrl->type = type;
	cntrl->device = device;
	switch(type){
		case CAL_CNTRL_TYPE_CPU:
			CAL_CntrlCPUCreate(&cntrl->impl.cpu, num_cpus);
			break;
		case CAL_CNTRL_TYPE_XPHI:
			CAL_CntrlXPHICreate(&cntrl->impl.xphi, num_cpus);
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUCreate(&cntrl->impl.gpu, num_cpus);
			break;
	}
}

void CAL_CntrlDestroy(CALCntrl* cntrl){

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			CAL_CntrlCPUDestroy(&cntrl->impl.cpu);
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUDestroy(&cntrl->impl.gpu);
			break;
			break;
		case CAL_CNTRL_TYPE_XPHI:
			CAL_CntrlXPHIDestroy(&cntrl->impl.xphi);
			break;
	}
#ifdef DEBUG
	printf("Controller eliminated.\n");
	fflush(stdout);
#endif

}

//Yuri
void CAL_CntrlGlobalSync(CALCntrl* cntrl){

	switch(cntrl->type){
//		case CAL_CNTRL_TYPE_CPU:
//			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUGlobalSync(&cntrl->impl.gpu);
			break;
//			break;
//		case CAL_CNTRL_TYPE_XPHI:
//			break;
		default:
			break;
	}

#ifdef DEBUG
	printf("Waiting to Controller.\n");
	fflush(stdout);
#endif

}


void CAL_CntrlAttach(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Add Attach task.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			CAL_CntrlCPUAttach(&cntrl->impl.cpu, tile);
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUAttach(&cntrl->impl.gpu, tile);
			break;
		case CAL_CNTRL_TYPE_XPHI:
			CAL_CntrlXPHIAttach(&cntrl->impl.xphi, tile);
		//	attachingToXPHI(&cntrl->impl.xphi, tile);
			break;
	}

}

// TODO: created by ismael @ 2017-12-15 13:52:56
// Mask stream's use
//Yuri
void CAL_CntrlAttachAsyn(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Add Attach task.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			CAL_CntrlCPUAttach(&cntrl->impl.cpu, tile);
			break;
		case CAL_CNTRL_TYPE_GPU:
			// CAL_CntrlGPUAttachAsyn(&cntrl->impl.gpu, tile);
			// FIXME: noticed by ismael @ 2018-4-9 12:26:05
			// Change to async
			CAL_CntrlGPUAttach(&cntrl->impl.gpu, tile);
			break;
		case CAL_CNTRL_TYPE_XPHI:
			CAL_CntrlXPHIAttach(&cntrl->impl.xphi, tile);
		//	attachingToXPHI(&cntrl->impl.xphi, tile);
			break;
	}

}

//Yuri
void CAL_CntrlMoveTo(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Add MoveTo task.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			printf("Aun no implementado\n"); //TODO
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUMoveTo(&cntrl->impl.gpu, tile);
			break;
		case CAL_CNTRL_TYPE_XPHI:
			printf("Aun no implementado\n"); //TODO
			break;
	}

}

// TODO: created by ismael @ 2017-12-15 13:52:37
// Mask stream's use
//Yuri
void CAL_CntrlMoveToAsyn(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Add MoveTo task.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			printf("Aun no implementado\n"); //TODO
			break;
		case CAL_CNTRL_TYPE_GPU:
			// CAL_CntrlGPUMoveToAsyn(&cntrl->impl.gpu, tile);
			// FIXME: noticed by ismael @ 2018-4-9 12:25:37
			// Change to async
			CAL_CntrlGPUMoveTo(&cntrl->impl.gpu, tile);
			break;
		case CAL_CNTRL_TYPE_XPHI:
			printf("Aun no implementado\n"); //TODO
			break;
	}

}

void CAL_CntrlDetach(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Add Detach task.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			CAL_CntrlCPUDetach(&cntrl->impl.cpu, tile);
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUDetach(&cntrl->impl.gpu, tile); 
			break;
		case CAL_CNTRL_TYPE_XPHI:
			CAL_CntrlXPHIDetach(&cntrl->impl.xphi, tile);
			break;
	}
}

// TODO: created by ismael @ 2017-12-15 13:52:20
// Mask stream's use
//Yuri
// void CAL_CntrlDetachAsyn(CALCntrl* cntrl, HitTile* tile){

// #ifdef DEBUG
// 	printf("Add Detach task.\n");
// 	fflush(stdout);
// #endif

// 	switch(cntrl->type){
// 		case CAL_CNTRL_TYPE_CPU:
// 			CAL_CntrlCPUDetach(&cntrl->impl.cpu, tile);
// 			break;
// 		case CAL_CNTRL_TYPE_GPU:
// 			CAL_CntrlGPUDetachAsyn(&cntrl->impl.gpu, tile);
// 			break;
// 		case CAL_CNTRL_TYPE_XPHI:
// 			CAL_CntrlXPHIDetach(&cntrl->impl.xphi, tile);
// 			break;
// 	}
// }

//Yuri
void CAL_CntrlMoveFrom(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Add MoveFrom task.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			printf("Aun no implementado\n"); //TODO
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUMoveFrom(&cntrl->impl.gpu, tile);
			break;
		case CAL_CNTRL_TYPE_XPHI:
			printf("Aun no implementado\n"); //TODO
			break;
	}
}

// TODO: created by ismael @ 2017-12-15 13:53:25
// Mask stream's use
//Yuri
void CAL_CntrlMoveFromAsyn(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Add MoveFrom task.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			printf("Aun no implementado\n"); //TODO
			break;
		case CAL_CNTRL_TYPE_GPU:
			// CAL_CntrlGPUMoveFromAsyn(&cntrl->impl.gpu, tile);
			// FIXME: noticed by ismael @ 2018-4-9 12:26:35
			// Change to async
			CAL_CntrlGPUMoveFrom(&cntrl->impl.gpu, tile);
			break;
		case CAL_CNTRL_TYPE_XPHI:
			printf("Aun no implementado\n"); //TODO
			break;
	}
}

// TODO: created by ismael @ 2017-12-15 13:53:44
// Implement async version
void CAL_CntrlInternal(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Internal variable created\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			//TODO for CPUS
		 //	CAL_CntrlCPUInternal(&cntrl->impl.cpu, tile);
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUInternal(&cntrl->impl.gpu, tile);
			break;
	}

}

// TODO: created by ismael @ 2017-12-15 13:54:10
// Implement async version
void CAL_CntrlDestroyInternal(CALCntrl* cntrl, HitTile* tile, int recover){

#ifdef DEBUG
	printf("Internal variable eliminated.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			//TODO for CPUS
		//	CAL_CntrlCPUDestroyInternal(&cntrl->impl.cpu, tile);
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUDestroyInternal(&cntrl->impl.gpu, tile, recover);
			break;
	}

}

void CAL_CntrlWaitTile(CALCntrl* cntrl, HitTile* tile){

#ifdef DEBUG
	printf("Internal variable eliminated.\n");
	fflush(stdout);
#endif

	switch(cntrl->type){
		case CAL_CNTRL_TYPE_CPU:
			//TODO for CPUS
		//	CAL_CntrlCPUDestroyInternal(&cntrl->impl.cpu, tile);
			break;
		case CAL_CNTRL_TYPE_GPU:
			CAL_CntrlGPUWaitTile(&cntrl->impl.gpu, tile);
			break;
	}

}

/*
 * Function to choose the best implementation alternative
 * Masks: 4bits for Xhi implementations, 5 bits for GPU implementations, 
 * 4 bits for CPU implementations, and 1 bit for a Generic implementation
 */
int CAL_dev(CAL_Impl cntrl, int sumImpl){
        int result;

        if(cntrl == dGPU){
		int tmp = sumImpl & 0b10000111100000;
		int pos = tmp ^ (tmp&(tmp-1));
                result=pos;
        }
        else if(cntrl == CPU){
		int tmp = sumImpl & 0b10000000011110;
		int pos = tmp ^ (tmp&(tmp-1));
                result=pos;
        }
        else if(cntrl == XPhi){
		int tmp = sumImpl & 0b11111000000000;
		int pos = tmp ^ (tmp&(tmp-1));
                result=pos;
        }
	return result;
}

//Yuri
void  CAL_CntrlWaitStream(CALCntrl* cntrl, cudaStream_t stream){
		CAL_CntrlGPUWaitTile(&cntrl->impl.gpu, stream);	
}
