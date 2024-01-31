/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Fpu pipeline
*/

#include "fpu_pipeline.h"


FpuPipeline::FpuPipeline(int pipelineStages, int queueDepth = 0) : NUM_PIPELINE_STAGES(pipelineStages), QUEUE_DEPTH(queueDepth), pipeline(pipelineStages), operationQueue(queueDepth) {
  pipeline = std::deque<FpuPipeObj>(NUM_PIPELINE_STAGES, FpuPipeObj({}));//Initialize empty pipeline
  operationQueue = std::deque<FpuPipeObj>(QUEUE_DEPTH, FpuPipeObj({}));//Initialize empty queue
}

FpuPipeline::~FpuPipeline() {
}

FpuPipeObj FpuPipeline::step(FpuPipeObj nextOp, bool* pipelineFull){
  FpuPipeObj op = {};
  op = pipeline.front();
  pipeline.pop_front();
  pipeline.push_back(nextOp); //Use first in queue instead. Only if not stalled
  if (0) { //TODO:pipeline.size = stages doesn't work. BUT, only applicable once stalls/multi cycle execution is added
      //Check for full pipeline
    if (pipelineFull != nullptr){
      *pipelineFull = true;
    }
  } else {
    if (pipelineFull != nullptr){
      *pipelineFull = false;
    }
  }
  return op;
};


void FpuPipeline::flush(){
  pipeline = std::deque<FpuPipeObj>(NUM_PIPELINE_STAGES, FpuPipeObj({}));
};

int FpuPipeline::getNumStages(){
  return NUM_PIPELINE_STAGES;
};

unsigned int FpuPipeline::getId(int stage) {
  return pipeline.at(stage).id;
};

