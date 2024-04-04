/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    Fpu Controller
*/

#pragma once
#include "fpu_config.h"
#include "fpu_pipeline.h"
#include "fpu_rf.h"
#include <iostream>

class Controller {
  private:
    FpuPipeline& fpu_pipeline;
    FpuRf& registerFile;
    bool& fpuReady;


    //Memory request interface
    bool mem_valid; //set by core, polled in rvfpm.sv
    x_mem_req_t mem_req; //set by core, polled in rvfpm.sv
    bool mem_ready; //set by rvfpm.sv, polled in core

    //Memory response






  public:
    Controller(FpuRf& rf, FpuPipeline& pipe, bool& ready);
    ~Controller();
    void reset();
    void addAcceptedInstruction(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c, unsigned int mode, bool commit_valid, unsigned int commit_id, bool commit_kill);//and other necessary inputs (should be somewhat close to in_xif type)

    void commitInstruction(unsigned int id, bool kill);
    void pollMemoryRequest(bool& mem_valid, x_mem_req_t& mem_req);
    void writeMemoryResponse(bool mem_ready, bool exc, unsigned int exccode, bool dbg);
    void writeMemoryResult(unsigned int id, uint32_t rdata, bool err, bool dbg);
  //Mange av funksjonene fra pipeline kan flyttes hit

};