CXX = g++
CXXFLAGS := -std=c++11 -fno-fast-math
LD_LIBRARY_PATH=/cad/gnu/gcc/12.2.0/dependencies_lib

COMPILER := $(shell g++ --version)
ifneq (,$(findstring clang,$(COMPILER)))
	CXXFLAGS := $(CXXFLAGS)
	ARCH = arm
	TININESS = -tininessbefore
else
	CXXFLAGS := $(CXXFLAGS) -Wattributes -frounding-math -fsignaling-nans -ffloat-store  -static-libstdc++
	ARCH = x86
	TININESS = -tininessafter
endif


#Filepaths for the c++ core
COREFILES = src/fpu_top.cpp src/fpu_predecoder.cpp src/controller.cpp src/fpu_decode.cpp src/fpu_execute.cpp src/fpu_pipeline.cpp src/fpu_rf.cpp src/fp_number.cpp

CONFIG:=""
SIM_LENGTH = -all
N_GUI_OPTS = -c -do "run $(SIM_LENGTH); quit"
LEVEL= 1 #use LEVEL=2 for more rigerous testing - takes a long time (especially for mulAdd operations)

sim_full:
	make setup
	vlog -work work src/in_xif.sv
	vlog -work work src/config.svh
	vlog -work work src/rvfpm.sv
	vlog -work work sim/inTest_rvfpm.sv
	vlog -work work sim/assertions_rvfpm.sv
	vlog -work work sim/testPr_rvfpm.sv
	vlog -work work sim/rvfpm_tb.sv
	vsim -sv_lib bin/lib_rvfpm -novopt work.rvfpm_tb -suppress 12110 $(N_GUI_OPTS)

gui:
	make sim_full N_GUI_OPTS=""

TestFloat: #Compile and test for IEEE754 compliance
	make clean
	python3 run/setup.py run/testfloat_config.yaml
	$(CXX) $(CXXFLAGS) -I ./include src/in_TestFloat.cpp $(COREFILES)  -o bin/rvfpm_testFloat
	chmod a+x,g+x ./bin/testfloat_ver_$(ARCH)
	chmod a+x,g+x ./bin/testfloat_gen_$(ARCH)
	+make -i -j4 test_all fmt="S" fbits=32
	+make -i -j4 test_all fmt="D" fbits=64

	python3 tests/result_parser.py -arch=$(ARCH)

test_all: test_near_even test_minMag test_min test_max

test_near_even:
	+make -i TestFloat_all args="-rnear_even"

test_minMag:
	+make -i TestFloat_all args="-rminMag"

test_min:
	+make -i TestFloat_all args="-rmin"

test_max:
	+make -i TestFloat_all args="-rmax"

.PHONY: test_all test_near_even test_minMag test_min test_max result_parser

TestFloat_all:
	mkdir -p tests/$(ARCH)/$(fmt)/rm$(args)
	./bin/testfloat_gen_$(ARCH) f$(fbits)_add 		-level $(LEVEL)	| ./bin/rvfpm_testFloat fadd  $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_add										-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_add.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_sub 		-level $(LEVEL)	| ./bin/rvfpm_testFloat fsub  $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_sub										-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_sub.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_mul 		-level $(LEVEL)	| ./bin/rvfpm_testFloat fmul  $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_mul				$(TININESS)	-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_mul.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_div 		-level $(LEVEL)	| ./bin/rvfpm_testFloat fdiv  $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_div										-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_div.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_sqrt		-level $(LEVEL)	| ./bin/rvfpm_testFloat fsqrt $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_sqrt									-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_sqrt.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_eq  		-level $(LEVEL)	| ./bin/rvfpm_testFloat feq   $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_eq										-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_eq.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_lt  		-level $(LEVEL)	| ./bin/rvfpm_testFloat flt   $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_lt										-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_lt.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_le  		-level $(LEVEL)	| ./bin/rvfpm_testFloat fle   $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_le										-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_le.txt 2>&1
	./bin/testfloat_gen_$(ARCH) ui32_to_f$(fbits) -level $(LEVEL)	| ./bin/rvfpm_testFloat ui32_to_f$(fbits) $(fmt) $(args)	| ./bin/testfloat_ver_$(ARCH) $(args)	ui32_to_f$(fbits)								-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_uitf.txt 2>&1
	./bin/testfloat_gen_$(ARCH) i32_to_f$(fbits)  -level $(LEVEL)	| ./bin/rvfpm_testFloat i32_to_f$(fbits)  $(fmt) $(args)	| ./bin/testfloat_ver_$(ARCH) $(args)	i32_to_f$(fbits)								-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_itf.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_to_ui32 -level $(LEVEL)	| ./bin/rvfpm_testFloat f$(fbits)_to_ui32 $(fmt) $(args)	| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_to_ui32								-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_ftui.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_to_i32  -level $(LEVEL)	| ./bin/rvfpm_testFloat f$(fbits)_to_i32  $(fmt) $(args)	| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_to_i32								-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_fti.txt 2>&1
	./bin/testfloat_gen_$(ARCH) ui64_to_f$(fbits) -level $(LEVEL)	| ./bin/rvfpm_testFloat ui64_to_f$(fbits) $(fmt) $(args)	| ./bin/testfloat_ver_$(ARCH) $(args)	ui64_to_f$(fbits)								-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_ui64tf.txt 2>&1
	./bin/testfloat_gen_$(ARCH) i64_to_f$(fbits)  -level $(LEVEL)	| ./bin/rvfpm_testFloat i64_to_f$(fbits)  $(fmt) $(args)	| ./bin/testfloat_ver_$(ARCH) $(args)	i64_to_f$(fbits)								-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_i64tf.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_to_ui64 -level $(LEVEL)	| ./bin/rvfpm_testFloat f$(fbits)_to_ui64 $(fmt) $(args)	| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_to_ui64								-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_ftui64.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_to_i64  -level $(LEVEL)	| ./bin/rvfpm_testFloat f$(fbits)_to_i64  $(fmt) $(args)	| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_to_i64								-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_fti64.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f$(fbits)_mulAdd  -level $(LEVEL)	| ./bin/rvfpm_testFloat fmadd $(fmt) $(args)					| ./bin/testfloat_ver_$(ARCH) $(args)	f$(fbits)_mulAdd		$(TININESS)	-errors 0 > tests/$(ARCH)/$(fmt)/rm$(args)/f$(fbits)_muladd.txt 2>&1

setup:
	make clean
	python3 run/setup.py $(CONFIG)
	make sharedLib

sharedLib: #Only tested for x86_64
	$(CXX) $(CXXFLAGS) -shared -fPIC -o bin/lib_rvfpm.so -I ./include src/dpi_wrapper.cpp $(COREFILES)

clean:
	rm -f bin/rvfpm
	rm -f bin/rvfpm_D
	rm -f bin/rvfpm_Q
	rm -f bin/rvfpm_test
	rm -f bin/rvfpm_testFloat
	rm -f bin/lib_rvfpm.dylib
	rm -f bin/lib_rvfpm.so
	rm -f tests/*/*/*/*
