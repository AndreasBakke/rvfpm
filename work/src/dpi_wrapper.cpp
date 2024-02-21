/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  DPI-C Wrapper for rvfpm.sv to interface with fpu_top.cpp.
*/


#include "fpu_top.h"
// #include "svdpi.h"
#include <iostream>
// #include <svdpi.h>

extern "C" {
  void* create_fpu_model(int pipelineStages, int queueDepth, int rfDepth){
    std::cout << "pipelineStages: " << pipelineStages << " queueDepth: " << queueDepth << "  rfDepth: " << rfDepth <<std::endl;
    return new FPU(pipelineStages, queueDepth, rfDepth); //Return pointer to FPU
  };

  void reset_fpu(void* fpu_ptr){
    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
    fpu->resetFPU();
  };

  void clock_event(void* fpu_ptr, bool& fpu_ready){
    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
    fpu->clockEvent(fpu_ready);
  };

  void add_accepted_instruction(void* fpu_ptr, uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->addAcceptedInstruction(instruction, id, operand_a, operand_b, operand_c);
  };

  void reset_predecoder(void* fpu_ptr){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->resetPredecoder();
  };

  void predecode_instruction(void* fpu_ptr, uint32_t instruction, unsigned int id){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->predecodeInstruction(instruction, id);
  };

  void poll_predecoder_result(void* fpu_ptr, x_issue_resp_t& resp, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->pollPredecoderResult(resp, use_rs_a, use_rs_b, use_rs_c);
  };

  void poll_mem_req(void* fpu_ptr, bool& mem_valid, unsigned int& id,  unsigned int& addr, unsigned int& wdata){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    x_mem_req_t mem_req = {};
    fpu->pollMemReq(mem_valid, mem_req);
    id = mem_req.id;
    addr = mem_req.addr;
    wdata = mem_req.wdata;
  };

  void write_mem_res(void* fpu_ptr, bool mem_ready, bool mem_result_valid, unsigned int id, unsigned int rdata, bool err, bool dbg){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->writeMemRes(mem_ready, mem_result_valid, id, rdata, err, dbg);
  };

  void poll_res(void* fpu_ptr, bool& result_valid, unsigned int& id, unsigned int& data, unsigned int& rd){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    x_result_t result = {};
    fpu->pollResult(result_valid, result);
    id = result.id;
    data = result.data;
    rd = result.rd;
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

  unsigned int getQueueStageId(void* fpu_ptr, int stage) {
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    return fpu->bd_getQueueStageId(stage);
  }

  unsigned int randomFloat() { //Generate pseudorandom float (not available in SV.)
    uint32_t randomInt = random();
    int sign = rand()%2;
    return randomInt; //Generate random float
  }
}
