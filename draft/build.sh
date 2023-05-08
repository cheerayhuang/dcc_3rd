#!/bin/bash
#g++ main.cpp  -std=c++17 -I/home/chee/3rd_libs/include -mfma -O3 -Ofast -ffast-math   -fopenmp -Wl,-Bstatic -L/home/chee/3rd_libs/lib -lopenblas -Wl,-Bdynamic
g++ main.cpp -std=c++17 -mfma -O3 -Ofast -ffast-math -fopenmp
#g++ main.cpp  -std=c++17 -mfma -O3 -Ofast -ffast-math -static -Wl,-Bdynamic -fopenmp
