/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Decode operations for fpu.
*/

#include "fpu_decode.h"



FpuPipeObj decodeOp(uint32_t instruction, unsigned int id) { //Add more fields if needed by eXtension interface
  //Get result of operation
  unsigned int opcode = instruction & 127 ; //Get first 7 bit
  FpuPipeObj result = {};
  result.valid = true;
  switch (opcode)
  {
  case FLW:
    result = decode_ITYPE(instruction);
    break;
  case FSW:
    result = decode_STYPE(instruction);
    break;
  case FMADD_S:
    result = decode_R4TYPE(instruction);
    break;
  case FMSUB_S:
    result = decode_R4TYPE(instruction);
    break;
  case FNMSUB_S:
    result = decode_R4TYPE(instruction);
    break;
  case FNMADD_S:
    result = decode_R4TYPE(instruction);
    break;
  case OP_FP:
    result = decode_RTYPE(instruction);
    break;
  default:
    result.valid = false; //TODO: add tests for validity in each decode aswell
    break;
  }
  result.id = id;
  return result;
};


FpuPipeObj decode_R4TYPE(uint32_t instr) {
  RTYPE dec_instr = {.instr = instr};
  FpuPipeObj result = {};
  result.addrFrom = {dec_instr.parts_r4type.rs1, dec_instr.parts_r4type.rs2, dec_instr.parts_r4type.rs3};
  result.addrTo = {dec_instr.parts_r4type.rd};
  result.instr = instr; //save instruction
  result.instr_type = it_R4TYPE;
  return result;
}

FpuPipeObj decode_RTYPE(uint32_t instr) {
  RTYPE dec_instr = {.instr = instr}; //"Decode" into ITYPE
  FpuPipeObj result = {};
  result.addrFrom = {dec_instr.parts.rs1, dec_instr.parts.rs2};
  result.addrTo = {dec_instr.parts.rd};
  result.instr_type = it_RTYPE; //For decoding in execution step.
  result.instr = instr; //Save instruction
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
      break;
    }
    case FCLASS_FMV_X_W:
    {
      result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
      result.toXReg = true;
    }
    case FMV_W_X:
    {
      result.fromXReg = true;
      break;
    }
  }
  return result;
}


FpuPipeObj decode_ITYPE(uint32_t instr) {
  ITYPE dec_instr = {.instr = instr}; //"Decode" into ITYPE
  FpuPipeObj result = {};
  result.addrFrom = {}; //FLW is atomic
  result.addrTo = dec_instr.parts.rd;
  result.instr = instr; //Save instruction
  result.instr_type = it_ITYPE;
  return result;
}

FpuPipeObj decode_STYPE(uint32_t instr){
  STYPE dec_instr = {.instr = instr}; //Decode into STYPE
  FpuPipeObj result = {};
  result.addrFrom = {dec_instr.parts.rs2};
  result.addrTo = 0; //destination is memory
  result.toMem = true;
  result.instr = instr; //Save instruction
  result.instr_type = it_STYPE;
  return result;
}
