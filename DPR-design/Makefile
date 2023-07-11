CC       = gcc
CFLAGS   = -O3
LFLAGS   = -lpynq -lcma -lpthread

all:
	mkdir -p Bitstreams Checkpoint Implement Synth/Static Synth/reconfig_modules
	cp ../kernels/Pynq/median-blur/median_blur/solution1/impl/verilog/report/MedianBlur.dcp Synth/reconfig_modules/
	cp ../kernels/Pynq/gaussian-blur/gaussian_median/solution1/impl/verilog/report/GaussianBlur.dcp Synth/reconfig_modules/
	vivado -mode batch -source generate_design.tcl -notrace -tclargs 2 MedianBlur GaussianBlur

clean:
	rm -rf Bitstreams/* Checkpoint/* Implement/*
	rm *.jou *.log
