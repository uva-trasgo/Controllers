#!/bin/bash

declare -A archs
archs["CPU"]="OFF"
archs["CUDA"]="OFF"
archs["OPENCL"]="OFF"
archs["HIP"]="OFF"
archs["FPGA"]="OFF"
fpga_emulation=0

declare -A libs
libs["CUBLAS"]="OFF"
libs["HIPBLAS"]="OFF"
libs["MKL"]="OFF"
libs["MAGMA"]="OFF"

CMAKE_FLAGS=""
while :; do
	case $1 in
	-h | -\? | --help)
		echo "-------- Controllers compilation script --------"
		echo "This script compiles Hitmap if it isn't already, deletes everything in 'build' directory and compiles Controllers with the options specified."
		echo "The user is responsible for loading the relevant modules/environments for the compilation, except for what is inside env.sh, which is sourced automatically if present."
		echo "Usage: bash compile.sh OPTIONS"
		echo "	-h|-?|--help            Show this message and exit."
		echo "	-a|--arch archs         Select Controller architectures to support."
		echo "	                        Comma separated. Valid values (case insensitive): CUDA, HIP, CPU, OPENCL, FPGA, FPGA-EMU."
		echo "	                        If not specified, the defaults specified in CMakeLists.txt will be used."
		echo "	-e|--exp                Compile in 'experimentation mode' (easier to process the output from benchmarks)."
		echo "	--alb_exp               Enable EPSILOD Automatic Load Balancing experimentation output."
		echo "	-d|--debug              Debug mode. Compile with -O0 -g and extra error checking and info."
		echo "	-c|--clean              Allways clean and recompile Hitmap."
		echo "	-p|--profile            Enable marks for host tasks for profiling on CUDA and OpenCL AMD."
		echo "	--cc compiler           Use a diferent C/C++ compiler. Also applies for CUDA host compiler and Hitmap compilation, if necessary."
		echo "	                        By default uses the value on \$CC/\$CXX or cc/c++ if \$CC/\$CXX do not exist."
		echo "	-l|--libs libs          Select Blas libs to support. Comma separated. Valid values (case insensitive): CUBLAS, HIPBLAS, MKL, MAGMA."
		echo "	                        If not specified, the defaults specified in CMakeLists.txt will be used."
		echo "	-f|--flags              Specify extra flags for the compiler. Comma separated to specify multiple extra flags."
		echo "	-s|--skip-examples      Skip Controller examples compilation."
		exit
		;;
	-a | --arch)
		# takes an option argument; ensure it has been specified.
		if [ "$2" ]; then
			archs_arg=(${2//,/ })
			for arch in "${archs_arg[@]}"; do
				arch=${arch^^}
				# check if arch exists as a key in archs array
				if [ ! "${archs[$arch]+abc}" ]; then
					# check special case: FPGA Emulation
					if [ "$arch" = "FPGA-EMU" ]; then
						echo "Compiling for FPGA Emulation (FPGA arch will be disabled, if selected)."
						archs["FPGA"]="ON"
						fpga_emulation=1
					else
						echo "ERROR: Arch $arch not found."
						exit
					fi
				fi
				echo "Compiling for $arch."
				archs["$arch"]="ON"
			done
			shift
		else
			echo 'ERROR: "--arch" requires a non-empty option argument.'
			exit
		fi
		;;
	-e | --exp)
		echo "Compiling experimentation examples"
		CMAKE_FLAGS+="-DUSE_EXPERIMENTATION_EXAMPLES:BOOL=ON "
		;;
	--alb_exp)
		echo "ALB exp mode enabled"
		CMAKE_FLAGS+="-DEPSILOD_ALB_EXPERIMENTATION_MODE:BOOL=ON "
		;;
	-d | --debug)
		echo "Compiling with debug"
		CMAKE_FLAGS+="-DCTRL_DEBUG=ON "
		;;
	-c | --clean)
		recompile_hitmap=1
		;;
	-p | --profile)
		echo "Compiling with profiling"
		CMAKE_FLAGS+="-DPROFILING_ENABLED=ON "
		;;
	--cc)
		if [ "$2" ]; then
			echo "using $2 C/C++ compiler"
			C_COMP=$2
			$2 --version
			shift
		else
			echo 'ERROR: "--arch" requires a non-empty option argument.'
			exit
		fi
		;;
	-l | --libs)
		if [ "$2" ]; then
			libs_arg=(${2//,/ })
			for lib in "${libs_arg[@]}"; do
				lib=${lib^^}
				# check if lib exists as a key in libs array
				if [ ! "${libs[$lib]+abc}" ]; then
					echo "ERROR: lib $lib not supported."
					exit
				fi
				echo "Compiling with $lib"
				libs["$lib"]="ON"
			done
			shift
		else
			echo 'ERROR: "--libs" requires a non-empty option argument.'
			exit
		fi
		;;
	-f | --flags)
		if [ "$2" ]; then
			c_flags=(${2//,/ })
			echo "Using flags: ${c_flags[@]}"
			CMAKE_FLAGS+="-DCMAKE_C_FLAGS='${c_flags[@]}' "
			CMAKE_FLAGS+="-DCMAKE_CXX_FLAGS='${c_flags[@]}' "
			CMAKE_FLAGS+="-DCMAKE_CUDA_FLAGS='${c_flags[@]}' "
			shift
		else
			echo 'ERROR: "--flags" requires a non-empty option argument.'
			exit
		fi
		;;
	-s | --skip-examples)
		echo "Skipping examples"
		CMAKE_FLAGS+="-DCOMPILE_CTRL_EXAMPLES:BOOL=OFF "
		;;
	--)
		# End of all options.
		shift
		break
		;;
	-?*)
		echo "Unknown option: $1"
		echo "Use '-h' or '--help' for help."
		exit
		;;
	*)
		# Default case: No more options, so break out of the loop.
		break
		;;
	esac
	shift
done

if [ "$archs_arg" ]; then
	for a in "${!archs[@]}"; do
		if [ $a = "FPGA-EMU" ]; then
			continue
		fi
		CMAKE_FLAGS+="-DSUPPORT_$a:BOOL=${archs[$a]} "
	done
	# select FPGA emulation properly
	if [ $fpga_emulation -eq 1 ]; then
		CMAKE_FLAGS+="-DFPGA_EMULATION:BOOL=ON "
	elif [ "${archs["FPGA"]}" = "ON" ]; then
		CMAKE_FLAGS+="-DFPGA_EMULATION:BOOL=OFF "
	fi
fi
if [ "$libs_arg" ]; then
	for l in "${!libs[@]}"; do
		CMAKE_FLAGS+="-D$l:BOOL=${libs[$l]} "
	done
fi

# move to controllers dir
cd "$(dirname "${BASH_SOURCE[0]}")"

# load modules
[ -f env.sh ] && . env.sh

# overwrite c and c++ compilers if indicated
CC=${CC:-"cc"}
CXX=${CXX:-"c++"}

CC=${C_COMP:-$CC}
CXX=${C_COMP:-$CXX}

# number of threads to use for compilation
compilation_threads=$(($(nproc) > 1 ? $(nproc) / 2 : 1))
# use less threads if compiling for real FPGA
if [ "${archs["FPGA"]}" = "ON" ] && [ $fpga_emulation -eq 0 ]; then
	compilation_threads=2
fi

CMAKE_FLAGS+="-DCMAKE_C_COMPILER=$CC -DCMAKE_CXX_COMPILER=$CXX "

# check if hitmap is compiled, if not, compile it
echo "Checking extern libs..."
if [ ! -f "extern/hitmap/lib/libhit.a" ] || [ $recompile_hitmap ]; then
	echo "Hitmap not found, compiling..."
	cd extern/hitmap
	make clean
	make -j $compilation_threads CC=$CC
	cd ../../
fi
echo "... Done!"

echo "Clean and rebuild..."
rm -rf build/
mkdir -p build && cd build

eval "cmake $CMAKE_FLAGS .."

make -j $compilation_threads
cd ..
echo "... Done!"
