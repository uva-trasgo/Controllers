#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set home $::env(HOME)

# Create a project
open_project -reset median_blur

# The source file and test bench
add_files median-blur.cpp -cflags "-D_CTRL_ARCH_PYNQ_ -D_CTRL_COMPILE_PYNQ_KERNEL -I ../../../Ctrl/inc/ -I ../../../extern/hitmap/include/ -I ../../../extern/hitmap/extern/metis/Lib/ -I ../../../extern/hitmap/extern/iohb1.0/ -I ../../../extern/hitmap/extern/matrix_io/ -I ../../../controllers/Ctrl/inc -I /usr/lib/gcc/x86_64-redhat-linux/4.8.5/include/ -I ../../../extern/pynq_api/src"

# Specify the top-level function for synthesis
set_top MedianBlur

###########################
# Solution settings

# Create solution1
open_solution -reset solution1 -flow_target vivado 

# Define technology and clock rate
set_part  {xc7z020-clg400-1}
create_clock -period "100MHz"
config_rtl -reset all

#csim_design
# Run Synthesis, RTL Simulation, RTL implementation and Exit
csynth_design
#cosim_design
# Generate pcore
# export_design -format ip_catalog
export_design -format syn_dcp

exit


