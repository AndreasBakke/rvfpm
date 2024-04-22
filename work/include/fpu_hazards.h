#pragma once
#include "config.h"
#include "fpu_pipe.h"


enum HazardType {
  NO_HAZARD,
  RAW,
  WAR,
  WAW,
  STRUCTURAL, //Wait (applicable for multi-cycle executions )
};



typedef struct {
  HazardType type;
  FpuPipeObj* op;
  int pipeline_stage;
  //Some info about what addresses are conflicting


} hazard;