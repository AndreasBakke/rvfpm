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

FpuPipeObj decodeOp(uint32_t instruction, unsigned int id, unsignedType operand_a, unsignedType operand_b, unsignedType operand_c, unsigned int mode);
FpuPipeObj decode_RTYPE(uint32_t instr, unsignedType operand_a, unsignedType operand_b);
FpuPipeObj decode_R4TYPE(uint32_t instr, unsignedType operand_a, unsignedType operand_b, unsignedType operand_c);
FpuPipeObj decode_ITYPE(uint32_t instr, unsignedType operand_a);
FpuPipeObj decode_STYPE(uint32_t instr, unsignedType operand_a);
FpuPipeObj decode_CSRTYPE(uint32_t instr, unsignedType operand_a);