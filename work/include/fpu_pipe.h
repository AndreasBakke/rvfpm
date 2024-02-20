/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Struct for any information that needs to be passed down the pipeline.
*/
#pragma once
#include "fp_number.h"
#include <vector>
#include <cmath>
#include <cstdint>


struct FpuPipeObj {
  uint32_t instr; //Save instruction
  unsigned int id;  //from Core-V-XIF standard
  FPNumber operand_a;
  FPNumber operand_b;
  FPNumber operand_c;
  bool use_rs_i[3] ; //which input operands are used
  std::vector<uint32_t> addrFrom;
  uint32_t addrTo;
  FPNumber data;
  int instr_type;
  unsigned int flags : 5;
  bool toXReg = 0;
  bool toMem = 0;
  bool fromXReg = 0;
  bool fromMem = 0;
  uint32_t uDataToXreg = 0; //unsigned DataToXreg
  int32_t dataToXreg = 0;
  unsigned int remaining_ex_cycles: 1; //1 cycle for execution as standard
  bool valid = 0; //Is the instruction valid?


  bool isEmpty() const {
    return addrFrom.empty() &&
        addrTo == 0 &&
        instr_type == 0 &&
        flags == 0 &&
        !toXReg &&
        !toMem &&
        !fromXReg &&
        !fromMem &&
        uDataToXreg == 0 &&
        dataToXreg == 0;
  }
};
