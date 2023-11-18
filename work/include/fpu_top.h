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
    int numPipeStages;
    

  public:
    FPU(int pipelineStages, int rfDepth=32);
    ~FPU();
    void resetFPU();
    FpuPipeObj operation(uint32_t instruction, int fromXReg, float fromMem, float* toMem, uint32_t* toXReg, bool* pipelineFull); //add toXReg and toMem
    FpuPipeObj decodeOp(uint32_t instruction);
    void executeOp(FpuPipeObj& op, float fromMem, int fromXReg, float* toMem, uint32_t* toXReg);


    //Pipeline operations
    FpuPipeObj pipelineStep(FpuPipeObj nextOp, bool* pipelineFull); //Advance pipeline by one step (called by clock in interface)
    int pipelineFlush(); //Flush pipeline


    //Backdoor functions
    FPNumber bd_getData(uint32_t addr);
    void bd_setRoundingMode(unsigned int rm);
    std::vector<float> bd_getRF();

};