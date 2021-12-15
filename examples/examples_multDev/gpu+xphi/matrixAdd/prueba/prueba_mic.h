// This is the REAL "hello world" for CUDA!
// It takes the string "Hello ", prints it, then passes it to CUDA with an array
// of offsets. Then the offsets are added in parallel to produce the string "World!"
// By Ingemar Ragnemalm 2010
#include <stdio.h>
#include <unistd.h>
#include <hitmap.h>
//#include <CAL.h>
#include "CAL_Cntrl.h"

hit_tileNewType( float );

//#include "CAL_KernelXPHI.h"

/*
void  __attribute__((target(mic))) name_mic(){
 #pragma offload target(mic:0) mandatory
{
        char michostname[100];
        gethostname(michostname, sizeof(michostname));
        printf("MIC: Hello world from MIC. I am %s and I have %ld logical cores. I was called from host: CHIMERA \n", michostname, sysconf(_SC_NPROCESSORS_ONLN));
 }
}
*/

void name_mic();

/*
int main()
{



  name_mic();

 printf("%s\n", a);
// return EXIT_SUCCESS;
}
*/
