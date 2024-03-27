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

void FpuPredecoder::predecodeInstruction(uint32_t instruction, unsigned int id, x_issue_resp_t& resp_ref, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c) {
  current_decode_id = id;
  FpuPipeObj res = {};
  res = decodeOp(instruction, id, 0, 0, 0, 0);
  use_rs_a = res.use_rs_i[0];
  use_rs_b = res.use_rs_i[1];
  use_rs_c = res.use_rs_i[2];
  if (res.valid) {
    resp_ref.accept = true;
    resp_ref.writeback = res.toXReg;
    resp_ref.loadstore = res.toMem || res.fromMem;
    resp_ref.ecswrite = false; //Todo: understand this
    resp_ref.exc = false; //Todo: understand this
  } else {
    resp_ref.accept = false;
    resp_ref.writeback = false;
    resp_ref.loadstore = false;
    resp_ref.ecswrite = false;
    resp_ref.exc = false;
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