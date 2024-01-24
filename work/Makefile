CXX = g++
CXXFLAGS := -std=c++11 -fno-fast-math
LD_LIBRARY_PATH=/cad/gnu/gcc/12.2.0/dependencies_lib

COMPILER := $(shell g++ --version)
ifneq (,$(findstring clang,$(COMPILER)))
	CXXFLAGS := $(CXXFLAGS)
	ARCH = arm
	TININESS = -tininessbefore
else
	CXXFLAGS := $(CXXFLAGS) -frounding-math -fsignaling-nans -ffloat-store  -static-libstdc++
	ARCH = x86
	TININESS = -tininessafter
endif

#Filepaths for the c++ core
COREFILES = src/fpu_top.cpp src/fpu_pipeline.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp

TestFloat: #Compile and test for IEEE754 compliance
	make clean
	$(CXX) $(CXXFLAGS) -I ./include src/in_TestFloat.cpp $(COREFILES)  -o bin/rvfpm_testFloat -D TESTFLOAT
	chmod a+x,g+x ./bin/testfloat_ver_$(ARCH)
	chmod a+x,g+x ./bin/testfloat_gen_$(ARCH)
	make -i TestFloat_all args="-rnear_even"
	make -i TestFloat_all args="-rminMag"
	make -i TestFloat_all args="-rmin"
	make -i TestFloat_all args="-rmax"
	python3 tests/result_parser.py -arch=$(ARCH)

TestFloat_all:
	mkdir -p tests/$(ARCH)/rm$(args)
	./bin/testfloat_gen_$(ARCH) f32_add 	| ./bin/rvfpm_testFloat fadd  $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_add						-errors 0 > tests/$(ARCH)/rm$(args)/f32_add.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_sub 	| ./bin/rvfpm_testFloat fsub  $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_sub						-errors 0 > tests/$(ARCH)/rm$(args)/f32_sub.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_mul 	| ./bin/rvfpm_testFloat fmul  $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_mul $(TININESS)			-errors 0 > tests/$(ARCH)/rm$(args)/f32_mul.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_div 	| ./bin/rvfpm_testFloat fdiv  $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_div						-errors 0 > tests/$(ARCH)/rm$(args)/f32_div.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_sqrt 	| ./bin/rvfpm_testFloat fsqrt $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_sqrt					-errors 0 > tests/$(ARCH)/rm$(args)/f32_sqrt.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_eq  	| ./bin/rvfpm_testFloat feq   $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_eq						-errors 0 > tests/$(ARCH)/rm$(args)/f32_eq.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_lt  	| ./bin/rvfpm_testFloat flt   $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_lt						-errors 0 > tests/$(ARCH)/rm$(args)/f32_lt.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_le  	| ./bin/rvfpm_testFloat fle   $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_le						-errors 0 > tests/$(ARCH)/rm$(args)/f32_le.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_mulAdd 	| ./bin/rvfpm_testFloat fmadd $(args) 		| ./bin/testfloat_ver_$(ARCH) $(args)	f32_mulAdd $(TININESS)	-errors 0 > tests/$(ARCH)/rm$(args)/f32_muladd.txt 2>&1
	./bin/testfloat_gen_$(ARCH) ui32_to_f32 | ./bin/rvfpm_testFloat ui32_to_f32  $(args) | ./bin/testfloat_ver_$(ARCH) $(args)	ui32_to_f32					-errors 0 > tests/$(ARCH)/rm$(args)/f32_uitf.txt 2>&1
	./bin/testfloat_gen_$(ARCH) i32_to_f32  | ./bin/rvfpm_testFloat i32_to_f32   $(args) | ./bin/testfloat_ver_$(ARCH) $(args)	i32_to_f32 					-errors 0 > tests/$(ARCH)/rm$(args)/f32_itf.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_to_ui32 | ./bin/rvfpm_testFloat f32_to_ui32  $(args) | ./bin/testfloat_ver_$(ARCH) $(args)	f32_to_ui32 				-errors 0 > tests/$(ARCH)/rm$(args)/f32_ftui.txt 2>&1
	./bin/testfloat_gen_$(ARCH) f32_to_i32  | ./bin/rvfpm_testFloat f32_to_i32   $(args) | ./bin/testfloat_ver_$(ARCH) $(args)	f32_to_i32					-errors 0 > tests/$(ARCH)/rm$(args)/f32_fti.txt 2>&1


sharedLib: #Only tested for x86_64
	$(CXX) $(CXXFLAGS) -shared -fPIC -o bin/lib_rvfpm.so -I ./include src/dpi_wrapper.cpp $(COREFILES)

sim_full:
	make clean
	make sharedLib
	vlog -work work /work/anb7/prosjektoppgave/rvfpm/work/src/rvfpm.sv
	vlog -work work /work/anb7/prosjektoppgave/rvfpm/work/sim/inTest_rvfpm.sv
	vlog -work work /work/anb7/prosjektoppgave/rvfpm/work/sim/assertions_rvfpm.sv
	vlog -work work /work/anb7/prosjektoppgave/rvfpm/work/sim/testPr_rvfpm.sv
	vlog -work work /work/anb7/prosjektoppgave/rvfpm/work/sim/rvfpm_tb.sv
	vsim -sv_lib bin/lib_rvfpm -novopt work.rvfpm_tb -suppress 12110

clean:
	rm -f bin/rvfpm
	rm -f bin/rvfpm_D
	rm -f bin/rvfpm_Q
	rm -f bin/rvfpm_test
	rm -f bin/rvfpm_testFloat
	rm -f bin/lib_rvfpm.dylib
	rm -f bin/lib_rvfpm.so
	rm -f tests/*/*/*
