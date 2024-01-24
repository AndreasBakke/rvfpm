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


FpuPipeObj decode_RTYPE(uint32_t instr);
FpuPipeObj decode_R4TYPE(uint32_t instr);
FpuPipeObj decode_ITYPE(uint32_t instr);
FpuPipeObj decode_STYPE(uint32_t instr);