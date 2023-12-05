/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    Fpu register file. and fcsr
*/

#pragma once
#include "fp_number.h"
#include <vector>
#include <cmath>
#include <cstdint>

typedef union {
  uint32_t v;
  struct {
    unsigned int NX : 1;
    unsigned int UF : 1;
    unsigned int OF : 1;
    unsigned int DZ : 1;
    unsigned int NV : 1;
    unsigned int frm : 3;
    unsigned int reserved : 24;
  } parts;

  struct { //consentrated format for bulk-write of flags
    unsigned int flags: 5;
    unsigned int frm : 3;
    unsigned int reserved: 24;
  } parts_cons;
} FCSR_type;


class FpuRf {
  private:
    int NUM_F_REGISTERS;
    std::vector<FPNumber> registerFile;
    FCSR_type fcsr;
  public:
    // Constructor/Deconstructor
    FpuRf(int depth);
    ~FpuRf();
    void resetFpuRf();
    // READ/WRITE
    int write(int w_address, FPNumber data);
    FPNumber read(int r_address);


    //fcsr
    int write_fcsr(uint32_t data);
    FCSR_type read_fcsr();

    //fcsr-flags
    unsigned int raiseFlags(unsigned int flags);
    unsigned int setFlags(unsigned int flags); //Overrides current flags
    unsigned int clearFlags();
    void raiseInvalidFlag();
    void raiseDevideByZeroFlag();
    void raiseOverflowFlag();
    void raiseUnderflowFlag();
    void raiseInexactFlag();

  //fcsr-rm
    void setfrm(unsigned int rm);
    unsigned int readfrm();
    //Helper functions
    int get_NUM_F_REGISTERS();
    int get_length();
    std::vector<float> getRf();



};