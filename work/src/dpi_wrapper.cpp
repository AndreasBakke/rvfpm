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

    void fpu_operation(void* fpu_ptr, unsigned int instruction, unsigned int id, int fromXReg, float fromMem, unsigned int* id_out, float* toMem, uint32_t* toXReg, bool* pipelineFull){ //data only passed for operations using int(X)-registers
	    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
        fpu->operation(instruction, fromXReg, fromMem, toMem, toXReg, pipelineFull);
        // return fpu->operation(instruction, fromXReg, fromMem);
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
}