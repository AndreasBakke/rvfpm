/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Decode operations for fpu.
*/

#include "fpu_decode.h"

#include <iostream>

FpuPipeObj decodeOp(uint32_t instruction, unsigned int id, unsignedType operand_a, unsignedType operand_b, unsignedType operand_c, unsigned int mode) { //Add more fields if needed by eXtension interface
  //Get result of operation
  unsigned int opcode = instruction & 127 ; //Get first 7 bit
  FpuPipeObj result = {};
  switch (opcode)
  {
  case FLW:
    result = decode_ITYPE(instruction, operand_a);
    break;
  case FSW:
    result = decode_STYPE(instruction, operand_a);
    break;
  case FMADD_S:
    result = decode_R4TYPE(instruction, operand_a, operand_b, operand_c);
    break;
  case FMSUB_S:
    result = decode_R4TYPE(instruction, operand_a, operand_b, operand_c);
    break;
  case FNMSUB_S:
    result = decode_R4TYPE(instruction, operand_a, operand_b, operand_c);
    break;
  case FNMADD_S:
    result = decode_R4TYPE(instruction, operand_a, operand_b, operand_c);
    break;
  case OP_FP:
    result = decode_RTYPE(instruction, operand_a, operand_b);
    break;
  case CSR:
    result = decode_CSRTYPE(instruction, operand_a);
    break;
  default:
    result.valid = 0;
    break;
  }
  result.speculative = 1;
  result.id = id;
  result.mode = mode;
  #ifdef ZFINX
    result.toXReg = true;
  #endif

  return result;
};


FpuPipeObj decode_R4TYPE(uint32_t instr, unsignedType operand_a, unsignedType operand_b, unsignedType operand_c) {
  RTYPE dec_instr = {.instr = instr};
  FpuPipeObj result = {};
  result.valid = 1;
  result.addrFrom = {dec_instr.parts_r4type.rs1, dec_instr.parts_r4type.rs2, dec_instr.parts_r4type.rs3};
  result.addrTo = {dec_instr.parts_r4type.rd};
  #ifdef RV64
    result.operand_a.bitpattern_64 = operand_a; //Only used if ZFINX
    result.operand_b.bitpattern_64 = operand_b; //Only used if ZFINX
    result.operand_c.bitpattern_64 = operand_c; //Only used if ZFINX
  #else
    result.operand_a.bitpattern = operand_a; //Only used if ZFINX
    result.operand_b.bitpattern = operand_b; //Only used if ZFINX
    result.operand_c.bitpattern = operand_c; //Only used if ZFINX
  #endif
  #ifdef ZFINX
    result.use_rs_i[0] = true;
    result.use_rs_i[1] = true;
    result.use_rs_i[2] = true;
  #else
    result.use_rs_i[0] = false;
    result.use_rs_i[1] = false;
    result.use_rs_i[2] = false;
  #endif
  result.instr = instr; //save instruction
  result.instr_type = it_R4TYPE;
  return result;
}

FpuPipeObj decode_RTYPE(uint32_t instr, unsignedType operand_a, unsignedType operand_b) {
  //TODO: add execution
  RTYPE dec_instr = {.instr = instr}; //"Decode" into ITYPE
  FpuPipeObj result = {};
  result.valid = 1;
  result.addrFrom = {dec_instr.parts.rs1, dec_instr.parts.rs2};
  result.addrTo = {dec_instr.parts.rd};
  result.instr_type = it_RTYPE; //For decoding in execution step.
  result.instr = instr; //Save instruction
  #ifdef RV64
    result.operand_a.bitpattern_64 = operand_a; //Only used if ZFINX
    result.operand_b.bitpattern_64 = operand_b; //Only used if ZFINX
  #else
    result.operand_a.bitpattern = operand_a; //Overwritten for relevant functions
    result.operand_b.bitpattern = operand_b; //Overwritten for relevant functions
  #endif
  #ifdef ZFINX
    result.use_rs_i[0] = true;
    result.use_rs_i[1] = true;
  #else
    result.use_rs_i[0] = false;
    result.use_rs_i[1] = false;
  #endif
  result.use_rs_i[2] = false;
  result.remaining_ex_cycles = NUM_CYCLES_DEFAULT; //Default number of cycles
  //Override relevant parameters based on function
  switch (dec_instr.parts.funct5)
  {
    case FADD:
    {
      #ifdef NUM_CYCLES_FADD
        result.remaining_ex_cycles = NUM_CYCLES_FADD;
      #endif
      break;
    }
    case FSUB:
    {
      #ifdef NUM_CYCLES_FSUB
        result.remaining_ex_cycles = NUM_CYCLES_FSUB;
      #endif
      break;
    }
    case FMUL:
    {
      #ifdef NUM_CYCLES_FMUL
        result.remaining_ex_cycles = NUM_CYCLES_FMUL;
      #endif
      break;
    }
    case FDIV:
    {
      #ifdef NUM_CYCLES_FDIV
        result.remaining_ex_cycles = NUM_CYCLES_FDIV; //TODO: we need to check if this has been added. Thats not given
      #endif
      break;
    }
    case FSGNJ:
    {
      #ifdef NUM_CYCLES_FSGNJ
        result.remaining_ex_cycles = NUM_CYCLES_FSGNJ;
      #endif
      break;
    }
    case FMIN_MAX:
    {
      #ifdef NUM_CYCLES_FMIN_MAX
        result.remaining_ex_cycles = NUM_CYCLES_FMIN_MAX;
      #endif
      break;
    }
    case FSQRT:
    {
      #ifdef NUM_CYCLES_FSQRT
        result.remaining_ex_cycles = NUM_CYCLES_FSQRT; //TODO: we need to check if this has been added. Thats not given
      #endif
      result.addrFrom = {dec_instr.parts.rs1, 999}; //sqrt only dependent on rs1
      break;
    }
    case FCMP:
    {
      #ifdef NUM_CYCLES_FCMP
        result.remaining_ex_cycles = NUM_CYCLES_FCMP;
      #endif
      result.toXReg = true;
      break;
    }
    case FCVT_W_S:
    {
      #ifdef NUM_CYCLES_FCVT_W
        result.remaining_ex_cycles = NUM_CYCLES_FCVT_W;
      #endif
      result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
      result.toXReg = true;
      break;
    }
    case FCVT_S_W: //FCVT.S.W[U]
    {
      #ifdef NUM_CYCLES_FCVT_S_W
        result.remaining_ex_cycles = NUM_CYCLES_FCVT_S_W;
      #endif
      result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
      result.fromXReg = true;
      result.use_rs_i[0] = true;
      result.operand_a.u = operand_a;
      break;
    }
    case FCLASS_FMV_X_W:
    {
      if (dec_instr.parts.funct3 == 0) {
        #ifdef NUM_CYCLES_FMV_X_W
        result.remaining_ex_cycles = NUM_CYCLES_FMV_X_W;
        #endif
      } else if (dec_instr.parts.funct3 == 1) {
        #ifdef NUM_CYCLES_FCLASS
        result.remaining_ex_cycles = NUM_CYCLES_FCLASS;
        #endif
      }

      result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
      result.addrTo = {dec_instr.parts.rd};
      result.toXReg = true;
      break;
    }
    case FMV_W_X:
    {
      #ifdef NUM_CYCLES_FMV_W_X
        result.remaining_ex_cycles = NUM_CYCLES_FMV_W_X;
      #endif
      result.fromXReg = true;
      result.use_rs_i[0] = true;
      result.operand_a.bitpattern = operand_a;
      break;
    }
  }
  return result;
}

FpuPipeObj decode_ITYPE(uint32_t instr, unsignedType operand_a) {
  ITYPE dec_instr = {.instr = instr}; //"Decode" into ITYPE
  FpuPipeObj result = {};
  result.valid = 1;
  int32_t offset = dec_instr.parts.offset;
  int32_t extendedOffset = (offset << 20) >> 20; //Sign extend - TODO: Extension independent
  result.addrFrom = {operand_a + extendedOffset};
  result.addrTo = dec_instr.parts.rd;
  result.fromMem = 1;
  result.instr = instr; //Save instruction
  result.instr_type = it_ITYPE;
  #ifdef RV64
    result.operand_a.bitpattern_64 = operand_a; //Only used if ZFINX
  #else
    result.operand_a.bitpattern = operand_a;
  #endif
  result.size = dec_instr.parts.funct3; //Size of word
  return result;
}

FpuPipeObj decode_STYPE(uint32_t instr, unsignedType operand_a){
  STYPE dec_instr = {.instr = instr}; //Decode into STYPE
  FpuPipeObj result = {};
  result.valid = 1;
  result.addrFrom = {dec_instr.parts.rs2};
  int32_t upper_offset = (dec_instr.parts.offset << 25) >> 20;
  int32_t lower_offset = dec_instr.parts.imm_4_0;
  #ifdef RV64
    result.operand_a.bitpattern_64 = operand_a; //Only used if ZFINX
  #else
    result.operand_a.bitpattern = operand_a;
  #endif
  int32_t full_offset = 0 | upper_offset | lower_offset; //Sign extend - TODO: Extension independent
  result.addrTo = {operand_a + full_offset};
  result.toMem = true;
  result.instr = instr; //Save instruction
  result.instr_type = it_STYPE;
  result.size = dec_instr.parts.funct3; //Size of word

  return result;
}

FpuPipeObj decode_CSRTYPE(uint32_t instr, unsignedType operand_a) {
  CSRTYPE dec_instr = {.instr = instr}; //Decode into CSRTYPE
  FpuPipeObj result = {};
  if (!(0x001 <= dec_instr.parts.csr <= 0x003)) { //If CSR op is not a FCSR instruction, return invalid
    FpuPipeObj result = {};
    result.valid = 0;
    return result;
  }
  if (dec_instr.parts.funct3 == 0b001){
    result.use_rs_i[0] = true;
    result.operand_a.bitpattern = operand_a;
    result.addrFrom = {dec_instr.parts.rs1};
  }
  result.valid = 1;
  result.addrTo = dec_instr.parts.rd;
  result.instr = instr; //Save instruction
  result.instr_type = it_CSRTYPE;
  result.toXReg = true;
  return result;
}
