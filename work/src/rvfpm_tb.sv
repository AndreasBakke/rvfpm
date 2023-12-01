/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    rvfpm testbench.
    Prerequisites: Compile c++ model using make rhelSL (macSL not tested/verified)
    Then run vsim using -sv_lib bin/lib_rvfpm -novopt work.rvfpm_tb -suppress 12110
*/

`timescale 1ns/1ps
module rvfpm_tb;
    //-----------------------
    //-- Parameters
    //-----------------------

    parameter TB_FLEN = 32;
    parameter TB_XLEN = 32;
    parameter TB_NUM_FPU_REGS = 32;
    parameter TB_PIPELINE_STAGES = 1; // Example value
    parameter TB_X_ID_WIDTH = 4;

    localparam time ck_period = 40ns;


    //-----------------------
    //-- Declarations
    //-----------------------
    int errorCnt; //errorCount
    //Test interface
    inTest_rvfpm #(
        .X_ID_WIDTH(TB_X_ID_WIDTH),
        .NUM_REGS(TB_NUM_FPU_REGS),
        .XLEN(XLEN)
    ) uin_rvfpm ();


    //-----------------------
    //-- Clk gen
    //-----------------------
    initial begin
        uin_rvfpm.ck=0;
        forever begin
            #(ck_period/2);
            uin_rvfpm.ck=!uin_rvfpm.ck;
        end
    end

    //-----------------------
    //-- DUT
    //-----------------------

    rvfpm #(
        .NUM_REGS(TB_NUM_FPU_REGS),
        .PIPELINE_STAGES(TB_PIPELINE_STAGES)
        // ... other parameters if needed ...
    ) dut (
        .ck(uin_rvfpm.ck),
        .rst(uin_rvfpm.rst),
        .enable(uin_rvfpm.enable),
        .instruction(uin_rvfpm.instruction),
        .id(uin_rvfpm.id),
        .id_out(uin_rvfpm.id_out),
        .data_fromXReg(uin_rvfpm.data_fromXReg),
        .data_fromMem(uin_rvfpm.data_fromMem),
        .data_toXReg(uin_rvfpm.data_toXReg),
        .data_toMem(uin_rvfpm.data_toMem),
        .toXReg_valid(uin_rvfpm.toXReg_valid),
        .toMem_valid(uin_rvfpm.toMem_valid),
	    .fpu_ready(uin_rvfpm.fpu_ready) 
    );
    import "DPI-C" function shortreal getRFContent(input chandle fpu_ptr, input int addr);

    always @(posedge uin_rvfpm.ck) begin
        //Get entire rf for verification
        for (int i=0; i< TB_NUM_FPU_REGS; ++i) begin
            uin_rvfpm.registerFile[i] = getRFContent(dut.fpu, i);
        end
        //Get entire pipeline for verification
        // for()

    end

    //-----------------------
    //-- Assertions
    //-----------------------
    assertions_rvfpm #(
        .NUM_REGS(TB_NUM_FPU_REGS),
        .PIPELINE_STAGES(TB_PIPELINE_STAGES)
    ) u_assertions_rvfpm (
        .uin_rvfpm(uin_rvfpm)
    );

    //-----------------------
    //-- Test Program
    //-----------------------
    testPr_rvfpm #(
        .NUM_REGS(TB_NUM_FPU_REGS),
        .PIPELINE_STAGES(TB_PIPELINE_STAGES)
    ) u_testPr(
        .uin_rvfpm(uin_rvfpm)
    );
    
    //-----------------------
    //-- Result
    //-----------------------
    assign errorCnt = uin_rvfpm.errorCntAssertions;

    final begin
        printResult();
    end    


    function void printResult;
        $display("");
        $display("");
        $display($time, "ns: ");
        $display("------------------------------------");
        $display("------------------------------------");
        $display("");
        $display("Simulation finished, errors: $0d", errorCnt);
        $display("");
        $display("------------------------------------");
        $display("------------------------------------");
    endfunction

endmodule