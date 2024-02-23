/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    Signal interface for rvfpm_tb
*/
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
    //-- Memory
    //-----------------------
    //-----------------------
    //-- DUT
    //-----------------------
    logic enable;
    logic fpu_ready;

    `ifndef ZFINX
        logic [FLEN-1:0] registerFile[NUM_F_REGS]; //For verification
    `endif
    `ifdef PIPELINE
        logic unsigned pipelineIds[PIPELINE_STAGES];
    `endif
    `ifdef QUEUE
        logic unsigned queueIds[QUEUE_DEPTH];
    `endif

    //-----------------------
    //-- Error count
    //-----------------------
    int errorCntAssertions;
    int errorCntPr;


endinterface

