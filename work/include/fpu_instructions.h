/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Instruction enumerators and types for fpu.
*/
#pragma once

enum {
  it_NONE, it_RTYPE, it_R4TYPE, it_ITYPE, it_STYPE
} instr_type;


//One operation for each opcode
// See p 118 in riscv specification for opcodes
enum  {
  FLW       = 7,
  FSW       = 39,
  FMADD_S   = 67,
  FMSUB_S   = 71,
  FNMSUB_S  = 75,
  FNMADD_S  = 79,
  OP_FP     = 83
};

//Typedef for extracting fields of different instructions
typedef union {
  uint32_t instr;
  struct {
    unsigned int opcode : 7;
    unsigned int rd     : 5;
    unsigned int funct3 : 3;
    unsigned int rs1    : 5;
    unsigned int rs2    : 5;
    unsigned int funct7 : 7;
  } parts;
  struct {
    unsigned int opcode : 7;
    unsigned int rd     : 5;
    unsigned int funct3 : 3;
    unsigned int rs1    : 5;
    unsigned int rs2    : 5;
    unsigned int fmt    : 2;
    unsigned int rs3    : 5; //rs3
  } parts_r4type;
} RTYPE;

typedef union { //Only used for FLW
  uint32_t instr;
  struct {
    unsigned int opcode     : 7;
    unsigned int rd         : 5;
    unsigned int funct3     : 3;
    unsigned int rs1        : 5;
    unsigned int offset  : 12;
    //Etc for all fields that are of type RTYPE. Then the instructions can be mapped when the type is R.
  } parts;
} ITYPE;

typedef union { //Only used for FSW
  uint32_t instr;
  struct {
    unsigned int opcode     : 7;
    unsigned int imm_4_0    : 5;
    unsigned int funct3     : 3;
    unsigned int rs1        : 5;
    unsigned int rs2        : 5;
    unsigned int offset  : 7;
  } parts;
} STYPE;


//Enumerator for RTYPE instructions
enum RTYPE_funct7 {
  FADD_S            = 0,
  FSUB_S            = 4,
  FMUL_S            = 8,
  FDIV_S            = 12,
  FSGNJ             = 16,
  FMIN_MAX          = 20,
  FSQRT_S           = 44,
  FCMP              = 80,
  FCVT_W_S          = 96,
  FCVT_S_W          = 104,
  FCLASS_FMV_X_W    = 112, //FMW_X_W and FCLASS share the same funct5(7)
  FMV_W_X           = 120
};