#pragma once
#include "fp_number.h"
#include <vector>
#include <cmath>
#include <cstdint>


struct FpuPipeObj {
    std::vector<uint32_t> addrFrom; //For hazards
    unsigned int id;  //from Core-V-XIF standard
    uint32_t addrTo;  //For Hazards
    FPNumber data;
    int instr_type;
    unsigned int flags : 5;
    bool toXreg = 0;
    bool toMem = 0;
    bool fromXreg = 0;
    bool fromMem = 0;
    uint32_t uDataToXreg = 0; //unsigned DataToXreg
    int32_t dataToXreg = 0;
    // int data_dependency; //Amount of clocks before that need to be "clear"
    //Stalls - signify how long to stall operation (Todo: need to add a "ready" of some sort from FPU)

    bool isEmpty() const {
        return addrFrom.empty() &&
               addrTo == 0 &&
               instr_type == 0 &&
               flags == 0 &&
               !toXreg &&
               !toMem &&
               !fromXreg &&
               !fromMem &&
               uDataToXreg == 0 &&
               dataToXreg == 0;
    }
};
