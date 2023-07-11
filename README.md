Controller library
====
Controller is a library written in C99 that provides a simplified way to program applications that can exploit heterogeneous computational platforms including accelerators and/or multi-core CPUs.
Controller uses dependence analysis on a sequence of data-transfer, host computation and kernel execution operations, automatically overlapping them whenever possible.
The runtime library supports several technologies and lower-level programming models (such as OpenMP, CUDA, or OpenCL), choosing the proper kernel for any device at runtime. This avoids the need to recompile the whole application to execute it using a different device.
The solution maps the execution control directly to the coprocessor driver resources, avoiding the use of a full scheduler or task-graph analysis techniques. This derives in a high efficiency and minimum overhead introduced by the control system.

Controller currently supports 5 different backends for different accelerator architectures:
 * CUDA (for NVIDIA's GPUs)
 * OpenCL (for other GPUs, such as AMD)
 * CPU (for subsets of CPU cores using OpenMP)
 * FPGA (for Intel and Xilinx FPGAs using OpenCL and the Intel AOC compiler)
 * Pynq (for the Zynq 7000 family, with support for Dynamic Partial Reconfiguration)

The backend for Intel XeonPhi devices has been discontinued in the latest version of Controller.

**Note**: The [examples/](examples/) directory contains only examples that make use of our DPR scheduler. For other examples running on other backends checkout the master branch.

Prerequisites
----
Controller has some core dependencies, which are always required and then support for a specific architecture might have some aditional dependencies.

Core dependencies:
 - MPI (3.x or higher)
 - OpenMP (4.x or higher)
 - hwloc (1.11.x)

CUDA architecture dependencies:
 - CUDA (9.x or higher)

OpenCL GPU architecture dependencies:
 - OpenCl

FPGA architecture dependencies:
 - OpenCL
 - aoc (20.3)

Compiling Controller
----
Compilation is done using CMake (3.17 or higher).

```
mkdir -p build
cd build
cmake ..
make
```

When compiling Controller, the supported architectures for which you have the proper compiler and libraries installed, should be specified either by changing default values in CMake files
```cmake
# Architectures
option(SUPPORT_CPU "SUPPORT_CPU" OFF)
option(SUPPORT_CUDA "SUPPORT_CUDA" OFF)
option(SUPPORT_OPENCL_GPU "SUPPORT_OPENCL_GPU" OFF)
option(SUPPORT_OPENCL_GPU_AMD "Use AMD platform" OFF)
option(SUPPORT_FPGA "SUPPORT_FPGA" OFF)
```
or by selecting them when running cmake like this:
```
cmake -DSUPPORT_CUDA=ON ..
```

To see the full list of configuration options and their default values run:
```
mkdir -p build
cd build
cmake -LH ..
```

Executing an example
----
```
./build/examples/Matrix_Add/Matrix_Add_Cuda_Ctrl 100 1 0 0 1
```

Generating documentation
----
Documentation for the Controller library can be generated using Doxygen.
```
cd Ctrl/doc
doxygen Doxyfile
```

Dynamic Partial Reconfiguration
----
We have included Dynamic Partial Reconfiguration (DPR) capabilities on our latest Controller release for the Xilinx Zynq-7000 family of FPGAs. Through the integration of this technology and our new extensible FPGA task [scheduler/](scheduler) built on top of the Controller library we aim to bridge the gap between FPGAs and CPU/GPU in terms of flexibility: we enable the preemption of tasks running on the FPGA whilst we keep the rest of the tasks on the device running, in contrast to the traditional approach, in which the whole device would stall upon task removal.

The new scheduler needs a DPR-capable shell for which we provide the design in the directory [DPR-design/](DPR-design), where you can find detailed instructions on how to build it.

The experimentation for the evaluation of this work can be found under [exp/pynq](exp/pynq/). The data for resource utilisation presented in the paper is available under [kernels/Pynq], for example [kernels/Pynq/gaussian-blur/resource_usage.rpt](kernels/Pynq/gaussian-blur/resource_usage.rpt) for the Gaussian Blur kernel compiled with the preemption infrastructure.

Contributing
----
Thanks for your interest in contributing! There are many ways to contribute to this project.

Feel free to contact us at trasgo@infor.uva.es to share your experience with us or visit our page https://trasgo.infor.uva.es/ to learn more about us.
If you find any bugs or problems, or want to request a new feature feel free to create an issue detailing it.

If you want to contribute code to the project get started [here](./CONTRIBUTE.md).

Relevant publications
---
Here are some of the more important publications about this project:
- Task-based preemptive scheduling on FPGAs leveraging partial reconfiguration. G. Rodriguez-Canal, N. Brown, Y. Torres, A. Gonzalez-Escribano. To appear on Concurrency And Computation: Practice And Experience, Wiley, 2023.

 - Efficient heterogeneous programming with FPGAs using the Controller model. G. Rodriguez-Canal, Y. Torres, F.J. Andújar, A. Gonzalez-Escribano. The Journal of Supercomputing, Springer, 2021. DOI: 10.1007/s11227-021-03792-7

 - Controller: An abstraction to ease the use of hardware accelerators. A. Moretón-Fernández, H. Ortega-Arranz, A. Gonzalez-Escribano. The International Journal on High Performance Computing Aplications (IJHPCA), 32 (6), pag. 838-853, SAGE Journals, 2018. DOI: 10.1177/1094342017702962
