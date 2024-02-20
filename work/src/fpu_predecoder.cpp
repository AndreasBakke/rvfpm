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
  if (res.valid && fpuReady) { // And the fpu is ready
    resp.accept = true;
    resp.writeback = !(res.toMem || res.fromMem);
    resp.dualwrite = false; //Not used by any instructions in F-extension
    resp.dualread = 0; //TODO: Implement dual read
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

void FpuPredecoder::pollPredecoderResult(x_issue_resp_t& resp_ref) {
  resp_ref = resp;
}