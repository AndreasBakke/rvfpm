/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    Signal interface for rvfpm_tb
*/
interface inTest_rvfpm #(
    parameter int X_ID_WIDTH = 4,
    parameter int NUM_REGS = 32,
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
    //-- Memory
    //-----------------------
    //-----------------------
    //-- DUT
    //-----------------------
    logic enable;
    logic fpu_ready;
    logic[31:0] instruction;

    logic[0:NUM_REGS-1][FLEN-1:0] registerFile; //For verification
    int unsigned pipelineIds[PIPELINE_STAGES];
    int unsigned queueIds[QUEUE_DEPTH];

    //-----------------------
    //-- Error count
    //-----------------------
    int errorCntAssertions;

endinterface

