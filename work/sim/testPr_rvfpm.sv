/*  rvfpm - 2024
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
    inTest_rvfpm uin_rvfpm,
    in_xif uin_xif
);
    import "DPI-C" function int unsigned randomFloat(); //C++ function for random float generation

    localparam NUM_TESTS = 10;


    initial begin
        $display("--- Starting simulation ---");
        init();
        fillRF();
        repeat(NUM_TESTS) doRTYPE();
        doRTYPE(.rd(19)); //Test with specified destination
        // @(posedge uin_rvfpm.ck);
        // repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);
        // repeat(10) @(posedge uin_rvfpm.ck); //Wait a bit

        repeat(NUM_TESTS) doSTYPE();
        doSTYPE(.rs2(4)); //Read register 4.
        // @(posedge uin_rvfpm.ck);
        // repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);
        // repeat(10) @(posedge uin_rvfpm.ck); //Wait a bit

        // init();
        // fillRF();
        // repeat(NUM_TESTS) begin //test FMV.X:W (move to integer).
        //    doRTYPE(.funct7(7'b1110000), .rs2(0), .funct3(0));
        //    @(posedge uin_rvfpm.ck) uin_rvfpm.instruction = 0; //Set instr to 0 to toggle toXReg_valid.
        // end
        // init();
        // fillRF();
        // repeat(NUM_TESTS) begin //test FMV.W:X (move from integer).
        //     doRTYPE(.funct7(7'b1111000), .rs2(0), .funct3(0));
        //     fork
        //         begin
        //             repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck);
        //             uin_rvfpm.data_fromXReg = randomFloat(); //set data at appropriate time
        //         end
        //     join_none
        // end
        // uin_rvfpm.data_fromXReg = 0;
        // init();
        // fillRF();
        // repeat(NUM_TESTS) begin //test NUM_TESTS number of min-operations using random registers
        //     doRTYPE(.funct7(7'b0010100), .funct3(0));
        // end
        // init();
        // fillRF();
        // repeat(NUM_TESTS) begin //test NUM_TESTS number of max-operations using random registers
        //     doRTYPE(.funct7(7'b0010100), .funct3(3'b001));
        // end
        // init();
        // fillRF();

        // repeat(NUM_TESTS) begin //test NUM_TESTS number of FSGNJ-operations using random registers
        //     doRTYPE(.funct7(7'b0010000), .funct3(3'b000));
        // end
        // init();
        // fillRF();
        // repeat(NUM_TESTS) begin //test NUM_TESTS number of FSGNJN-operations using random registers
        //     doRTYPE(.funct7(7'b0010000), .funct3(3'b001));
        // end
        // init();
        // fillRF();

        // repeat(NUM_TESTS) begin //test NUM_TESTS number of FSGNJX-operations using random registers
        //     doRTYPE(.funct7(7'b0010000), .funct3(3'b010));
        // end

        // repeat(10) @(posedge uin_rvfpm.ck);
        // doRTYPE(.funct7(7'b0000100), .rs1(3), .rs2(2), .rd(4));
        // @(posedge uin_rvfpm.ck);
        // uin_rvfpm.instruction = 0;
        // repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);

        // //Classify
        // doITYPE(.rd(0), .data(32'b11111111100000000000000000000000)); //-inf
        // doITYPE(.rd(1), .data($shortrealtobits(-1.4125))); //Negative normal
        // doITYPE(.rd(2), .data(32'b10000000000001000010000100000000)); //Negative subnormal
        // doITYPE(.rd(3), .data(32'b10000000000000000000000000000000)); //-0
        // doITYPE(.rd(4), .data(0)); //positive 0
        // doITYPE(.rd(5), .data(32'b00000000000001000010000100000000)); //Positive subnormal
        // doITYPE(.rd(6), .data($shortrealtobits(1.4125))); //positive normal (12.125)
        // doITYPE(.rd(7), .data(32'b01111111100000000000000000000000)); //inf
        // doITYPE(.rd(8), .data(32'b01111111101000000000000000000000)); //Signaling NaN
        // doITYPE(.rd(9), .data(32'b01111111110000000000000000000000)); //qNaN
        // for (int i=0; i<10; ++i) begin
        //     doRTYPE(.funct7(7'b1110000), .rs1(i), .rs2(0), .rd(0), .funct3(3'b001)); //Class
        // end
        // @(posedge uin_rvfpm.ck);
        // uin_rvfpm.instruction = 0;
        // repeat(PIPELINE_STAGES*2) @(posedge uin_rvfpm.ck);

        // //Sign testing //For waveforms
        // // + and  +
        // doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(6), .rd(10), .funct3(3'b000)); //FSGNJ.S (get from +)
        // doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(6), .rd(10), .funct3(3'b001)); //FSGNJN.S (get negated +)
        // doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(6), .rd(10), .funct3(3'b010)); //FSGNJX.S (0 xor 0)
        // // + and -
        // doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(1), .rd(11), .funct3(3'b000)); //FSGNJ.S (get from -)
        // doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(1), .rd(11), .funct3(3'b001)); //FSGNJN.S (get negated-)
        // doRTYPE(.funct7(7'b0010000), .rs1(6), .rs2(1), .rd(11), .funct3(3'b010)); //FSGNJX.S (0 xor 1)
        // // - and -
        // doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(1), .rd(12), .funct3(3'b000)); //FSGNJ.S (get from -)
        // doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(1), .rd(12), .funct3(3'b001)); //FSGNJN.S (get negated-)
        // doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(1), .rd(12), .funct3(3'b010)); //FSGNJX.S (1 xor 1)
        // // - and +
        // doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(6), .rd(13), .funct3(3'b000)); //FSGNJ.S (get from -)
        // doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(6), .rd(13), .funct3(3'b001)); //FSGNJN.S (get negated-)
        // doRTYPE(.funct7(7'b0010000), .rs1(1), .rs2(6), .rd(13), .funct3(3'b010)); //FSGNJX.S (0 xor 1)
        // @(posedge uin_rvfpm.ck)
        // uin_rvfpm.instruction = 0;
        // uin_rvfpm.id=0;
        repeat(PIPELINE_STAGES*4) @(posedge uin_rvfpm.ck);

end


    task reset();
        $display("Reset");
        uin_rvfpm.rst = 1;
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.rst = 0;
        @(posedge uin_rvfpm.ck);
    endtask

    logic[X_ID_WIDTH-1:0] id = 0;
    task nextId();
        id = (id+1); //Reserve 0 for resets TODO: find a better way to do it.
    endtask;

    task init();
        reset();
        //TODO: init all signals
        uin_xif.issue_valid = 0;
        uin_xif.mem_result_valid = 0;
        uin_xif.mem_result ={};
        @(posedge uin_rvfpm.ck);
        uin_rvfpm.enable = 1;
    endtask

    task fillRF();
        //Fills register file with random value
        for (int i=0; i<NUM_REGS; ++i) begin
            doITYPE(.rd(i), .data(randomFloat()));
            @(negedge uin_xif.issue_valid);
        end
        repeat(PIPELINE_STAGES) @(posedge uin_rvfpm.ck); //wait for all operations to finish
    endtask

    task doRTYPE(input int funct7 = 0, input int rs2 = $urandom_range(0, NUM_REGS-1), input int rs1 = $urandom_range(0, NUM_REGS-1), input int funct3 = 0, input int rd = $urandom_range(0, NUM_REGS-1));
        automatic logic[31:0]  instr_r = 0;
        @(posedge uin_rvfpm.ck)
        uin_xif.issue_valid = 1;
        instr_r[31:25] = funct7;
        instr_r[24:20] = rs2; //rs2
        instr_r[19:15] = rs1; //rs1 (base)
        instr_r[14:12] = funct3; //RM
        instr_r[11:7] = rd;  //rd (dest)
        instr_r[6:0] = 7'b1010011;  //OPCODE
        doIssueInst(instr_r, id);
    endtask

    task doSTYPE(input int imm = 17, input int rs2 = $urandom_range(0, NUM_REGS-1), input int rs1 = 0, input int offset = 0); //Default get value from random register
        automatic logic [31:0] instr_s = 0;
        automatic logic[X_ID_WIDTH-1:0] issue_id = 0;
        @(posedge uin_rvfpm.ck)
        instr_s[31:25] = imm;
        instr_s[24:20] = rs2; //rs2 (src)
        instr_s[19:15] = rs1; //rs1 (base)
        instr_s[14:12] = 3'b010; //rm (W)
        instr_s[11:7] = offset;  //rd imm_4_0
        instr_s[6:0] = 7'b0100111;  //OPCODE
        issue_id = id;
        doIssueInst(instr_s, id);
        fork
            begin
                while (1) begin
                    @(posedge uin_rvfpm.ck) //Wait for memory request from CPU
                    if (uin_xif.mem_valid && uin_xif.mem_req.id == issue_id) begin
                        @(posedge uin_rvfpm.ck)
                        uin_xif.mem_ready = 1;//Todo: add response (dbg etc)
                        @(posedge uin_rvfpm.ck)
                        uin_xif.mem_ready = 0;//Todo: add response (dbg etc)
                        break;
                    end
                end
            end
        join_none
        //Fork and repond with memory write

    endtask

    task doITYPE(input int imm = 17, input int rs1 = 6, input int funct3 = 0, input int rd = $urandom_range(0, NUM_REGS-1), input int unsigned data = randomFloat()); //Default: Store random value into random register
        automatic logic [31:0] instr_i = 0;
        automatic logic[X_ID_WIDTH-1:0] issue_id = 0;
        @(posedge uin_rvfpm.ck)
        instr_i[31:20] = imm; //imm
        instr_i[19:15] = rs1; //rs1 (base)
        instr_i[14:12] = 3'b010; //rm (W)
        instr_i[11:7] = rd;  //rd (dest)
        instr_i[6:0] = 7'b0000111;  //OPCODE
        issue_id = id;
        doIssueInst(instr_i, id);
        fork
            begin
                while (1) begin
                    @(posedge uin_rvfpm.ck) //Wait for memory request from CPU
                    if (uin_xif.mem_valid && uin_xif.mem_req.id == issue_id) begin
                        @(posedge uin_rvfpm.ck)
                        uin_xif.mem_ready = 1;//Todo: add response (dbg etc)
                        uin_xif.mem_result_valid = 1;
                        uin_xif.mem_result.id = issue_id;//TODO: read as 8 in core
                        uin_xif.mem_result.rdata = data;
                        uin_xif.mem_result.err = 0;
                        uin_xif.mem_result.dbg = 0;
                        @(posedge uin_rvfpm.ck)
                        uin_xif.mem_result_valid = 0;
                        uin_xif.mem_ready = 0;
                        uin_xif.mem_result = {};
                        break;
                    end
                end
            end
        join_none
    endtask;

    task automatic doIssueInst(input logic[31:0] instruction = 0, input logic[X_ID_WIDTH-1:0] id = 0); //Issue instruction to coproc
        static semaphore s = new(1);
        while (!s.try_get) @(posedge uin_rvfpm.ck);
            uin_xif.issue_valid = 1;
            uin_xif.issue_req.instr = instruction;
            uin_xif.issue_req.id = id;
            //TODO: make this compliant- what is "not accepted"
            fork
                begin
                    @(uin_xif.issue_ready && uin_xif.issue_resp.accept)
                    @(posedge uin_rvfpm.ck)
                    uin_xif.issue_valid = 0;
                    uin_xif.issue_req ={};
                    nextId();
                    @(posedge uin_rvfpm.ck);
                    #0 s.put();
                    disable fork;
                end
                begin

                    #10000; //some timeout to release s. and raise some error
                    $error("Timeout on issue instruction");
                    uin_xif.issue_valid = 0;
                    uin_xif.issue_req ={};
                    nextId();
                    #0 s.put();
                    disable fork;
                end
            join_none

    endtask;


    task doMemRes(); //Issue memoryRequest response to coproc

    endtask;

    task doMemResp(); //Issue response to memResult

    endtask;



endprogram