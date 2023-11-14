#include "fpu_rf.h"
#include "fp_number.h"
#include "fpu_pipe.h"
#include "fpu_operations.h"
#include <cstdint>
#include <deque> //Double ended queue

class FPU {
  private:
    FpuRf registerFile;
    std::deque<FpuPipeObj> pipeline;
    

  public:
    FPU(int pipelineStages, int rfDepth=32);
    ~FPU();
    void resetFPU();
    FpuPipeObj operation(uint32_t instruction, int fromXReg, float fromMem, float* toMem, uint32_t* toXreg, unsigned int* flags_out); //add toXreg and toMem
    FpuPipeObj decodeOp(uint32_t instruction, int fromXReg, float fromMem);
    void executeOp(FpuPipeObj op, float* toMem, uint32_t* toXreg, unsigned int* flags_out);

    //Pipeline operations
    FpuPipeObj pipelineStep(FpuPipeObj nextOp); //Advance pipeline by one step (called by clock in interface)
    int pipelineFlush(); //Flush pipeline


    //Backdoor functions
    FPNumber bd_getData(uint32_t addr);
    void bd_setRoundingMode(unsigned int rm);

};