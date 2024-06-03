/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Instruction enumerators and types for fpu.
*/
#pragma once

enum {
  it_NONE, it_RTYPE, it_R4TYPE, it_ITYPE, it_STYPE, it_CSRTYPE
} instr_type;


//One operation for each opcode (or multiple for R4TYPE and CSR)
enum  {
  FLW       = 7,
  FSW       = 39,
  FMADD     = 67,
  FMSUB     = 71,
  FNMSUB    = 75,
  FNMADD    = 79,
  OP_FP     = 83,
  CSR       = 115
};
enum RISCV_FMT {
  S = 0b00,
  D = 0b01,
  H = 0b10,
  Q = 0b11
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
    RISCV_FMT fmt    : 2;
    unsigned int funct5 : 5;
  } parts;
  struct {
    unsigned int opcode : 7;
    unsigned int rd     : 5;
    unsigned int funct3 : 3;
    unsigned int rs1    : 5;
    unsigned int rs2    : 5;
    RISCV_FMT fmt    : 2;
    unsigned int rs3    : 5;
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

typedef union {
  uint32_t instr;
  struct {
    unsigned int opcode   : 7;
    unsigned int rd       : 5;
    unsigned int funct3   : 3;
    unsigned int rs1      : 5;
    unsigned int csr      : 12;
  } parts;
} CSRTYPE;


//Enumerator for RTYPE instructions
enum RTYPE_funct5 {
  FADD              = 0,
  FSUB              = 1,
  FMUL              = 2,
  FDIV              = 3,
  FSGNJ             = 4,
  FMIN_MAX          = 5,
  FSQRT             = 11,
  FCMP              = 20,
  FCVT_W_S          = 24,
  FCVT_S_W          = 26,
  FCLASS_FMV_X_W    = 28, //FMW_X_W and FCLASS share the same funct5
  FMV_W_X           = 30
};

