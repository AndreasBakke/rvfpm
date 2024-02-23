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
  execute_done = false;
  mem_done = false;
  wb_done = false;
  stalled = false;
  mem_valid = 0;
  mem_req = {};
  result_valid = 0;
  result = {};
}

FpuPipeline::~FpuPipeline() {
}

FpuPipeObj FpuPipeline::step(){ //TODO: add special handling if multiple stages are done in one step. (We can use execute done etc for this.)
  //Operations are decoded before adding to the pipeline
  //Check for memory dependencies and request throough interface
  //Pipeline stucture set in run/setup.yaml TODO:actually make it set in run/setup.yaml

  if (NUM_PIPELINE_STAGES == 0) { //TODO: add stall-checking and interface usage
    executeOp(waitingOp, registerFilePtr, mem_valid, mem_req);
    registerFilePtr->write(waitingOp.addrTo, waitingOp.data); //only if not mem
    return waitingOp;
  }


  if(!execute_done) { //If we are not done executing the op in execute step. Flag reset by stallCheck() if we advance
    bool speculative = false;
    bool wait_for_mem = false;
    bool more_cycles_rem = false;
    std::cout << pipeline.at(EXECUTE_STEP).remaining_ex_cycles << std::endl;
    wait_for_mem = mem_valid && (pipeline.at(EXECUTE_STEP).toMem || pipeline.at(EXECUTE_STEP).fromMem); //Should be 0 if a memory op has not been done last cycle
    if(pipeline.at(EXECUTE_STEP).speculative){
      speculative = true;
    }
    else if (wait_for_mem){
      wait_for_mem = !mem_ready;
      mem_valid = wait_for_mem; //set to 0 if done
    }
    else if (pipeline.at(EXECUTE_STEP).remaining_ex_cycles > 1){
      pipeline.at(EXECUTE_STEP).remaining_ex_cycles--;
      more_cycles_rem = true;
    }
    else{ //If we reach this point, we can assume that this operation has not been executed yet (or have been decremented enugh).
      executeOp(pipeline.at(EXECUTE_STEP), registerFilePtr, mem_valid, mem_req);
      wait_for_mem = pipeline.at(EXECUTE_STEP).toMem || pipeline.at(EXECUTE_STEP).fromMem; //Should be 0 if a memory op has not been done last cycle
    }
    execute_done = !speculative && !wait_for_mem && !more_cycles_rem;
    if(!wait_for_mem){
      mem_valid =0;
      this->mem_req = {};
    }

  } else {
    execute_done = true;
    //Wait - means we are stalled by some other step
  }


  //Mem
  if (pipeline.at(MEMORY_STEP).fromMem){ //Only need to check for mem_done if we are reading from emm
    mem_done = false; //Start by setting to false
    if (memoryResultValid && memoryResults.id == pipeline.at(MEMORY_STEP).id){
      pipeline.at(MEMORY_STEP).data.bitpattern = memoryResults.rdata;
      memoryResults = x_mem_result_t({0, 0, 0, 0});
      memoryResultValid = false;
      mem_done = true;
    }
  } else {
    mem_done = true;
  }

  //WB
  if(!wb_done){
    //If not to xreg, or not memory, and not empty. Just write to register file and set done = 1
    if (!pipeline.at(WRITEBACK_STEP).toMem && !pipeline.at(WRITEBACK_STEP).toXReg && !pipeline.at(WRITEBACK_STEP).isEmpty()){
      registerFilePtr->write(pipeline.at(WRITEBACK_STEP).addrTo, pipeline.at(WRITEBACK_STEP).data);
      wb_done = true;
    } else if (result_valid) {
      wb_done = result_ready;
    } else if (pipeline.at(WRITEBACK_STEP).toXReg) {
      result_valid = 1; //TODO: add wait for result ready bedore setting this to 0
      result.id = pipeline.at(WRITEBACK_STEP).id;
      result.data = pipeline.at(WRITEBACK_STEP).data.u;
      result.rd = pipeline.at(WRITEBACK_STEP).addrTo;
      wb_done = false;
    } else {
      wb_done = true;
    }
    if (wb_done){
      result_valid = 0;
      result = {};
    }
  } else {
    wb_done = true;
  }

  stallCheck();
  //for testfloat
  return pipeline.at(2);
};

void FpuPipeline::stallCheck(){ //TODO: also check for hazards.
  bool all_done = execute_done || mem_done || wb_done;
  //Check if writeback if finished:
  if (wb_done){
    wb_done = false;
    pipeline.at(WRITEBACK_STEP) = FpuPipeObj({});
  }

  if (mem_done && pipeline.at(WRITEBACK_STEP).isEmpty()){
    mem_done = false;
    pipeline.at(WRITEBACK_STEP) = pipeline.at(MEMORY_STEP); //TODO: better way to do this. There might be a "no operation" stage here intended by the user. EG. Execute - nothing - memory - writeback...
    pipeline.at(MEMORY_STEP) = FpuPipeObj({});
  }

  if (execute_done && pipeline.at(MEMORY_STEP).isEmpty()){
    execute_done = false;
    pipeline.at(MEMORY_STEP) = pipeline.at(EXECUTE_STEP);
    pipeline.at(EXECUTE_STEP) = FpuPipeObj({});
  }

  if (pipeline.at(EXECUTE_STEP).isEmpty()){
    pipeline.erase(pipeline.begin()+EXECUTE_STEP); //removes the empty op
    pipeline.push_back(waitingOp);
    if (QUEUE_DEPTH > 0){
      setWaitingOp(operationQueue.front());
      operationQueue.pop_front();
      operationQueue.push_back(FpuPipeObj({})); //Push back empty op to keep size
    } else {
      setWaitingOp(FpuPipeObj({}));
    }
  }
  if (all_done) {
    stalled = false;
    return; //No need to check for stall if nothing have stalled us this cycle
  }
  // if queue is empty, we can accept new operations
  if (QUEUE_DEPTH > 0) {
    if (!operationQueue.back().isEmpty()){
      stalled = true;
    }
  } else if (!waitingOp.isEmpty()){
    stalled = true;
  }


}

bool FpuPipeline::isStalled(){
  return stalled;
};


void FpuPipeline::commitInstruction(unsigned int id, bool kill){
  for (auto& op : pipeline) {
    if (op.id == id) {
      if (kill) {
        op = FpuPipeObj({});
      } else {
        op.speculative = 0;
      }
    }
  }
  for (auto& op : operationQueue) { //if the operation is in the queue, commit it
    if (op.id == id) {
      if (kill) {
        op = FpuPipeObj({});
      } else {
        op.speculative = 0;
      }
    }
  }
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
  execute_done = false;
  mem_done = false;
  wb_done = false;
  stalled = false;
  pipelineFull = false;
  mem_valid = 0;
  mem_req = {};
  result_valid = 0;
  result = {};
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

