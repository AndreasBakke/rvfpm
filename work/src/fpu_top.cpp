/*  rvfpm - 2023
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model with FP registers, and parameterized number of pipelines
*/
#include "fpu_top.h"

FPU::FPU (int pipelineStages, int rfDepth) : pipeline(pipelineStages), registerFile(rfDepth) {
  #ifndef ZFINX
    // registerFile(rfDepth)
  #else
    //Todo: Expand to support ZFINX
  #endif
};

FPU::~FPU(){

};

void FPU::resetFPU(){
  #ifndef ZFINX
    registerFile.resetFpuRf();
  #endif
  pipeline.flush();
};


FpuPipeObj FPU::decodeOp(uint32_t instruction, unsigned int id) {
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
    break;
  }
  result.id = id;
  return result;
};

void FPU::executeOp(FpuPipeObj& op, unsigned int fromMem, int fromXReg, unsigned int* id_out, uint32_t* toMem, uint32_t* toXReg, bool* toMem_valid, bool* toXReg_valid) {
  #ifndef NO_ROUNDING  // NO_ROUNDING uses c++ default rounding mode.
    unsigned int rm = registerFile.readfrm();
    if (rm == 0b111) //0b111 is dynamic rounding, and is handled for the relevant instructions later.
    {
      RTYPE rm_instr = {.instr = op.instr}; //Decode into RTYPE to extract rm (same field for R4Type)
      setRoundingMode(rm_instr.parts.funct3);
    } else {
      setRoundingMode(rm);
    }
  #endif

  //Set outputs to zero -> Overwritten in ex.
  if (toMem != nullptr) {
    *toMem = 0;
  }
  if (toXReg != nullptr) {
    *toXReg = 0;
  }
  if (toMem_valid != nullptr) {
    *toMem_valid = false;
  }
  if (toXReg_valid != nullptr) {
    *toXReg_valid = false;
  }
  if (id_out != nullptr) {
    *id_out = 0;
  }


  switch (op.instr_type)
    {
    case it_ITYPE:
    {
      execute_ITYPE(op, &registerFile, fromMem);
      break;
    }
    case it_STYPE:
    {
      execute_STYPE(op, &registerFile, id_out, toMem, toMem_valid);
      break;
    }
    case it_RTYPE:
    {
      execute_RTYPE(op, &registerFile, fromXReg, id_out, toXReg, toXReg_valid);
      break;
    }
    case it_R4TYPE:
    {
      execute_R4TYPE(op, &registerFile);
      break;
    }
    default:
      //If no operation is in pipeline: do nothing
      break;
  }
  registerFile.raiseFlags(op.flags);
  // if (flags_out != nullptr) {
  //   // *flags_out = op.flags;
  // }
}




FpuPipeObj FPU::operation(uint32_t instruction, unsigned int id, int fromXReg, unsigned int fromMem, unsigned int* id_out, uint32_t* toMem, uint32_t* toXReg, bool* pipelineFull, bool* toMem_valid, bool* toXReg_valid) {
  FpuPipeObj newOp = decodeOp(instruction, id);
  FpuPipeObj currOp = {};
  if(pipeline.getNumStages() == 0){ //Execute immediately
    currOp = newOp;
  } else
  { //add to pipeline - check for full pipeline/stalls etc.
    currOp = pipeline.step(newOp, pipelineFull);
  }
  executeOp(currOp, fromMem, fromXReg, id_out, toMem, toXReg, toMem_valid, toXReg_valid);
  return currOp; //Only for testing
}



//Backdoor functions
FPNumber FPU::bd_getData(uint32_t addr){
  return registerFile.read(addr);
};
void FPU::bd_setRoundingMode(unsigned int rm){
  registerFile.setfrm(rm);
};

void FPU::bd_setFcsr(uint32_t data) {
  registerFile.write_fcsr(data);
}

uint32_t FPU::bd_getFcsr() {
  return registerFile.read_fcsr().v;
}




std::vector<float> FPU::bd_getRF(){
  return registerFile.getRf();
};

unsigned int FPU::bd_getPipeStageId(int stage) {
  return pipeline.getId(stage);
}