/*  rvfpm - 2023
  Andreas S. Bakke

  Description:
  Fpu register file. and fcsr
*/

#include "fpu_rf.h"

#if defined(EXT_Q)
  static int FLEN = 128;
#elif defined(EXT_D)
  static int FLEN = 64;
#else
  static int FLEN = 32;
#endif


FpuRf::FpuRf(int depth) : NUM_F_REGISTERS(depth), registerFile(depth) {
  for (auto &reg : registerFile) {
    reg = {.f = NAN}; //Initialize to NAN //TODO: Depends on extension
  }
  fcsr = {.v = 0};
}

FpuRf::~FpuRf() {
}

void FpuRf::resetFpuRf(){
  fcsr.v=0;
  for (auto &reg : registerFile) {
    reg = {.f = NAN}; //Initialize to NAN
  }
};


// READ/WRITE
FPNumber FpuRf::read(int r_address) {
  return registerFile[r_address];
}

int FpuRf::write(int w_address, FPNumber data) { //Return int to give error messageS?
  if (0 <= w_address <= NUM_F_REGISTERS) {
    registerFile[w_address] = data; //Might need more management about multiple writes and giving pri.
    return 1; //Success
  }
  return 0;
}


//Fcsr
int FpuRf::write_fcsr(uint32_t data){
  fcsr = {.v = data};
  return 1; //?
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


//HelperFunctions
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
    v[i] = registerFile[i].f;
  }

  return v;
};