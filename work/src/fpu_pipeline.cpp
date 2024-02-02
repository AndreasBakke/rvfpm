/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Fpu pipeline
*/

#include "fpu_pipeline.h"


FpuPipeline::FpuPipeline(int pipelineStages = 0, int queueDepth = 0, FpuRf* rf_pointer = nullptr) : NUM_PIPELINE_STAGES(pipelineStages), QUEUE_DEPTH(queueDepth), pipeline(pipelineStages), operationQueue(queueDepth) {
  pipeline = std::deque<FpuPipeObj>(NUM_PIPELINE_STAGES, FpuPipeObj({}));//Initialize empty pipeline
  operationQueue = std::deque<FpuPipeObj>(QUEUE_DEPTH, FpuPipeObj({}));//Initialize empty queue
  registerFilePtr = rf_pointer;
  waitingOp = FpuPipeObj({}); //Initialize empty waiting op
  pipelineFull = false;
  stalled = false;
}

FpuPipeline::~FpuPipeline() {
}

FpuPipeObj FpuPipeline::step(){
  //Operations are decoded before adding to the pipeline
  //Check for memory dependencies and request throough interface
  //stall here untill memory is ready (or reorder)

 //Execute operation in execute step (get from pa_....)
 //If multicycle, stall pipeline, and decrement cycle counter

 //Return result if Zfinx or similar

  //Decode is already done when adding to queue

  if (NUM_PIPELINE_STAGES == 0) {
    executeOp(waitingOp, registerFilePtr);
    return waitingOp;
  }
  executeOp(pipeline.at(2), registerFilePtr); //Compute operation at execute stage. //Wait for memory if needed?
  if (pipeline.at(2).remaining_ex_cycles > 0){
    stalled = true;
  } else {
    stalled = false;
  }
  //Mem
  if (pipeline.at(1).fromMem || pipeline.at(1).toMem){
    //Request memory access
  }

  //Forwarding?
  //WB
  if (!pipeline.at(0).toMem && !pipeline.at(0).toXReg){ //if writing to rf, write to register file
    registerFilePtr->write(pipeline.at(0).addrTo, pipeline.at(0).data);
  }

  //Check for hazards underway, dependant on if OOO/fowarding is 1
  if (!stalled){
    pipeline.push_back(waitingOp);
    pipeline.pop_front();//should be dependent on what the front op is
    if (QUEUE_DEPTH > 0){
      setWaitingOp(operationQueue.front());
      operationQueue.pop_front();
    } else {
      setWaitingOp(FpuPipeObj({}));
    }
  }
  //if testfloat
  return pipeline.at(2);
};

void FpuPipeline::addOpToQueue(FpuPipeObj op){
  operationQueue.push_back(op);
};

void FpuPipeline::setWaitingOp(FpuPipeObj op){
  waitingOp=op;
};

void FpuPipeline::flush(){
  pipeline = std::deque<FpuPipeObj>(NUM_PIPELINE_STAGES, FpuPipeObj({}));
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

