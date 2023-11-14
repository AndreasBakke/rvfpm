# riscv-fpm - Verification

prerequisites: g++
    setenv CXX

export LD_LIBRARY_PATH=/cad/gnu/gcc/12.2.0/dependencies_lib
^Set path to libmpfr.so.6

make TestFloat
