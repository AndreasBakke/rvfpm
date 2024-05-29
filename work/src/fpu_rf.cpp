/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Fpu register file. and fcsr
*/

#include "fpu_rf.h"
#include <iostream>


FpuRf::FpuRf(int depth) : NUM_F_REGISTERS(depth), registerFile(depth) {
  resetFpuRf();
}

FpuRf::~FpuRf() {
}

void FpuRf::resetFpuRf(){
  fcsr.v=0; //Set to 0
  #ifdef FCSR_RM_RESET_VALUE
    fcsr.parts.frm = FCSR_RM_RESET_VALUE;
  #endif
  for (auto &reg : registerFile) {
    #ifdef RF_RESET_VALUE
      reg = RF_RESET_VALUE; //Initialize
    #else
      reg = NAN; //Initialize to NAN
    #endif
  }
};


// READ/WRITE
FPNumber FpuRf::read(int r_address) {
  return registerFile[r_address];
}

int FpuRf::write(int w_address, FPNumber data) {
  if (0 <= w_address && w_address <= NUM_F_REGISTERS) {
    registerFile[w_address] = data;
    return 1; //Success
  }
  return 0;
}


//Fcsr
int FpuRf::write_fcsr(uint32_t data){
  fcsr = {.v = data};
  return 1;
};

FCSR_type FpuRf::read_fcsr(){
  return fcsr;
};

unsigned int FpuRf::raiseFlags(unsigned int flags){
  fcsr.parts_cons.flags = fcsr.parts_cons.flags | flags;
  return fcsr.parts_cons.flags;
};

unsigned int FpuRf::setFlags(unsigned int flags){
  fcsr.parts_cons.flags = flags;
  return fcsr.parts_cons.flags;
};

unsigned int FpuRf::getFlags(){
  return fcsr.parts_cons.flags;
};

unsigned int FpuRf::clearFlags(){
  fcsr.parts_cons.flags = 0b00000;
  return fcsr.parts_cons.flags;
};

void FpuRf::raiseInvalidFlag(){
  fcsr.parts.NV = 1;
};

void FpuRf::raiseDevideByZeroFlag(){
  fcsr.parts.DZ = 1;
};

void FpuRf::raiseOverflowFlag(){
  fcsr.parts.OF = 1;
};

void FpuRf::raiseUnderflowFlag(){
  fcsr.parts.UF = 1;
};

void FpuRf::raiseInexactFlag(){
  fcsr.parts.NX = 1;
};

void FpuRf::setfrm(unsigned int rm){
  fcsr.parts.frm = rm;
};

unsigned int FpuRf::readfrm(){
  return fcsr.parts.frm;
};


//Helper functions
int FpuRf::get_length() {
  return registerFile.size();
}

int FpuRf::get_NUM_F_REGISTERS() {
  return NUM_F_REGISTERS;
}


std::vector<float> FpuRf::getRf(){
  std::vector<float> v;
  for (int i = 0; i < registerFile.size(); i++)
  {
    v[i] = registerFile[i];
  }

  return v;
};