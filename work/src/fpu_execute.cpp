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
    case it_CSRTYPE:
    {
      execute_CSRTYPE(op, registerFile);
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
  RISCV_FMT fmt = dec_instr.parts_r4type.fmt;
  //Get data from registerFile or from forwarded data if forwarded
  FPNumber data1(fmt), data2(fmt), data3(fmt);
  #ifdef ZFINX
    data1 = op.operand_a;
    data2 = op.operand_b;
    data3 = op.operand_c;
  #else
    data1 = registerFile->read(op.addrFrom[0]);
    data2 = registerFile->read(op.addrFrom[1]);
    data3 = registerFile->read(op.addrFrom[2]);
  #endif
  #ifdef FORWARDING
    if(op.stalledByCtrl){
      data1 = op.addrFrom[0] == op.fw_addr ? op.fw_data : data1;
      data2 = op.addrFrom[1] == op.fw_addr ? op.fw_data : data2;
      data3 = op.addrFrom[2] == op.fw_addr ? op.fw_data : data3;
    }
  #endif

  //Switch signs to match relevant op.
  switch (dec_instr.parts_r4type.opcode)
  {
  case FMADD:
  {
    break;
  }
  case FMSUB:
  {
    data3 = -data3;
    break;
  }
  case FNMSUB:
  {
    data2 = -data2;
    break;
  }
  case FNMADD:
  {
    data2 = -data2;
    data3 = -data3;
    break;
  }
  default:
  {
    std::feraiseexcept(FE_INVALID); //Invalid operation
    break;
  }
  }

  //Execute
  switch (dec_instr.parts_r4type.fmt)
  {
  case S:
  {
    op.data = fmaf(data1, data2, data3);
    if(((float)data1 == INFINITY && (float)data2 == 0) || ((float)data1 == 0 && (float)data2 == INFINITY) || ((float)data1 == -INFINITY && (float)data2 == 0) || ((float)data1 == 0 && (float)data2 == -INFINITY)) { //Required by RISC-V ISA, but not IEEE 754
      std::feraiseexcept(FE_INVALID); //raise invalid
    }
    break;
  }
  case D:
  {
    op.data = fma(data1, data2, data3);
    if(((double)data1 == INFINITY && (double)data2 == 0) || ((double)data1 == 0 && (double)data2 == INFINITY) || ((double)data1 == -INFINITY && (double)data2 == 0) || ((double)data1 == 0 && (double)data2 == -INFINITY)) { //Required by RISC-V ISA, but not IEEE 754
      std::feraiseexcept(FE_INVALID); //raise invalid
    }
    break;
  }
  }
  op.flags |= getFlags(); //Get flags and add to result.
};



void execute_RTYPE(FpuPipeObj& op, FpuRf* registerFile){
  std::feclearexcept(FE_ALL_EXCEPT); //Clear all flags
  RTYPE dec_instr = {.instr = op.instr}; //"Decode" into RTYPE
  RISCV_FMT fmt = dec_instr.parts.fmt;
  FPNumber data1(fmt), data2(fmt);
  op.data = FPNumber(fmt);
  #ifdef ZFINX
    data1 = op.operand_a;
    data2 = op.operand_b;
  #else
    data1 = registerFile->read(op.addrFrom[0]);
    data2 = registerFile->read(op.addrFrom[1]);
  #endif
  #ifdef FORWARDING
    if(op.stalledByCtrl){
      data1 = op.addrFrom[0] == op.fw_addr ?  op.fw_data : data1;
      data2 = op.addrFrom[1] == op.fw_addr ?  op.fw_data : data2;
    }
  #endif

  switch (dec_instr.parts.funct5)
  {
  case FADD:
  {
    op.data = data1 + data2;
    break;
  }
  case FSUB:
  {
    op.data = data1 - data2;
    break;
  }
  case FMUL:
  {
    op.data = data1 * data2;
    break;
  }
  case FDIV:
  {
    op.data = data1 / data2;
    break;
  }
  case FSQRT:
  {
    if (fmt == D) {
      op.data = sqrt(data1);
    } else {
      op.data = sqrtf(data1);
    }
    break;
  }
  case FSGNJ:
  {
    // op.data.parts.exponent = data1.parts.exponent;
    // op.data.parts.mantissa = data1.parts.mantissa;
    switch (dec_instr.parts.funct3)
    {
    case 0b000: //FSGNJ.S
    {
      // op.data.parts.sign = data2.parts.sign;
      break;
    }
    case 0b001: //FSGNJN.S
    {
      // op.data.parts.sign = !data2.parts.sign;
      break;
    }
    case 0b010: //FSGNJX.S
    {
      // op.data.parts.sign = data2.parts.sign ^ data1.parts.sign;
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
      if (fmt == D) {
        op.data = std::fmin((double)data1, (double)data2);
      } else {
        op.data = std::fminf((float)data1, (float)data2);
      }

      if (data1 == 0 && data2 == 0) //Fmin is not sensitive to 0 sign
      {
        op.data = data1.getSign() ? data1 : data2;
      }
      break;
    }
    case 0b001: //FMAX
    {
      if (fmt == D) {
        op.data = std::fmax((double)data1, (double)data2);
      } else {
        op.data = std::fmaxf((float)data1, (float)data2);
      }

      if (data1 == 0 && data2 == 0)//Fmax is not sensitive to 0 sign
      {
        op.data = data1.getSign() ? data2 : data1;
      }
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
  case FCMP:
  {
    switch (dec_instr.parts.funct3)
    {
    case 0b010: //FEQ.S
    {
      data1 == data2 ? op.data = (unsigned int) 1 : op.data = (unsigned int) 0;
      break;
    }
    case 0b001: //FLT.s
    {
      data1 < data2 ? op.data = (unsigned int) 1 : op.data = (unsigned int) 0;
      break;
    }
    case 0b000: //FLE.S
    {
      data1 <= data2 ? op.data = (unsigned int) 1 : op.data = (unsigned int) 0;
      break;
    }
    default:
    {
      op.data = (unsigned int) 0;
      std::feraiseexcept(FE_INVALID); //raise invalid
      break;
    }
    }
    break;
  }
  case FCVT_W_S:
  {
    setRoundingMode(dec_instr.parts.funct3); //Fcvt always uses RM for rounding mode
    switch (dec_instr.parts.rs2)
    {
    case 0b00000:
    {
      if (fmt == D) { //FCVT.W.D
        op.data = static_cast<int32_t>(nearbyint(data1)); //Use nearbyint instead of round, round does not follow rounding mode set in cfenv
      } else { //FCVT.W.S
        op.data = static_cast<int32_t>(nearbyintf(data1)); //Use nearbyint instead of round, round does not follow rounding mode set in cfenv
      }
      break;
    }
    case 0b00001:
    {
      if (fmt == D) { //FCVT.WU.D
        if (nearbyint(data1) < 0.0f || nearbyint(data1) > UINT32_MAX) {  // Check for out-of-range values
          op.data = (unsigned int) 0xFFFFFFFF;
          op.flags |= 0b00001;
        } else { //FCVT.WU.S
          // Convert if within range
          op.data = static_cast<unsigned int>(nearbyint(data1));
        }
      } else {
        if (std::isnan((float)data1) && !((unsigned int) data1 && 0x00400000)) {  // Check for sNaN
          op.data =(unsigned int) 0xFFFFFFFF;
          op.flags |= 0b00001;
        } else if (nearbyint((float)data1) < 0.0f || nearbyint((float)data1) > UINT32_MAX) {  // Check for out-of-range values
          op.data =(unsigned int) 0xFFFFFFFF;
          op.flags |= 0b00001;
        } else if(!std::isfinite((float)data1)){
          op.data = 0xFFFFFFFF;
          op.flags |= 0b00001;
        } else {
          // Convert if within range
          op.data = static_cast<unsigned int>(nearbyint((float)data1));
        }
      }

      break;
    }
    #ifdef RV64
      case 0b00010:  //64 bit conversion for RV64F
      {
        if(fmt == D){ //FCVT.L.D
          op.data = static_cast<int64_t>(nearbyint((double)data1));
        } else { //FCVT.L.S
          op.data = static_cast<int64_t>(nearbyintf(data1));
        }
        break;
      }
      case 0b00011:
      {
        if (fmt == D){ //FCVT.LU.D
          if (std::isnan((double)data1) && !( (unsigned long) data1 && 0x0008000000000000)) {  // Check for sNaN
            op.data = 0xFFFFFFFFFFFFFFFF;
            op.flags |= 0b00001;
          } else if (nearbyint((double)data1) < 0.0f || (double)data1 > UINT64_MAX) {  // Check for out-of-range values
            op.data = 0xFFFFFFFFFFFFFFFF;
            op.flags |= 0b00001;
          } else if(!std::isfinite((double)data1)){
            op.data = 0xFFFFFFFFFFFFFFFF;
            op.flags |= 0b00001;
          } else {
            op.data = static_cast<uint64_t>(nearbyint((double)data1));
          }
        } else { //FCVT.LU.S
          if (std::isnan((float)data1) && !( (unsigned int) data1 && 0x00400000)) {  // Check for sNaN
            op.data = 0xFFFFFFFFFFFFFFFF;
            op.flags |= 0b00001;
          } else if (nearbyintf(data1) < 0.0f || (float)data1 > UINT64_MAX) {  // Check for out-of-range values
            op.data = 0xFFFFFFFFFFFFFFFF;
            op.flags |= 0b00001;
          } else if(!std::isfinite((float)data1)){
            op.data = 0xFFFFFFFFFFFFFFFF;
            op.flags |= 0b00001;
          } else {
            op.data = static_cast<uint64_t>(nearbyintf(data1));
          }
        }

        break;
      }
    #endif
    default:
    {
      op.data = (int) 0;
      std::feraiseexcept(FE_INVALID); //raise invalid
    }
    }
    break;
  }
  case FCVT_S_W: //FCVT.S.W[U]
  {
    setRoundingMode(dec_instr.parts.funct3); //Fcvt always uses RM for rounding mode
    switch (dec_instr.parts.rs2)
    {
    case 0b00000: //FCVT.S.W
    {
      if(fmt == D){
        op.data = static_cast<double>((int)op.operand_a);
      } else {
        op.data = static_cast<float>((int)op.operand_a);
      }
      break;
    }
    case 0b00001: //FCVT.S.WU
    {
      if(fmt == D){
        op.data = static_cast<double>((unsigned int)op.operand_a);
      } else {
        op.data = static_cast<float>((unsigned int)op.operand_a);
      }
      break;
    }
    #ifdef RV64
      case 0b00010: //FCVT.S.L
      {
        if(fmt == D){
          op.data = static_cast<double>((long)op.operand_a);
        } else {
          op.data = static_cast<float>((long)op.operand_a);
        }
        break;
      }
      case 0b00011: //FCVT.S.LU
      {
        if(fmt == D){
          op.data = static_cast<double>((unsigned long)op.operand_a);
        } else {
          op.data = static_cast<float>((unsigned long)op.operand_a);
        }
        break;
      }
    #endif
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
      op.data = data1;
      break;
    }
    case 0b001: //FCLASS.S
    {
      float class_data = data1;
      if (fmt == D) {
        double class_data  = data1;
      }
      if(isSubnormal(class_data))
      {
        //subnormal number
        if (class_data < 0)
        {
          op.data = 0b0000000100;
        } else if (class_data > 0)
        {
          op.data = 0b0000100000;
        } else {
          std::feraiseexcept(FE_INVALID); //raise invalid
        }
      } else
      {
        //normal numbers
        if (class_data == -INFINITY) //negative inf.
        {
          op.data = 0b0000000001;
        } else if (class_data < 0) //negative normal number
        {
          op.data = 0b0000000010;
        } else if (class_data == 0 && class_data == 1) //negative zero
        {
          op.data = 0b0000001000;
        } else if (class_data == 0 && class_data == 0) //positive 0
        {
          op.data = 0b0000010000;

        } else if (class_data == INFINITY) //positive inf
        {
          op.data = 0b0010000000;
        } else if (std::isnan(class_data) && !( (unsigned int)data1  & (unsigned int) data1 && 0x00400000)) //If leading mantissa-bit is not set -> sNaN
        {
          op.data = 0b0100000000; //SNaN
        } else if (std::isnan(class_data))
        {
          op.data = 0b1000000000; //QNaN
        } else if (class_data > 0) //positive normal number
        {
          op.data = 0b0001000000;
        } else {
          op.data = 0b0000000000;
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
    op.data =  op.operand_a;
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
  #ifdef FORWARDING
    if(op.stalledByCtrl){
      op.data = op.addrFrom[0] == op.fw_addr ? op.fw_data : op.data;
    }
  #endif
}


void execute_CSRTYPE(FpuPipeObj& op, FpuRf* registerFile){
  CSRTYPE dec_instr = {.instr = op.instr}; //"Decode" into CSRTYPE to get parts
  switch(dec_instr.parts.funct3)
  {
    case(0b001): //CSRRW
    {
      switch(dec_instr.parts.csr)
      {
        case(0x001): //fflags
        {
          //Swap fflags with rs1
          unsigned int flags = registerFile->getFlags();
          op.data = flags;
          registerFile->setFlags(op.operand_a);
          break;
        }
        case(0x002): //frm
        {
          //Swap frm with rs1
          unsigned int rm = registerFile->readfrm();
          op.data = rm;
          registerFile->setfrm(op.operand_a);
          break;
        }
        case(0x003): //fcsr
        {
          //Swap fcsr with rs1
          unsigned int data = registerFile->read_fcsr().v;
          op.data = data;
          registerFile->write_fcsr(op.operand_a);
          break;
        }
        default:
        {
          std::feraiseexcept(FE_INVALID); //raise invalid
          break;
        }
      }
      break;
    }
    case(0b010): //CSRRS
    {
      //Go through different csr indicators (0x001 0x002 0x003)
      switch(dec_instr.parts.csr)
      {
        case(0x001):
        {
          //read fflags
          unsigned int flags = registerFile->getFlags();
          op.data = flags;
          break;
        }
        case(0x002):
        {
          //read frm
          unsigned int rm = registerFile->readfrm();
          op.data = rm;
          break;
        }
        case(0x003):
        {
          //read fcsr
          unsigned int data = registerFile->read_fcsr().v;
          op.data = data;
          break;
        }
      }
      break;
    }
    default:
    {
      std::feraiseexcept(FE_INVALID); //raise invalid
      break;
    }
  }
  op.flags |=  getFlags();
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
    // std::fesetround(); //RMM NOT A PART OF cfenv.
    break;
  }
  case 0b101: //Invalid. Reserved for future use
  {
    break;
  }
  case 0b110: //Invalid. Reserved for future use
  {
    break;
  }
  case 0b111: //Dynamic rounding mode. Do nothing - handled in executeOp()
  {
    break;
  }
  default:
    break;
  }

};


//Helper function
bool isSubnormal(float num) {
  return 0;//num.parts.exponent == 0 && num.parts.mantissa != 0;
}

bool isSubnormal(double num) {
  return 0;//num.parts.exponent == 0 && num.parts.mantissa != 0;
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

