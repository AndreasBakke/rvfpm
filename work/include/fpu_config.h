/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  Configuration file for the FPU model.
  Set execution times, decode steps, and other parameters.
  TODO: Is it some better way to do this, where it can be used by both sv and cpp?
  Can i eg import the pa_rvfpm? Or is it better to have a separate file for the cpp and sv?
*/

#pragma once
// #pragma scalar_storage_order little-endian
//Configuration for the pipeline steps. Multiple steps can be set to the same value. 0 is the last step of pipeline, NUM_PIPELINE_STAGES-1 is the first step
//Decoding is done when adding to queue


//TODO: get this from the package. Use 1 file to define both pa_rvfpm and fpu_config.
#define X_ID_WIDTH 4

enum pipelineConfig {
  EXECUTE_STEP = 2, //Defines at what stage the execute step is in the pipeline
  MEMORY_STEP = 1, //Defines at what stage request memory/integer results are returned in the pipeline
  WRITEBACK_STEP = 0 //Defines at what stage the writeback step is in the pipeline
};


typedef struct {
  bool          accept    : 1;     // Is the offloaded instruction (id) accepted by the coprocessor?
  bool          writeback : 1;  // Will the coprocessor perform a writeback in the core to rd?
  bool          dualwrite : 1;  // Will the coprocessor perform a dual writeback in the core to rd and rd+1?
  unsigned int  dualread  : 3;   // Will the coprocessor require dual reads from rs1\rs2\rs3 and rs1+1\rs2+1\rs3+1?
  bool          loadstore : 1;  // Is the offloaded instruction a load/store instruction?
  bool          ecswrite  : 1;  // Will the coprocessor write the Extension Context Status in mstatus?
  bool          exc       : 1;        // Can the offloaded instruction possibly cause a synchronous exception in the coprocessor itself?
}  __attribute__((packed)) x_issue_resp_t;

typedef struct {
  unsigned int  id    : X_ID_WIDTH;
  uint32_t  rdata : 32;
  bool          err   : 1;  // Will the coprocessor write the Extension Context Status in mstatus?
  bool          dbg   : 1;        // Can the offloaded instruction possibly cause a synchronous exception in the coprocessor itself?
} __attribute__((packed)) x_mem_result_t;

typedef struct {
  unsigned int  id    : X_ID_WIDTH;
  unsigned int  addr  : 32;
  unsigned int  mode  : 2 ; //TODO: ?
  bool          we    : 1; //Write enable
  unsigned int  size  : 3;
  unsigned int  be    : 4; //Byte enable
  unsigned int  attr  : 2; //TODO: ?
  unsigned int  wdata : 32;
  bool          last  : 1;
  bool          spec  : 1;
} __attribute__((packed)) x_mem_req_t;

typedef struct{
    unsigned int  id  : X_ID_WIDTH;      // Identification of the offloaded instruction
    unsigned int data : 32;    // Register file write data value(s)
    unsigned int rd   : 5;      // Register file destination address(es)
    // logic [X_RFW_WIDTH/XLEN-1:0] we;      // Register file write enable(s)
    // logic [                 5:0] ecsdata; // Write data value for {mstatus.xs, mstatus.fs, mstatus.vs}
    // logic [                 2:0] ecswe;   // Write enables for {mstatus.xs, mstatus.fs, mstatus.vs}
    // bool                        exc :1;     // Did the instruction cause a synchronous exception?
    // logic [                 5:0] exccode; // Exception code
    // bool                        err :1;     // Did the instruction cause a bus error?
    // bool                        dbg :1;     // Did the instruction cause a debug trigger match with ``mcontrol.timing`` = 0?
  } __attribute__((packed)) x_result_t;
