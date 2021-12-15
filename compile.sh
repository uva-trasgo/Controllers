#!/bin/sh

cwdfront

#Compile normal examples
module purge;
module load mpich3/gcc
cd build
make clean; 
make; 
cd ..


#Compile xeon phi examples
module purge;
module load comp/intel; module load mpich3/intel
cd build
make clean; 
make; 
cd ..

#Compile GPU+XeonPhi examples
#cd examples_xphi/gpu+xphi/matrixAdd/src/
#sh compile.sh
#cd ../../../../

