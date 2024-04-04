/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Execute operations for fpu.
*/
#include "fpu_execute.h"
#include <iostream>

void executeOp(FpuPipeObj& op, FpuRf* registerFile) {
  #ifndef NO_ROUNDING  // NO_ROUNDING uses c++ default rounding mode.
    unsigned int rm = registerFile->readfrm();
    if (rm == 0b111) //0b111 is dynamic rounding, and is handled for the relevant instructions later.
    {
      RTYPE rm_instr = {.instr = op.instr}; //Decode into RTYPE to extract rm (same field for R4Type)
      setRoundingMode(rm_instr.parts.funct3);
    } else {
      setRoundingMode(rm);
    }
  #endif


  switch (op.instr_type)
    {
    case it_ITYPE:
    {
      execute_ITYPE(op, registerFile);
      break;
    }
    case it_STYPE:
    {
      execute_STYPE(op, registerFile);
      break;
    }
    case it_RTYPE:
    {
      execute_RTYPE(op, registerFile);
      break;
    }
    case it_R4TYPE:
    {
      execute_R4TYPE(op, registerFile);
      break;
    }
    default:
      //If no operation is in pipeline: do nothing
      break;
  }
  registerFile->raiseFlags(op.flags);
}

void execute_R4TYPE(FpuPipeObj& op, FpuRf* registerFile){
  std::feclearexcept(FE_ALL_EXCEPT); //Clear all flags
  RTYPE dec_instr = {.instr = op.instr};
  //Get data from registerFile
  FPNumber data1 = registerFile->read(op.addrFrom[0]);
  FPNumber data2 = registerFile->read(op.addrFrom[1]);
  FPNumber data3 = registerFile->read(op.addrFrom[2]);
  //Compute result -- will be added to pipeline
  switch (dec_instr.parts_r4type.opcode)
  {
  case FMADD_S:
  {
    op.data.f = fmaf(data1.f, data2.f, data3.f);
    if((data1.f == INFINITY && data2.f == 0) || (data1.f == 0 && data2.f == INFINITY) || (data1.f == -INFINITY && data2.f == 0) || (data1.f == 0 && data2.f == -INFINITY)) { //Required by RISC-V ISA, but not IEEE 754
      std::feraiseexcept(FE_INVALID); //raise invalid
    }
    break;
  }
  case FMSUB_S:
  {
    op.data.f = fmaf(data1.f, data2.f, -data3.f);
    if((data1.f == INFINITY && data2.f == 0) || (data1.f == 0 && data2.f == INFINITY) || (data1.f == -INFINITY && data2.f == 0) || (data1.f == 0 && data2.f == -INFINITY)) { //Required by RISC-V ISA, but not IEEE 754
      std::feraiseexcept(FE_INVALID); //raise invalid
    }
    break;
  }
  case FNMSUB_S:
  {
    op.data.f = fmaf(data1.f, -data2.f, data3.f); //Counterintuitively named wrt FMADD/FMSUB
    if((data1.f == INFINITY && data2.f == 0) || (data1.f == 0 && data2.f == INFINITY) || (data1.f == -INFINITY && data2.f == 0) || (data1.f == 0 && data2.f == -INFINITY)) { //Required by RISC-V ISA, but not IEEE 754
      std::feraiseexcept(FE_INVALID); //raise invalid
    }
    break;
  }
  case FNMADD_S:
  {
    op.data.f = fmaf(data1.f, -data2.f, -data3.f);
    if((data1.f == INFINITY && data2.f == 0) || (data1.f == 0 && data2.f == INFINITY) || (data1.f == -INFINITY && data2.f == 0) || (data1.f == 0 && data2.f == -INFINITY)) { //Required by RISC-V ISA, but not IEEE 754
      std::feraiseexcept(FE_INVALID); //raise invalid
    }
    break;
  }
  default:
    std::feraiseexcept(FE_INVALID); //Invalid operation
    break;
  }
  op.flags |= getFlags(); //Get flags and add to result.

  // if(registerFile != nullptr) {
  //   registerFile->write(op.addrTo, op.data); //This might need to be moved to WriteBack stage
  // }

};



void execute_RTYPE(FpuPipeObj& op, FpuRf* registerFile){
  std::feclearexcept(FE_ALL_EXCEPT); //Clear all flags
  RTYPE dec_instr = {.instr = op.instr}; //"Decode" into ITYPE
  FPNumber data1 = registerFile->read(op.addrFrom[0]);
  FPNumber data2 = registerFile->read(op.addrFrom[1]);
  switch (dec_instr.parts.funct7)
  {
  case FADD_S:
  {
    op.data.f = data1.f + data2.f;
    break;
  }
  case FSUB_S:
  {
    op.data.f = data1.f - data2.f;
    break;
  }
  case FMUL_S:
  {
    op.data.f = data1.f * data2.f;
    break;
  }
  case FDIV_S:
  {
    op.data.f = data1.f / data2.f;
    break;
  }
  case FSQRT_S:
  {
    op.data.f = sqrt(data1.f);
    break;
  }
  case FSGNJ:
  {
    op.data.parts.exponent = data1.parts.exponent;
    op.data.parts.mantissa = data1.parts.mantissa;
    switch (dec_instr.parts.funct3)
    {
    case 0b000: //FSGNJ.S
    {
      op.data.parts.sign = data2.parts.sign;
      break;
    }
    case 0b001: //FSGNJN.S
    {
      op.data.parts.sign = !data2.parts.sign;
      break;
    }
    case 0b010: //FSGNJX.S
    {
      op.data.parts.sign = data2.parts.sign ^ data1.parts.sign;
      break;
    }
    default:
    {
      std::feraiseexcept(FE_INVALID); //raise invalid
      op.data = data1;
      break;
    }
    }
    break;
  }
  case FMIN_MAX:
  {
    switch (dec_instr.parts.funct3)
    {
    case 0b000: //FMIN
    {
      op.data.f = std::min(data1.f, data2.f);
      break;
    }
    case 0b001: //FMAX
    {
      op.data.f = std::max(data1.f, data2.f);
      break;
    }
    default:
    {
    std::feraiseexcept(FE_INVALID); //raise invalid
    op.data = data1;
    break;
    }
    }
  }
  case FCMP:
  {
    switch (dec_instr.parts.funct3)
    {
    case 0b010: //FEQ.S
    {
      data1.f == data2.f ? op.data.u = 1 : op.data.u = 0;
      break;
    }
    case 0b001: //FLT.s
    {
      data1.f < data2.f ? op.data.u = 1 : op.data.u = 0;
      break;
    }
    case 0b000: //FLE.S
    {
      data1.f <= data2.f ? op.data.u = 1 : op.data.u = 0;
      break;
    }
    default:
    {
      op.data.u = 0;
      std::feraiseexcept(FE_INVALID); //raise invalid
      break;
    }
    }
    break;
  }
  case FCVT_W_S:
  {
    switch (dec_instr.parts.rs2)
    {
    case 0b00000: //FCVT.W.S
    {
      op.data_signed = true;
      op.data.s= static_cast<int32_t>(nearbyint(data1.f)); //Use nearbyint instead of round, round does not follow rounding mode set in cfenv
      break;
    }
    case 0b00001: //FCVT.WU.S
    {
      if (std::isnan(data1.f) && !(data1.parts.mantissa & 0x00400000)) {  // Check for sNaN
        op.data.u = 0xFFFFFFFF;
        op.flags |= 0b00001;
      } else if (nearbyint(data1.f) < 0.0f || nearbyint(data1.f) > UINT32_MAX) {  // Check for out-of-range values
        op.data.u = 0xFFFFFFFF;
        op.flags |= 0b00001;
      } else {
        // Convert if within range
        op.data.u = static_cast<unsigned int>(nearbyint(data1.f));
      }
      break;
    }
    default:
    {
      op.data.s = 0;
      std::feraiseexcept(FE_INVALID); //raise invalid
    }
    }
    break;
  }
  case FCVT_S_W: //FCVT.S.W[U]
  {
    switch (dec_instr.parts.rs2)
    {
    case 0b00000: //FCVT.S.W
    {
      op.data.f = static_cast<float>(op.operand_a.s);
      break;
    }
    case 0b00001: //FCVT.S.WU
    {
      op.data.f = static_cast<float>(static_cast<uint32_t>(op.operand_a.u)); //Cast to unsigned first - needs to be requested from cpu
      break;
    }
    default:
      std::feraiseexcept(FE_INVALID); //raise invalid
      break;
    }
    break;
  }
  case FCLASS_FMV_X_W:
  {
    switch (dec_instr.parts.funct3)
    {
    case 0b000: //FMV_X_W
    {
      op.data_signed = true;
      op.data.s = data1.bitpattern;
      break;
    }
    case 0b001: //FCLASS.S
    {
      if(isSubnormal(data1))
      {
        //subnormal number
        if (data1.f < 0)
        {
          op.data.bitpattern = 0b0000000100;
        } else if (data1.f > 0)
        {
          op.data.bitpattern = 0b0000100000;
        } else {
          std::feraiseexcept(FE_INVALID); //raise invalid
        }
      } else
      {
        //normal numbers
        if (data1.f == -INFINITY) //negative inf.
        {
          op.data.bitpattern = 0b0000000001;
        } else if (data1.f < 0) //negative normal number
        {
          op.data.bitpattern = 0b0000000010;
        } else if (data1.f == 0 && data1.parts.sign == 1) //negative zero
        {
          op.data.bitpattern = 0b0000001000;
        } else if (data1.f == 0 && data1.parts.sign == 0) //positive 0
        {
          op.data.bitpattern = 0b0000010000;

        } else if (data1.f == INFINITY) //positive inf
        {
          op.data.bitpattern = 0b0010000000;
        } else if (std::isnan(data1.f) && !(data1.bitpattern & 0x00400000)) //If leading mantissa-bit is not set -> sNaN
        {
          op.data.bitpattern = 0b0100000000; //SNaN
        } else if (std::isnan(data1.f))
        {
          op.data.bitpattern = 0b1000000000; //QNaN
        } else if (data1.f > 0) //positive normal number
        {
          op.data.bitpattern = 0b0001000000;
        } else {
          op.data.bitpattern = 0b0000000000;
          std::feraiseexcept(FE_INVALID); //raise invalid
        };
      }
      break;
    }
    default:
      std::feraiseexcept(FE_INVALID); //raise invalid
      break;
    }
    break;
  }
  case FMV_W_X:
  {
    //Moves bitpattern from X to W(F)
    op.data.bitpattern =  op.operand_a.bitpattern;
    break;
  }
  default:
    std::feraiseexcept(FE_INVALID); //raise invalid
    break;
  }

  op.flags |=  getFlags();
};


void execute_ITYPE(FpuPipeObj& op, FpuRf* registerFile){
}
void execute_STYPE(FpuPipeObj& op, FpuRf* registerFile){
  if (registerFile != nullptr) {
    op.data = registerFile->read(op.addrFrom.front());
  }
}

void setRoundingMode(unsigned int rm){ //Sets c++ rounding mode. FCSR is written seperately
  switch (rm)
  {
  case 0b000: //RNE
  {
    std::fesetround(FE_TONEAREST);
    break;
  }
  case 0b001: //RTZ
  {
    std::fesetround(FE_TOWARDZERO);
    break;
  }
  case 0b010: //RDN
  {
    std::fesetround(FE_DOWNWARD);
    break;
  }
  case 0b011: //RUP
  {
    std::fesetround(FE_UPWARD);
    break;
  }
  case 0b100: //RMM
  {
    // std::fesetround(); //RMM NOT A PART OF cfenv. TODO: is it okay to skip?
    break;
  }
  case 0b101: //Invalid. Reserved for future use TODO: write NV?
  {
    break;
  }
  case 0b110: //Invalid. Reserved for future use
  {
    break;
  }
  case 0b111: //Dynamic rounding mode. Do nothing - handled in fpu_top.cpp
  {
    break;
  }
  default:
    break;
  }

};


//Helper function
bool isSubnormal(FPNumber num) {
  return num.parts.exponent == 0 && num.parts.mantissa != 0;
}

// void test_isSubormal

unsigned int getFlags() {
  unsigned int flags = 0;
  flags |= std::fetestexcept(FE_INVALID) ? 1 : 0;
  flags |= std::fetestexcept(FE_DIVBYZERO) ? (1 << 1) : 0;
  flags |= std::fetestexcept(FE_OVERFLOW) ? (1 << 2) : 0;
  flags |= std::fetestexcept(FE_UNDERFLOW) ? (1 << 3) : 0;
  flags |= std::fetestexcept(FE_INEXACT) ? (1 << 4) : 0;
  return flags;
}

