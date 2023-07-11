#!/bin/bash

# Tables
python3 gen_tables.py

# Gantt charts
python3 plots_json.py 1368297677 0 gantt

# Service time plots
python3 plots_json.py 1120249751 1 stacked

# Throughput plots
python3 plots_json.py 28871727 0 points
python3 plots_json.py 1368297677 0 points
