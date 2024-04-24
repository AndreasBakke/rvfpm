/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    Fpu Controller
*/

#pragma once
#include "xif_config.h"
#include "fpu_pipeline.h"
#include "fpu_rf.h"
#include "fpu_decode.h"
#include "fpu_pipe.h"
#include <iostream>
#include <deque>

class Controller {
  private:
    FpuPipeline& fpu_pipeline;
    FpuRf& registerFile;
    bool& fpuReady;
    std::deque<x_mem_req_t> mem_req_queue;


    //Memory request interface
    bool mem_valid; //set by core, polled in rvfpm.sv
    x_mem_req_t mem_req; //set by core, polled in rvfpm.sv
    bool mem_ready; //set by rvfpm.sv, polled in core


  public:
    Controller(FpuRf& rf, FpuPipeline& pipe, bool& ready);
    ~Controller();
    void reset();
    void addAcceptedInstruction(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c, unsigned int mode, bool commit_valid, unsigned int commit_id, bool commit_kill);//and other necessary inputs (should be somewhat close to in_xif type)
    bool hasSameTarget(FpuPipeObj first, FpuPipeObj last);
    void detectHazards();
    void reorder();

    void commitInstruction(unsigned int id, bool kill);
    void pollMemoryRequest(bool& mem_valid, x_mem_req_t& mem_req);
    void resetMemoryRequest(unsigned int id);
    void writeMemoryResponse(bool mem_ready, bool exc, unsigned int exccode, bool dbg);
    void writeMemoryResult(unsigned int id, uint32_t rdata, bool err, bool dbg);
    void pollResult(bool& result_valid_ptr, x_result_t& result_ptr);
    void resetResult(unsigned int id);
  //Mange av funksjonene fra pipeline kan flyttes hit

};