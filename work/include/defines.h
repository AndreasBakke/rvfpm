//####################################################################
// rvfpm - 2024
// Andreas S. Bakke
//
// DO NOT MODIFY
// AUTOMATICALLY OVERWRITTEN BY run/setup.py
//####################################################################

#pragma once

//System-parameters
#define NUM_F_REGS 32

//CORE-V-XIF-parameters
#define XLEN 32
#define FLEN 32
#define X_NUM_RS 3
#define X_ID_WIDTH 4
#define X_MEM_WIDTH 32
#define X_RFR_WIDTH 32
#define X_RFW_WIDTH 32
#define X_MISA 0
#define X_ECS_XS 0

//Pipeline-parameters
#define NUM_PIPELINE_STAGES 4
#define QUEUE_DEPTH 4
#define OOO 0
#define FORWARDING 0

enum pipelineConfig {
  EXECUTE_STEP = 2,
  MEMORY_STEP = 1,
  WRITEBACK_STEP = 0,
};
