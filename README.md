# riscv-fpm

Compiled, tested and verified for macOS - Apple clang version 15.0
For other OS. Update flags to the compiler in work/Makefile.

Library paths might also need to be updated.
export LD_LIBRARY_PATH=/cad/gnu/gcc/12.2.0/dependencies_lib
^Set path to libmpfr.so.6

make TestFloat



run:
make rhelSL
 vsim -sv_lib bin/lib_rvfpm -novopt work.rvfpm_tb -suppress 12110