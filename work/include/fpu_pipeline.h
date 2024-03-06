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

    //Memory request interface
    bool mem_valid; //set by core, polled in rvfpm.sv
    x_mem_req_t mem_req; //set by core, polled in rvfpm.sv
    bool mem_ready; //set by rvfpm.sv, polled in core

    //Memory result interface
    bool wait_for_mem_resp;
    bool wait_for_mem_result; //For pipeline stalls
    bool memoryResultValid; //Set during memory result transaction
    x_mem_result_t memoryResults; //Set during memory result transaction


    //Result interface
    bool result_valid; //set by core, polled in rvfpm.sv
    x_result_t result; //set by core, polled in rvfpm.sv
    bool result_ready; //set by rvfpm.sv, polled in core

  public:
    FpuPipeline(FpuRf* rf_pointer);
    ~FpuPipeline();
    FpuPipeObj step(); //Advance pipeline by one step (called by clock in interface)
    void stallCheck();
    bool isStalled();

    //Issue/Commit interface
    void addOpToQueue(FpuPipeObj op);
    void setWaitingOp(FpuPipeObj op);
    void commitInstruction(unsigned int id, bool kill);


    //Memory request/result interface
    void pollMemReq(bool& mem_valid, x_mem_req_t& mem_req);
    void writeMemRes(bool mem_ready, bool mem_result_valid, unsigned int id, unsigned int rdata, bool err, bool dbg);

    //Resultinterface
    void writeResult(bool result_ready);
    void pollResult(bool& result_valid, x_result_t& result);

    void flush();
    int getNumStages();
    int getQueueDepth();
    unsigned int getId_pipeline(int stage);
    unsigned int getId_operationQueue(int stage);

};