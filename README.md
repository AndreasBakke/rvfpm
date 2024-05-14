# RISC-V Floating-point Coprocessor Model (rvfpm)
The RISC-V Floating-point Coprocessor Model (rvfpm) is a customizable model for verification, design exploration and performance modeling. It provides an easy to configure model, supporting the RISC-V "F" and "Zfinx" extensions, a variety of pipeline structures and optimizations using the eXtension Interface for ease of integration.

## Status
Compiled, tested and verified on x86_64 using GCC 12.0.0. Ibex verification done in [Ibex fork](https://github.com/AndreasBakke/rvfpm_ibex_testing).
|Extension|Status| Comment |
|---|---|---|
|"F"|Implemented. 100% compliance using Berkeley Testfloat| Default extension.|
|"Zfinx"| Implemented. | Verified functionally using Ibex. |
|"D", "Q" |In progress.| Branch "precisions"|
|"Zfh", "Zfhmin"| In progress.| Branch "precisions"|
|"Zfa"|Not started.||
|"bfloat"|Not started.||

## Structure
### Core
![Core structure of the RISC-V Floating-point Coprocessor Model](doc/core_w_controller.png)
The rvfpm core is structured as above. All files relevant to it, and its verification can be found in work/. Headerfiles are located in work/include, and need to be included during compilation. Core cpp files are located in work/src.

### Interface
The RISC-V Floating-point Coprocessor Model implements all but the compressed interface of the eXtension Interface. Using [rvfpm.sv](work/src/rvfpm.sv) located in work/src, relevant functions in the C++ core are called using DPI-C. 

### Configuration and compilation
Prerequisites:
- Python
- Pyyaml: ```pip3 install pyyaml```
- Simulator tool - depending on use-case of rvfpm.

The core is configured using Yaml configurations in work/run. If no config is specified using CONFIG="path/to/config/" the [default config](work/run/default_config.yaml) will be used. 

Depending on the target, adapt compiler, flags and simulation tools in the [Makefile](work/Makefile) in work/.

By running ```make setup CONFIG=<optional_path>```, the C++ core is compiled into a shared library which can be loaded into a variety of simulators (verified for QuestaSim 2020) along with _config.svh_, _in_xif.sv_ and _rvfpm&#46;sv_ .


### Compliance testing
Testing for compliance to the IEEE 754-2008 standard done using Berkeley TestFloat. Binaries are located in work/bin. The [C++ interface](work/src/in_TestFloat.cpp) for the test-suite available in work/src, note that conversion functions are partially generated by ChatGPT.

To test the system using Berkeley Testfloat, run

```make TestFloat LEVEL=1/2```

Results are written to work/tests/\<target\>/rm-\<rounding mode\>/\<test\>.txt

A summary of tests performed, and errors encountered can be seen below. Refer to the summary in work/tests for additional details.

|Extension|Tests preformed|Errors|
|---|---|---|
F | 58,260,633,824 | 0
D | 0 | 0

### Functional verification
For instructions not available in Berkeley Testfloat, functional verification were done by integrating rvfpm into lowRISCs [Ibex](https://github.com/lowRISC/ibex) in a seperate [Ibex fork](https://github.com/AndreasBakke/rvfpm_ibex_testing).