/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    Simple test for fpu_top.cpp. Checks that no core functionality has changed. NOT a compliance test.

    Run:
    make test (macOS).
    make test_RHEL (RHEL8)
*/

#include "test.h"
//Scoreboard
static int errorcount = 0;
static int rfDepth = 32;
static int numTests = 10000;

//Initialize

//NOTE: Not a test for compliance - just to verify that changes to the fpu doesn't affect anything
//TODO: Add testing with variable number of pipelines

int main() {
    srand(time(NULL)); //Seed randomNAN, NAN
    FPU testFPU(0, rfDepth); //0 pipeline stages -> instant execution
    std::vector<float> testFPU_clone(rfDepth, NAN); //Compare with "correct" rf

    cout << endl;
    cout << "Started testing of rvfpm" <<endl;

    cout << "Read/Write fcsr" <<endl;
    //TODO: add testing for fcsr

    cout << "Testing reset/memory load" <<endl;
    //LOAD rf (FLW)
    resetRegisters(testFPU, testFPU_clone);
    //STORE(read) rf (FSW) and compare
    verifyReset(testFPU, testFPU_clone);

    //I/S TYPE test
    for (unsigned int i = 0; i < numTests; i++)
    {
        resetRegisters(testFPU, testFPU_clone);
        verifyReset(testFPU, testFPU_clone);
    }


    //R4TYPE tests
    cout << "Testing R4TYPE instructions" <<endl;
    testR4TYPE(testFPU, testFPU_clone);

    //reset registers
    resetRegisters(testFPU, testFPU_clone);
    verifyReset(testFPU, testFPU_clone);

    //RTYPE tests
    cout << "Testing RTYPE instructions" <<endl;
    testRTYPE(testFPU, testFPU_clone);

    //Test with all roundingmodes (except RMM)
    cout << "Testing rounding modes" <<endl;
    for (unsigned int i = 0; i < 0b100; i++)
    {
        testFPU.bd_setRoundingMode(i);
        resetRegisters(testFPU, testFPU_clone);
        verifyReset(testFPU, testFPU_clone);
        testR4TYPE(testFPU, testFPU_clone);
        resetRegisters(testFPU, testFPU_clone);
        verifyReset(testFPU, testFPU_clone);
        testRTYPE(testFPU, testFPU_clone);
    }
    


    cout << "----------------" <<endl;
    if (errorcount > 0)
    {
        cout << "TEST FAILED!" <<endl;
        cout << "With " +to_string(errorcount) + "errors." <<endl;
    } else {
        cout << "TEST SUCCEEDED!" <<endl;
        cout << "With 0 errors." <<endl;
    }
    cout << "----------------" <<endl;
    cout << endl;
    cout << "exiting" <<endl;
    return 1;
}


void resetRegisters(FPU& fpu, std::vector<float>& clone){
    //Helper function to reset registers between tests
    for (unsigned int i = 0; i < rfDepth; i++)
    {
        //Should be random float
        float w_data =  static_cast<float>(rand())/static_cast<float>(rand()); //Generate random float
        clone[i] = w_data;
        ITYPE instr_load = {.parts= {7, i, 0b010, 0, 0}};
        fpu.operation(instr_load.instr, 0, w_data, nullptr, nullptr, nullptr, nullptr, nullptr);
    }
}

void verifyReset(FPU& fpu, std::vector<float>& clone){
    //Helper function to verify register reset between tests
    float toMem;
    bool toMem_valid;
    for (unsigned int i = 0; i < rfDepth; i++)
    {
        STYPE instr_read = {.parts= {39, 0, 0b010, 0, i, 0}};
        FpuPipeObj res = fpu.operation(instr_read.instr, 0, 0, &toMem, nullptr, nullptr, &toMem_valid, nullptr);
        if (toMem != clone[i])
        {
            cout << "Read does not match write at addr: " + to_string(i) <<endl;
            cout << "Expected: " + to_string(clone[i]) + " but got: rfvf=" +to_string(res.data.f) <<endl;
            errorcount++;
        }
    }
}

//TODO: should I use test functions instead of writing a huge main()? Probably yes

// Can also write helper functions for setting registers between tests (add a backdoor to FpuRf?)

void testR4TYPE(FPU& fpu, std::vector<float>& clone) {
    for (int i = 0; i < numTests; i++) //do numTests random tests
    {
        //Random instruction
        int type = rand() % 4 + 1;
        //Using random registers
        uint32_t r1 = rand() % rfDepth;
        uint32_t r2 = rand() % rfDepth;
        uint32_t r3 = rand() % rfDepth;
        uint32_t rd = rand() % rfDepth;

        //Store operands in case of errors
        STYPE instr_read_1 = {.parts= {39, 0, 0b010, 0, r1, 0}};
        float d1 = fpu.operation(instr_read_1.instr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr).data.f;
        STYPE instr_read_2 = {.parts= {39, 0, 0b010, 0, r2, 0}};
        float d2 = fpu.operation(instr_read_2.instr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr).data.f;

        RTYPE instr_r4type;
        switch (type) 
        {
        case 1: //FMADD_S
        {
            clone[rd] = clone[r1]*clone[r2]+clone[r3];
            instr_r4type = {.parts_r4type= {FMADD_S, rd, 0b000, r1, r2, 0b00, r3}};
            break;
        }
        case 2: //FMSUB_S
        {
            clone[rd] = clone[r1]*clone[r2]-clone[r3];
            instr_r4type = {.parts_r4type= {FMSUB_S, rd, 0b000, r1, r2, 0b00, r3}};
            break;
        }
        case 3: //FNMSUB_S
        {
            clone[rd] = -(clone[r1]*clone[r2])-clone[r3];
            instr_r4type = {.parts_r4type= {FNMSUB_S, rd, 0b000, r1, r2, 0b00, r3}};
            break;
        }
        case 4: //FNMADD_S
        {
            clone[rd] = -(clone[r1]*clone[r2])+clone[r3];
            instr_r4type = {.parts_r4type= {FNMADD_S, rd, 0b000, r1, r2, 0b00, r3}};
            break;
        }
        }
        fpu.operation(instr_r4type.instr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr); //do operation

        //Read and compare
        STYPE instr_read = {.parts= {39, 0, 0b010, 0, rd, 0}};
        float res = fpu.operation(instr_read.instr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr).data.f;
        
        if (res != clone[rd] && !(isnan(res)&& isnan(clone[rd]))) //TODO: Find a better way to check for NANS /sNAN vs QNaN
        {
            cout << "ui:" + to_string(static_cast<uint32_t>(res)) <<endl;
            cout << endl << "iteration: " + to_string(i) <<endl;
            cout << "instr: " + to_string(type) <<endl;
            cout << "Results does not match for operation: " + to_string(instr_r4type.parts.funct7) <<endl;
            cout << "Expected: " + to_string(clone[rd]) + " but got: " +to_string(res) <<endl;
            cout << "Clone: r1: " + to_string(clone[r1]) + " r2: " +to_string(clone[r2]) <<endl;
            cout << "RF: r1: " + to_string(d1) + " r2: " +to_string(d2) <<endl;
            
            errorcount++;
        }
    }
}

void testRTYPE(FPU& fpu, std::vector<float>& clone) {
    for (int i = 0; i < numTests; i++) //do numTests random tests 
    {
        //Random instruction
        int type = rand() % 7; //TODO: Adjust to match implemented operations
        //Using random registers
        uint32_t r1 = rand() % rfDepth;
        uint32_t r2 = rand() % rfDepth;
        uint32_t rd = rand() % rfDepth;

        //Store operands in case of errors
        STYPE instr_read_1 = {.parts= {39, 0, 0b010, 0, r1, 0}};
        float d1 = fpu.operation(instr_read_1.instr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr).data.f;
        STYPE instr_read_2 = {.parts= {39, 0, 0b010, 0, r2, 0}};
        float d2 = fpu.operation(instr_read_2.instr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr).data.f;

        RTYPE instr_rtype;
        switch (type)
        {
        case 0: //FADD_S:
        {
            instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FADD_S}};
            clone[rd] = clone[r1]+ clone[r2];
            break;
        }
        case 1: //FSUB_S:
        {
            instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FSUB_S}};
            clone[rd] = clone[r1]- clone[r2];
            break;
        }
        case 2: //FMUL_S:
        {
            instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FMUL_S}};
            clone[rd] = clone[r1]* clone[r2];
            break;
        }
        case 3: //FDIV_S:
        {
            instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FDIV_S}};
            clone[rd] = clone[r1]/ clone[r2];
            break;
        }
        case 4: //FSQRT_S:
        {
            instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, 0b00000, FSQRT_S}};
            clone[rd] = sqrt(clone[r1]);
            break;
        }
        case 5: //FSGNJ:
        {
            FPNumber fwsign = {.f = clone[r2]};
            instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FSGNJ}};
            fwsign.parts.sign == 0 ? clone[rd] = abs(clone[r1]) : clone[rd] = -abs(clone[r1]) ;
            break;
        }
        case 6: //FSGNJN.S
        {
            FPNumber fwsign = {.f = clone[r2]};
            instr_rtype = {.parts= {OP_FP, rd, 0b001, r1, r2, FSGNJ}};
            fwsign.parts.sign == 0 ? clone[rd] = -abs(clone[r1]) : clone[rd] = +abs(clone[r1]) ;
            break;
        }
        case 7: //FSGNJX.S
        { 
            FPNumber fwsign = {.f = clone[r2]};
            instr_rtype = {.parts= {OP_FP, rd, 0b010, r1, r2, FSGNJ}};
            if (fwsign.parts.sign == 0)
            {
                clone[rd] = clone[r1]; //dont flip sign 
            } else {
                clone[rd] = -clone[r1]; //flip sign
            }
            break;
        }
        // case 8: //FEQ.S
        // {
        //     instr_rtype = {.parts = {OP_FP, rd, 0b010, r1, r2, FCMP}};
        //     clone[r1] == clone[r2] && clone[r1] != NAN && clone[r2] != NAN ? clone[rd] = 1 : clone[rd] = 0;
        //     break;
        // }
        // case 9: //FLT.S
        // {
        //     instr_rtype = {.parts = {OP_FP, rd, 0b001, r1, r2, FCMP}};
        //     clone[r1] < clone[r2] && clone[r1] != NAN && clone[r2] != NAN ? clone[rd] = 1 : clone[rd] = 0;
        //     break;
        // }
        // case 10: //FLE.S
        // {
        //     instr_rtype = {.parts = {OP_FP, rd, 0b000, r1, r2, FCMP}};
        //     clone[r1] <= clone[r2] && clone[r1] != NAN && clone[r2] != NAN ? clone[rd] = 1 : clone[rd] = 0;
        //     break;
        // }
        case 11: //FCVT:
        {
            break;
        }
        case 12: //FCLASS:
        {
            break;
        }
        case 13: //FMV:
        {
            break;
        }
        //TODO: check remaining ops
        };

        fpu.operation(instr_rtype.instr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr); //do operation
        //Read and compare
        STYPE instr_read = {.parts= {39, 0, 0b010, 0, rd, 0}};
        float res = fpu.operation(instr_read.instr, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr).data.f;
        if (res != clone[rd] && !(isnan(res)&& isnan(clone[rd])))
        {
            cout << "ui:" + to_string(static_cast<uint32_t>(res)) <<endl;
            cout << endl << "iteration: " + to_string(i) <<endl;
            cout << "instr: " + to_string(type) <<endl;
            cout << "Results does not match for operation: " + to_string(instr_rtype.parts.funct7) <<endl;
            cout << "Expected: " + to_string(clone[rd]) + " but got: " +to_string(res) <<endl;
            cout << "Clone: r1: " + to_string(clone[r1]) + " r2: " +to_string(clone[r2]) <<endl;
            cout << "RF: r1: " + to_string(d1) + " r2: " +to_string(d2) <<endl;

            errorcount++;
        }
    }
}
