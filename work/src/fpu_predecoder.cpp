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
  resp.dualread = 0;
  resp.loadstore = false;
  resp.ecswrite = false;
  resp.exc = false;
}

FpuPredecoder::~FpuPredecoder() {
}

void FpuPredecoder::predecodeInstruction(uint32_t instruction, unsigned int id) { //TODO: reset respo when new instruction is accepted
  current_decode_id = id;
  FpuPipeObj res = {};
  res = decodeOp(instruction, id, 0, 0, 0);
  std::cout << "Predecoding instruction: " << std::hex << instruction << std::dec << " with id: " << id << std::endl;
  this->use_rs_i[0] = res.use_rs_i[0];
  this->use_rs_i[1] = res.use_rs_i[1];
  this->use_rs_i[2] = res.use_rs_i[2];
  if (res.valid && fpuReady) { // And the fpu is ready
    resp.accept = true;
    resp.writeback = res.toXReg; //TODO: this should be writeback to XREG
    resp.dualwrite = false; //Not used by any instructions in F-extension
    resp.dualread = 0; //TODO: Not used
    resp.loadstore = res.toMem || res.fromMem;
    resp.ecswrite = false; //Todo: understand this
    resp.exc = false; //Todo: understand this
  } else {
    resp.accept = false;
    resp.writeback = false;
    resp.dualwrite = false;
    resp.dualread = 0;
    resp.loadstore = false;
    resp.ecswrite = false;
    resp.exc = false;
  }
}

void FpuPredecoder::pollPredecoderResult(x_issue_resp_t& resp_ref, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c) {
  resp_ref = this->resp;
  use_rs_a = this->use_rs_i[0];
  use_rs_b = this->use_rs_i[1];
  use_rs_c = this->use_rs_i[2];
}

void FpuPredecoder::reset() {
  this->resp.accept = false;
  this->resp.writeback = false;
  this->resp.dualwrite = false;
  this->resp.dualread = 0;
  this->resp.loadstore = false;
  this->resp.ecswrite = false;
  this->resp.exc = false;
  this->current_decode_id = 0;
  this->use_rs_i[0] = false;
  this->use_rs_i[1] = false;
  this->use_rs_i[2] = false;
  std::cout << "Predecoder reset" << std::endl;
}