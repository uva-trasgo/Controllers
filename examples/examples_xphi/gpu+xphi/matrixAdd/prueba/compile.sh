#!/bin/sh

rm exec libpru.so 

##Compilar libreria dinamica
#g++ -c -fPIC -shared prueba_mic.c -o prueba_mic.o
#ld -o libprueba.so prueba_mic.o -shared
#nvcc prueba.cu -o sal -L. -lprueba

##Compilar libreria estatica
#icpc -c  prueba_mic.c -o prueba_micS.o
#ar -rv libpruebaS.a prueba_micS.o
#xiar –qoffload-build ar libpruebaS.a prueba_micS.o
#nvcc prueba.cu -o salS -L. -lpruebaS

LIB=/frontend/ana/28-10-10_controladores/v2/HitControllers/build/cal

#Funciona 
icpc -c  -I/usr/local/cuda-8.0/include -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI -I/home/eduardo/newcontrollers/HitControllers/Hitmap1.2/include -I/opt/mpich3.gcc/include -I/home/eduardo/newcontrollers/HitControllers/cal/inc -I/home/eduardo/newcontrollers/HitControllers/calXPHI/inc prueba_mic.c 

nvcc -c prueba.cu  -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI -I/home/eduardo/newcontrollers/HitControllers/Hitmap1.2/include -I/opt/mpich3.gcc/include  -I/home/eduardo/newcontrollers/HitControllers/cal/inc -I/home/eduardo/newcontrollers/HitControllers/calXPHI/inc 

#ld -shared -o libpru.so ${LIB}XPHI/CMakeFiles/calXPHI.dir/src/CAL_CntrlXPHI.c.o $LIB/CMakeFiles/cal.dir/src/CAL_CntrlCPU.c.o $LIB/CMakeFiles/cal.dir/src/CAL_CntrlGPU.c.o $LIB/CMakeFiles/cal.dir/src/CAL_Cntrl.c.o 
ld -shared -o libpru.so  $LIB/CMakeFiles/cal.dir/src/CAL_CntrlCPU.c.o $LIB/CMakeFiles/cal.dir/src/CAL_CntrlGPU.c.o $LIB/CMakeFiles/cal.dir/src/CAL_Cntrl.c.o 


icpc -o exec -L/usr/local/cuda-8.0/lib64 -L/home/eduardo/newcontrollers/HitControllers/Hitmap1.2/lib -L/frontend/ana/28-10-10_controladores/v2/HitControllers/build/cal -L. prueba.o  prueba_mic.o ${LIB}XPHI/CMakeFiles/calXPHIicc.dir/__/src/CAL_CntrlXPHI.c.o -lcudart -lhit  -lpru  







