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
#include "xif_config.h"
#include "fp_number.h"
#include <deque> //Double ended queue

class FpuPipeline {
  private:
    std::deque<FpuPipeObj> pipeline;
    std::deque<FpuPipeObj> operationQueue;
    FpuRf* registerFilePtr;

    FpuPipeObj waitingOp; //Next in line to pipeline

    //Pipeline status
    bool pipelineFull;
    bool execute_done;
    bool mem_done;
    bool wb_done;
    bool stalled; //Do not accept new operations

    //Result interface
    bool result_valid; //set by core, polled in rvfpm.sv
    x_result_t result; //set by core, polled in rvfpm.sv
    bool result_ready; //set by rvfpm.sv, polled in core

    #ifdef FORWARDING
      FPNumber fw_data; //Forwarded data
      unsigned int fw_addr; //Address of forwarded data.
    #endif

  public:
    FpuPipeline(FpuRf* rf_pointer);
    ~FpuPipeline();
    FpuPipeObj& at(int i);
    FpuPipeObj& at_queue(int i);

    void step(); //Advance pipeline by one step (called by clock in interface)
    void executeStep();
    void memoryStep();
    void writebackStep(); //wb
    void addResult(FpuPipeObj op);
    void advanceStages(); //Move non-stalled stages one step forward
    void stallCheck(); //Set stalled if pipeline(& optionally queue) is full
    bool isStalled();
    bool isEmpty();

    //Issue/Commit interface
    void addOpToQueue(FpuPipeObj op);
    void setWaitingOp(FpuPipeObj op);
    FpuPipeObj getWaitingOp();
    void commitInstruction(unsigned int id, bool kill);

    //Memory request interface
    std::deque<x_mem_req_t> mem_req_queue;

    //Resultinterface
    std::deque<x_result_t> result_queue;
    void flush();
    int getNumStages();
    int getQueueDepth();
    unsigned int getId_pipeline(int stage);
    unsigned int getId_operationQueue(int stage);

};