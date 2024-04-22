/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    Signal interface for rvfpm_tb
*/
`include "../src/config.svh"
import in_xif::*;
interface inTest_rvfpm #(
    parameter int X_ID_WIDTH = 4,
    parameter int NUM_F_REGS = 32,
    parameter int PIPELINE_STAGES = 4,
    parameter int QUEUE_DEPTH = 4,
    parameter int XLEN = 32,
    parameter int FLEN = 32

);

    //-----------------------
    //-- Clock, reset enable
    //-----------------------
    logic ck;
    logic rst;
    //-----------------------
    //-- eXtension Interface
    //-----------------------
    //eXtension interface
    logic issue_valid;
    logic issue_ready;
    x_issue_req_t issue_req;
    x_issue_resp_t issue_resp;

    // Commit Interface
    logic commit_valid;
    x_commit_t commit;

    // Memory Eequest/Response Interface
    logic mem_valid;
    logic mem_ready;
    x_mem_req_t mem_req;
    x_mem_resp_t mem_resp;

    // Memory Result Interface
    logic mem_result_valid;
    x_mem_result_t mem_result;

    // Result Interface
    logic result_valid;
    logic result_ready;
    x_result_t result;

    //-----------------------
    //-- DUT
    //-----------------------
    logic enable;
    logic fpu_ready;

    `ifndef ZFINX
        logic [FLEN-1:0] registerFile[NUM_F_REGS]; //For verification
    `endif
    `ifdef INCLUDE_PIPELINE
        int unsigned pipelineIds[PIPELINE_STAGES];
        int unsigned waitingOpId;
    `endif
    `ifdef INCLUDE_QUEUE
        int unsigned queueIds[QUEUE_DEPTH];
    `endif

    int speculative_ids [$];

    //-----------------------
    //-- Error count
    //-----------------------
    int errorCntAssertions;
    int errorCntPr;


endinterface

