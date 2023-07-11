# Run without CPU
sudo ./Median-Gaussian_Pynq_Ctrl 600 600 ~/controller-DPR/scheduler/Saltpepper.pgm 0 2 1 0 10 0.1
for i in {0..9}; do mv -f output_image_$i.ppm output_image_noswitch_noCPU_$i.ppm; done

# Run with CPU
sudo ./Median-Gaussian_Pynq_Ctrl 600 600 ~/controller-DPR/scheduler/Saltpepper.pgm 1 2 1 1 10 0.1
for i in {0..9}; do mv -f output_image_$i.ppm output_image_noswitch_CPU_$i.ppm; done

# Compare outputs
for i in {0..9}; do cmp output_image_noswitch_noCPU_$i.ppm output_image_noswitch_CPU_$i.ppm; done
