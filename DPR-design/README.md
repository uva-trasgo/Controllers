Dynamic Partial Reconfiguration shell
====
This directory contains the scripts to build the Dynamic Partial Reconfiguration (DPR) shell for the Zynq-7000 family used by our extensible DPR task scheduler. The script [generate_design.tcl](generate_design.tcl) will build a bespoke DPR shell for the kernels that we specified. We adopted this approach instead of a generic shell with fixed size Reconfigurable Regions (RR) to optimise resource usage. This specially important in this FPGA family.

To build your design, paste the netlists for your kernels in the Synth/reconfig_modules directory and invoke the build script with:

```vivado -mode batch -source generate_design.tcl -notrace -tclargs <n_kernels> <kernel_list>```

Where:
* 1 $\leq$ ```<n_kernels>``` $\leq$ 4
* ```kernel_list``` is the list of the names of the kernels to instantiate in the design seperated by spaces.

In the directories design-\<NRR\>-axi_firewall/ we provide the shell designs for 2, 3 and 4 RRs. The limitation in resources of the Pynq does not allow for more RRs, although this limitation would be easily overcome in larger boards. Each of this directories provide the floorplanning constraints in the Sources/xdc/fplan.xdc file, which contain a description of the geometry of the pblocks where the RRs are instantiated. static/design_1.tcl contains the block design of the shell so that the user can experiment with it modify at will. In order to do that, the tcl script should be sourced in Vivado. Finally, the Static/ directory contains the netlists of the kernels to be instantiated under reconfig_modules/ and the netlist of the static design (which the user can produce by synthesising the block design) under Static/.


If you would rather build the example we use in our evaluation for the Wiley paper instead, you can invoke ```make```.