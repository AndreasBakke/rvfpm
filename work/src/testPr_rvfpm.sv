
program automatic testPr_rvfpm #(
    parameter NUM_FPU_REGS,
    parameter PIPELINE_STAGES
)
(
    inTest_rvfpm uin_rvfpm
);
    
    initial begin
        $display("--- Starting simulation ---");
        init();
        basic();
        // testLoad();
        // testRead();
        $finish;
    end


    task reset();
        $display("Reset")
        uin_rvfpm.rst = 1;
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.rst = 0;
        @(posedge uin_rvfpm.ck);
    endtask


    task init();
        reset();
        uin_rvfpm.instruction = 0;
        uin_rvfpm.fromMem = 0;
        uin_rvfpm.fromXReg = 0;
    endtask


    task basic();
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.enable = 1;

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 32'b0000000_00000_00000_010_00001_0000111; //load from mem into register1;

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.data_fromMem = 1.7; //Data for the previous instruction (1 pipeline stage)
        uin_rvfpm.instruction = 32'b0000000_00000_00000_010_00010_0000111;; //load from mem into register2;

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.data_fromMem = 11.4; //Data for previous pipe stage
        uin_rvfpm.instruction = 32'b0000000_00010_00001_000_00011_1010011; //Add r1 r2 and store in r3

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.data_fromMem = 0;
        uin_rvfpm.instruction = 0;
        
        #200;
        uin_rvfpm.instruction = 32'b0000000_00000_00011_010_00011_0100111; //store r3 value to memory;
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 0;
    endtask

endprogram