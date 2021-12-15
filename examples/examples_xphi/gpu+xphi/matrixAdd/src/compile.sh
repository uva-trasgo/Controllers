#!/bin/sh

rm libpru.so matrixAdd_GPU_XPHI matrixAddCntrlGPU_XPHI.o  kernel_xphi.o 

LIB=/home/eduardo/newcontrollers/HitControllers/build/cal

##################################
#Programa principal
##################################
echo "\n PROGRAMA \n"
#icpc -c kernel_xphi.cpp -fopenmp -I/usr/local/cuda-8.0/include -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI -I/home/eduardo/newcontrollers/HitControllers/Hitmap1.2/include -I/opt/mpich3.gcc/include -I/home/eduardo/newcontrollers/HitControllers/cal/inc -I/home/eduardo/newcontrollers/HitControllers/calXPHI/inc 

#nvcc -c matrixAddCntrlGPU_XPHI.cu -Xcompiler -fopenmp -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI -I/home/eduardo/newcontrollers/HitControllers/Hitmap1.2/include -I/opt/mpich3.gcc/include  -I/home/eduardo/newcontrollers/HitControllers/cal/inc -I/home/eduardo/newcontrollers/HitControllers/calXPHI/inc

#ld -shared -o libpru.so  $LIB/CMakeFiles/cal.dir/src/CAL_CntrlCPU.c.o $LIB/CMakeFiles/cal.dir/src/CAL_CntrlGPU.c.o $LIB/CMakeFiles/cal.dir/src/CAL_Cntrl.c.o 

#icpc -o matrixAdd_GPU_XPHI -fopenmp -L/usr/local/cuda-8.0/lib64 -L/home/eduardo/newcontrollers/HitControllers/Hitmap1.2/lib  -L/home/eduardo/newcontrollers/HitControllers/examples_xphi/gpu+xphi/matrixAdd/src matrixAddCntrlGPU_XPHI.o kernel_xphi.o ${LIB}XPHI/ICC/CMakeFiles/calXPHIicc.dir/__/src/CAL_CntrlXPHI.c.o -lcudart -lhit -lpru 

##################################
#Programa principal con la libreria de Edu
##################################
echo "\n PROGRAMA \n"
icpc -c kernel_xphi.cpp -o kernel_edu.o -fopenmp -I/usr/local/cuda-8.0/include -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI -I/home/eduardo/newcontrollers/HitControllers/Hitmap_Edu/include -I/opt/mpich3.intel/include -I/home/eduardo/newcontrollers/HitControllers/cal/inc -I/home/eduardo/newcontrollers/HitControllers/calXPHI/inc 

nvcc -c matrixAddCntrlGPU_XPHI.cu -o program_Edu.o -Xcompiler -fopenmp -DMPICH_SKIP_MPICXX -DMPI_NO_CPPBIND -DSGIMPI -I/home/eduardo/newcontrollers/HitControllers/Hitmap_Edu/include -I/opt/mpich3.intel/include  -I/home/eduardo/newcontrollers/HitControllers/cal/inc -I/home/eduardo/newcontrollers/HitControllers/calXPHI/inc

ld -shared -o libpru.so  $LIB/CMakeFiles/cal.dir/src/CAL_CntrlCPU.c.o $LIB/CMakeFiles/cal.dir/src/CAL_CntrlGPU.c.o $LIB/CMakeFiles/cal.dir/src/CAL_Cntrl.c.o 

icpc -o matrixAdd_GPU_XPHI_Edu -fopenmp -L/usr/local/cuda-8.0/lib64 -L/opt/mpich3.intel/lib  -L/home/eduardo/newcontrollers/HitControllers/Hitmap_Edu/lib  -L/home/eduardo/newcontrollers/HitControllers/examples_xphi/gpu+xphi/matrixAdd/src program_Edu.o kernel_edu.o ${LIB}XPHI/ICC/CMakeFiles/calXPHIicc.dir/__/src/CAL_CntrlXPHI.c.o -lcudart -lhit -lpru -lmpi 



