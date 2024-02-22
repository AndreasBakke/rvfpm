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
#include "fpu_config.h"
#include <cstdint>
#include <cfenv>
#include <limits>


#pragma STDC FENV_ACCESS ON //To set roundingmode

void executeOp(FpuPipeObj& op, FpuRf* registerFile, bool& mem_valid, x_mem_req_t& mem_req);

void execute_RTYPE(FpuPipeObj& op, FpuRf* registerFile);
void execute_R4TYPE(FpuPipeObj& op, FpuRf* registerFile);
void execute_ITYPE(FpuPipeObj& op, FpuRf* registerFile, bool& mem_valid, x_mem_req_t& mem_req);
void execute_STYPE(FpuPipeObj& op, FpuRf* registerFile, bool& mem_valid, x_mem_req_t& mem_req);
void execute_ISTYPE(FpuPipeObj& op, FpuRf* registerFile, bool& mem_valid, x_mem_req_t& mem_req);


void setRoundingMode(unsigned int rm);

bool isSubnormal(FPNumber num);
unsigned int getFlags();