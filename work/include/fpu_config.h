/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Configuration file for the FPU model.
  Set execution times, decode steps, and other parameters.
  TODO: Is it some better way to do this, where it can be used by both sv and cpp?
  Can i eg import the pa_rvfpm? Or is it better to have a separate file for the cpp and sv?
*/

#pragma once

//Configuration for the pipeline steps. Multiple steps can be set to the same value. 0 is the last step of pipeline, NUM_PIPELINE_STAGES-1 is the first step
//Decoding is done when adding to queue

enum pipelineConfig {
  EXECUTE_STEP = 2, //Defines at what stage the execute step is in the pipeline
  MEMORY_STEP = 1, //Defines at what stage request memory/integer results are returned in the pipeline
  WRITEBACK_STEP = 0 //Defines at what stage the writeback step is in the pipeline
};


typedef struct {
  unsigned int  id;
  unsigned int  rdata;
  bool          err ;  // Will the coprocessor write the Extension Context Status in mstatus?
  bool          dbg;        // Can the offloaded instruction possibly cause a synchronous exception in the coprocessor itself?
} x_memory_res_t;

typedef struct {
  unsigned int  id;
  unsigned int  addr;
  unsigned int  mode; //TODO: ?
  bool          we; //Write enable
  unsigned int  size;
  bool          be; //Byte enable
  unsigned int  attr; //TODO: ?
  unsigned int  wdata;
  bool          last;
  bool          spec;
} x_mem_req_t;