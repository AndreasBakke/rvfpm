/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    Fpu pipeline
*/

#pragma once
#include "fpu_pipe.h"
#include "fpu_instructions.h"
#include <deque> //Double ended queue

class FpuPipeline {
  private:
    int NUM_PIPELINE_STAGES;
    int QUEUE_DEPTH;
    std::deque<FpuPipeObj> pipeline;
    std::deque<FpuPipeObj> operationQueue;


  public:
    FpuPipeline(int pipelineStages, int queueDepth);
    ~FpuPipeline();
    FpuPipeObj step(FpuPipeObj nextOp, bool* pipelineFull); //Advance pipeline by one step (called by clock in interface)
    void flush();
    int getNumStages();
    unsigned int getId(int stage);
};