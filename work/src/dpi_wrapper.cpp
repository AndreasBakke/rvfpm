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

    void fpu_operation(void* fpu_ptr, uint32_t instruction, unsigned int id, uint32_t fromXReg, unsigned int fromMem, unsigned int* id_out, uint32_t* toMem, uint32_t* toXReg, bool* pipelineFull, bool* toMem_valid, bool* toXReg_valid){ //data only passed for operations using int(X)-registers
	    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
        fpu->operation(instruction, id, fromXReg, fromMem, id_out, toMem, toXReg, pipelineFull, toMem_valid, toXReg_valid);
    }

    void reset_fpu(void* fpu_ptr){
        FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
        fpu->resetFPU();
    };

    void destroy_fpu(void* fpu_ptr) {
        FPU* fpu = static_cast<FPU*>(fpu_ptr);
        delete fpu;
    }

    unsigned int getRFContent(void* fpu_ptr, int reg) { //Backdoor to read content of the entire fp_register
        FPU* fpu = static_cast<FPU*>(fpu_ptr);
        return  fpu->bd_getData(reg).bitpattern;
    }

    unsigned int getPipeStageId(void* fpu_ptr, int stage) {
        FPU* fpu = static_cast<FPU*>(fpu_ptr);
        return fpu->bd_getPipeStageId(stage);
    }

    unsigned int randomFloat() { //Generate pseudorandom float (not available in SV.)
        uint32_t randomInt = random();
        int sign = rand()%2;
        return randomInt; //Generate random float
    }
}
