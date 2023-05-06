#!/bin/bash
#g++ --std=c++17 main.cc -mfma -O3 -Ofast -ffast-math  -I/home/chee/3rd_libs/include -L/home/chee/3rd_libs/lib -pthread -lopenblas

#MKLROOT='/home/chee/3rd_libs/intel/oneapi/mkl/latest'
MKL_INCLUDE="-I${MKLROOT}/include"
MKL_LIBS="-L${MKLROOT}/lib/intel64 -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -lpthread -lm -ldl -lspdlog"
MKL_CMP_LIBS="-L${MKLROOT}/../../compiler/latest/linux/compiler/lib/intel64_lin -liomp5"
#-L${MKLROOT}/lib/intel64 -Wl,--no-as-needed -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread -lm -ldl

#echo ${MKL_INCLUDE}
#echo ${MKL_LIBS}

g++   --std=c++17 main.cc -fopenmp -mfma -O3 -Ofast -ffast-math ${MKL_INCLUDE} ${MKL_LIBS} ${MKL_CMP_LIBS}
