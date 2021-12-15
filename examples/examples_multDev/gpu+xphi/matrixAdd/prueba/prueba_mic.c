#include "prueba_mic.h"
//#include "CAL_KernelXPHI.h"


void  __attribute__((target(mic))) name_mic(){

 #pragma offload target(mic:0) mandatory
{
	HitTile_float F;

        char michostname[100];
        gethostname(michostname, sizeof(michostname));
        printf("MIC: Hello world from MIC. I am %s and I have %ld logical cores. I was called from host: CHIMERA \n", michostname, sysconf(_SC_NPROCESSORS_ONLN));
 }
}

void wrapper_xphi_MatSum(int ind, void** args ){}

void a(){
 CALCntrlXPHI *comm_x;
 CALCntrlCPU *comm;
 CALThread thread; 
CALCntrl a;
//CAL_CntrlXPHICreate(comm,4);
CAL_CntrlCreate(&a, CAL_CNTRL_XPHI, 228);
CAL_CntrlDestroy(&a);

 HitTile_float C;
 hit_tileDomain( &C, float, 2, 10, 10 );

 CAL_CntrlXPHIAddTask(comm_x, wrapper_xphi_MatSum,thread, 0 );
//CAL_CntrlXPHIDestroy(comm_x);
 CAL_CntrlCPUAddTask(comm, wrapper_xphi_MatSum,thread, 0 );

}
/*
void __attribute__((target(mic)))  name_mic(){

printf("HHHHHHHHHHH\n");
}
*/
