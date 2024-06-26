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
#include "controller.h"

class FPU {
  private:
    FpuRf registerFile;
    FpuPipeline pipeline;
    FpuPredecoder predecoder;
    bool fpuReady;

  public:
    FPU();
    ~FPU();
    Controller controller;
    void resetFPU();
    void clockEvent();
    bool pollReady();

    void addAcceptedInstruction(uint32_t instruction, unsigned int id, unsignedType operand_a, unsignedType operand_b, unsignedType operand_c, unsigned int mode, bool commit_valid, unsigned int commit_id, bool commit_kill);//and other necessary inputs (should be somewhat close to in_xif type)

    //Issue/Commit interface
    void predecodeInstruction(uint32_t instruction, unsigned int id, bool& accept, bool& loadstore, bool& writeback, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c);
    void resetPredecoder();
    void commitInstruction(unsigned int id, bool kill);
    void executeStep();

    //Memory interface
    void pollMemoryRequest(bool& mem_valid, x_mem_req_t& mem_req);
    void resetMemoryRequest(unsigned int id);
    void writeMemoryResult(unsigned int id, loadType rdata, bool err, bool dbg);
    void writeMemoryResponse(bool mem_ready, bool exc, unsigned int exccode, bool dbg);
    void memoryStep();

    //Result interface
    void pollResult(bool& result_valid, x_result_t& result);
    void resetResult(unsigned int id);
    void writebackStep();

    //Backdoor functions
    FpuPipeObj testFloatOp();
    void bd_load(uint32_t instruction, loadType dataFromMem);
    FPNumber bd_getData(uint32_t addr);
    void bd_setRoundingMode(unsigned int rm);
    void bd_setFcsr(uint32_t data);
    uint32_t bd_getFcsr();
    std::vector<float> bd_getRF();
    unsigned int bd_getPipeStageId(int stage);
    unsigned int bd_getQueueStageId(int stage);
    unsigned int bd_getWaitingOpId();
};