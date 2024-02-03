/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  DPI-C Wrapper for rvfpm.sv to interface with fpu_top.cpp.
*/


#include "fpu_top.h"
#include <iostream>
// #include <svdpi.h>

extern "C" {
  void* create_fpu_model(int pipelineStages, int queueDepth, int rfDepth){
    std::cout << "pipelineStages: " << pipelineStages << " queueDepth: " << queueDepth << "  rfDepth: " << rfDepth <<std::endl;
    return new FPU(pipelineStages, rfDepth); //Return pointer to FPU
  };

  void reset_fpu(void* fpu_ptr){
    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
    fpu->resetFPU();
  };

  void clock_event(void* fpu_ptr){
    FPU* fpu = static_cast<FPU*>(fpu_ptr); //from generic pointer to FPU pointer
    fpu->clockEvent();
  };

  void add_accepted_instruction(void* fpu_ptr, uint32_t instruction, unsigned int id){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->addAcceptedInstruction(instruction, id);
  };

  void predecode_instruction(void* fpu_ptr, uint32_t instruction, unsigned int id){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->predecodeInstruction(instruction, id);
  };

  void poll_predecoder_result(void* fpu_ptr, bool& accept, x_issue_resp_t& resp){
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    fpu->pollPredecoderResult(accept, resp);
  };




  void destroy_fpu(void* fpu_ptr) {
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    delete fpu;
  }

  unsigned int getRFContent(void* fpu_ptr, int reg) { //Backdoor to read content of the entire fp_register
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    return  fpu->bd_getData(reg).bitpattern;
  }

  unsigned int getPipeStageId(void* fpu_ptr, int stage) {
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    return fpu->bd_getPipeStageId(stage);
  }

  unsigned int getQueueStageId(void* fpu_ptr, int stage) {
    FPU* fpu = static_cast<FPU*>(fpu_ptr);
    return fpu->bd_getQueueStageId(stage);
  }

  unsigned int randomFloat() { //Generate pseudorandom float (not available in SV.)
    uint32_t randomInt = random();
    int sign = rand()%2;
    return randomInt; //Generate random float
  }
}
