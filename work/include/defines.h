//####################################################################
// rvfpm - 2024
// Andreas S. Bakke
//
// DO NOT MODIFY
// AUTOMATICALLY OVERWRITTEN BY run/setup.py
//####################################################################

#pragma once

//System-parameters
const int NUM_F_REGS=32;

//CORE-V-XIF-parameters
const int XLEN=32;
const int FLEN=32;
const int X_NUM_RS=3;
const int X_ID_WIDTH=4;
const int X_MEM_WIDTH=32;
const int X_RFR_WIDTH=32;
const int X_RFW_WIDTH=32;
const int X_MISA=0;
const int X_ECS_XS=0;

//Pipeline-parameters
const int NUM_PIPELINE_STAGES=4;
const int QUEUE_DEPTH=4;
const bool OOO= 0;
const bool FORWARDING= 0;

enum pipelineConfig {
  EXECUTE_STEP = 2,
  MEMORY_STEP = 1,
  WRITEBACK_STEP = 0,
};

//Ex-cycles-parameters
#define NUM_CYCLES_DEFAULT 1
#define NUM_CYCLES_FADD 4
#define NUM_CYCLES_FSQRT 19
#define NUM_CYCLES_FDIV 16
