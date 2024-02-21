/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Decode operations for fpu.
*/

#include "fpu_decode.h"

#include <iostream>

FpuPipeObj decodeOp(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c) { //Add more fields if needed by eXtension interface
  //Get result of operation
  unsigned int opcode = instruction & 127 ; //Get first 7 bit
  FpuPipeObj result = {};
  switch (opcode)
  {
  case FLW:
    result = decode_ITYPE(instruction);
    break;
  case FSW:
    result = decode_STYPE(instruction);
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
  default:
    result.valid = 0; //TODO: add tests for validity in each decode aswell
    break;
  }
  result.id = id;

  return result;
};


FpuPipeObj decode_R4TYPE(uint32_t instr, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c) {
  RTYPE dec_instr = {.instr = instr};
  FpuPipeObj result = {};
  result.valid = 1;
  result.addrFrom = {dec_instr.parts_r4type.rs1, dec_instr.parts_r4type.rs2, dec_instr.parts_r4type.rs3};
  result.addrTo = {dec_instr.parts_r4type.rd};
  result.operand_a.f = operand_a; //Only used if ZFINX
  result.operand_b.f = operand_b; //Only used if ZFINX
  result.operand_c.f = operand_c; //Only used if ZFINX
  result.instr = instr; //save instruction
  result.instr_type = it_R4TYPE;
  return result;
}

FpuPipeObj decode_RTYPE(uint32_t instr, unsigned int operand_a, unsigned int operand_b) {
  RTYPE dec_instr = {.instr = instr}; //"Decode" into ITYPE
  FpuPipeObj result = {};
  result.valid = 1;
  result.addrFrom = {dec_instr.parts.rs1, dec_instr.parts.rs2};
  result.addrTo = {dec_instr.parts.rd};
  result.instr_type = it_RTYPE; //For decoding in execution step.
  result.instr = instr; //Save instruction
  result.operand_a.f = operand_a; //Overwritten for relevant functions
  result.operand_b.f = operand_b; //Overwritten for relevant functions
  result.use_rs_i[0] = false;
  result.use_rs_i[1] = false;
  result.use_rs_i[2] = false;
  //Override relevant parameters based on function
  switch (dec_instr.parts.funct7)
  {
    case FSQRT_S:
    {
      result.addrFrom = {dec_instr.parts.rs1, 999}; //sqrt only dependent on rs1
      break;
    }
    case FCMP:
    {
      result.toXReg = true;
      break;
    }
    case FCVT_W_S:
    {
      result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
      result.toXReg = true;
      break;
    }
    case FCVT_S_W: //FCVT.S.W[U]
    {
      result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
      result.fromXReg = true;
      result.use_rs_i[0] = true;
      result.operand_a.u = operand_a;
      break;
    }
    case FCLASS_FMV_X_W:
    {
      result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
      result.addrTo = {dec_instr.parts.rd};
      result.toXReg = true;
      break;
    }
    case FMV_W_X:
    {
      result.fromXReg = true;
      result.use_rs_i[0] = true;
      result.operand_a.bitpattern = operand_a;
      break;
    }
  }
  return result;
}


FpuPipeObj decode_ITYPE(uint32_t instr) {
  ITYPE dec_instr = {.instr = instr}; //"Decode" into ITYPE
  FpuPipeObj result = {};
  result.valid = 1;
  int32_t offset = dec_instr.parts.offset;
  int32_t extendedOffset = (offset << 20) >> 20; //Sign extend - TODO: Extension independent
  result.addrFrom = {dec_instr.parts.rs1 + extendedOffset};
  result.addrTo = dec_instr.parts.rd;
  result.fromMem = 1;
  result.instr = instr; //Save instruction
  result.instr_type = it_ITYPE;
  return result;
}

FpuPipeObj decode_STYPE(uint32_t instr){
  STYPE dec_instr = {.instr = instr}; //Decode into STYPE
  FpuPipeObj result = {};
  result.valid = 1;
  result.addrFrom = {dec_instr.parts.rs2};
  int32_t offset = dec_instr.parts.offset;
  int32_t extendedOffset = (offset << 25) >> 25; //Sign extend - TODO: Extension independent
  result.addrTo = {dec_instr.parts.rs1 + extendedOffset};
  result.toMem = true;
  result.instr = instr; //Save instruction
  result.instr_type = it_STYPE;
  return result;
}
