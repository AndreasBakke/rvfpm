#include "fpu_operations.h"
#include <cfenv> //To get flags
#include <iostream>


FpuPipeObj operation_R4TYPE(RTYPE instr, FpuRf* registerFile){
    std::feclearexcept(FE_ALL_EXCEPT); //Clear all flags
    //Get data from registerFile
    FPNumber data1 = registerFile->read(instr.parts_r4type.rs1);
    FPNumber data2 = registerFile->read(instr.parts_r4type.rs2);
    FPNumber data3 = registerFile->read(instr.parts_r4type.rs3);

    FpuPipeObj result = {};
    result.addrFrom = {instr.parts_r4type.rs1, instr.parts_r4type.rs2, instr.parts_r4type.rs3};
    result.addrTo = {instr.parts_r4type.rd};
    result.instr_type = it_RTYPE;

    //Compute result -- will be added to pipeline
    switch (instr.parts_r4type.opcode)
    {
    case FMADD_S:
    {
        result.data.f = data1.f * data2.f + data3.f;
        break;
    }
    case FMSUB_S:
    {
        result.data.f = data1.f * data2.f - data3.f; 
        break;
    }
    case FNMSUB_S:
    {    
        result.data.f = -(data1.f * data2.f) - data3.f; 
        break;
    }
    case FNMADD_S:
    {    
        result.data.f = -(data1.f * data2.f) + data3.f; 
        break;
    }
    default:
        result.flags = 0b10000; //Invalid operation
        break;
    }
    result.flags |=  std::fetestexcept(FE_ALL_EXCEPT); //Get flags and add to result.
    return result;
};

FpuPipeObj operation_RTYPE(RTYPE instr, FpuRf* registerFile, int32_t fromXReg){
    std::feclearexcept(FE_ALL_EXCEPT); //Clear all flags
    FPNumber data1 = registerFile->read(instr.parts.rs1);
    FPNumber data2 = registerFile->read(instr.parts.rs2);

    FpuPipeObj result = {};
    result.addrFrom = {instr.parts.rs1, instr.parts.rs2};
    result.addrTo = {instr.parts.rd};
    result.instr_type = it_RTYPE; //For decoding in execution step.
    switch (instr.parts.funct7)
    {
    case FADD_S:
    {
        result.data.f = data1.f + data2.f;
        break;
    }
    case FSUB_S:
    {
        result.data.f = data1.f - data2.f;
        break;
    }
    case FMUL_S:
    {
        result.data.f = data1.f * data2.f;
        break;
    }
    case FDIV_S:
    {
        result.data.f = data1.f / data2.f;
        break;
    }
    case FSQRT_S:
    {
        result.addrFrom = {instr.parts.rs1}; //sqrt only dependent on rs1
        result.data.f = sqrt(data1.f);
        break;
    }
    case FSGNJ:
    {
        result.data.parts.exponent = data1.parts.exponent;
        result.data.parts.mantissa = data1.parts.mantissa;
        switch (instr.parts.funct3)
        {
        case 0b000: //FSGNJ.S
        {
            result.data.parts.sign = data2.parts.sign;
            break;
        }
        case 0b001: //FSGNJN.S
        {
            result.data.parts.sign = !data2.parts.sign;
            break;
        }
        case 0b010: //FSGNJX.S
        {
            result.data = data1;
            result.data.parts.sign = data2.parts.sign ^ data1.parts.sign;
            break;
        }
        default:
        {
            result.flags = 0b10000; //Invalid operation
            result.data = data1;
            break;
        }
        }
        break;
    }
    case FCMP:
    {
        result.toXreg = true;
        switch (instr.parts.funct3)
        {
        case 0b010: //FEQ.S //TODO: Destination should be Xreg!
        {
            data1.f == data2.f ? result.uDataToXreg = 1 : result.uDataToXreg = 0;
            break;
        }
        case 0b001: //FLT.s //TODO: Destination should be Xreg!
        {
            data1.f < data2.f ? result.uDataToXreg = 1 : result.uDataToXreg = 0;
            break;
        }
        case 0b000: //FLE.S //TODO: Destination should be Xreg!
        {
            data1.f <= data2.f ? result.uDataToXreg = 1 : result.uDataToXreg = 0;
            break;
        }
        default:
        {
            result.uDataToXreg = 0;
            result.flags = 0b10000; //invalid operation
            break;
        }
        }
        break;
    }
    case FCVT_W_S:
    {
        result.addrFrom = {instr.parts.rs1}; //Overwrite since only one address is used
        result.toXreg = true;
        switch (instr.parts.rs2) 
        {
        case 0b00000: //FCVT.W.S
        {
            result.dataToXreg= static_cast<int32_t>(nearbyint(data1.f)); //Use nearbyint instead of round, round does not follow rounding mode set in cfenv
            break;
        }
        case 0b00001: //FCVT.WU.S
        {
            result.uDataToXreg = static_cast<uint32_t>(nearbyint(data1.f));
            break;
        }
        default:
        {
            //TODO: what should dataToXreg be?
            result.dataToXreg = 0;
            result.flags = 0b10000; //invalid operation
        }
        }
        break;
    }    
    case FCVT_S_W: //FCVT.S.W[U]
    {
        result.addrFrom = {instr.parts.rs1}; //Overwrite since only one address is used
        result.fromXreg = true;
        switch (instr.parts.rs2)
        {
        case 0b00000: //FCVT.S.W
        {
            result.data.f = static_cast<float>(fromXReg);
            break;
        }
        case 0b00001: //FCVT.S.WU
        {
            result.data.f = static_cast<float>(static_cast<uint32_t>(fromXReg)); //Cast to unsigned first
            break;
        }
        default:
            result.flags = 0b10000; //Invalid operation
            break;
        }
        break;
    }
    case FCLASS_FMV_X_W:
    {
        result.toXreg = true; //write to integer register
        result.addrFrom = {instr.parts.rs1}; //Overwrite since only one address is used
        switch (instr.parts.funct3)
        {
        case 0b000: //FMV_X_W
        {  
            result.uDataToXreg = data1.bitpattern;
            break;
        }    
        case 0b001: //FCLASS.S
        {
            if(isSubnormal(data1))
            {
                //subnormal number
                if (data1.f < 0) 
                {
                    result.uDataToXreg = 0b0000000100;
                } else if (data1.f > 0)
                {
                    result.uDataToXreg = 0b0000100000;
                } else {
                    result.flags = 0b10000; //Invalid operation
                }
            } else
            {
                //normal numbers
                if (data1.f == -INFINITY) //negative inf.
                {
                    result.uDataToXreg = 0b0000000001;
                } else if (data1.f < 0) //negative normal number
                {
                    result.uDataToXreg = 0b0000000010;
                } else if (data1.f == -0) //negative zero
                {
                    result.uDataToXreg = 0b0000001000;
                } else if (data1.f == 0) //positive 0
                {
                    result.uDataToXreg = 0b0000010000;
                } else if (data1.f > 0) //positive normal number
                {
                    result.uDataToXreg = 0b0001000000;
                } else if (data1.f == INFINITY) //positive inf
                {
                    result.uDataToXreg = 0b0010000000;
                } else if (std::isnan(data1.f) && !(data1.parts.mantissa & 0x00400000)) //If leadning mantissa-bit is not set -> sNaN
                {
                    result.uDataToXreg = 0b0100000000; //SNaN
                } else if (std::isnan(data1.f))
                {
                    result.uDataToXreg = 0b1000000000; //QNaN
                } else {
                    result.uDataToXreg = 0b0000000000; 
                    result.flags = 0b10000; //Invalid operation
                };
            }
            break;
        }
        default:
            result.flags = 0b10000; //Invalid operation
            break;
        }
    }
    case FMV_W_X:
    {   
        //Moves data from X to W(F)
        result.fromXreg = true;
        result.data.bitpattern =  fromXReg;
        break;
    }
    //TODO: check pseudops 
    default:
        result.flags = 0b10000; //Invalid operation
        break;
    }

    result.flags |=  std::fetestexcept(FE_ALL_EXCEPT);
    return result;
};

FpuPipeObj operation_ITYPE(ITYPE instr, FpuRf* registerFile, float fromMem){
    //Only ITYPE operation implemented is FLW
    FpuPipeObj result = {};
    result.fromMem = true;
    result.addrFrom = {}; //FLW is atomic
    result.data.f = fromMem;
    result.addrTo = instr.parts.rd;
    result.instr_type = it_ITYPE;
    return result;
}

FpuPipeObj operation_STYPE(STYPE instr, FpuRf* registerFile){
    FpuPipeObj result = {};
    result.addrFrom = {instr.parts.rs2};
    result.addrTo = 0; //destination is memory
    result.toMem = true;
    result.data = registerFile->read(instr.parts.rs2);
    result.instr_type = it_STYPE;
    return result;
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