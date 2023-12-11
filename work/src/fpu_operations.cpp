/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    Decode and execute operations for fpu.
*/
#include "fpu_operations.h"
#include <cfenv> //To get flags
#include <iostream>
#include <limits>

FpuPipeObj decode_R4TYPE(uint32_t instr) {
    RTYPE dec_instr = {.instr = instr};
    FpuPipeObj result = {};
    result.addrFrom = {dec_instr.parts_r4type.rs1, dec_instr.parts_r4type.rs2, dec_instr.parts_r4type.rs3};
    result.addrTo = {dec_instr.parts_r4type.rd};
    result.instr = instr; //save instruction
    result.instr_type = it_R4TYPE;
    return result;
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
        op.data.f = fma(data1.f, data2.f, data3.f);
        break;
    }
    case FMSUB_S:
    {
        op.data.f = fma(data1.f, data2.f, -data3.f); 
        break;
    }
    case FNMSUB_S:
    {    
        op.data.f = fma(data1.f, -data2.f, -data3.f); 
        break;
    }
    case FNMADD_S:
    {    
        op.data.f = fma(data1.f, -data2.f, data3.f); 
        break;
    }
    default:
        op.flags = 0b10000; //Invalid operation
        break;
    }
    op.flags |= getFlags(); //Get flags and add to result.

    if(registerFile != nullptr) {
        registerFile->write(op.addrTo, op.data);
    }

};

FpuPipeObj decode_RTYPE(uint32_t instr) {
    RTYPE dec_instr = {.instr = instr}; //"Decode" into ITYPE
    FpuPipeObj result = {};
    result.addrFrom = {dec_instr.parts.rs1, dec_instr.parts.rs2};
    result.addrTo = {dec_instr.parts.rd};
    result.instr_type = it_RTYPE; //For decoding in execution step.
    result.instr = instr; //Save instruction
    //Override relevant parameters based on function
    switch (dec_instr.parts.funct7)
    {
        case FSQRT_S:
        {
            result.addrFrom = {dec_instr.parts.rs1, 999}; //sqrt only dependent on rs1
            break;
        }
        case FCMP:
        {
            result.toXReg = true;
            break;
        }
        case FCVT_W_S:
        {
            result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
            result.toXReg = true;
            break;
        }
        case FCVT_S_W: //FCVT.S.W[U]
        {
            result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
            result.fromXReg = true;
            break;
        }
        case FCLASS_FMV_X_W:
        {
            result.addrFrom = {dec_instr.parts.rs1, 999}; //Overwrite since only one address is used
            result.toXReg = true;
        }
        case FMV_W_X:
        {   
            result.fromXReg = true;
            break;
        }
    }
    return result;
}

// FpuPipeObj FPU::operation(uint32_t instruction, int fromXReg, float fromMem, float* toMem, uint32_t* toXReg, bool* pipelineFull) {

void execute_RTYPE(FpuPipeObj& op, FpuRf* registerFile, int fromXReg, unsigned int* id_out, uint32_t* toXReg, bool* toXReg_valid){
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
            op.data = data1;
            op.data.parts.sign = data2.parts.sign ^ data1.parts.sign;
            break;
        }
        default:
        {
            op.flags = 0b10000; //Invalid operation
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
            data1.f == data2.f ? op.uDataToXreg = 1 : op.uDataToXreg = 0;
            break;
        }
        case 0b001: //FLT.s
        {
            data1.f < data2.f ? op.uDataToXreg = 1 : op.uDataToXreg = 0;
            break;
        }
        case 0b000: //FLE.S
        {
            data1.f <= data2.f ? op.uDataToXreg = 1 : op.uDataToXreg = 0;
            break;
        }
        default:
        {
            op.uDataToXreg = 0;
            op.flags = 0b10000; //invalid operation
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
            op.dataToXreg= static_cast<int32_t>(nearbyint(data1.f)); //Use nearbyint instead of round, round does not follow rounding mode set in cfenv
            break;
        }
        case 0b00001: //FCVT.WU.S
        {
            //ADD ifdef x86_64 for this implementation
            if (std::isnan(data1.f) && !(data1.parts.mantissa & 0x00400000)) {  // Check for sNaN
                op.uDataToXreg = 0xFFFFFFFF;
                op.flags |= 0b00001;
            } else if (nearbyint(data1.f) < 0.0f || nearbyint(data1.f) > UINT32_MAX) {  // Check for out-of-range values
                //Last fail: +9F.000000  => 00000000 .....  expected FFFFFFFF v.... for near and max = 1^(2^32) - > Not captured by ^. But if we use >=, it captures too many for min-rounding
                op.uDataToXreg = 0xFFFFFFFF;
                op.flags |= 0b00001;
            } else {
                // Convert if within range
                op.uDataToXreg = static_cast<unsigned int>(nearbyint(data1.f));
            }
            //And arm64_apple for this one (If the above doesn't work for apple)
            // op.uDataToXreg = static_cast<uint32_t>(nearbyint(data1.f));
            break;
        }
        default:
        {
            op.dataToXreg = 0;
            op.flags = 0b10000; //invalid operation
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
            op.data.f = static_cast<float>(fromXReg);
            break;
        }
        case 0b00001: //FCVT.S.WU
        {
            op.data.f = static_cast<float>(static_cast<uint32_t>(fromXReg)); //Cast to unsigned first
            break;
        }
        default:
            op.flags = 0b10000; //Invalid operation
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
            op.dataToXreg = data1.bitpattern;
            break;
        }    
        case 0b001: //FCLASS.S
        {
            if(isSubnormal(data1))
            {
                //subnormal number
                if (data1.f < 0) 
                {
                    op.uDataToXreg = 0b0000000100;
                } else if (data1.f > 0)
                {
                    op.uDataToXreg = 0b0000100000;
                } else {
                    op.flags = 0b10000; //Invalid operation
                }
            } else
            {
                //normal numbers
                if (data1.f == -INFINITY) //negative inf.
                {
                    op.uDataToXreg = 0b0000000001;
                } else if (data1.f < 0) //negative normal number
                {
                    op.uDataToXreg = 0b0000000010;
                } else if (data1.f == 0 && data1.parts.sign == 1) //negative zero
                {
                    op.uDataToXreg = 0b0000001000;
                } else if (data1.f == 0 && data1.parts.sign == 0) //positive 0
                {
                    op.uDataToXreg = 0b0000010000;
                
                } else if (data1.f == INFINITY) //positive inf
                {
                    op.uDataToXreg = 0b0010000000;
                } else if (std::isnan(data1.f) && !(data1.bitpattern & 0x00400000)) //If leading mantissa-bit is not set -> sNaN //TODO: rewrite the check so it works
                {
                    op.uDataToXreg = 0b0100000000; //SNaN
                } else if (std::isnan(data1.f))
                {
                    std::cout << std::hex << data1.bitpattern << std::endl;
                    std::cout << (data1.bitpattern & 0x00400000) << std::endl;
                    op.uDataToXreg = 0b1000000000; //QNaN
                } else if (data1.f > 0) //positive normal number
                {
                    op.uDataToXreg = 0b0001000000;
                } else {
                    op.uDataToXreg = 0b0000000000; 
                    op.flags = 0b10000; //Invalid operation
                };
            }
            break;
        }
        default:
            op.flags = 0b10000; //Invalid operation
            break;
        }
    }
    case FMV_W_X:
    {   
        //Moves bitpattern from X to W(F)
        op.data.bitpattern =  fromXReg;
        break;
    }
    //TODO: check pseudops like read/write to fcsr
    default:
        op.flags = 0b10000; //Invalid operation
        break;
    }

    op.flags |=  getFlags();

    if (op.toXReg)
    {
        //Raise out ready flag and write to pointer
        if (toXReg != nullptr){
            *toXReg = op.uDataToXreg ^ op.dataToXreg;
        };
        if (toXReg_valid != nullptr) {
            *toXReg_valid = true;
        }
        if (id_out != nullptr) {
            *id_out = op.id;
        }
    } else
    {
        if(registerFile != nullptr) {
            registerFile->write(op.addrTo, op.data);
        }
    }
};

FpuPipeObj decode_ITYPE(uint32_t instr) {
    ITYPE dec_instr = {.instr = instr}; //"Decode" into ITYPE
    FpuPipeObj result = {};
    result.addrFrom = {}; //FLW is atomic
    result.addrTo = dec_instr.parts.rd;
    result.instr_type = it_ITYPE;
    return result;
}

void execute_ITYPE(FpuPipeObj& op, FpuRf* registerFile, float fromMem){
    //Only ITYPE operation implemented is FLW
    op.data.f = fromMem;
    if (registerFile != nullptr) {
        registerFile->write(op.addrTo, op.data);
    }
}

FpuPipeObj decode_STYPE(uint32_t instr){
    STYPE dec_instr = {.instr = instr}; //Decode into STYPE
    FpuPipeObj result = {};
    result.addrFrom = {dec_instr.parts.rs2};
    result.addrTo = 0; //destination is memory
    result.toMem = true;
    result.instr_type = it_STYPE;
    return result;
}

void execute_STYPE(FpuPipeObj& op, FpuRf* registerFile, unsigned int* id_out, float* toMem, bool* toMem_valid){
    if (registerFile != nullptr) {
        op.data = registerFile->read(op.addrFrom.front()); 
    }
    if (toMem != nullptr) {
        *toMem = op.data.f;
    }
    if (toMem_valid != nullptr) {
        *toMem_valid = true; //TODO: check if it is actually valid. TODO: rename to ready?
    }
    if (id_out != nullptr) {
        *id_out = op.id;
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

unsigned int getFlags() {
    unsigned int flags = 0;
    flags |= std::fetestexcept(FE_INVALID) ? 1 : 0;
    flags |= std::fetestexcept(FE_DIVBYZERO) ? (1 << 1) : 0;
    flags |= std::fetestexcept(FE_OVERFLOW) ? (1 << 2) : 0;
    flags |= std::fetestexcept(FE_UNDERFLOW) ? (1 << 3) : 0;
    flags |= std::fetestexcept(FE_INEXACT) ? (1 << 4) : 0;
    return flags;
}