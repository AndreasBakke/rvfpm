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
  unsigned int id = 99;  //from Core-V-XIF standard
  FPNumber operand_a;
  FPNumber operand_b;
  FPNumber operand_c;
  bool use_rs_i[3] ; //which input operands are used
  std::vector<uint32_t> addrFrom;
  uint32_t addrTo = 999;
  FPNumber data;
  int instr_type;
  unsigned int flags : 5;
  bool toXReg = 0;
  bool toMem = 0;
  bool fromXReg = 0;
  bool fromMem = 0;
  int remaining_ex_cycles= NUM_CYCLES_DEFAULT; //1 cycle for execution as standard
  bool valid = 0; //Is the instruction valid
  bool speculative = 0; //Set to 1 in decode, so empty operations isn't speculative. Set to 0 when commited do not execute if 1
  unsigned int size = 0;
  unsigned int mode = 0;
  bool added_to_mem_queue = 0;
  bool mem_result_valid = 0;
  uint32_t mem_result = 0;
  bool stalledByCtrl = 0;
  #ifdef FORWARDING
    FPNumber fw_data;
    unsigned int fw_addr;
  #endif



  bool isEmpty() const {
    return addrFrom.empty() &&
        addrTo == 999 &&
        instr_type == 0 &&
        flags == 0 &&
        !toXReg &&
        !toMem &&
        !fromXReg &&
        !fromMem &&
        !valid &&
        !speculative &&
        size == 0 &&
        mode == 0 &&
        !mem_result_valid &&
        mem_result == 0 &&
        !stalledByCtrl;
  }
  bool operator==(const FpuPipeObj& rhs)const{
    return (id == rhs.id && instr == rhs.instr && use_rs_i == rhs.use_rs_i) ;
  }
};


