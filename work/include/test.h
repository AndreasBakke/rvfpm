#include "fpu_top.h"
#include <iostream> // Include necessary header files
using namespace std; // Use the standard namespace

void resetRegisters(FPU& fpuptr, std::vector<float>& cloneptr);

void verifyReset(FPU& fpuptr, std::vector<float> & cloneptr);
void testR4TYPE(FPU& fpuptr, std::vector<float>& cloneptr);
void testRTYPE(FPU& fpuptr, std::vector<float>& cloneptr);
