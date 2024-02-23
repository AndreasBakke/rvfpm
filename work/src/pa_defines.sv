//####################################################################
// rvfpm - 2024
// Andreas S. Bakke
//
// DO NOT MODIFY
// AUTOMATICALLY OVERWRITTEN BY run/setup.py
//####################################################################

package pa_defines;

//System-parameters
  parameter NUM_F_REGS = 32;

//CORE-V-XIF-parameters
  parameter XLEN = 32;
  parameter FLEN = 32;
  parameter X_NUM_RS = 3;
  parameter X_ID_WIDTH = 4;
  parameter X_MEM_WIDTH = 32;
  parameter X_RFR_WIDTH = 32;
  parameter X_RFW_WIDTH = 32;
  parameter X_MISA = 0;
  parameter X_ECS_XS = 0;

//Pipeline-parameters
  parameter NUM_PIPELINE_STAGES = 4;
  `define QUEUE
  parameter QUEUE_DEPTH = 4;
  parameter OOO = 0;
  parameter FORWARDING = 0;
endpackage: pa_defines