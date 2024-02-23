//####################################################################
// rvfpm - 2024
// Andreas S. Bakke
//
// DO NOT MODIFY
// AUTOMATICALLY OVERWRITTEN BY run/setup.py
//####################################################################

`ifndef MY_DEFINES_SV
`define MY_DEFINES_SV
//System-parameters
  `define NUM_F_REGS 32

//CORE-V-XIF-parameters
  `define XLEN 32
  `define FLEN 32
  `define X_NUM_RS 3
  `define X_ID_WIDTH 4
  `define X_MEM_WIDTH 32
  `define X_RFR_WIDTH 32
  `define X_RFW_WIDTH 32
  `define X_MISA 0
  `define X_ECS_XS 0

//Pipeline-parameters
  `define INCLUDE_PIPELINE
  `define NUM_PIPELINE_STAGES 4
  `define INCLUDE_QUEUE
  `define QUEUE_DEPTH 4
  `define OOO 0
  `define FORWARDING 0
`endif