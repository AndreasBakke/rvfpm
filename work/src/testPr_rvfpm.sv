`timescale 1ns/1ps
program automatic testPr_rvfpm #(
    parameter NUM_REGS,
    parameter PIPELINE_STAGES
)
(
    inTest_rvfpm uin_rvfpm
);
    
    initial begin
        $display("--- Starting simulation ---");
        init();
        fillRF();
        basic();
        // testLoad();
        // testRead();
        $finish;
    end


    task reset();
        $display("Reset");
        uin_rvfpm.rst = 1;
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.rst = 0;
        @(posedge uin_rvfpm.ck);
    endtask


    task init();
        reset();
        uin_rvfpm.instruction = 0;
        uin_rvfpm.data_fromMem = 0;
        uin_rvfpm.data_fromXReg = 0;
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.enable = 1;
    endtask

    task fillRF();
        //Fills register file with random value
        uin_rvfpm.instruction[31:20] = 0; //imm
        uin_rvfpm.instruction[19:15] = 0; //rs1 (base)
        uin_rvfpm.instruction[14:12] = 0; //W
        uin_rvfpm.instruction[11:7] = 0;  //rd (dest)
        uin_rvfpm.instruction[6:0] = 7'b0000111;  //OPCODE
        for (int i=1; i<NUM_REGS; ++i) begin
            @(posedge uin_rvfpm.ck)
            uin_rvfpm.instruction[11:7] = i; //set register
            fork
                repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                uin_rvfpm.data_fromMem = $random; //set to random real at the appropriate time
            join_none
        end
        @(posedge uin_rvfpm.ck)
        uin_rvfpm.instruction = 0;
        repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck); //wait for all operations to finish
        uin_rvfpm.data_fromMem = 0; //set to random real //for last register (if pipelined by 1.)
    endtask

    task setFromMem();

    endtask

    task basic();
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.enable = 1;

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 32'b0000000_00000_00000_010_00001_0000111; //load from mem into register1;
        fork
                repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                uin_rvfpm.data_fromMem = 1.7; //Data for the previous instruction (1 pipeline stage)
        join_none

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 32'b0000000_00000_00000_010_00010_0000111;; //load from mem into register2;
        fork
                repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                uin_rvfpm.data_fromMem = 11.4; //Data for the previous instruction
        join_none


        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 32'b0000000_00010_00001_000_00011_1010011; //Add r1 r2 and store in r3
        fork
                repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                uin_rvfpm.data_fromMem = 0; //set back to 0
                
        join_none

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 0;
        #200;
        uin_rvfpm.instruction = 32'b0000000_00000_00011_010_00011_0100111; //store r3 value to memory;
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 0;
    endtask

endprogram