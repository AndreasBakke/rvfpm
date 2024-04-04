/*  rvfpm - 2024
  Andreas S. Bakke

  Description

*/

#include "controller.h"
#include "fpu_decode.h"
#include "fpu_pipe.h"

Controller::Controller(FpuRf& rf, FpuPipeline& pipe, bool& ready) : registerFile(rf), fpu_pipeline(pipe), fpuReady(ready){

}


void Controller::reset(){
  mem_valid = false;
  mem_ready = false;
  mem_req = {};
}



void Controller::addAcceptedInstruction(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c, unsigned int mode, bool commit_valid, unsigned int commit_id, bool commit_kill){ //and other necessary inputs (should be somewhat close to in_xif type)
  FpuPipeObj newOp = decodeOp(instruction, id, operand_a, operand_b, operand_c, mode);
  if (newOp.id == fpu_pipeline.at(0).id && !fpu_pipeline.at(0).isEmpty()){
    return;
  }

  if (newOp.toMem || newOp.fromMem){
    mem_valid = 1;
    mem_req.id = newOp.id;
    mem_req.addr = newOp.addrFrom.front();
    newOp.data.bitpattern = registerFile.read(newOp.addrFrom.front()).bitpattern;
    mem_req.wdata = newOp.toMem ? newOp.data.bitpattern : 0;
    mem_req.last = 1;
    mem_req.size = newOp.size;
    mem_req.mode = newOp.mode;
  } else {
    mem_valid = 0;
    mem_req = {};
  }

  if (commit_valid && commit_id == newOp.id){
    if (commit_kill){
      newOp = {};
    } else {
      newOp.speculative = 0;
    }
  }
  if (fpu_pipeline.getQueueDepth() > 0){
    fpu_pipeline.addOpToQueue(newOp);
  }
  else {
    fpu_pipeline.setWaitingOp(newOp); //set waitingOp (if queue=0, this will be empty given the instruction is accepted
  }
  if (fpu_pipeline.isEmpty()){
    fpu_pipeline.step();
    fpu_pipeline.step();
  }
}



//--------------------
// Commit interface
//--------------------
void Controller::commitInstruction(unsigned int id, bool kill){
  fpu_pipeline.commitInstruction(id, kill);
};

void Controller::writeMemoryResult(unsigned int id, uint32_t rdata, bool err, bool dbg) {
  // std::cout << "wmr - id: " << id << " - rdata: " << rdata << std::endl;
  if (err) {
    std::cout << "Error in memory result - id: " << id << std::endl;
    return;
  }

  for (int i = 0; i < fpu_pipeline.getNumStages(); i++) {
    if (fpu_pipeline.at(i).id == id) {
      fpu_pipeline.at(i).mem_result_valid = 1;
      fpu_pipeline.at(i).mem_result = rdata;
      return;
    }
  }

  if (fpu_pipeline.getWaitingOp().id == id) {
    FpuPipeObj tmpOp = fpu_pipeline.getWaitingOp();
    tmpOp.mem_result_valid = 1;
    tmpOp.mem_result = rdata;
    fpu_pipeline.setWaitingOp(tmpOp);
    return;
  }

  for (int i = 0; i < fpu_pipeline.getQueueDepth(); i++) {
    if (fpu_pipeline.at_queue(i).id == id) {
      fpu_pipeline.at_queue(i).mem_result_valid = 1;
      fpu_pipeline.at_queue(i).mem_result = rdata;
      return;
    }
  }
}

void Controller::pollMemoryRequest(bool& mem_valid, x_mem_req_t& mem_req){
  mem_valid = this->mem_valid;
  mem_req = this->mem_req;
};

void Controller::writeMemoryResponse(bool mem_ready, bool exc, unsigned int exccode, bool dbg){
  this->mem_ready = mem_ready;
  this->mem_valid = mem_valid && !mem_ready;
  if (exc) {
    std::cout << "Exception in memory request - id: " << this->mem_req.id << std::endl;
    std::cout << "Exception code: " << exccode << std::endl;
  }
};