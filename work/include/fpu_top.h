/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    RISC-V Floating Point Unit Model with FP registers, and parameterized number of pipelines
*/
#include "fpu_rf.h"
#include "fpu_pipeline.h"
#include "fpu_decode.h"
#include "fpu_execute.h"
#include "fpu_predecoder.h"

class FPU {
  private:
    FpuRf registerFile;
    FpuPipeline pipeline;
    FpuPredecoder predecoder;
    bool fpuReady;


  public:
    FPU(int pipelineStages=4, int queueDepth=0, int rfDepth=32);
    ~FPU();
    void resetFPU();
    void clockEvent();
    void addAcceptedInstruction(uint32_t instruction, unsigned int id);//and other necessary inputs (should be somewhat close to in_xif type)
    void predecodeInstruction(uint32_t instruction, unsigned int id);
    void pollPredecoderResult(bool& accept_ref, x_issue_resp_t& resp_ref);

    //Backdoor functions
    FpuPipeObj testFloatOp();
    void bd_load(uint32_t instruction, unsigned int dataFromMem);
    FPNumber bd_getData(uint32_t addr);
    void bd_setRoundingMode(unsigned int rm);
    void bd_setFcsr(uint32_t data);
    uint32_t bd_getFcsr();
    std::vector<float> bd_getRF();
    unsigned int bd_getPipeStageId(int stage);
    unsigned int bd_getQueueStageId(int stage);

};