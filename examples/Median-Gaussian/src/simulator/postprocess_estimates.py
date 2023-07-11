from statistics import mean
from statistics import stdev
import sys

option=int(sys.argv[1])

time = []

if option == 1:
    for i in range(1,11):
        f = open(f"swap-{i}.txt")
        time.append(float(f.read().split("\n")[-2]))

if option == 2:
    for i in range(1,11):
        f = open(f"gaussian-{i}.txt")
        time.append(float(f.read().split("\n")[-2]))

if option == 3:
    for i in range(1,11):
        f = open(f"median_1-{i}.txt")
        time.append(float(f.read().split("\n")[-2]))

if option == 4:
    for i in range(1,11):
        f = open(f"median_2-{i}.txt")
        time.append(float(f.read().split("\n")[-2]))

if option == 5:
    for i in range(1,11):
        f = open(f"median_3-{i}.txt")
        time.append(float(f.read().split("\n")[-2]))

if option == 6:
    for i in range(1,11):
        f = open(f"full-{i}.txt")
        time.append(float(f.read().split("\n")[-2]))


print(f"{mean(time)} {stdev(time)}\n")
