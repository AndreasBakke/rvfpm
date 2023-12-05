CXX = g++
CXXFLAGS = -std=c++11
LD_LIBRARY_PATH=/cad/gnu/gcc/12.2.0/dependencies_lib

#TODO: for everything except test - do not link. Linking to be done later when interface has been completed.

test:  #run using args="-D EXT_D/Q" to test with D/Q
	$(CXX) $(CXXFLAGS) -fno-fast-math -O2 -I ./include src/test.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp  -o bin/rvfpm_test $(args) 
	./bin/rvfpm_test

test_no_rounding:  #run using args="-D EXT_D/Q" to test with D/Q
	$(CXX) $(CXXFLAGS) -fno-fast-math -O2 -I ./include src/test.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp  -o bin/rvfpm_test -D NO_ROUNDING
	./bin/rvfpm_test

test_RHEL:  #run using args="-D EXT_D/Q" to test with D/Q
	$(CXX) $(CXXFLAGS) -static-libstdc++  -fno-fast-math -frounding-math -fsignaling-nans -O2  -I ./include src/test.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp  -o bin/rvfpm_test_RHEL $(args) 
	./bin/rvfpm_test_RHEL

TestFloat: #Compile and test for IEEE754 compliance
	$(CXX) $(CXXFLAGS) -fno-fast-math -O2 -I ./include src/in_TestFloat.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp  -o bin/rvfpm_testFloat -D TESTFLOAT
	make TestFloat_all args="-rnear_even"
	make TestFloat_all args="-rminMag"
	make TestFloat_all args="-rmin"
	make TestFloat_all args="-rmax"

testMake: 
	$(CXX) $(CXXFLAGS) --target=x86_64-pc-linux-gnu -static-libstdc++ -fno-fast-math -O2 -I ./include src/in_TestFloat.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp  -o bin/rvfpm_testFloat_RHEL -D TESTFLOAT
	make TestFloat_all_RHEL args="-rnear_even"
	make TestFloat_all_RHEL args="-rminMag"
	make TestFloat_all_RHEL args="-rmin"
	make TestFloat_all_RHEL args="-rmax"

TestFloat_RHEL:
	$(CXX) $(CXXFLAGS) -static-libstdc++ -fno-fast-math -frounding-math -fsignaling-nans -O2  -I ./include src/in_TestFloat.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp  -o bin/rvfpm_testFloat_RHEL -D TESTFLOAT 
	make TestFloat_all_RHEL args="-rnear_even"
	make TestFloat_all_RHEL args="-rminMag"
	make TestFloat_all_RHEL args="-rmin"
	make TestFloat_all_RHEL args="-rmax"

TestFloat_all:
	./bin/testfloat_gen_arm f32_add 	| ./bin/rvfpm_testFloat fadd  $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_add
	./bin/testfloat_gen_arm f32_sub 	| ./bin/rvfpm_testFloat fsub  $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_sub
	./bin/testfloat_gen_arm f32_mul 	| ./bin/rvfpm_testFloat fmul  $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_mul -tininessbefore
	./bin/testfloat_gen_arm f32_div 	| ./bin/rvfpm_testFloat fdiv  $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_div
	./bin/testfloat_gen_arm f32_sqrt 	| ./bin/rvfpm_testFloat fsqrt $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_sqrt
	./bin/testfloat_gen_arm f32_eq  	| ./bin/rvfpm_testFloat feq   $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_eq
	./bin/testfloat_gen_arm f32_lt  	| ./bin/rvfpm_testFloat flt   $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_lt
	./bin/testfloat_gen_arm f32_le  	| ./bin/rvfpm_testFloat fle   $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_le
	./bin/testfloat_gen_arm f32_mulAdd 	| ./bin/rvfpm_testFloat fmadd $(args) 		| ./bin/testfloat_ver_arm $(args)	f32_mulAdd -tininessbefore
	./bin/testfloat_gen_arm ui32_to_f32 | ./bin/rvfpm_testFloat ui32_to_f32  $(args) | ./bin/testfloat_ver_arm $(args)	ui32_to_f32
	./bin/testfloat_gen_arm i32_to_f32  | ./bin/rvfpm_testFloat i32_to_f32   $(args) | ./bin/testfloat_ver_arm $(args)	i32_to_f32 
	./bin/testfloat_gen_arm f32_to_ui32 | ./bin/rvfpm_testFloat f32_to_ui32  $(args) | ./bin/testfloat_ver_arm $(args)	f32_to_ui32 
	./bin/testfloat_gen_arm f32_to_i32  | ./bin/rvfpm_testFloat f32_to_i32   $(args) | ./bin/testfloat_ver_arm $(args)	f32_to_i32



TestFloat_all_l2: #NOTE: Takes WAY too long for mulAdd function
	./bin/testfloat_gen_arm f32_add 	-level 2 | ./bin/rvfpm_testFloat fadd  $(args) | ./bin/testfloat_ver_arm $(args)  	f32_add
	./bin/testfloat_gen_arm f32_sub 	-level 2 | ./bin/rvfpm_testFloat fsub  $(args) | ./bin/testfloat_ver_arm $(args)		f32_sub
	./bin/testfloat_gen_arm f32_mul 	-level 2 | ./bin/rvfpm_testFloat fmul  $(args) | ./bin/testfloat_ver_arm $(args)		f32_mul -tininessbefore
	./bin/testfloat_gen_arm f32_div 	-level 2 | ./bin/rvfpm_testFloat fdiv  $(args) | ./bin/testfloat_ver_arm $(args)		f32_div
	./bin/testfloat_gen_arm f32_sqrt 	-level 2 | ./bin/rvfpm_testFloat fsqrt $(args) | ./bin/testfloat_ver_arm $(args)		f32_sqrt
	./bin/testfloat_gen_arm f32_eq  	-level 2 | ./bin/rvfpm_testFloat feq   $(args) | ./bin/testfloat_ver_arm $(args)		f32_eq
	./bin/testfloat_gen_arm f32_lt  	-level 2 | ./bin/rvfpm_testFloat flt   $(args) | ./bin/testfloat_ver_arm $(args)		f32_lt
	./bin/testfloat_gen_arm f32_le  	-level 2 | ./bin/rvfpm_testFloat fle   $(args) | ./bin/testfloat_ver_arm $(args)		f32_le
	./bin/testfloat_gen_arm f32_mulAdd 	-level 2 | ./bin/rvfpm_testFloat fmadd $(args) | ./bin/testfloat_ver_arm $(args)  	f32_mulAdd -tininessbefore


TestFloat_all_RHEL:
	./bin/testfloat_gen_x86 f32_add 	| ./bin/rvfpm_testFloat_RHEL fadd  $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_add
	./bin/testfloat_gen_x86 f32_sub 	| ./bin/rvfpm_testFloat_RHEL fsub  $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_sub
	./bin/testfloat_gen_x86 f32_mul 	| ./bin/rvfpm_testFloat_RHEL fmul  $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_mul -tininessbefore
	./bin/testfloat_gen_x86 f32_div 	| ./bin/rvfpm_testFloat_RHEL fdiv  $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_div
	./bin/testfloat_gen_x86 f32_sqrt 	| ./bin/rvfpm_testFloat_RHEL fsqrt $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_sqrt
	./bin/testfloat_gen_x86 f32_eq  	| ./bin/rvfpm_testFloat_RHEL feq   $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_eq
	./bin/testfloat_gen_x86 f32_lt  	| ./bin/rvfpm_testFloat_RHEL flt   $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_lt
	./bin/testfloat_gen_x86 f32_le  	| ./bin/rvfpm_testFloat_RHEL fle   $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_le
	./bin/testfloat_gen_x86 f32_mulAdd 	| ./bin/rvfpm_testFloat_RHEL fmadd $(args) 		| ./bin/testfloat_ver_x86 $(args)	f32_mulAdd -tininessbefore
	./bin/testfloat_gen_x86 ui32_to_f32 | ./bin/rvfpm_testFloat_RHEL ui32_to_f32  $(args) | ./bin/testfloat_ver_x86 $(args)	ui32_to_f32
	./bin/testfloat_gen_x86 i32_to_f32  | ./bin/rvfpm_testFloat_RHEL i32_to_f32   $(args) | ./bin/testfloat_ver_x86 $(args)	i32_to_f32 
	./bin/testfloat_gen_x86 f32_to_ui32 | ./bin/rvfpm_testFloat_RHEL f32_to_ui32  $(args) | ./bin/testfloat_ver_x86 $(args)	f32_to_ui32 
	./bin/testfloat_gen_x86 f32_to_i32  | ./bin/rvfpm_testFloat_RHEL f32_to_i32   $(args) | ./bin/testfloat_ver_x86 $(args)	f32_to_i32

macSL:
	$(CXX) $(CXXFLAGS) -shared -undefined dynamic_lookup -fno-fast-math -O2 -o bin/lib_rvfpm.dylib -I ./include src/dpi_wrapper.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp

testSL:
	$(CXX) $(CXXFLAGS) -arch x86_64 -shared -undefined dynamic_lookup -fno-fast-math -O2 -o bin/lib_rvfpm_2.so -I ./include src/dpi_wrapper.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp
rhelSL:
	$(CXX) $(CXXFLAGS) -shared -fPIC -static-libstdc++ -frounding-math -fsignaling-nans -O2 -o bin/lib_rvfpm.so -I ./include src/dpi_wrapper.cpp src/fpu_top.cpp src/fpu_rf.cpp src/fpu_operations.cpp src/fp_number.cpp

sim_full:
	make clean
	make rhelSL
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
	rm -f bin/rvfpm_testFloat_RHEL
	rm -f bin/lib_rvfpm.dylib
	rm -f bin/lib_rvfpm.so
