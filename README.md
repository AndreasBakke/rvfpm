# rvfpm
rvfpm (RISC-V Floating Point (unit) Model) is a customizable model for verification and performance modeling. Aimed at simplifying design exploration and verification in the RISC-V space, the underlying core is eventually going to be fully customizable.



## Testing
### Compliance
To test for IEEE754 compliance run make TestFloat from work/
- Results is written to work/tests/\<target\>/rm-\<rounding mode\>/\<test\>.txt

### Functional verification
For functional verification run make sim_full from work/

## Status
Currently only supports the F-Extension.

Compiled and tested for compliance using Apple Arm - Apple clang version 15.0

Compiled, tested for compliance and simulated using x86_64 - GCC 12.2.0

For other architectures, adapt flags in work/Makefile
