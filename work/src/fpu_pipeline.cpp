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
  wait_for_mem_result = false;
  result_valid = 0;
  result = {};
}

FpuPipeline::~FpuPipeline() {
}

void FpuPipeline::step(){
  //Operations are decoded before adding to the pipeline
  //Check for memory dependencies and request throough interface
  //Pipeline stucture set in run/setup.yaml

  #ifdef TESTFLOAT
    executeOp(waitingOp, registerFilePtr, mem_valid, mem_req); //Values are loaded to register using bd_load
    return;
  #endif

  if (pipeline.at(pipeline.size()-1).isEmpty()){
    pipeline.pop_back();//removes the empty op
    if (QUEUE_DEPTH > 0){
      pipeline.push_back(operationQueue.front());
      operationQueue.pop_front();
      operationQueue.push_back(FpuPipeObj({})); //Push back empty op to keep size
    } else {
      pipeline.push_back(waitingOp);
      setWaitingOp(FpuPipeObj({}));
    }
  }

  //EX TODO: make EX and MEM "independent" in terms of which comes first. Requires check for speculative in mem_step
  if(!execute_done) { //If we are not done executing the op in execute step. Flag reset by stallCheck() if we advance
    bool speculative = false;
    bool more_cycles_rem = false;
    if(pipeline.at(EXECUTE_STEP).speculative){
      speculative = true; //Wait until operation has been committed before executing
    }
    else if (pipeline.at(EXECUTE_STEP).remaining_ex_cycles > 1){
      pipeline.at(EXECUTE_STEP).remaining_ex_cycles--;
      more_cycles_rem = true;
    }
    else if (!(pipeline.at(EXECUTE_STEP).fromMem || pipeline.at(EXECUTE_STEP).toMem)){ //If we reach this point, we can assume that this operation has not been executed yet (or have been decremented enugh).
      executeOp(pipeline.at(EXECUTE_STEP), registerFilePtr, mem_valid, mem_req);
    }
    execute_done = !speculative  && !more_cycles_rem;
  } else {
    execute_done = true;
    //Wait - means we are stalled by some other step
  }
  //MEM
  if (MEMORY_STEP == EXECUTE_STEP && !execute_done) {
    mem_done = false;
  } else if ((pipeline.at(MEMORY_STEP).fromMem || pipeline.at(MEMORY_STEP).toMem ) && !mem_done){
    if (!wait_for_mem_resp && !wait_for_mem_result){
      executeOp(pipeline.at(MEMORY_STEP), registerFilePtr, mem_valid, mem_req);
      wait_for_mem_resp = true;
    }
    if(wait_for_mem_resp) {
      wait_for_mem_resp = !mem_ready;
      mem_valid = !mem_ready; //set to 0 if done, keep to 1 if not
      wait_for_mem_result = mem_ready; //set to 1 if mem_ready is 1
    } else {
      mem_valid = 0;
      this->mem_req = {};
    }

    if(wait_for_mem_result && memoryResultValid && memoryResults.id == pipeline.at(MEMORY_STEP).id){
      pipeline.at(MEMORY_STEP).data.bitpattern = memoryResults.rdata;
      memoryResults = x_mem_result_t({0, 0, 0, 0});
      memoryResultValid = false;
      mem_done = true;
      wait_for_mem_result = false;
    }
  } else {
    mem_done = true;
    wait_for_mem_resp = false;
    wait_for_mem_result = false;
  }

  //WB
  //TODO: set result_valid also if not a writeback when finished.
  if ((WRITEBACK_STEP == MEMORY_STEP && !mem_done) || (WRITEBACK_STEP == EXECUTE_STEP && !execute_done)) {
    wb_done = false;
    result_valid = 0;
    result = {};
  } else if(!wb_done){ //TODO: structure more like the memory step
    //If not to xreg, or not memory, and not empty. Just write to register file and set done = 1
    wb_done = true;
     if (!pipeline.at(WRITEBACK_STEP).toMem && !pipeline.at(WRITEBACK_STEP).toXReg && !pipeline.at(WRITEBACK_STEP).isEmpty()){
      registerFilePtr->write(pipeline.at(WRITEBACK_STEP).addrTo, pipeline.at(WRITEBACK_STEP).data);
      result_valid = 1;
      result.id = pipeline.at(WRITEBACK_STEP).id;
      wb_done = false;
    } else if (pipeline.at(WRITEBACK_STEP).toXReg) {
      result_valid = 1;
      result.id = pipeline.at(WRITEBACK_STEP).id;
      result.data = pipeline.at(WRITEBACK_STEP).data.u;
      result.rd = pipeline.at(WRITEBACK_STEP).addrTo;
      wb_done = false;
    }
    if (result_valid) {
      wb_done = result_ready;
    }
  } else {
    wb_done = true;
    result_valid = 0;
    result = {};
  }
  advanceStages();
  stallCheck();
};


//Todo: split this into smaller functions one for cheking if we are stalled, and one to move operations down the pipeline
void FpuPipeline::advanceStages(){ //TODO: also check for hazards.
  bool all_done = execute_done && mem_done && wb_done;
  stalled = false;
  for (int i = 0; i < pipeline.size(); i++){
    if (i == WRITEBACK_STEP) {
      if (wb_done && (WRITEBACK_STEP != MEMORY_STEP || mem_done) && (WRITEBACK_STEP != EXECUTE_STEP || execute_done)){
        wb_done = false;
        i==0 ? pipeline.at(i) = FpuPipeObj({}) : pipeline.at(i-1) = pipeline.at(i); //Move the operation to the next stage
        pipeline.at(i) = FpuPipeObj({}); //Clear the current stage
        i == MEMORY_STEP ? mem_done = false : mem_done = mem_done;
        i == EXECUTE_STEP ? execute_done = false : execute_done = execute_done;
      }
      continue;
    }

    if (i == MEMORY_STEP) {
      if(mem_done && (MEMORY_STEP != EXECUTE_STEP || execute_done)){
        mem_done = false;
        pipeline.at(i-1) = pipeline.at(i);
        pipeline.at(i) = FpuPipeObj({});
        i == EXECUTE_STEP ? execute_done = false : execute_done = execute_done;
      }
      continue;
    }

    if (i == EXECUTE_STEP) {
      if(execute_done && pipeline.at(i-1).isEmpty()){
        execute_done = false;
        pipeline.at(i-1) = pipeline.at(i);
        pipeline.at(i) = FpuPipeObj({});
      }
      continue;
    }

    //If nothing happens at the step, move the operation to the next stage (if it is empty)
    if (i==0) {
      pipeline.at(i) = FpuPipeObj({});
    }
     else if (pipeline.at(i-1).isEmpty()){
      pipeline.at(i-1) = pipeline.at(i);
      pipeline.at(i) = FpuPipeObj({});
    }
  }

  if (pipeline.at(pipeline.size()-1).isEmpty()){
    pipeline.pop_back();//removes the empty op
    if (QUEUE_DEPTH > 0){
      pipeline.push_back(operationQueue.front());
      operationQueue.pop_front();
      operationQueue.push_back(FpuPipeObj({})); //Push back empty op to keep size
    } else {
      pipeline.push_back(waitingOp);
      setWaitingOp(FpuPipeObj({}));
    }
  }

  #ifdef INCLUDE_QUEUE
    for (int i = 1; i < operationQueue.size(); i++){
      if (operationQueue.at(i-1).isEmpty()){
        operationQueue.at(i-1) = operationQueue.at(i);
        operationQueue.at(i) = FpuPipeObj({});
      }
    }
  #endif
}


void FpuPipeline::stallCheck(){
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


//Checkforhazards
//Hazard type?
//Could include
//step of pipeline
//Addresses involved
//IDs involved
//ID stalled

// //Return array of theese. OOO can use the information to reorder
// bool FpuPipeLine::checkForHazards(){
//   bool stallEx = checkExecuteHazards();
//   bool stallMem = checkMemoryHazards(); // If write to Mem and there is a writeback to be done to the same register. Stall
//   bool stallWB = checkWritebackHazards();//None?
//   //Execute step:
//   // IF addrFrom of execute step is the same as the toAddr of steps in front in pipeline
//   // Not if forwarding is enabled
//   // And not if its the same step
// }


//If addrFrom of execute step is the same as the toAddr of memory or writeback step, stall
//But not if forwarding is enabled

//Reorder
//See if any stalling can be avoided by reordering executions
//OBS: Be careful not to reorder anything that might affect results
//eg. do not reorder operations if there are dependencies between instructions regarding targets
// Especially nice to do if a division/sqrt is done, we can do other operations first, if we have empty queues later.
//This would reduce the overal execution time

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
  if (waitingOp.id == id) {
    if (kill) {
      waitingOp = FpuPipeObj({});
    } else {
      waitingOp.speculative = 0;
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

FpuPipeObj FpuPipeline::getWaitingOp(){
  return waitingOp;
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

