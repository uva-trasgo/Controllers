# Controllers device-selection files

This document describes the format of the runtime configuration files for Controllers programs, to select the devices associated to the Controller objets on each node and process in distributed environments.

## Summary: Launching MPI processes and the role of the device-selection file

To distribute the execution of a Controllers program, you can use your usual MPI program launcher (`mpirun`, `srun`, etc.)
The nodes selected and the number of processes are controlled with the MPI launcher parameters. At the start of the Controllers section of the program, each MPI process automatically retrieves the local node name, and gets an internal rank inside the local node. The process tries to locate in the device-selection file an entry that matches the node name and the local rank. If it is found, the entry describes each Controller object that should be created in that process, the backend to use, and the specific device that should be attached to the object. If no matching entry is found, a fatal error is raised and the program terminates.


## Specifying the device-selection file

The Controllers code block receives as parameter a string with the relative or absolute path to a readable device-selection configuration file. This configuration is used to automatically create and initialize the Controller objects described in the file for the specific node and the specific process identification (rank) inside the node.

Example: 
```
	__ctrl_block__( "my_config_file" ) {
		...
	}
```

**NOTE:** The file path string can be supplied as a program argument, or built as a string by the program.

Alternatively, if NULL is passed, the content of the `CTRL_CFG_PATH` environment variable are used as the config\_file path.


## Accesing and using the Controller objects

Inside the Controllers code block, a Controller object can be accesed using the function: `Ctrl Ctrl_Get( int )`;

Example:
```
	Ctrl first_device = Ctrl_Get(0);
	Ctrl second_device = Ctrl_Get(1);
```


## File format specification

Device-Selection files are organized in lines, with one declaration on each line. Blank spaces and tabs at the start of the line are ignored. The keywords and parameters of each declaration are separated by exactly one space. Keywords are case-sensitive.

Declarations:
```
	<config_file> ::= { <node_section> \n }

	<node_section> ::= node ( <machine_name> | * ) \n  { <process_section> \n }

	<process_section> ::= proc <rank_in_node> [ numa = <int> ] [ weight = <float> ] [ align = <int> ] \n { <device_section> \n }
	<rank_in_node> ::= <int>

	<device_section> ::= ( cpu <ctrl_cpu_parameters> | cuda <ctrl_cuda_parameters> | hip <ctrl_hip_parameters> | opencl <ctrl_opencl_parameters> | fpga <ctrl_fpga_parameters> ) \n

	<ctrl_cpu_parameters> ::= threads = <int> [ numa_range = <affinity_numa_nodes_range> ] [ memmoves = < 0 | 1 > ] [ align = <int> ]
	<affinity_numa_nodes_range> ::= <int> - <int>

	<ctrl_cuda_parameters> ::= dev = <int> [ kstreams = <int> ] [ align = <int> ]
	<ctrl_hip_parameters> ::= dev = <int> [ kstreams = <int> ] [ align = <int> ]
	<ctrl_opencl_parameters> ::= platform = <int> dev = <int> [ kstreams = <int> ] [ align = <int> ]
	<ctrl_fpga_parameters> ::= platform = <int> dev = <int> [ kstreams = <int> ] [ align = <int> ]
```

`<machine_name>` corresponds to the hostname of the execution node as reported by MPI, as such, it's format follows the hostname rules specified in RFC 1123.
`<int>` and `<float>` follow the rules in C11 specification for string to decimal number and floating point number conversion, respectively. 

### Node declaration

When the MPI process executes the Controller code block initializaton, it first queries the node name and reads the corresponding `node` section. In the case that there is no section with that machine name, it fallbacks to the `node` section with the wildcard * as name, if present. In the case that there is no valid section for the machine name, an error is issued and the process stops.

### Process declaration

The processes in the same node are numbered, starting at 0, to obtain a local rank in the node group. The process locates the `proc` section with that local rank inside the selected node section. One Controller object is created by the process for each device declaration found in its process section.

The affinity for the host threads of a process can be controlled by specifying the index of a NUMA node in `<numa_affinity>`. If not specified, the default is `0`.

If a `proc` section includes a `weight` argument, the specified weight will be used to compute the amount of data to assign to the corresponding process when distributing tiles (data buffers) in the application, if using a weighted partition. Weights must be positive real numbers. They will be normalized automatically, and data will be assigned to each process proportionally. By default, processes have a weight equal to `1.0`.

Alignment for the pitch (in bytes) of the last dimension of multidimensional tiles can be specified both for the host, on the `proc` section, and for each device, via the optional `align` argument.
If a tile with aligned memory is requested this value will be used. If left unset or 0 it will fallback to a device specific mechanism (eg. `cudaMallocPitched`) or a default value.
The selected alignment must be a multiple of the tile's datatype.

### Device declarations

A device declaration line contains a keyword indicating the desired Controller backend, and a list of parameters for selecting the specific device. The parameters are specific for each type of backend.

* Multicore CPUs
A Controller is created to manage a set of CPU cores with a number of logical threads equal to `<num_threads>`, with affinity set to the physical cores in the NUMA nodes included in the range indicated by the `numa_range` parameter. The start node parameter is inclusive, the end node parameter is not inclusive. By default the empty range `0-0` is used, allowing the use of the whole machine. Duplicated memory and data transfers are activated with the `memmoves` parameter. If the memory copies are not activated, the kernels launched through this Controller object read/write directly from/to the host memory images.

* NVIDIA GPUs using CUDA
A Controller is created to manage an NVIDIA GPU with the given CUDA identifier. Use the command `nvidia-smi` to obtain information about the devices available in a machine.
The number of concurrent kernel queues on this device can be adjusted with `kstreams`. Default is `1`.

* AMD GPUs using HIP
A Controller is created to manage the selected device using HIP. Use the command `rocm-smi` to obtain information about the devices available in a machine.
The number of concurrent kernel queues on this device can be adjusted with `kstreams`. Default is `1`.

* GPU devices using OpenCL
A Controller is created to manage, with OpenCL, the chosen device in the chosen platform. Use the command `clinfo` to obtain information about the platforms and devices available in a machine.
The number of concurrent kernel queues on this device can be adjusted with `kstreams`. Default is `1`.

* Intel FPGA devices using OpenCL
Similarly to the `opencl` backend, a Controller is created to manage, with the Intel FPGA SDK for OpenCL, the chosen device in the chosen platform. Use the command `clinfo` to obtain information about the platforms and devices available in a machine. Note that, for running a Controllers FPGA program using emulation, a specific platform must be chosen in the configuration file.
The number of concurrent kernel queues on this device can be adjusted with `kstreams`. Default is `1`.

## Examples

Examples of device-selection files can be found in [`examples/Device_Selection_Files`](./examples/Device_Selection_Files). Here is the content for the `dev_epsilod_exp` configuration:
```
node manticore
	proc 0 
		cuda dev = 0 kstreams = 8
	proc 1 numa = 1
		cuda dev = 1 kstreams = 8
	proc 2 numa = 0
		opencl platform = 0 dev = 0 kstreams = 8
	proc 3
		opencl platform = 0 dev = 1 kstreams = 8
	proc 4
		cpu threads = 16 numa = 0-0 memmoves = 0
	proc 5
		cpu threads = 16 numa = 0-0 memmoves = 1

node chimera
	proc 0 
		cuda dev = 0 kstreams = 8
	proc 1 numa = 1
		cuda dev = 1 kstreams = 8
	proc 2 numa = 0
		opencl platform = 0 dev = 0 kstreams = 8
	proc 3
		opencl platform = 0 dev = 1 kstreams = 8
	proc 4
		cpu threads = 16 numa = 0-0 memmoves = 0
	proc 5
		cpu threads = 16 numa = 0-0 memmoves = 1

node gorgon
	proc 0 numa = 0
		cuda dev = 0 kstreams = 8
	proc 1 numa = 0
		opencl platform = 0 dev = 0 kstreams = 8
	proc 2 numa = 0
		cuda dev = 1 kstreams = 8
	proc 3 numa = 0
		cuda dev = 2 kstreams = 8
	proc 4 numa = 0
		cpu threads = 8 numa = 0-0 memmoves = 0
	proc 5 numa = 0
		cpu threads = 8 numa = 0-0 memmoves = 1

node medusa
	proc 0 numa = 0 align = 256
		cuda dev = 0 kstreams = 8 align = 128
	proc 1 numa = 0
		cpu threads = 8 numa = 0-0 memmoves = 0
	proc 2 numa = 0
		cpu threads = 8 numa = 0-0 memmoves = 1

node unicornio
	proc 0 numa = 0	weight = 0.6007239666638606
		fpga platform = 2 dev = 0 kstreams = 8
	proc 1 numa = 0	weight = 0.3992760333361394
		cpu threads = 12 numa_range = 0-0 memmoves = 0
	proc 2 numa = 0	weight = 0.70212392487274
		cuda dev = 0 kstreams = 8

node *
	proc 0 numa = 0
		cpu threads = 1 numa = 0-0 memmoves = 1
	proc 1 numa = 0
		cpu threads = 1 numa = 0-0 memmoves = 1
```
With this example,
 * the node with name `manticore` would create 2 CUDA Controllers, 2 OpenCL Controllers, and 2 CPU Controllers,
 * the node with name `gorgon` would create 3 CUDA Controllers, 1 OpenCL Controller, and 2 CPU Controllers,
 * the node with name `medusa` would create 1 CUDA Controller, and 2 CPU Controllers,
 * the node with name `unicornio` would create 1 FPGA Controller, 1 CPU Controller, and 1 CUDA Controller, and
 * any other node would create 2 CPU Controllers.
Note that not all the nodes listed in the configuration file must be present for execution. The entries for unused nodes will just be ignored.
