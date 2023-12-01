//TODO: Use opcode and stuff like this to detect what kind of instructions, and enforce the best assertion.
    uin_rvfpm.instruction[31:20] = 0; //imm
    uin_rvfpm.instruction[19:15] = 0; //rs1 (base)
    uin_rvfpm.instruction[14:12] = 0; //W
    uin_rvfpm.instruction[11:7] = 0;  //rd (dest)
    uin_rvfpm.instruction[6:0] = 7'b0000111;  //OPCODE


//And use the parameters for when to check (like in testPR writing to fromMem)

/*  rvfpm - 2023
    Andreas S. Bakke
    
    Description:
    Assertions for rvfpm testPr
*/

module assertions_rvfpm #(
    parameter int NUM_REGS,
    parameter int PIPELINE_STAGES
)(
    inTest_rvfpm uin_rvfpm
);

//-----------------------
//-- Local error counter and signals
//-----------------------
int errorCnt = 0;
assign uin_rvfpm.errorCntAssertions = errorCnt;
logic ck;
logic rst;
assign ck = uin_rvfpm.ck;
assign rst = uin_rvfpm.rst;



//-----------------------
//-- Pipeline
//-----------------------

property prop_pipelineStep;
    @(posedge ck) 
    disable iff (rst || uin_rvfpm.enable) 
    (for (int i=0; i<PIPELINE_STAGES; ++i) begin
        
    end);
endproperty;

//-----------------------
//-- Operations
//-----------------------

uin_rvfpm.instruction[14:12] = 3'b010; //rm (W)
    uin_rvfpm.instruction[11:7] = offset;  //rd
    uin_rvfpm.instruction[6:0] = 7'b0100111;  //OPCODE
sequence seq_FMV_X_W_start
    uin_rvfpm.instruction[6:0] == 7'b0100111 && uin_rvfpm.instruction[14:12] == 3'b010
endsequence

sequence seq_FMV_X_W_out
    ##PIPELINE_STAGES uin_rvfpm.data_toXReg == uin_rvfpm.registerFile[uin_rvfpm.instruction[24:20]]
endsequence

property prop_FMV_X_W;
    @(posedge ck)
    disable iff (rst || uin_rvfpm.enable) seq_FMV_X_W_start |-> seq_FMV_X_W_out;
endproperty

as_rvfpm_FMV_X_W: assert property (prop_FMV_X_W)
    else begin $error("Data to XReg not matching data in registerFile"); errorCnt++; end;



//-----------------------
//-- fcsr
//-----------------------


//-----------------------
//-- Reset
//-----------------------