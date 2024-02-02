/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    Fpu pipeline
*/

#pragma once
#include "fpu_pipe.h"
#include "fpu_rf.h"
#include "fpu_instructions.h"
#include "fpu_decode.h"
#include "fpu_execute.h"
#include <deque> //Double ended queue

class FpuPipeline {
  private:
    int NUM_PIPELINE_STAGES;
    int QUEUE_DEPTH;
    std::deque<FpuPipeObj> pipeline;
    std::deque<FpuPipeObj> operationQueue;
    FpuRf* registerFilePtr;

    FpuPipeObj waitingOp; //Next in line to pipeline
    bool pipelineFull;
    bool stalled; //or something like this


  public:
    FpuPipeline(int pipelineStages, int queueDepth, FpuRf* rf_pointer);
    ~FpuPipeline();
    FpuPipeObj step(); //Advance pipeline by one step (called by clock in interface)

    void addOpToQueue(FpuPipeObj op);
    void setWaitingOp(FpuPipeObj op);
    void flush();
    int getNumStages();
    int getQueueDepth();
    unsigned int getId_pipeline(int stage);
    unsigned int getId_operationQueue(int stage);

};