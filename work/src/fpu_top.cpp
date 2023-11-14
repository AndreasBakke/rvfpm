#include "fpu_top.h"
#include <bitset>
#include <iostream> // Include necessary header files
using namespace std; // Use the standard namespace


FPU::FPU (int pipelineStages, int rfDepth) : registerFile(rfDepth), pipeline(pipelineStages) {
    #ifndef ZFINX
    for (auto &pipe : pipeline) { 
        pipe = {}; //Initialize to empty operations
    }
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
    pipelineFlush();
};


FpuPipeObj FPU::decodeOp(uint32_t instruction, int fromXReg, float fromMem) {
    //Get result of operation
    unsigned int opcode = instruction & 127 ; //Get first 7 bit
    #ifndef NO_ROUNDING  // NO_ROUNDING uses c++ default rounding mode.
        unsigned int rm = registerFile.readfrm();
        if (rm == 0b111) //0b111 is dynamic rounding, and is handled for the relevant instructions later.
        {
            RTYPE rm_instr = {.instr = instruction}; //Decode into RTYPE to extract rm (same field for R4Type)
            setRoundingMode(rm_instr.parts.funct3);
        } else {
            setRoundingMode(rm);
        }
    #endif
    

    FpuPipeObj result = {};
    switch (opcode)
    {
    case FLW:
    {
        ITYPE dec_instr = {.instr = instruction}; //"Decode" into ITYPE
        result = operation_ITYPE(dec_instr, &registerFile, fromMem);
        break;
    }
    case FSW:
    {
        STYPE dec_instr = {.instr = instruction}; //"Decode" into STYPE
        result = operation_STYPE(dec_instr, &registerFile);
        break;
    }
    case FMADD_S:
    {
        RTYPE dec_instr = {.instr = instruction}; //"Decode" into R4TYPE (funct5=rs3)
        result = operation_R4TYPE(dec_instr, &registerFile);
        break;
    }
    case FMSUB_S:
    {
        RTYPE dec_instr = {.instr = instruction}; //"Decode" into R4TYPE (funct5=rs3)
        result = operation_R4TYPE(dec_instr, &registerFile);
        break;
    }
    case FNMSUB_S:
    {
        RTYPE dec_instr = {.instr = instruction}; //"Decode" into R4TYPE (funct5=rs3)
        result = operation_R4TYPE(dec_instr, &registerFile);
        break;
    }
    case FNMADD_S:
    {
        RTYPE dec_instr = {.instr = instruction}; //"Decode" into R4TYPE (funct5=rs3)
        result = operation_R4TYPE(dec_instr, &registerFile);
        break;
    }
    case OP_FP:
    {
        RTYPE dec_instr = {.instr = instruction}; //"Decode" RTYPE instructions
        result = operation_RTYPE(dec_instr, &registerFile, fromXReg);
        break;
    }
    default:
        break;
    }

    return result;
};

void FPU::executeOp(FpuPipeObj op, float* toMem, uint32_t* toXreg, unsigned int* flags_out) {
    switch (op.instr_type)
        {
        case it_ITYPE:
            registerFile.write(op.addrTo, op.data);
            break;
        case it_STYPE:
            if (toMem != nullptr){
                *toMem = op.data.f;
            };
            break;
        case it_RTYPE:
            if (op.toXreg)
            {
                //Raise out ready flag and write to pointer
                if (toXreg != nullptr){
                    *toXreg = op.uDataToXreg ^ op.dataToXreg;
                };
            } else if (op.toMem)
            {
                //raise out ready flag and write to pointer
                if (toMem != nullptr){
                    *toMem = op.data.f;
                };
            } else
            {
                registerFile.write(op.addrTo, op.data);
            }
            break;
        default:
            //If no operation is in pipeline: do nothing
            break;
    }
    registerFile.raiseFlags(op.flags);
    if (flags_out != nullptr) {
        // *flags_out = op.flags;
    }
}



FpuPipeObj FPU::pipelineStep(FpuPipeObj nextOp){
    FpuPipeObj op = {};
    if (!pipeline.empty()) {
        op = pipeline.front();
        pipeline.pop_front();
    }

    if (!nextOp.isEmpty()) {
        pipeline.push_back(nextOp); //Check for full pipeline
    }
    // if stall()
    //     add to bufferqueue;
    //     lower ready flag;
    //TODO: Advance pipeline one step, check hazards, and write the popped result to register/return to Xregs
    // cout << pipeline.size() << endl;
    return op;
};


FpuPipeObj FPU::operation(uint32_t instruction, int fromXReg, float fromMem, float* toMem, uint32_t* toXreg, unsigned int* flags_out) {
    FpuPipeObj newOp = decodeOp(instruction, fromXReg, fromMem);
    FpuPipeObj currOp = {};
    if(pipeline.size() == 0){ //Execute immediately
        currOp = newOp;
    } else 
    { //add to pipeline - check for full pipeline/stalls etc.
        currOp = pipelineStep(newOp);
    }
    executeOp(currOp, toMem, toXreg, flags_out);
    return currOp; //Only for testing
}


int FPU::pipelineFlush(){
    pipeline.clear();
    return 1;
};


//Backdoor functions
FPNumber FPU::bd_getData(uint32_t addr){
    return registerFile.read(addr);
};
void FPU::bd_setRoundingMode(unsigned int rm){
    registerFile.setfrm(rm);
};

