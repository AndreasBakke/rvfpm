#pragma once
#include "fp_number.h"
#include <vector>
#include <cmath>
#include <cstdint>


struct FpuPipeObj {
    uint32_t instr; //Save instruction
    unsigned int id;  //from Core-V-XIF standard
    std::vector<uint32_t> addrFrom; //For hazards
    uint32_t addrTo;  //For Hazards
    FPNumber data;
    int instr_type;
    unsigned int flags : 5;
    bool toXReg = 0;
    bool toMem = 0;
    bool fromXReg = 0;
    bool fromMem = 0;
    uint32_t uDataToXreg = 0; //unsigned DataToXreg
    int32_t dataToXreg = 0;

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
