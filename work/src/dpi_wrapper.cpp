/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  DPI-C Wrapper for rvfpm.sv to interface with fpu_top.cpp.
*/


#include "fpu_top.h"
// #include "svdpi.h"
#include <iostream>
// #include <svdpi.h>
#include <functional>

extern "C" {

  void* create_fpu_model(){
    std::cout << "pipelineStages: " << NUM_PIPELINE_STAGES << " queueDepth: " << QUEUE_DEPTH << "  rfDepth: " << NUM_F_REGS <<std::endl;
    return new FPU(); //Return pointer to FPU
  };

  void destroy_fpu(void* fpu_ptr) {
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    delete fpu;
  }

  void reset_fpu(void* fpu_ptr){
    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
    fpu->resetFPU();
  };

  void clock_event(void* fpu_ptr){
    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
    fpu->clockEvent();
  };

  void poll_ready(void* fpu_ptr, bool& stalled){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    stalled = fpu->pollReady();
  };

  //-----------------------
  // ISSUE/COMMIT INTERFACE
  //-----------------------


  void add_accepted_instruction(void* fpu_ptr, uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c, unsigned int mode, bool commit_valid, unsigned int commit_id, bool commit_kill){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->addAcceptedInstruction(instruction, id, operand_a, operand_b, operand_c, mode, commit_valid, commit_id, commit_kill);
  };

  void reset_predecoder(void* fpu_ptr){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->resetPredecoder();
  };

  void predecode_instruction(void* fpu_ptr, uint32_t instruction, unsigned int id,  bool& accept, bool& loadstore, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->predecodeInstruction(instruction, id, accept, loadstore, use_rs_a, use_rs_b, use_rs_c);
  };

  void commit_instruction(void* fpu_ptr, unsigned int id, bool kill){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->commitInstruction(id, kill);
  };

  //-----------------------
  // MEM REQ/RES INTERFACE
  //-----------------------

  void poll_memory_request(void* fpu_ptr, bool& mem_valid, unsigned int& id,  unsigned int& addr, unsigned int& wdata, bool& last, unsigned int& size, unsigned int& mode, bool& we){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    x_mem_req_t mem_req = {};
    fpu->pollMemoryRequest(mem_valid, mem_req);
    id = mem_req.id;
    addr = mem_req.addr;
    wdata = mem_req.wdata;
    last = mem_req.last;
    size = mem_req.size;
    mode = mem_req.mode;
    we = mem_req.we;
  };

  void write_sv_state(void* fpu_ptr, bool mem_ready, bool result_ready){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->writeMemoryResponse(mem_ready, 0, 0, 0);
    fpu->writeResult(result_ready);
  };

  void write_memory_result(void* fpu_ptr, unsigned int id, uint32_t rdata, bool err, bool dbg){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->writeMemoryResult(id, rdata, err, dbg);
  };

  void memoryStep(void* fpu_ptr) {
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->memoryStep();
  };

  //-----------------------
  // RESULT INTERFACE
  //-----------------------

  void poll_res(void* fpu_ptr, bool& result_valid, unsigned int& id, unsigned int& data, unsigned int& rd, unsigned int& ecswe, unsigned int& ecsdata){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    x_result_t result = {};
    fpu->pollResult(result_valid, result);
    id = result.id;
    data = result.data;
    rd = result.rd;
    ecswe = result.ecswe;
    ecsdata = result.ecsdata;
  };

  void resultStep(void* fpu_ptr) {
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->resultStep();
  };

  //-----------------------
  // BACKDOOR FUNCTIONS
  //-----------------------

  unsigned int getRFContent(void* fpu_ptr, int reg) { //Backdoor to read content of register file
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    return  fpu->bd_getData(reg).bitpattern;
  }

  unsigned int getPipeStageId(void* fpu_ptr, int stage) { //Get id of instruction in pipeline stage
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    return fpu->bd_getPipeStageId(stage);
  }

  unsigned int getQueueStageId(void* fpu_ptr, int stage) { //Get id of instruction in queue stage
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    return fpu->bd_getQueueStageId(stage);
  }

  unsigned int getWaitingOpId(void* fpu_ptr) { //Get id of instruction in waiting queue
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    return fpu->bd_getWaitingOpId();
  }

  unsigned int randomFloat() { //Generate pseudorandom float
    uint32_t randomInt = random();
    int sign = rand()%2;
    return sign ? -randomInt : randomInt;
  }
}
