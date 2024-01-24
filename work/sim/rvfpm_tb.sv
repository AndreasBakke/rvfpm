/*  rvfpm - 2024
    Andreas S. Bakke

    Description:
    rvfpm testbench.
    Prerequisites: Compile c++ model using make sharedLib (macSL not tested/verified)
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
    parameter TB_PIPELINE_STAGES = 4; // Example value
    parameter TB_X_ID_WIDTH = 8;

    localparam time ck_period = 40ns;


    //-----------------------
    //-- Declarations
    //-----------------------
    int errorCnt; //errorCount
    //Test interface
    inTest_rvfpm #(
        .X_ID_WIDTH(TB_X_ID_WIDTH),
        .NUM_REGS(TB_NUM_FPU_REGS),
        .PIPELINE_STAGES(TB_PIPELINE_STAGES),
        .XLEN(TB_XLEN)
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
        .PIPELINE_STAGES(TB_PIPELINE_STAGES),
        .X_ID_WIDTH(TB_X_ID_WIDTH),
        .XLEN(TB_XLEN)
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
    import "DPI-C" function int unsigned getRFContent(input chandle fpu_ptr, input int addr);
    import "DPI-C" function int unsigned getPipeStageId(input chandle fpu_ptr, input int stage);

    always @(posedge uin_rvfpm.ck) begin
        //Get entire rf for verification
        for (int i=0; i < TB_NUM_FPU_REGS; ++i) begin
            uin_rvfpm.registerFile[i] = getRFContent(dut.fpu, i);
        end
        //Get entire pipeline for verification
        for (int i=0; i < TB_PIPELINE_STAGES; ++i) begin
            uin_rvfpm.pipelineIds[i] = getPipeStageId(dut.fpu, i);
        end

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
        .PIPELINE_STAGES(TB_PIPELINE_STAGES),
        .X_ID_WIDTH(TB_X_ID_WIDTH)
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