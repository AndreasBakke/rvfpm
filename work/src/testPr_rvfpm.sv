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
        repeat(100) testRTYPE(); //TODO: do we need to seed random?
        testRTYPE(.rd(19)); //Test with specified destination
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 0;
        repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);
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
        for (int i=0; i<NUM_REGS; ++i) begin 
            uin_rvfpm.instruction[11:7] = i; //set register
            fork
                begin
                    repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                    uin_rvfpm.data_fromMem = $random; //set to random real at the appropriate time
                end
            join_none
            @(posedge uin_rvfpm.ck);
        end
        uin_rvfpm.instruction = 0;
        repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck); //wait for all operations to finish
        uin_rvfpm.data_fromMem = 0;
    endtask

    task testRTYPE(input int funct5 = 0, input int rs2 = $urandom_range(0, NUM_REGS-1), input int rs1 = $urandom_range(0, NUM_REGS-1), input int rd = $urandom_range(0, NUM_REGS-1));
        @(posedge uin_rvfpm.ck)
        uin_rvfpm.instruction[31:28] = funct5;
        uin_rvfpm.instruction[26:25] = 0; //fmt
        uin_rvfpm.instruction[24:20] = rs2; //rs2
        uin_rvfpm.instruction[19:15] = rs1; //rs1 (base)
        uin_rvfpm.instruction[14:12] = 0; //RM
        uin_rvfpm.instruction[11:7] = rd;  //rd (dest)
        uin_rvfpm.instruction[6:0] = 7'b1010011;  //OPCODE
        $display(rs2);
        $display(funct5);
        $display(rd);
    endtask

    task basic();
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.enable = 1;

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 32'b0000000_00000_00000_010_00001_0000111; //load from mem into register1;
        fork
            begin
                repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                uin_rvfpm.data_fromMem = 1.7; //Data for the previous instruction (1 pipeline stage)
            end
        join_none

        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 32'b0000000_00000_00000_010_00010_0000111;; //load from mem into register2;
        fork
            begin
                repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                uin_rvfpm.data_fromMem = 11.4; //Data for the previous instruction
            end
        join_none


        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 32'b0000000_00010_00001_000_00011_1010011; //Add r1 r2 and store in r3
        fork
            begin
                repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                uin_rvfpm.data_fromMem = 0; //set back to 0
            end
        join_none
    endtask

endprogram