/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    Wrapper to test for IEEE754 complaince using Berkley TestFloat.

    Run:
    make TestFloat.

    Operations not tested by BTF:
        FSGNJ - sign bit modified directly
        FCLASS
        FMV
        (FMSUB, FNMSUB, FNMADD not directly tested, but should have the same behavior as FMADD with varied signs)
*/


#include <iostream>
#include <string>
#include <cstring>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <cstdint>
#include <limits>
#include "fpu_top.h"
#include <sstream>
#include <iomanip>

//Generated by ChatGPT. Prompt: Can you write me two c++ function to convert a floating point value to hex and vice versa?
// Function to convert a hexadecimal string to a float
float hexToFloat(const std::string& hexStr) {
    // Convert the hexadecimal string to a uint32_t
    uint32_t intRep;
    std::stringstream ss;
    ss << std::hex << hexStr;
    ss >> intRep;

    // Reinterpret the bits of the integer as float
    float floatValue;
    std::memcpy(&floatValue, &intRep, sizeof(floatValue));
    return floatValue;
}
std::string floatToHex(float floatValue) {
    // Create an output string stream with hexadecimal formatting and no showbase
    std::ostringstream oss;
    oss << std::hex << std::noshowbase;

    // Interpret the bits of the float as an unsigned integer
    auto value = *reinterpret_cast<unsigned int*>(&floatValue);

    // Set the width to 8 characters and fill with zeroes
    oss << std::setw(8) << std::setfill('0') << value;

    // Return the formatted string
    return oss.str();
}

//Generated by ChatGPT. Prompt: Similar to the hexToFloat, can you write me one for uint32_t and int32_t?
uint32_t hexToUnsignedInt(const std::string& hexStr) {
    uint32_t value;
    std::stringstream ss(hexStr);
    ss >> std::hex >> value;
    return value;
}

int32_t hexToInt(const std::string& hexStr) {
    int32_t value;
    std::stringstream ss(hexStr);
    ss >> std::hex >> value;
    return value;
}

//Generated by ChatGPT. Prompt: Can you write yet another to translate from uint32_t to hex string?
std::string uint32ToHexString(uint32_t uintValue) {
    // Create an output string stream with hexadecimal formatting and no showbase
    std::ostringstream oss;
    oss << std::hex << std::noshowbase;

    // Interpret the bits of the float as an unsigned integer
    auto value = *reinterpret_cast<unsigned int*>(&uintValue);

    // Set the width to 8 characters and fill with zeroes
    oss << std::setw(8) << std::setfill('0') << value;

    // Return the formatted string
    return oss.str();
}

//Partly generated by ChatGpt. Prompt:  Given a unsigned int where the last 5 bit indicates flags raised, can you write me a function to reverse the order of the bits, and write as hex?
//Function to change position of flags and convert to Hex (reverse)
std::string convFlags(unsigned int flags) {
    unsigned int newFlags = 0;
    for (unsigned int i = 0; i<5; i++) {
        if(flags & (1<<i)) {
            newFlags |= (1 << (4-i));
        }
    }
    std::stringstream ss;
    // Output the flags as a hex number, padded with zeros up to the necessary number of digits (2 for 8 bits)
    ss  << std::setfill('0') << std::setw(2) << std::hex << (newFlags & 0x1F); // Mask to get only the least significant 5 bits
    return ss.str();

}



//Initialize testFPU
static int rfDepth = 32;
FPU testFPU(0, rfDepth); //0 pipeline stages -> instant execution


int main(int argc, char** argv) {
    std::string op, rm, input1, input2, input3, input4, flags;
    unsigned int mem;
    uint32_t xReg;
    //Use first 4 registers
    uint32_t r1 = 0;
    uint32_t r2 = 1;
    uint32_t r3 = 2;
    uint32_t rd = 3;

    op=argv[1]; //Get operation type:
    rm=argv[2]; //Set appropriate rounding mode
    if (rm == "-rnear_even") {
        testFPU.bd_setRoundingMode(0b000);
    } else if (rm== "-rminMag") {
        testFPU.bd_setRoundingMode(0b001);
    } else if (rm== "-rmin") {
        testFPU.bd_setRoundingMode(0b010);
    } else if (rm== "-rmax") {
        testFPU.bd_setRoundingMode(0b011);
    } else {
        //-rnear_maxMag & -rodd are not supported
        testFPU.bd_setRoundingMode(0b000);
    }

    if (op == "fmadd") //Fmadd has an extra input compared to other
    {
        while (std::cin >> input1 >> input2 >> input3 >> input4 >> flags) {
            unsigned int a = hexToUnsignedInt(input1);
            unsigned int b = hexToUnsignedInt(input2);
            unsigned int c = hexToUnsignedInt(input3);
            uint32_t r1 = 1;
            uint32_t r2 = 2; 
            uint32_t r3 = 3; 
            ITYPE instr_load = {.parts= {7, r1, 0b010, 0, 0}};
            testFPU.operation(instr_load.instr, 0, 0, a, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            instr_load = {.parts= {7, r2, 0b010, 0, 0}};
            testFPU.operation(instr_load.instr, 0, 0, b, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            instr_load = {.parts= {7, r3, 0b010, 0, 0}};
            testFPU.operation(instr_load.instr, 0, 0, c, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
        
            RTYPE instr_r4type = {.parts_r4type= {FMADD_S, 0, 0b000, r1, r2, 0b00, r3}};
            FpuPipeObj result = testFPU.operation(instr_r4type.instr, 0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            std::cout << input1 << " " << input2 << " " << input3 << " " << floatToHex(result.data.f) << " " << convFlags(result.flags)  << std::endl;
        }
    } else if (op == "fsqrt" || op == "ui32_to_f32" || op == "i32_to_f32" || op == "f32_to_ui32" || op == "f32_to_i32") { //fsqrt uses only three inputs
        while (std::cin >> input1 >> input2 >> flags) {
            unsigned int a = hexToUnsignedInt(input1);
            unsigned int b = hexToUnsignedInt(input2);
            unsigned int c = hexToUnsignedInt(input3);
            //Load values
            ITYPE instr_load = {.parts= {7, r1, 0b010, 0, 0}};
            testFPU.operation(instr_load.instr, 0, 0, a, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            instr_load = {.parts= {7, r2, 0b010, 0, 0}};
            testFPU.operation(instr_load.instr, 0, 0, b, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            //Do operation
            RTYPE instr_rtype = {};
            FpuPipeObj result = {};
            if (op == "fsqrt") {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, 0b00000, FSQRT_S}};
                result =  testFPU.operation(instr_rtype.instr, 0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            }
            else if (op == "ui32_to_f32") {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, 0b00001, FCVT_S_W}};
                result =  testFPU.operation(instr_rtype.instr, 0, hexToUnsignedInt(input1), 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            }
            else if (op == "i32_to_f32") {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, 0b00000, FCVT_S_W}};
                result =  testFPU.operation(instr_rtype.instr, 0, hexToUnsignedInt(input1), 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            }
            else if (op == "f32_to_ui32") {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, 0b00001, FCVT_W_S}};
                result =  testFPU.operation(instr_rtype.instr, 0, 0, 0, nullptr, nullptr, &xReg, nullptr, nullptr, nullptr);
            }
            else if (op == "f32_to_i32") {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, 0b00000, FCVT_W_S}};
                result =  testFPU.operation(instr_rtype.instr, 0, 0, 0, nullptr, nullptr, &xReg, nullptr, nullptr, nullptr);
            }
            if (op == "fsqrt" || op == "ui32_to_f32" || op == "i32_to_f32") {
                std::cout << input1 << " " << floatToHex(result.data.f) << " " << convFlags(result.flags)  << std::endl;
            } else {
                std::cout << input1 << " " << uint32ToHexString(static_cast<uint32_t>(xReg)) << " " << convFlags(result.flags & 0b01111)  << std::endl; //Conversion to int doesn't check for exactness
            }
        }
    } else {
        while (std::cin >> input1 >> input2 >> input3 >> flags) {
            unsigned int a = hexToUnsignedInt(input1);
            unsigned int b = hexToUnsignedInt(input2);
            unsigned int c = hexToUnsignedInt(input3);
            //Load values
            ITYPE instr_load = {.parts= {7, r1, 0b010, 0, 0}};
            testFPU.operation(instr_load.instr, 0, 0, a, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            instr_load = {.parts= {7, r2, 0b010, 0, 0}};
            testFPU.operation(instr_load.instr, 0, 0, b, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            //Do operation
            RTYPE instr_rtype;
            if (op == "fadd")
            {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FADD_S}};
            }
            else if (op == "fsub") //FSUB_S:
            {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FSUB_S}};
            }
            else if (op == "fmul") //FMUL_S:
            {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FMUL_S}};
            }
            else if (op == "fdiv") //FDIV_S:
            {
                instr_rtype = {.parts= {OP_FP, rd, 0b000, r1, r2, FDIV_S}};
            }
            else if (op == "feq") //FEQ.S
            {
                instr_rtype = {.parts = {OP_FP, rd, 0b010, r1, r2, FCMP}};
            }
            else if (op == "flt") //FLT.S
            {
                instr_rtype = {.parts = {OP_FP, rd, 0b001, r1, r2, FCMP}};
            }
            else if (op == "fle") //FLE.s
            {
                instr_rtype = {.parts = {OP_FP, rd, 0b000, r1, r2, FCMP}};
            }
            FpuPipeObj result =  testFPU.operation(instr_rtype.instr, 0, 0, 0, nullptr, &mem, &xReg, nullptr, nullptr, nullptr);
            if (op == "feq" || op == "flt" || op == "fle"){
                std::cout << input1 << " " << input2 << " " << std::to_string(xReg).front() << " " << convFlags(result.flags)  << std::endl;
            } else {
                std::cout << input1 << " " << input2 << " " << floatToHex(result.data.f) << " " << convFlags(result.flags)  << std::endl;
            }
        }
    }
   
    return 0;
}