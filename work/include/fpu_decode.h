/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Decode operations for fpu.
*/

#pragma once

#include "fp_number.h"
#include "fpu_pipe.h"
#include "fpu_instructions.h"
#include <cstdint>
#include <cfenv>
#include <limits>

FpuPipeObj decodeOp(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c);

FpuPipeObj decode_RTYPE(uint32_t instr, unsigned int operand_a, unsigned int operand_b);
FpuPipeObj decode_R4TYPE(uint32_t instr, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c);
FpuPipeObj decode_ITYPE(uint32_t instr, unsigned int operand_a);
FpuPipeObj decode_STYPE(uint32_t instr, unsigned int operand_a);