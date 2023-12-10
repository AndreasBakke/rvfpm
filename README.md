# riscv-fpm

Compiled and tested for compliance using Apple Arm - Apple clang version 15.0

Compiled, tested for compliance and simulated using x86_64 - GCC 12.2.0

For other OS. Update flags to the compiler in work/Makefile.



To test for IEEE754 compliance run make TestFloat from work/
- Results is written to work/tests/<target>/rm-<rounding mode>/<test>.txt

For functional verification run make sim_full from work/