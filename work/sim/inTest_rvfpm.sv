/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    Signal interface for rvfpm_tb
*/
interface inTest_rvfpm #(
    parameter int X_ID_WIDTH = 4,
    parameter int NUM_REGS = 32,
    parameter int XLEN = 32
);

    //-----------------------
    //-- Clock, reset enable
    //-----------------------
    logic ck;
    logic rst;
    //-----------------------
    //-- Memory
    //-----------------------
    shortreal data_fromMem, data_toMem;
    logic toMem_valid;

    //-----------------------
    //-- Xreg
    //-----------------------
    logic[XLEN-1:0] data_fromXReg, data_toXReg;
    logic toXReg_valid;

    //-----------------------
    //-- DUT
    //-----------------------
    logic enable;
    logic fpu_ready;
    logic[31:0] instruction;

    shortreal registerFile[NUM_REGS]; //For verification

    //-----------------------
    //-- CORE-V-XIF
    //-----------------------
    logic [X_ID_WIDTH-1:0] id, id_out;

    //-----------------------
    //-- Error count
    //-----------------------
    int errorCntAssertions;

endinterface

