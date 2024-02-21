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
#include "fpu_config.h"

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
    bool ex_stalled; //Stalled due to ex?
    bool mem_stalled; //stalled due to mem?
    bool stalled;

    //Memory request interface
    bool mem_valid; //set by core, polled in rvfpm.sv
    x_mem_req_t mem_req; //set by core, polled in rvfpm.sv
    bool mem_ready; //set by rvfpm.sv, polled in core

    //Memory result interface
    bool memoryResultValid; //Set during memory result transaction
    x_mem_result_t memoryResults; //Set during memory result transaction

  public:
    FpuPipeline(int pipelineStages, int queueDepth, FpuRf* rf_pointer);
    ~FpuPipeline();
    FpuPipeObj step(); //Advance pipeline by one step (called by clock in interface)
    bool isStalled();

    void addOpToQueue(FpuPipeObj op);
    void setWaitingOp(FpuPipeObj op);
    void pollMemReq(bool& mem_valid, x_mem_req_t& mem_req);
    void writeMemRes(bool mem_ready, bool mem_result_valid, unsigned int id, unsigned int rdata, bool err, bool dbg);

    void flush();
    int getNumStages();
    int getQueueDepth();
    unsigned int getId_pipeline(int stage);
    unsigned int getId_operationQueue(int stage);

};