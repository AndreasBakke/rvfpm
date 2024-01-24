/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    Fpu pipeline
*/

#pragma once
#include "fpu_pipe.h"
#include <deque> //Double ended queue

class FpuPipeline {
  private:
    int NUM_PIPELINE_STAGES;
    std::deque<FpuPipeObj> pipeline;

  public:
    FpuPipeline(int pipelineStages);
    ~FpuPipeline();
    FpuPipeObj step(FpuPipeObj nextOp, bool* pipelineFull); //Advance pipeline by one step (called by clock in interface)
    void flush();
    int getNumStages();
    unsigned int getId(int stage);
};