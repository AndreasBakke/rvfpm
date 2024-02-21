/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model with FP registers, and parameterized number of pipelines
*/
#include "fpu_top.h"
#include <iostream>

FPU::FPU (int pipelineStages, int queueDepth, int rfDepth) : registerFile(rfDepth),  pipeline(pipelineStages, queueDepth, &registerFile), predecoder(fpuReady) {
  #ifndef ZFINX
    // registerFile(rfDepth)
  #else
    //Todo: Expand to support ZFINX
  #endif
};

FPU::~FPU(){

};

void FPU::resetFPU(){
  #ifndef ZFINX
    registerFile.resetFpuRf();
    //reset queue
  #endif
  //TODO: reset new features aswell
  pipeline.flush();
  predecoder.reset(); //TODO: should this be deleted?
  //Reset pipeline queue (update flush)
};


void FPU::clockEvent(bool& fpu_ready){
  pipeline.step();
  fpuReady = !pipeline.isStalled();
  fpu_ready = fpuReady;
};

void FPU::predecodeInstruction(uint32_t instruction, unsigned int id){ //TODO: wait for opready
  predecoder.predecodeInstruction(instruction, id);
};

void FPU::pollPredecoderResult(x_issue_resp_t& resp_ref, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c){
  predecoder.pollPredecoderResult(resp_ref, use_rs_a, use_rs_b, use_rs_c);
};

void FPU::resetPredecoder(){
  predecoder.reset();
};

FpuPipeObj FPU::testFloatOp(){
  return pipeline.step();
}


void FPU::addAcceptedInstruction(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c){ //and other necessary inputs (should be somewhat close to in_xif type)
  FpuPipeObj newOp = decodeOp(instruction, id, operand_a, operand_b, operand_c);
  // newOp.id = id;
  if (pipeline.getQueueDepth() > 0){
    pipeline.addOpToQueue(newOp);
  }
  else {
    pipeline.setWaitingOp(newOp); //set waitingOp (if queue=0, this will be empty given the instruction is accepted
  }
}

void FPU::pollMemReq(bool& mem_valid, x_mem_req_t& mem_req){
  pipeline.pollMemReq(mem_valid, mem_req);
};

void FPU::writeMemRes(bool mem_ready, bool mem_result_valid, unsigned int id, unsigned int rdata, bool err, bool dbg){
  pipeline.writeMemRes(mem_ready, mem_result_valid, id, rdata, err, dbg);
};

void FPU::pollResult(bool& result_valid, x_result_t& result){
  pipeline.pollResult(result_valid, result);
};

//Backdoor functions

void FPU::bd_load(uint32_t instruction, unsigned int dataFromMem){
  FpuPipeObj op = decodeOp(instruction, 0, 0, 0, 0); //id is 0 for now
  op.data.bitpattern = dataFromMem;
  registerFile.write(op.addrTo, op.data);
};


FPNumber FPU::bd_getData(uint32_t addr){
  return registerFile.read(addr);
};
void FPU::bd_setRoundingMode(unsigned int rm){
  registerFile.setfrm(rm);
};

void FPU::bd_setFcsr(uint32_t data) {
  registerFile.write_fcsr(data);
}

uint32_t FPU::bd_getFcsr() {
  return registerFile.read_fcsr().v;
}




std::vector<float> FPU::bd_getRF(){
  return registerFile.getRf();
};

unsigned int FPU::bd_getPipeStageId(int stage) {
  return pipeline.getId_pipeline(stage);
}

unsigned int FPU::bd_getQueueStageId(int stage) {
  return pipeline.getId_operationQueue(stage);
}
