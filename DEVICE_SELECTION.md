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

TODO: Alternatively, use the content of an environment variable as the config\_file path.


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

	<process_section> ::= proc <rank_in_node> [ <numa_affinity> ] \n [ weight <float> \n ] { <device_section> \n }
	<rank_in_node> ::= <int>
	<numa_affinity> ::= <int>

	<device_section> ::= ( cpu <ctrl_cpu_parameters> | cuda <ctrl_cuda_parameters> | hip <ctrl_hip_parameters> | opencl <ctrl_opencl_parameters> | fpga <ctrl_fpga_parameters> ) \n

	<ctrl_cpu_parameters> ::= <num_threads> <affinity_numa_nodes_range> <memory_copy_flag>
	<num_threads> ::= <int>
	<affinity_numa_nodes_range> ::= <int> - <int>
	<memory_copy_flag> ::= 0 | 1

	<ctrl_cuda_parameters> ::= <device_id> [ <execution_streams> ]
	<ctrl_hip_parameters> ::= <device_id> [ <execution_streams> ]
	<ctrl_opencl_parameters> ::= <platform_id> <device_id> [ <execution_streams> ]
	<ctrl_fpga_parameters> ::= <platform_id> <device_id> [ <execution_streams> ]
	
	<platform_id> ::= <int>
	<device_id> ::= <int>
	<execution_streams> ::= <int>

```

`<machine_name>` corresponds to the hostname of the execution node as reported by MPI, as such, it's format follows the hostname rules specified in RFC 1123.
`<int>` and `<float>` follow the rules in C11 specification for string to decimal number and floating point number conversion, respectively. 

### Node declaration

When the MPI process executes the Controller code block initializaton, it first queries the node name and reads the corresponding `node` section. In the case that there is no section with that machine name, it fallbacks to the `node` section with the wildcard * as name, if present. In the case that there is no valid section for the machine name, an error is issued and the process stops.

### Process declaration

The processes in the same node are numbered, starting at 0, to obtain a local rank in the node group. The process locates the `proc` section with that local rank inside the selected node section. One Controller object is created by the process for each device declaration found in its process section.

The affinity for the host threads of a process can be controlled by specifying the index of a NUMA node in `<numa_affinity>`. If not specified, the default is `0`.

### Weight declaration

If a `proc` section includes a `weight` line, the specified weight will be used to compute the amount of data to assign to the corresponding process when distributing tiles (data buffers) in the application, if using a weighted partition. Weights must be positive real numbers. They will be normalized automatically, and data will be assigned to each process proportionally. By default, if no `weight` line is included, processes have a weight equal to `1.0`.
<!-- TODO @seralpa we should have an example using this -->

### Device declarations

A device declaration line contains a keyword indicating the desired Controller backend, and a list of parameters for selecting the specific device. The parameters are specific for each type of backend.

* Multicore CPUs
A Controller is created to manage a set of CPU cores with a number of logical threads equal to `<num_threads>`, with affinity set to the physical cores in the NUMA nodes included in the range indicated by the second parameter. The start node parameter is inclusive, the end node parameter is not inclusive. Duplicated memory and data transfers are activated with the third parameter. If the memory copies are not activated, the kernels launched through this Controller object read/write directly from/to the host memory images.

* NVIDIA GPUs using CUDA
A Controller is created to manage an NVIDIA GPU with the given CUDA identifier. Use the command `nvidia-smi` to obtain information about the devices available in a machine.

* AMD GPUs using HIP
A Controller is created to manage the selected device using HIP. Use the command `rocm-smi` to obtain information about the devices available in a machine.

* GPU devices using OpenCL
A Controller is created to manage, with OpenCL, the chosen device in the chosen platform. Use the command `clinfo` to obtain information about the platforms and devices available in a machine.

* Intel FPGA devices using OpenCL
Similarly to the `opencl` backend, a Controller is created to manage, with the Intel FPGA SDK for OpenCL, the chosen device in the chosen platform. Use the command `clinfo` to obtain information about the platforms and devices available in a machine. Note that, for running a Controllers FPGA program using emulation, a specific platform must be chosen in the configuration file.

## Examples

Examples of device-selection files can be found in [`examples/Device_Selection_Files`](./examples/Device_Selection_Files). Here is the content for the `dev_epsilod_exp` configuration:
```
node manticore
	proc 0 0
		cuda 0 8
	proc 1 0
		cuda 1 8
	proc 2 0
		opencl 0 0 8
	proc 3 0
		opencl 0 1 8
	proc 4 0
		cpu 16 0-0 0
	proc 5 0
		cpu 16 0-0 1

node gorgon
	proc 0 0
		cuda 0 8
	proc 1 0
		opencl 0 0 8
	proc 2 0
		cuda 1 8
	proc 3 0
		cuda 2 8
	proc 4 0
		cpu 8 0-0 0
	proc 5 0
		cpu 8 0-0 1

node medusa
	proc 0 0
		cuda 0 8
	proc 1 0
		cpu 8 0-0 0
	proc 2 0
		cpu 8 0-0 1

node unicornio
	proc 0 0
		fpga 1 0 8
	proc 1 0
		cpu 12 0-0 0
	proc 2 0
		cuda 0 8

node *
	proc 0 0
		cpu 1 0-0 1
	proc 1 0
		cpu 1 0-0 1
```
With this example,
 * the node with name `manticore` would create 2 CUDA Controllers, 2 OpenCL Controllers, and 2 CPU Controllers,
 * the node with name `gorgon` would create 3 CUDA Controllers, 1 OpenCL Controller, and 2 CPU Controllers,
 * the node with name `medusa` would create 1 CUDA Controller, and 2 CPU Controllers,
 * the node with name `unicornio` would create 1 FPGA Controller, 1 CPU Controller, and 1 CUDA Controller, and
 * any other node would create 2 CPU Controllers.
Note that not all the nodes listed in the configuration file must be present for execution. The entries for unused nodes will just be ignored.
