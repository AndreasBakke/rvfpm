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
    logic ck, rst, enable;
    logic [31:0] instruction;
    logic [3:0] id; // Assuming X_ID_WIDTH is 4
    logic [31:0] data_fromXreg, data_fromMem;



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
        .data_fromXreg(data_fromXreg),
        .data_fromMem(data_fromMem)
        // ... other ports if needed ...
    );

    
    // Clock generation
    always #20 ck = ~ck; // 20MHz clock

    // Initial block for test stimulus
    initial begin
        // Initialize signals
        ck = 0;
        rst = 1;
        enable = 0;
        instruction = 0;
        data_fromXreg = 0;
        data_fromMem = 0;

        #30 rst = 0; // Release reset after 30ns
        #5 enable = 1;
        data_fromMem = 'h3f800000; //Corresponds to 1
        instruction = 'b000000000100_00000_010_00001_0000011; //load from mem into register1;
        #20;
        data_fromMem = 'h412028f6; //Corresponds to 10.01
        instruction = 'b000000000100_00000_010_00010_0000011; //load from mem into register2;

        #20;
        data_fromMem = 0;
        instruction = 'b0000000_00010_00001_000_00011_1010011; //Add r1 r2 and store in r3
        #20;
        
        

        // Apply test vectors
        // ... Your test cases here ...

        // Finish the simulation
        #1000 $finish;
    end

    // Additional test scenarios, monitoring, checks, etc.

endmodule
