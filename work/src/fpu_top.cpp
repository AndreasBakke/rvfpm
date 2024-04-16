/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model with FP registers, and parameterized number of pipelines
*/
#include "fpu_top.h"
#include <iostream>

FPU::FPU () : registerFile(NUM_F_REGS),  pipeline(&registerFile), predecoder(fpuReady), controller(registerFile, pipeline, fpuReady) {
  #ifndef ZFINX
    // registerFile(rfDepth)
  #else
    //Todo: Expand to support ZFINX
  #endif

  //check for illegal options
  if (WRITEBACK_STEP > EXECUTE_STEP){
    std::cerr << "Error: WRITEBACK_STEP cannot be before EXECUTE_STEP" << std::endl;
    exit(1);
  }
  if (WRITEBACK_STEP > MEMORY_STEP){
    std::cerr << "Error: WRITEBACK_STEP cannot be before MEMORY_STEP" << std::endl;
    exit(1);
  }
};

FPU::~FPU(){

};

void FPU::resetFPU(){
  #ifndef ZFINX
    registerFile.resetFpuRf();
  #endif
  pipeline.flush();
  predecoder.reset();
  controller.reset();
};


void FPU::clockEvent(){
  pipeline.step();
};

bool FPU::pollReady(){
  pipeline.stallCheck();
  return !pipeline.isStalled();
};

//--------------------------
// Issue interface
//--------------------------
void FPU::predecodeInstruction(uint32_t instruction, unsigned int id, bool& accept, bool& loadstore, bool& writeback, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c){
  predecoder.predecodeInstruction(instruction, id, accept, loadstore, writeback, use_rs_a, use_rs_b, use_rs_c);
};

void FPU::resetPredecoder(){
  predecoder.reset();
};


void FPU::commitInstruction(unsigned int id, bool kill){
  controller.commitInstruction(id, kill);
};

void FPU::executeStep(){
  pipeline.executeStep();
}

FpuPipeObj FPU::testFloatOp(){
  pipeline.step();
  return pipeline.getWaitingOp();
}


void FPU::addAcceptedInstruction(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c, unsigned int mode, bool commit_valid, unsigned int commit_id, bool commit_kill){ //and other necessary inputs (should be somewhat close to in_xif type)
  controller.addAcceptedInstruction(instruction, id, operand_a, operand_b, operand_c, mode, commit_valid, commit_id, commit_kill);
}

//--------------------------
// Memory interface
//--------------------------
void FPU::pollMemoryRequest(bool& mem_valid, x_mem_req_t& mem_req){
  controller.pollMemoryRequest(mem_valid, mem_req);
};

void FPU::resetMemoryRequest(unsigned int id){
  controller.resetMemoryRequest(id);
};


void FPU::writeMemoryResult(unsigned int id, uint32_t rdata, bool err, bool dbg){
  controller.writeMemoryResult(id, rdata, err, dbg);
};
void FPU::writeMemoryResponse(bool mem_ready, bool exc, unsigned int exccode, bool dbg){
  controller.writeMemoryResponse(mem_ready, exc, exccode, dbg);
};


// void FPU::writeMemRes(bool mem_ready, bool mem_result_valid, unsigned int id, unsigned int rdata, bool err, bool dbg){
//   pipeline.writeMemRes(mem_ready, mem_result_valid, id, rdata, err, dbg);
// };

void FPU::memoryStep(){
  pipeline.memoryStep();
}

//--------------------------
// Result interface
//--------------------------
void FPU::writeResult(bool result_ready){
  pipeline.writeResult(result_ready);
};

void FPU::pollResult(bool& result_valid, x_result_t& result){
  pipeline.pollResult(result_valid, result);
};

void FPU::resultStep(){
  pipeline.resultStep();
};

//--------------------------
// Backdoor functions
//--------------------------
void FPU::bd_load(uint32_t instruction, unsigned int dataFromMem){
  FpuPipeObj op = decodeOp(instruction, 0, 0, 0, 0, 0); //id is 0 for now
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

unsigned int FPU::bd_getWaitingOpId() {
  return pipeline.getWaitingOp().id;
}
