/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    TestProgram for rvfpm verification
*/

`timescale 1ns/1ps
program automatic testPr_rvfpm #(
    parameter NUM_REGS,
    parameter PIPELINE_STAGES,
    parameter X_ID_WIDTH 
)
(
    inTest_rvfpm uin_rvfpm
);
    import "DPI-C" function int unsigned randomFloat(); //C++ function for random float generation
    initial begin
        $display("--- Starting simulation ---");
        init();
        fillRF();
        basic();

        repeat(100) doRTYPE();
        doRTYPE(.rd(19)); //Test with specified destination
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 0;

        repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);
        repeat(10) @(posedge uin_rvfpm.ck); //Wait a bit


        repeat(100) doSTYPE();
        doSTYPE(.rs2(4)); //Read register 4.
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 0;
        repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);
        repeat(10) @(posedge uin_rvfpm.ck); //Wait a bit

        init();
        fillRF();

        repeat(100) begin //test FMV.X:W (move to integer). 
           doRTYPE(.funct7(7'b1110000), .rs2(0), .funct3(0)); 
           @(posedge uin_rvfpm.ck) uin_rvfpm.instruction = 0; //Set instr to 0 to toggle toXReg_valid.
        end

        repeat(10) @(posedge uin_rvfpm.ck);
        doRTYPE(.funct7(7'b0000100), .rs1(3), .rs2(2), .rd(4));
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 0;
        repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);
        
        //Classify
        doITYPE(.rd(0), .data(32'b11111111100000000000000000000000)); //-inf
        doITYPE(.rd(1), .data($shortrealtobits(-1.4125))); //Negative normal
        doITYPE(.rd(2), .data(32'b10000000000001000010000100000000)); //Negative subnormal
        doITYPE(.rd(3), .data(32'b10000000000000000000000000000000)); //-0
        doITYPE(.rd(4), .data(0)); //positive 0
        doITYPE(.rd(5), .data(32'b00000000000001000010000100000000)); //Positive subnormal
        doITYPE(.rd(6), .data($shortrealtobits(1.4125))); //positive normal (12.125)
        doITYPE(.rd(7), .data(32'b01111111100000000000000000000000)); //inf
        doITYPE(.rd(8), .data(32'b01111111101000000000000000000000)); //Signaling NaN
        doITYPE(.rd(9), .data(32'b01111111110000000000000000000000)); //qNaN
        for (int i=0; i<10; ++i) begin
            doRTYPE(.funct7(7'b1110000), .rs1(i), .rs2(0), .rd(0), .funct3(001)); //Class
        end
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.instruction = 0;
        repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);

        //Sign testing
        // + and  +
        doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(6), .rd(10), .funct3(000)); //FSGNJ.S (get from +)
        doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(6), .rd(10), .funct3(001)); //FSGNJN.S (get negated +)
        doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(6), .rd(10), .funct3(010)); //FSGNJX.S (0 xor 0)
        // + and -
        doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(1), .rd(11), .funct3(000)); //FSGNJ.S (get from -)
        doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(1), .rd(11), .funct3(001)); //FSGNJN.S (get negated-)
        doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(1), .rd(11), .funct3(010)); //FSGNJX.S (0 xor 1)
        // - and -
        doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(1), .rd(12), .funct3(000)); //FSGNJ.S (get from -)
        doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(1), .rd(12), .funct3(001)); //FSGNJN.S (get negated-)
        doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(1), .rd(12), .funct3(010)); //FSGNJX.S (1 xor 1)
        // - and +
        doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(6), .rd(13), .funct3(000)); //FSGNJ.S (get from -)
        doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(6), .rd(13), .funct3(001)); //FSGNJN.S (get negated-)
        doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(6), .rd(13), .funct3(010)); //FSGNJX.S (0 xor 1)
        @(posedge uin_rvfpm.ck)
        uin_rvfpm.instruction = 0;
        uin_rvfpm.id=0;
        repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);

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
        uin_rvfpm.id = 0;
        uin_rvfpm.data_fromMem = 0;
        uin_rvfpm.data_fromXReg = 0;
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.enable = 1;
    endtask

    task fillRF();
        //Fills register file with random value
        for (int i=0; i<NUM_REGS; ++i) begin
            doITYPE(.rd(i));
        end
        uin_rvfpm.instruction = 0;
        repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck); //wait for all operations to finish
        uin_rvfpm.data_fromMem = 0;
    endtask

    task doRTYPE(input int funct7 = 0, input int rs2 = $urandom_range(0, NUM_REGS-1), input int rs1 = $urandom_range(0, NUM_REGS-1), input int funct3 = 0, input int rd = $urandom_range(0, NUM_REGS-1));
        @(posedge uin_rvfpm.ck)
        setId();
        uin_rvfpm.instruction[31:25] = funct7;
        uin_rvfpm.instruction[24:20] = rs2; //rs2
        uin_rvfpm.instruction[19:15] = rs1; //rs1 (base)
        uin_rvfpm.instruction[14:12] = funct3; //RM
        uin_rvfpm.instruction[11:7] = rd;  //rd (dest)
        uin_rvfpm.instruction[6:0] = 7'b1010011;  //OPCODE
    endtask

    // task testR4TYPE() 
    // endtask

    task doSTYPE(input int imm = 0, input int rs2 = $urandom_range(0, NUM_REGS-1), input int rs1 = 0, input int offset = 0); //Default get value from random register
        @(posedge uin_rvfpm.ck)
        setId();
        uin_rvfpm.instruction[31:25] = imm;
        uin_rvfpm.instruction[24:20] = rs2; //rs2 (src)
        uin_rvfpm.instruction[19:15] = rs1; //rs1 (base)
        uin_rvfpm.instruction[14:12] = 3'b010; //rm (W)
        uin_rvfpm.instruction[11:7] = offset;  //rd
        uin_rvfpm.instruction[6:0] = 7'b0100111;  //OPCODE
        @(posedge uin_rvfpm.ck)
        uin_rvfpm.instruction = 0; //So toMem_valid toggles
    endtask

    task doITYPE(input int imm = 0, input int rs1 = 0, input int funct3 = 0, input int rd = $urandom_range(0, NUM_REGS-1), input int unsigned data = randomFloat()); //Default: Store random value into random register
        @(posedge uin_rvfpm.ck)
        setId();
        uin_rvfpm.instruction[31:20] = imm; //imm
        uin_rvfpm.instruction[19:15] = rs1; //rs1 (base)
        uin_rvfpm.instruction[14:12] = 3'b010; //rm (W)
        uin_rvfpm.instruction[11:7] = rd;  //rd (dest)
        uin_rvfpm.instruction[6:0] = 7'b0000111;  //OPCODE
        fork
            begin
                repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
                uin_rvfpm.data_fromMem = data; //set data at appropriate time
            end
        join_none
        @(posedge uin_rvfpm.ck)
        uin_rvfpm.instruction = 0;
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

    task setId();
        uin_rvfpm.id = (uin_rvfpm.id +1) % 2**X_ID_WIDTH;
    endtask;

endprogram