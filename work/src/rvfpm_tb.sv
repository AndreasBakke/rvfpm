`timescale 1ns/1ps

module rvfpm_tb;
    //-----------------------
    //-- Parameters
    //-----------------------

    parameter TB_FLEN = 32;
    parameter TB_XLEN = 32;
    parameter TB_NUM_FPU_REGS = 32;
    parameter TB_PIPELINE_STAGES = 4; // Example value

    //-----------------------
    //-- Signals
    //-----------------------
    logic ck, rst, enable, fpu_ready, toMem_valid, toXreg_valid;
    int unsigned instruction;
    logic [3:0] id, id_out; // Assuming X_ID_WIDTH is 4
    int data_fromXreg, data_toXreg;
    shortreal data_fromMem, data_toMem;

    //-----------------------
    //-- DUT
    //-----------------------

    in_rvfpm #(
        .NUM_REGS(TB_NUM_FPU_REGS),
        .PIPELINE_STAGES(TB_PIPELINE_STAGES)
        // ... other parameters if needed ...
    ) dut (
        .ck(ck),
        .rst(rst),
        .enable(enable),
        .instruction(instruction),
        .id(id),
        .id_out(id_out),
        .data_fromXreg(data_fromXreg),
        .data_fromMem(data_fromMem),
        .data_toXreg(data_toXreg),
        .data_toMem(data_toMem),
        .toXreg_valid(toXreg_valid),
        .toMem_valid(toMem_valid),
	.fpu_ready(fpu_ready) 
    );

    
    // Clock generation
    always begin
        ck=0; #20;
        ck=1; #20;
    end

    // Initial block for test stimulus
    initial begin
        // Initialize signals
        ck = 0;
        rst = 1;
        enable = 0;
        instruction = 0;
        data_fromXreg = 0;
        data_fromMem = 0;

        #50 rst = 0; // Release reset after 30ns
        #45 enable = 1;
        #50;
        data_fromMem = 1.7;
	    $display("Instruction in SV: %h", instruction);
        instruction = 32'b0000000_00000_00000_010_00001_0000111; //load from mem into register1;
        #90;
        data_fromMem = 11.4;
        instruction = 32'b0000000_00000_00000_010_00010_0000111;; //load from mem into register2;
        #200;

        data_fromMem = 0;
        instruction = 32'b0000000_00010_00001_000_00011_1010011; //Add r1 r2 and store in r3
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