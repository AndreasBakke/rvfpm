#include "fpu_top.h"
// #include <svdpi.h>

extern "C" {
    void* create_fpu_model(int pipelineStages, int rfDepth){
        return new FPU(pipelineStages, rfDepth); //Return pointer to FPU
    };

    //Todo: What should the operation return?
    //Thought: create a new pipe object that can be passsed by reference from fpu_operation to the interface in sv. then back to pipelineStep/others
    //So that it works "As a signal". going from operation to pipeline.
    void operation(void* fpu_ptr, uint32_t instruction, unsigned int id, int fromXReg, float fromMem, unsigned int* id_out, float* toMem, uint32_t* toXreg, bool* pipelineFull){ //data only passed for operations using int(X)-registers
        FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
        //Call operation function for fpu.
        fpu->operation(instruction, fromXReg, fromMem, toMem, toXreg, pipelineFull);
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