#!/bin/bash

icc  -E -DUSE_CBLAS -DUSE_MKL -std=c99 -O3 -fopenmp -I/opt/intel/compilers_and_libraries_2017.0.098/linux/mkl/include -I/home/eduardo/newcontrollers/HitControllers/Hitmap_Edu/include -I/opt/mpich3.gcc/include -I/home/eduardo/newcontrollers/HitControllers/calXPHI/inc -I/home/eduardo/newcontrollers/HitControllers/cal/inc -I/usr/local/cuda/include    -o gemver2CntrlXPHI.c.o   -c /home/eduardo/newcontrollers/HitControllers/examples_xphi/gemver_gpuxphi/src/gemver2CntrlXPHI.c

mv gemver2CntrlXPHI.c.o gemver2CntrlXPHI_out.c

icc  -DUSE_CBLAS -DUSE_MKL -std=c99 -O3 -fopenmp -I/opt/intel/compilers_and_libraries_2017.0.098/linux/mkl/include -I/home/eduardo/newcontrollers/HitControllers/Hitmap_Edu/include -I/opt/mpich3.gcc/include -I/home/eduardo/newcontrollers/HitControllers/calXPHI/inc -I/home/eduardo/newcontrollers/HitControllers/cal/inc -I/usr/local/cuda/include    -o gemver2CntrlXPHI.c.o   -c gemver2CntrlXPHI_out.c
