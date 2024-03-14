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
    FPU();
    ~FPU();
    void resetFPU();
    void clockEvent(bool& fpu_ready);

    void addAcceptedInstruction(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c);//and other necessary inputs (should be somewhat close to in_xif type)

    //Issue/Commit interface
    void predecodeInstruction(uint32_t instruction, unsigned int id, x_issue_resp_t& resp_ref, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c);
    void pollPredecoderResult(x_issue_resp_t& resp_ref, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c);
    void resetPredecoder();
    void commitInstruction(unsigned int id, bool kill);

    //Memory interface
    void pollMemReq(bool& mem_valid, x_mem_req_t& mem_req);
    void writeMemRes(bool mem_ready, bool mem_result_valid, unsigned int id, unsigned int rdata, bool err, bool dbg);

    //Result interface
    void writeResult(bool result_ready);
    void pollResult(bool& result_valid, x_result_t& result);

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