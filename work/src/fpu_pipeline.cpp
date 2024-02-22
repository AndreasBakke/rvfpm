/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Fpu pipeline
*/

#include "fpu_pipeline.h"
#include "fpu_config.h"
#include <iostream>


FpuPipeline::FpuPipeline(FpuRf* rf_pointer) : pipeline(NUM_PIPELINE_STAGES), operationQueue(QUEUE_DEPTH) {
  pipeline = std::deque<FpuPipeObj>(NUM_PIPELINE_STAGES, FpuPipeObj({}));//Initialize empty pipeline
  operationQueue = std::deque<FpuPipeObj>(QUEUE_DEPTH, FpuPipeObj({}));//Initialize empty queue
  registerFilePtr = rf_pointer;
  waitingOp = FpuPipeObj({}); //Initialize empty waiting op
  pipelineFull = false;
  stalled = false;
  mem_stalled = false;
  ex_stalled = false;
}

FpuPipeline::~FpuPipeline() {
}

FpuPipeObj FpuPipeline::step(){
  //Operations are decoded before adding to the pipeline
  //Check for memory dependencies and request throough interface
  //Pipeline stucture set in run/setup.yaml TODO:actually make it set in run/setup.yaml

  if (NUM_PIPELINE_STAGES == 0) {
    executeOp(waitingOp, registerFilePtr, mem_valid, mem_req);
    registerFilePtr->write(waitingOp.addrTo, waitingOp.data); //only if not mem
    return waitingOp;
  }

  //Do some stall checking here
  //TODO:if load/store. keep mem_valid 1 until mem_ready is 1.
  if (!stalled){
    mem_valid = 0;
    this->mem_req = {};
    executeOp(pipeline.at(EXECUTE_STEP), registerFilePtr, mem_valid, this->mem_req); //Compute operation at execute stage. //Issue memory request to CPU at this stage
    if (pipeline.at(EXECUTE_STEP).remaining_ex_cycles > 0){
      ex_stalled = true;
    } else {
      ex_stalled = false;
    }
  } else { //TODO: add check fordependencies with the operation in the memory/WB step
    pipeline.at(EXECUTE_STEP).remaining_ex_cycles--;
    if (pipeline.at(EXECUTE_STEP).remaining_ex_cycles > 0){
      ex_stalled = true;
    } else {
      ex_stalled = false;
    }
  }

  //Mem
  if (pipeline.at(MEMORY_STEP).fromMem){ //wait for memory if the operation is dependant on memory
    //wait for memory result, stall if it has not come yet.
    if (memoryResultValid && memoryResults.id == pipeline.at(MEMORY_STEP).id){
      mem_stalled = false;
      if (pipeline.at(MEMORY_STEP).fromMem){
        pipeline.at(MEMORY_STEP).data.bitpattern = memoryResults.rdata;
      }
      memoryResults = x_mem_result_t({0, 0, 0, 0});
      memoryResultValid = false;
    } else {
      mem_stalled = true;
    }
    //TODO: checkHazards();
  }

  //WB
  if (!pipeline.at(WRITEBACK_STEP).toMem && !pipeline.at(WRITEBACK_STEP).toXReg && !pipeline.at(WRITEBACK_STEP).isEmpty()){ //if writing to rf, write to register file
    registerFilePtr->write(pipeline.at(WRITEBACK_STEP).addrTo, pipeline.at(WRITEBACK_STEP).data);
    result_valid = 0;
    result = {};
  } else if (pipeline.at(WRITEBACK_STEP).toXReg && !pipeline.at(WRITEBACK_STEP).isEmpty()){ //if writing to xreg, write to xreg
    //Use result interface to write to xreg
    result_valid = 1; //TODO: add wait for result ready bedore setting this to 0
    result.id = pipeline.at(WRITEBACK_STEP).id;
    result.data = pipeline.at(WRITEBACK_STEP).data.u;
    result.rd = pipeline.at(WRITEBACK_STEP).addrTo;
  } else {
    result_valid = 0;
    result = {};
  }

  // if (result_valid && !result_ready){
  //   result_stalled = true;
  //   std::cout << "result_stalled" << std::endl;
  // } else {
  //   result_stalled = false;
  // }
  //TODO: Check for hazards underway, dependant on if OOO/fowarding is 1
  //TODO: If there is no instruction at step-1, do not stall. (As long as there is no dependencies.)
  //Solution, a "stallcheck function" that evaluates the whole pipeline. We can then advance stages that does not need to be stalled.!

  stalled = mem_stalled || ex_stalled;// || result_stalled; //TODO: check stalling if we are waiting for result-ready
  //advance pipeline
  if (!stalled){
    pipeline.push_back(waitingOp);
    pipeline.pop_front();//should be dependent on what the front op is
    if (QUEUE_DEPTH > 0){
      setWaitingOp(operationQueue.front());
      operationQueue.pop_front();
      operationQueue.push_back(FpuPipeObj({})); //Push back empty op to keep size
    } else {
      setWaitingOp(FpuPipeObj({}));
    }
  }

  //for testfloat
  return pipeline.at(2);
};

bool FpuPipeline::isStalled(){
  return stalled;
};

//--------------------------
// Memory interface
//--------------------------
void FpuPipeline::pollMemReq(bool& mem_valid, x_mem_req_t& mem_req){
  mem_valid = this->mem_valid;
  mem_req = this->mem_req;
};

void FpuPipeline::writeMemRes(bool mem_ready, bool mem_result_valid, unsigned int id, unsigned int rdata, bool err, bool dbg){
  this->mem_ready = mem_ready;
  memoryResultValid = mem_result_valid;
  memoryResults.id = id;
  memoryResults.rdata = rdata;
  memoryResults.err = err;
  memoryResults.dbg = dbg;

};

//--------------------------
// Result interface
//--------------------------
void FpuPipeline::writeResult(bool result_ready){
  this->result_ready = result_ready;
};

void FpuPipeline::pollResult(bool& result_valid, x_result_t& result){
  result_valid = this->result_valid;
  result = this->result;
};


//--------------------------
// Pipeline functions
//--------------------------
void FpuPipeline::addOpToQueue(FpuPipeObj op){
  operationQueue.pop_back();
  operationQueue.push_back(op);//Replace empty op at the back with op. Safety is handeled in predecoder

};

void FpuPipeline::setWaitingOp(FpuPipeObj op){
  waitingOp=op;
};

void FpuPipeline::flush(){
  pipeline = std::deque<FpuPipeObj>(NUM_PIPELINE_STAGES, FpuPipeObj({}));
  //Reset all internal data
  operationQueue = std::deque<FpuPipeObj>(QUEUE_DEPTH, FpuPipeObj({}));
  mem_valid = 0;
  mem_req = {};
  ex_stalled = false;
  mem_stalled = false;
  stalled = false;
  pipelineFull = false;
};

int FpuPipeline::getNumStages(){
  return NUM_PIPELINE_STAGES;
};

int FpuPipeline::getQueueDepth(){
  return QUEUE_DEPTH;
};

unsigned int FpuPipeline::getId_pipeline(int stage) {
  return pipeline.at(stage).id;
};

unsigned int FpuPipeline::getId_operationQueue(int stage) {
  return operationQueue.at(stage).id;
};

