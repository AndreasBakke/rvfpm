/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    RISC-V Floating Point Unit Model with FP registers, and parameterized number of pipelines
*/
#include "fpu_rf.h"
#include "fpu_pipeline.h"
#include "fpu_operations.h"


class FPU {
  private:
    FpuPipeline pipeline;
    FpuRf registerFile;

  public:
    FPU(int pipelineStages, int rfDepth=32);
    ~FPU();
    void resetFPU();
    FpuPipeObj operation(uint32_t instruction, unsigned int id, int fromXReg, unsigned int fromMem, unsigned int* id_out, uint32_t* toMem, uint32_t* toXReg, bool* pipelineFull, bool* toMem_valid, bool* toXReg_valid); //add toXReg and toMem
    FpuPipeObj decodeOp(uint32_t instruction, unsigned int id);
    void executeOp(FpuPipeObj& op, unsigned int fromMem, int fromXReg, unsigned int* id_out, uint32_t* toMem, uint32_t* toXReg, bool* toMem_valid, bool* toXReg_valid);

    //Backdoor functions
    FPNumber bd_getData(uint32_t addr);
    void bd_setRoundingMode(unsigned int rm);
    void bd_setFcsr(uint32_t data);
    uint32_t bd_getFcsr();
    std::vector<float> bd_getRF();
    unsigned int bd_getPipeStageId(int stage);

};