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


void FpuPipeline::advanceStages(){ //TODO: also check for hazards.
  bool all_done = execute_done && mem_done && wb_done;
  if (wb_done) {
    if (!pipeline.at(WRITEBACK_STEP).toMem && !pipeline.at(WRITEBACK_STEP).toXReg && !pipeline.at(WRITEBACK_STEP).isEmpty()){
      registerFilePtr->write(pipeline.at(WRITEBACK_STEP).addrTo, pipeline.at(WRITEBACK_STEP).data);
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



void FpuPipeline::executeStep(){
  if(!execute_done) { //If we are not done executing the op in execute step. Flag reset by stallCheck() if we advance
    bool speculative = false;
    bool more_cycles_rem = false;
    if(pipeline.at(EXECUTE_STEP).speculative){
      speculative = true; //Wait until operation has been committed before executing
    }
    else if (pipeline.at(EXECUTE_STEP).remaining_ex_cycles > 1){
      more_cycles_rem = true;
    }
    else if(!(pipeline.at(EXECUTE_STEP).toMem || pipeline.at(EXECUTE_STEP).fromMem) && !pipeline.at(EXECUTE_STEP).isEmpty()) { //If we reach this point, we can assume that this operation has not been executed yet (or have been decremented enugh).
      executeOp(pipeline.at(EXECUTE_STEP), registerFilePtr);
    }
    execute_done = !speculative  && !more_cycles_rem;
  } else {
    execute_done = true;
  }
}

void FpuPipeline::memoryStep(){
  if (MEMORY_STEP == EXECUTE_STEP && !execute_done) {
    mem_done = false;
  } else if ((pipeline.at(MEMORY_STEP).fromMem || pipeline.at(MEMORY_STEP).toMem)){
    if (!pipeline.at(MEMORY_STEP).mem_result_valid){
      return;
    }
    mem_done = true;
    if (pipeline.at(MEMORY_STEP).fromMem){
      pipeline.at(MEMORY_STEP).data.bitpattern = pipeline.at(MEMORY_STEP).mem_result;
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
  addResult(pipeline.at(WRITEBACK_STEP));
  wb_done = true;
}

void FpuPipeline::addResult(FpuPipeObj op){
  x_result_t result_s = {};
  result_s.id = op.id;
  if (!op.fromMem && !op.toMem){
    result_s.rd = op.addrTo;
    result_s.data = op.data.bitpattern;
  }
  if(op.toXReg){ //TODO: or is ZFINX
    result_s.we = 1;
  }
  if(!op.toXReg && !op.toMem && !op.fromMem){ //TODO: and not ZFINX
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
    if (!operationQueue.back().isEmpty()){
      stalled = true;
    }
  } else if (!waitingOp.isEmpty()){
    stalled = true;
  }
    std::cout << "ex_done: " << execute_done << " mem: " <<mem_done << " wb "<< wb_done << std::endl;

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
    if (op.id == id) {
      if (kill) {
        op = FpuPipeObj({});
      } else {
        op.speculative = 0;
      }
      return;
    }
  }
  if (waitingOp.id == id) {
    if (kill) {
      waitingOp = FpuPipeObj({});
    } else {
      waitingOp.speculative = 0;
    }
    return;
  }
  for (auto& op : operationQueue) { //if the operation is in the queue, commit it
    if (op.id == id) {
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
  return QUEUE_DEPTH;
};

unsigned int FpuPipeline::getId_pipeline(int stage) {
  return pipeline.at(stage).id;
};

unsigned int FpuPipeline::getId_operationQueue(int stage) {
  return operationQueue.at(stage).id;
};

