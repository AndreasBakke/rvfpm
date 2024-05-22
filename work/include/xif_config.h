/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Structs for interfacing with the FPU.
  Loads common defines for both sv and cpp through config.h
*/

#pragma once
#include "config.h"
#include "fp_number.h"
#include <stdint.h>

typedef struct {
  bool          accept    : 1;
  bool          writeback : 1;
  bool          dualwrite : 1;
  unsigned int  dualread  : 3;
  bool          loadstore : 1;
  bool          ecswrite  : 1;
  bool          exc       : 1;
}  __attribute__((packed)) x_issue_resp_t;

typedef struct {
  unsigned int  id    : X_ID_WIDTH;
  uint32_t      rdata : 32;
  bool          err   : 1;
  bool          dbg   : 1;
} __attribute__((packed)) x_mem_result_t;

typedef struct {
  unsigned int  id    : X_ID_WIDTH;
  unsigned int  addr  : 32;
  unsigned int  mode  : 2;
  bool          we    : 1;
  unsigned int  size  : 3;
  unsigned int  be    : 4;
  unsigned int  attr  : 2;
  unsigned int  wdata : 32;
  bool          last  : 1;
  bool          spec  : 1;
} __attribute__((packed)) x_mem_req_t;

typedef struct{
  unsigned int id  : X_ID_WIDTH;
  unsignedType data : 32;
  unsigned int rd   : 5;
  bool         we    : 1;
  unsigned int ecswe : 3;
  unsigned int ecsdata : 6;
} __attribute__((packed)) x_result_t;
