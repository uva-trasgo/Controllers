# Run without preemption
sudo ./Median-Gaussian_Pynq_Ctrl 600 600 ~/controller-DPR/scheduler/Saltpepper.pgm 0 2 1 0 10 0.1
for i in {0..9}; do mv -f output_image_$i.ppm output_image_noswitch_$i.ppm; done

# Run with preemption
sudo ./Median-Gaussian_Pynq_Ctrl 600 600 ~/controller-DPR/scheduler/Saltpepper.pgm 0 2 1 1 10 0.1
for i in {0..9}; do mv -f output_image_$i.ppm output_image_switch_$i.ppm; done

# Compare outputs
for i in {0..9}; do cmp output_image_noswitch_$i.ppm output_image_switch_$i.ppm; done
