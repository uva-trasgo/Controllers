BUILD_DIR=/home/xilinx/controller-DPR/controllers/build/examples/Median-Gaussian
CURR_DIR=$(pwd)

cd $BUILD_DIR
make clean && make

cp Median-Gaussian_Pynq_Ctrl Median-Gaussian_Pynq_Ctrl-backup 
cd $CURR_DIR
