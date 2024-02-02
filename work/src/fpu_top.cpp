/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model with FP registers, and parameterized number of pipelines
*/
#include "fpu_top.h"
#include <iostream>

FPU::FPU (int pipelineStages, int queueDepth, int rfDepth) : registerFile(rfDepth),  pipeline(pipelineStages, queueDepth, &registerFile) {
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
  pipeline.flush();
};


void FPU::clockEvent(){
  pipeline.step();
  //pipeline checkForHazards
};

FpuPipeObj FPU::testFloatOp(){
  return pipeline.step();
}


void FPU::addAcceptedInstruction(uint32_t instruction, unsigned int id){ //and other necessary inputs (should be somewhat close to in_xif type)
  FpuPipeObj newOp = decodeOp(instruction, 0); //id is 0 for now
  newOp.id = id;
  if (pipeline.getQueueDepth() > 0){
    pipeline.addOpToQueue(newOp);
  }
  else {
    pipeline.setWaitingOp(newOp); //set waitingOp (if queue=0, this will be empty given the instruction is accepted

  }
}



//Backdoor functions

void FPU::bd_load(uint32_t instruction, unsigned int dataFromMem){
  FpuPipeObj op = decodeOp(instruction, 0); //id is 0 for now
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
