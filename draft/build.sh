#!/bin/bash
g++ main.cpp -std=c++17 -I/home/chee/3rd_libs/include -mfma -O3 -Ofast -ffast-math -fopenmp -L/home/chee/3rd_libs/lib #-ljemalloc #-static -lopenblas
#g++ main.cpp -std=c++17 -mfma -O3 -Ofast -ffast-math -fopenmp #-static -lopenblas
