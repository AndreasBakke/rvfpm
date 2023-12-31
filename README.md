# rvfpm
rvfpm (RISC-V Floating Point (unit) Model) is a customizable model for verification and performance modeling. Aimed at simplifying design exploration and verification in the RISC-V space, the underlying core is eventually going to be fully customizable.

## Structure
### Core
All files relevant to the C++ core and its verification can be found in work/. Header files are located in work/include and need to be included during compilation. See the Makefile in work/ for details. C++ files are locateed in work/src.

### Interface
The SystemVerilog interface can be found in work/src

### Compliance testing
The interface for Berkeley TestFloat can be found alongside the C++ core files in work/src. Note that conversion functions to/from hex-strings are generated by chatGPT.

### Functional verification
The testbench used for functional verification can be found in its entirity in work/sim.

## Testing
All compiled binaries for testing and verification are located in work/bin.

### Compliance
To test for IEEE754 compliance run make TestFloat from work/
- Results is written to work/tests/\<target\>/rm-\<rounding mode\>/\<test\>.txt

### Functional verification
For functional verification run make sim_full from work/
- Verification has been done using QuestaSim, adapt the Makefile in work/ if another toolset is used.

## Status
Currently only supports the F-Extension.

Compiled and tested for compliance using Apple Arm - Apple clang version 15.0

Compiled, tested for compliance and simulated using x86_64 - GCC 12.2.0 and QuestaSim 2020

For other architectures, adapt flags in work/Makefile
