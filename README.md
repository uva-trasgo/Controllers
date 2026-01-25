# Controllers library

Controllers is a C11 library implementing the Controller heterogeneous programming framework (also referred as _the Controller model_), as described by [Torres _et al._](https://doi.org/10.1016/j.jpdc.2023.04.009).

[![Controllers's license](https://img.shields.io/gitlab/license/trasgo-group-valladolid%2Fcontrollers?style=flat&color=dark-green)](./LICENSE.md)

## Overview

The Controller model is a heterogeneous programming model that enables performance portability across different computing accelerators. Being a C11 library, it is compatible with any modern C/C++ compiler. It is also easily interoperable with other libraries and parallel programming models. The current implementation of the Controllers library supports the following computing accelerator architectures:
 * Multicore CPUs, using an OpenMP backend
 * NVIDIA GPUs, using a CUDA backend
 * AMD GPUs, using a HIP backend
 * Generic GPUs, using an OpenCL backend
 * Intel FPGAs, using another OpenCL backend

The Controller model allows multiple explicit implementation per computing kernel, with specialized versions for each type of device. It also allows compiling applications that leverage _almost_ any combination of the supported backends concurrently; linking all the defined kernel implementation together. Therefore, the compiled programs can target choose multiple different target devices at runtime. Kernel implementations are compiled with the backend's own vendor or native compiler, thus enabling the full range of capabilities of the technology.

(Due to technical limitations of the CUDA and HIP headers, Controllers cannot currently support both backends at the same time. Nevertheless, any other combination of backends is allowed.)

The Controller model leverages dependence analysis on sequence of data transfers, host computation, and kernel execution operations, to automatically overlap them whenever possible. It maps the execution control directly to the coprocessor driver resources, avoiding the use of a full scheduler, or task-graph analisys techniques. This results in a highly efficient control system with minimal overhead.

## Prerequisites

Controllers has some core dependencies, which are always required to use the library. Support for specific architectures might have some aditional dependencies.

Core runtime dependencies:
 - MPI >= 3.0
 - OpenMP >= 4.0
 - hwloc >= 1.11.0

Compilation dependencies:
 - CMake >= 3.20

CUDA architecture dependencies:
 - CUDA >= 9.0

HIP architecture dependencies:
 - HIP >= 6.1.0

OpenCL GPU architecture dependencies:
 - OpenCL (1.2 or 2.0 depending on the features used)

FPGA architecture dependencies:
 - Intel FPGA SDK for OpenCL

## Compiling Controllers

Controllers uses CMake for compilation. The usual compilation flow is as follows:
```
mkdir -p build
cd build
cmake ..
make -j $(nproc)
```

A convenience compilation bash script, [`compile.sh`](./compile.sh), is also provided. For instructions on how to use it, execute it with the `-h` option:
```
bash compile.sh -h
```

### Configurable compilation options

When compiling Controllers, the architecture support should be specified either by changing default values in CMake files, or by selecting them when running the `cmake` command. The values to change in the CMake files (`CMakeLists.txt`) are the following (change `OFF` to `ON` accordingly).
```cmake
# Architectures
option(SUPPORT_CPU "Support for the CPU architecture (OpenMP backend)" OFF)
option(SUPPORT_CUDA "Support for the NVIDIA GPU architecture (CUDA backend)" OFF)
option(SUPPORT_HIP "Support for the AMD GPU architecture (HIP backend)" OFF)
option(SUPPORT_OPENCL "Support for the generic GPU architecture (OpenCL backend)" OFF)
option(SUPPORT_FPGA "Support for the Intel FPGA architecture (Intel FPGA SDK for OpenCL backend)" OFF)
```
To change the options when running the command, do the following:
```
cmake -DSUPPORT_CUDA=ON ..
```

To see the full list of configurable options and their default values run:
```
mkdir -p build
cd build
cmake -LH ..
```

The `compile.sh` script allows specifying some of these options through command-line arguments when executing it. Refer to its help message (`bash compile.sh -h`) for more information.

### Specifying paths to dependencies

Additionally, the installation paths for multiple dependencies can be manually specified in the `CMakeList.txt` file as hints to the CMake system. You should change the following values accordingly:
```cmake
# Dependencies' installation paths
set(ROCM_PATH "/opt/rocm" CACHE PATH "Path to ROCm installation")
set(OPENCL_PATH "/usr/local/cuda" CACHE PATH "Path to OpenCL installation")
set(MAGMA_ROOT "/opt/magma" CACHE PATH "Path to MAGMA installation")
set(FPGA_BOARD_PKG "/opt/intelFPGA_pro/21.2/hld/board/de10_agilex" CACHE PATH "Path to FPGA board package")
```

## Example programs

This repository provides multiple example programs that use the Controllers library for different kinds of tasks. 
The examples can be found in the [`examples/`](./examples) directory. By default, they are always compiled alongside the library.

**Tip:** If you want to develop your own programs using Controllers please refer to the provided examples.

After compiling the example programs, they can be executed as follows (using one of the implementations of `Matrix_Add` as an example):
```
./build/examples/Matrix_Add/Matrix_Add_Cuda_Ctrl 100 1 0 0 1 <path_to_device_selection_file>
```
Controllers is prepared to execute on distributed environments:
```
mpirun -n 4 ./build/examples/Matrix_Add/Matrix_Add_Cuda_Ctrl 100 1 0 0 1 <path_to_device_selection_file>

srun -w node1,node2 -n 4 ./build/examples/Matrix_Add/Matrix_Add_Cuda_Ctrl 100 1 0 0 1 <path_to_device_selection_file>
```
The user controls which device(s) are selected and used for each process on each node, using a runtime configuration file. More information about the syntax and options of the device-selection configuration files can be found [here](./DEVICE_SELECTION.md).

## Documentation

### API

The Controllers library uses Doxygen for function and macro documentation. The documentation can be generated as follows:
```
cd Ctrl/doc
doxygen Doxyfile
```

### Project structure

In the root directory of this project you can find the following subdirectories:
- `Ctrl`: the source code of the Controllers library.
- `CtrlBlas`: premade BLAS kernels for Controllers. Contains implementations for MKL, cuBLAS and Magma.
- `examples`: examples using the Controllers library.
- `exp`: results and scripts for experimentations.
- `extern`: Controllers dependencies (Hitmap).

## Contributing

Thank you for your interest in contributing to the Controllers library! There are many ways to contribute to this project. You can take a look at [the document on contributing](./CONTRIBUTING.md) for tips on how to get started.

Feel free to contact us at [trasgo@infor.uva.es](mailto://trasgo@infor.uva.es) to share your experience with us, or visit our page [https://trasgo.infor.uva.es/](https://trasgo.infor.uva.es/) to learn more about us.
If you find any bugs or problems, or want to request a new feature, feel free to [open an issue](https://gitlab.com/trasgo-group-valladolid/controllers/-/issues) detailing it.

## Citation and further reading

If you are interested in knowing more about Controllers and the Controller model, you can review the following published research papers about the project:
 - Supporting efficient overlapping of host-device operations for heterogeneous programming with CtrlEvents. Y. Torres, F.J. Andújar, . Gonzalez-Escribano, D.R. Llanos. Journal of Parallel and Distributed Computing, 179, Elsevier, 2023. DOI: [10.1016/j.jpdc.2023.04.009](https://doi.org/10.1016/j.jpdc.2023.04.009).
 - Efficient heterogeneous programming with FPGAs using the Controller model. G. Rodriguez-Canal, Y. Torres, F.J. Andújar, A. Gonzalez-Escribano. The Journal of Supercomputing,77, 13995-14010, Springer, 2021. DOI: [10.1007/s11227-021-03792-7](https://doi.org/10.1007/s11227-021-03792-7).
 - Controllers: An abstraction to ease the use of hardware accelerators. A. Moretón-Fernández, H. Ortega-Arranz, A. Gonzalez-Escribano. The International Journal on High Performance Computing Aplications (IJHPCA), 32 (6), pag. 838-853, SAGE Journals, 2018. DOI: [10.1177/1094342017702962](https://doi.org/10.1177/1094342017702962).

If you use Controllers in your research in any meaningful way, we would appreciate that you cite any of those papers. Here are the BibTeX references for them:
```BibTeX
@article{Torres2023:CtrlEvents,
	title = {Supporting efficient overlapping of host-device operations for heterogeneous programming with CtrlEvents},
	author = {Yuri Torres and Francisco J. And\'{u}jar and Arturo Gonzalez-Escribano and Diego R. Llanos},
	journal = {Journal of Parallel and Distributed Computing},
	volume = {179},
	pages = {104708},
	year = {2023},
	issn = {0743-7315},
	doi = {https://doi.org/10.1016/j.jpdc.2023.04.009},
}

@article{Rodriguez-Canal2021:FPGAControllers,
	title = {Efficient heterogeneous programming with FPGAs using the Controller model},
	author = {Rodriguez-Canal, Gabriel and Torres, Yuri and And\'{u}jar, Francisco J. and Gonzalez-Escribano, Arturo},
	year = {2021},
	issue_date = {Dec 2021},
	publisher = {Kluwer Academic Publishers},
	address = {USA},
	volume = {77},
	number = {12},
	issn = {0920-8542},
	doi = {10.1007/s11227-021-03792-7},
	journal = {J. Supercomput.},
	month = dec,
	pages = {13995-14010},
	numpages = {16},
}

@article{Moreton-Fernandez2018:Controllers,
	title = {Controllers: An abstraction to ease the use of hardware accelerators},
	author = {Ana Moreton-Fernandez and Hector Ortega-Arranz and Arturo Gonzalez-Escribano},
	journal = {The International Journal of High Performance Computing Applications},
	volume = {32},
	number = {6},
	pages = {838-853},
	year = {2018},
	doi = {10.1177/1094342017702962},
}
```
