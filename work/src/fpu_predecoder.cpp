/*  rvfpm - 2024
  Andreas S. Bakke

  Description
  Predecoder for the eXtension Issue interface
  Handles acceptance of offloaded instructions from the CPU
*/

#include "fpu_predecoder.h"
#include <iostream>

FpuPredecoder::FpuPredecoder(bool& fpuReady) : fpuReady(fpuReady) {
  current_decode_id = 0;
  use_rs_i[0] = false;
  use_rs_i[1] = false;
  use_rs_i[2] = false;
  resp.accept = false;
  resp.writeback = false;
  resp.dualwrite = false;
  resp.dualread = false;
  resp.loadstore = false;
  resp.ecswrite = false;
  resp.exc = false;
}

FpuPredecoder::~FpuPredecoder() {
}

void FpuPredecoder::predecodeInstruction(uint32_t instruction, unsigned int id, bool& accept, bool& loadstore, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c) {
  current_decode_id = id;
  FpuPipeObj res = {};
  res = decodeOp(instruction, id, 0, 0, 0, 0);
  use_rs_a = res.use_rs_i[0];
  use_rs_b = res.use_rs_i[1];
  use_rs_c = res.use_rs_i[2];
  // std::cout << "current: " << instruction  << " past " << past_instruction_accepted << std::endl;
  if (res.valid) {
    accept = true;
    loadstore = res.toMem || res.fromMem;
  } else {
    accept = false;
    loadstore = false;
  }
}


void FpuPredecoder::reset() {
  this->resp.accept = false;
  this->resp.writeback = false;
  this->resp.loadstore = false;
  this->resp.ecswrite = false;
  this->resp.exc = false;
  this->current_decode_id = 0;
  this->use_rs_i[0] = false;
  this->use_rs_i[1] = false;
  this->use_rs_i[2] = false;
}