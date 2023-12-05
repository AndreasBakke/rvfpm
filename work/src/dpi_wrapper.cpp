/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    DPI-C Wrapper for rvfpm.sv to interface with fpu_top.cpp.
*/


#include "fpu_top.h"
#include <iostream>
// #include <svdpi.h>

extern "C" {
    void* create_fpu_model(int pipelineStages, int rfDepth){
        std::cout << "pipelineStages: " << pipelineStages << "  rfDepth: " << rfDepth <<std::endl; 
        return new FPU(pipelineStages, rfDepth); //Return pointer to FPU
    };

    void fpu_operation(void* fpu_ptr, unsigned int instruction, unsigned int id, uint32_t fromXReg, float fromMem, unsigned int* id_out, float* toMem, uint32_t* toXReg, bool* pipelineFull, bool* toMem_valid, bool* toXReg_valid){ //data only passed for operations using int(X)-registers
	    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
        fpu->operation(instruction, fromXReg, fromMem, toMem, toXReg, pipelineFull, toMem_valid, toXReg_valid);
    }

    void reset_fpu(void* fpu_ptr){
        FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
        fpu->resetFPU();
    };

    void destroy_fpu(void* fpu_ptr) {
        FPU* fpu = static_cast<FPU*>(fpu_ptr);
        delete fpu;
    }

    float getRFContent(void* fpu_ptr, int reg) { //Backdoor to read content of the entire fp_register
        FPU* fpu = static_cast<FPU*>(fpu_ptr);
        return  fpu->bd_getData(reg).f;
    }

    float randomFloat() { //Generate pseudorandom float (not available in SV.)
        uint32_t randomInt = random();
        int sign = rand()%2;
        return sign ? *reinterpret_cast<float*>(&randomInt) : - *reinterpret_cast<float*>(&randomInt); //Generate random float
    }
}
