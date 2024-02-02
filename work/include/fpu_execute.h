/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Execute operations for fpu.
*/
#pragma once
#include "fp_number.h"
#include "fpu_pipe.h"
#include "fpu_rf.h"
#include "fpu_instructions.h"
#include <cstdint>
#include <cfenv>
#include <limits>

#pragma STDC FENV_ACCESS ON //To set roundingmode

void executeOp(FpuPipeObj& op, FpuRf* registerFile);

void execute_RTYPE(FpuPipeObj& op, FpuRf* registerFile);//, int fromXReg, unsigned int* id_out, uint32_t* toXReg, bool* toXReg_valid);
void execute_R4TYPE(FpuPipeObj& op, FpuRf* registerFile);
void execute_ITYPE(FpuPipeObj& op, FpuRf* registerFile, unsigned int fromMem);
void execute_STYPE(FpuPipeObj& op, FpuRf* registerFile, unsigned int* id_out, uint32_t* toMem, bool* toMem_valid);

void setRoundingMode(unsigned int rm);

bool isSubnormal(FPNumber num);
unsigned int getFlags();