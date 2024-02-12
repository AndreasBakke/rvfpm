/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Fpu pipeline
*/

#include "fpu_pipeline.h"
#include "fpu_config.h"
#include <iostream>


FpuPipeline::FpuPipeline(int pipelineStages, int queueDepth, FpuRf* rf_pointer) : NUM_PIPELINE_STAGES(pipelineStages), QUEUE_DEPTH(queueDepth), pipeline(pipelineStages), operationQueue(queueDepth) {
  pipeline = std::deque<FpuPipeObj>(NUM_PIPELINE_STAGES, FpuPipeObj({}));//Initialize empty pipeline
  operationQueue = std::deque<FpuPipeObj>(QUEUE_DEPTH, FpuPipeObj({}));//Initialize empty queue
  registerFilePtr = rf_pointer;
  waitingOp = FpuPipeObj({}); //Initialize empty waiting op
  pipelineFull = false;
  stalled = false;
  mem_stalled = false;
  ex_stalled = false;
  std::cout << "QUEUE DEPTH: " << QUEUE_DEPTH << std::endl;
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
    executeOp(waitingOp, registerFilePtr, mem_valid, mem_req);
    registerFilePtr->write(waitingOp.addrTo, waitingOp.data); //only if not mem
    return waitingOp;
  }

  //Do some stall checking here
  if (!stalled){
      mem_valid = 0;
      mem_req = x_mem_req_t({0, 0, 0, 0});
      executeOp(pipeline.at(EXECUTE_STEP), registerFilePtr, mem_valid, mem_req); //Compute operation at execute stage. //Issue memory request to CPU at this stage
    if (pipeline.at(EXECUTE_STEP).remaining_ex_cycles > 0){
      ex_stalled = true;
    } else {
      ex_stalled = false;
    }
  } else { //TODO: add check fordependencies with the operation in the memory step
    pipeline.at(EXECUTE_STEP).remaining_ex_cycles--;
    ex_stalled = pipeline.at(EXECUTE_STEP).remaining_ex_cycles > 0; //sets stalled to false if remaining cycles is 0, can still be stalled at other steps
  }

  //Mem
  std::cout << "mem id: " << pipeline.at(MEMORY_STEP).id << " fromM " << pipeline.at(MEMORY_STEP).fromMem << std::endl;
  if (pipeline.at(MEMORY_STEP).fromMem){ //wait for memory if the operation is dependant on memory
    //wait for memory result, stall if it has not come yet.
    std::cout << "valid?: " << memoryResultValid << std::endl;
    std::cout << "memiId: " << memoryResults.id << std::endl;
    std::cout << "memstep: " << pipeline.at(MEMORY_STEP).id << std::endl;
    if (memoryResultValid && memoryResults.id == pipeline.at(MEMORY_STEP).id){
      mem_stalled = false;
      if (pipeline.at(MEMORY_STEP).fromMem){
        pipeline.at(MEMORY_STEP).data.bitpattern = memoryResults.rdata;
        std::cout << "data: " << pipeline.at(MEMORY_STEP).data.bitpattern << std::endl;;
      }
      memoryResults = x_mem_result_t({0, 0, 0, 0});
      memoryResultValid = false;
    } else {
      mem_stalled = true;
    }
    //TODO: Move all stall checking to the end.
    //checkHazards();
  }

  //WB
  //TODO: check for stall
  if (!pipeline.at(WRITEBACK_STEP).toMem && !pipeline.at(WRITEBACK_STEP).toXReg){ //if writing to rf, write to register file

    std::cout << "Writing: " << pipeline.at(WRITEBACK_STEP).data.bitpattern << std::endl;
    registerFilePtr->write(pipeline.at(WRITEBACK_STEP).addrTo, pipeline.at(WRITEBACK_STEP).data);
  }

  //TODO: Check for hazards underway, dependant on if OOO/fowarding is 1
  std::cout << "ex_s: " << ex_stalled << std::endl;
  std::cout << "mem_s: " << mem_stalled << std::endl;

  stalled = mem_stalled || ex_stalled;
  //advance pipeline
  std::cout << "Stalled in step?: " << stalled << std::endl;
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


void FpuPipeline::pollMemReq(bool& mem_valid, x_mem_req_t& mem_req){
  mem_valid = this->mem_valid;
  mem_req = this->mem_req;
};

void FpuPipeline::writeMemRes(bool mem_result_valid, unsigned int id, unsigned int rdata, bool err, bool dbg){
  memoryResultValid = mem_result_valid;
  memoryResults.id = id;
  memoryResults.rdata = rdata;
  memoryResults.err = err;
  memoryResults.dbg = dbg;

};


void FpuPipeline::addOpToQueue(FpuPipeObj op){
  operationQueue.pop_back();
  operationQueue.push_back(op);//Replace empty op at the back with op. Safety is handeled in predecoder

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

