#!/bin/bash

for i in {1..10}; do
    # Swap
    sudo ./time_estimates 0 1 1 > swap-$i.txt

    # GaussianBlur
    sudo ./time_estimates 1 0 1 > gaussian-$i.txt

    # MedianBlur 1
    sudo ./time_estimates 1 1 1 > median_1-$i.txt

    # MedianBlur 2
    sudo ./time_estimates 1 2 1 > median_2-$i.txt

    # MedianBlur 3
    sudo ./time_estimates 1 3 1 > median_3-$i.txt

    # Full
    sudo ./time_estimates 0 1 0 > full-$i.txt
done

python3 postprocess_estimates.py 1 > swap_times.txt
python3 postprocess_estimates.py 2 > gaussian_times.txt
python3 postprocess_estimates.py 3 > median_1_times.txt
python3 postprocess_estimates.py 4 > median_2_times.txt
python3 postprocess_estimates.py 5 > median_3_times.txt
python3 postprocess_estimates.py 6 > full_times.txt

rm swap-*.txt
rm gaussian-*.txt
rm median*-*.txt
rm full-*.txt
