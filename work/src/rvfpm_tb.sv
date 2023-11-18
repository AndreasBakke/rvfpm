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
        .X_ID_WIDTH(TB_X_ID_WIDTH)
    ) uin_rvfpm ();


    //-----------------------
    //-- Clk gen
    //-----------------------
    intial begin
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
        .data_fromXreg(uin_rvfpm.data_fromXreg),
        .data_fromMem(uin_rvfpm.data_fromMem),
        .data_toXreg(uin_rvfpm.data_toXreg),
        .data_toMem(uin_rvfpm.data_toMem),
        .toXreg_valid(uin_rvfpm.toXreg_valid),
        .toMem_valid(uin_rvfpm.toMem_valid),
	    .fpu_ready(uin_rvfpm.fpu_ready) 
    );

    //-----------------------
    //-- Assertions
    //-----------------------
    assertions_rvfpm #(
        .NUM_REGS(TB_NUM_FPU_REGS),
        .PIPELINE_STAGES(TB_PIPELINE_STAGES)
    ) u_assertions_rvfpm (
        .uin_rvfpm(uin_rvfpm)
    )

    //-----------------------
    //-- Test Program
    //-----------------------
    testPr_rvfpm #(
        .NUM_REGS(TB_NUM_FPU_REGS),
        .PIPELINE_STAGES(TB_PIPELINE_STAGES)
    ) u_testPr(
        .uin_rvfpm(uin_rvfpm)
    )
    
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

    // Initial block for test stimulus
    initial begin
        // Initialize signals
        uin_rvfpm.ck = 0;
        uin_rvfpm.rst = 1;
        uin_rvfpm.enable = 0;
        uin_rvfpm.instruction = 0;
        uin_rvfpm.data_fromXreg = 0;
        uin_rvfpm.data_fromMem = 0;

        #50 rst = 0; // Release reset after 30ns
        #40 enable = 1;
        #40;
        instruction = 32'b0000000_00000_00000_010_00001_0000111; //load from mem into register1;
        #40;
        data_fromMem = 1.7; //Data for the previous instruction (1 pipeline stage)
        instruction = 32'b0000000_00000_00000_010_00010_0000111;; //load from mem into register2;
        #40;
        data_fromMem = 11.4; //Data for previous pipe stage
        instruction = 32'b0000000_00010_00001_000_00011_1010011; //Add r1 r2 and store in r3
        #40;
        data_fromMem = 0;
        instruction = 0;
        #200;
        instruction = 32'b0000000_00000_00011_010_00011_0100111; //store r3 value to memory;
        #40;
        instruction = 0;

        #1000;
	$display("Instruction in SV: %h", instruction);
	$stop;
    end

    // Additional test scenarios, monitoring, checks, etc.

endmodule