/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Fpu pipeline
*/

#include "fpu_pipeline.h"
#include "xif_config.h"
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
  result_valid = 0;
  result = {};
}

FpuPipeline::~FpuPipeline() {
}

FpuPipeObj& FpuPipeline::at(int i){
  return pipeline.at(i);
};

FpuPipeObj& FpuPipeline::at_queue(int i){
  return operationQueue.at(i);
};

void FpuPipeline::step(){
  //Operations are decoded before adding to the pipeline
  //Check for memory dependencies and request throough interface
  //Pipeline stucture set in run/setup.yaml
  //All steps done "combinatorially" on negative clock edge, we only decrement remaining_ex_cycles and advance/check for stalls.

  #ifdef TESTFLOAT
    executeOp(waitingOp, registerFilePtr); //Values are loaded to register using bd_load
    return;
  #endif

  pipeline.at(EXECUTE_STEP).remaining_ex_cycles--;
  advanceStages();
  stallCheck();
};


void FpuPipeline::advanceStages(){
  bool all_done = execute_done && mem_done && wb_done;
  if (wb_done) {
    if (!pipeline.at(WRITEBACK_STEP).toMem && !pipeline.at(WRITEBACK_STEP).toXReg && !pipeline.at(WRITEBACK_STEP).isEmpty()){
      #ifndef ZFINX
        registerFilePtr->write(pipeline.at(WRITEBACK_STEP).addrTo, pipeline.at(WRITEBACK_STEP).data);
      #endif
    }
    result_valid = 0;
    result = {};
  }

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
      #ifdef QUEUE_FALLTHROUGH
        if (operationQueue.empty()){
          pipeline.push_back(FpuPipeObj({}));
        } else {
          pipeline.push_back(operationQueue.front());
          operationQueue.pop_front();
        }
      #else
        pipeline.push_back(operationQueue.front());
        operationQueue.pop_front();
        operationQueue.push_back(FpuPipeObj{});
      #endif
    } else {
      pipeline.push_back(waitingOp);
      setWaitingOp(FpuPipeObj({}));
    }
  }

}



void FpuPipeline::executeStep(){
  FpuPipeObj& exOp = pipeline.at(EXECUTE_STEP);
  if(!execute_done) { //If we are not done executing the op in execute step. Flag reset by stallCheck() if we advance
    #ifndef FORWARDING
      if(exOp.stalledByCtrl){mem_done = false; return;}
    #else
      exOp.fw_data = fw_data;
      exOp.fw_addr = fw_addr;
    #endif
    bool speculative = false;
    bool more_cycles_rem = false;
    if(exOp.speculative){
      speculative = true; //Wait until operation has been committed before executing
    }
    else if (exOp.remaining_ex_cycles > 1){
      more_cycles_rem = true;
    }
    else if(!(exOp.toMem || exOp.fromMem) && !exOp.isEmpty()) { //If we reach this point, we can assume that this operation has not been executed yet (or have been decremented enugh).
      executeOp(pipeline.at(EXECUTE_STEP), registerFilePtr);
      #ifdef FORWARDING
        fw_data = exOp.data;
        fw_addr = exOp.addrTo;
      #endif
    }
    execute_done = !speculative  && !more_cycles_rem;
  } else {
    execute_done = true;
  }
}

void FpuPipeline::memoryStep(){
  FpuPipeObj& memOp = pipeline.at(MEMORY_STEP);
  if (MEMORY_STEP == EXECUTE_STEP && !execute_done || memOp.stalledByCtrl) {
    mem_done = false;
  } else if ((memOp.fromMem || memOp.toMem)){
    if(memOp.speculative){mem_done=false;return;}
    //Add op to queue if its not been added yet
    if (memOp.speculative){mem_done = false;return;}
    if(!memOp.added_to_mem_queue){
      x_mem_req_t mem_req_s = {};
    mem_req_s.id = memOp.id;
    mem_req_s.addr = memOp.toMem ? memOp.addrTo : memOp.addrFrom.front();
    STYPE dec_instr = {.instr = memOp.instr};
    mem_req_s.wdata = registerFilePtr->read(dec_instr.parts.rs2);
    mem_req_s.last = 1;
    mem_req_s.size = memOp.size;
    mem_req_s.mode = memOp.mode;
    mem_req_s.we = memOp.toMem ? 1 : 0;
    mem_req_queue.push_back(mem_req_s);
    memOp.added_to_mem_queue = 1;
    }

    if (!memOp.mem_result_valid){
      return;
    }
    mem_done = true;
    if (memOp.fromMem){
      memOp.data = memOp.mem_result; //Add result to op. Written at WB step
    }
  }

   else {
    mem_done = true;
  }
}

void FpuPipeline::writebackStep(){
  if (wb_done) {
    return;
  }
  if ((WRITEBACK_STEP == MEMORY_STEP && !mem_done) || (WRITEBACK_STEP == EXECUTE_STEP && !execute_done)) {
    wb_done = false;
    return;
  }
  addResult(pipeline.at(WRITEBACK_STEP)); //Add to result queue. Handled by controller
  wb_done = true;
}

void FpuPipeline::addResult(FpuPipeObj op){ //Set relevant eXtension Interface values and add to queue
  x_result_t result_s = {};
  result_s.id = op.id;
  if (!op.fromMem && !op.toMem){
    result_s.rd = op.addrTo;
    result_s.data = op.data;
  }
  if(op.toXReg){
    result_s.we = 1;
  }
  if(!op.toXReg && !op.toMem && !op.fromMem){
    result_s.ecswe   = 0b010;
    result_s.ecsdata = 0b001100;
  }
  if (!op.isEmpty()){
    result_queue.push_back(result_s);
  }
};



void FpuPipeline::stallCheck(){
  stalled = false;
  if (QUEUE_DEPTH > 0) {
    #ifdef QUEUE_FALLTHROUGH
      if (operationQueue.size()==QUEUE_DEPTH){
        stalled = true;
      }
    #else
      if(!operationQueue.back().isEmpty()){
        stalled = true;
      }
    #endif
  } else if (!waitingOp.isEmpty()){
    stalled = true;
  }

}

bool FpuPipeline::isStalled(){
  return stalled;
};

bool FpuPipeline::isEmpty(){
  //Check if all stages are empty
  for (auto& op : pipeline) {
    if (!op.isEmpty()) {
      return false;
    }
  }
  return true;
};



void FpuPipeline::commitInstruction(unsigned int id, bool kill){
  for (auto& op : pipeline) {
    if (op.id == id && op.speculative) {
      if (kill) {
        op = FpuPipeObj({});
      } else {
        op.speculative = 0;
      }
      return;
    }
  }
  if (waitingOp.id == id && waitingOp.speculative) {
    if (kill) {
      waitingOp = FpuPipeObj({});
    } else {
      waitingOp.speculative = 0;
    }
    return;
  }
  for (auto& op : operationQueue) { //if the operation is in the queue, commit it
    if (op.id == id && op.speculative) {
      if (kill) {
        op = FpuPipeObj({});
      } else {
        op.speculative = 0;
      }
      return;
    }
  }
};

//--------------------------
// Pipeline functions
//--------------------------
void FpuPipeline::addOpToQueue(FpuPipeObj op){
  #ifndef QUEUE_FALLTHROUGH
  operationQueue.pop_back(); //Remove empty op at the back if we are using fallthrough. Safety is handeled in predecoder
  #endif
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
  execute_done = false;
  mem_done = false;
  wb_done = false;
  stalled = false;
  pipelineFull = false;
  result_valid = 0;
  result = {};
};

int FpuPipeline::getNumStages(){
  return NUM_PIPELINE_STAGES;
};

int FpuPipeline::getQueueDepth(){
  return operationQueue.size();
};

unsigned int FpuPipeline::getId_pipeline(int stage) {
  return pipeline.at(stage).id;
};

unsigned int FpuPipeline::getId_operationQueue(int stage) {
  return operationQueue.at(stage).id;
};

