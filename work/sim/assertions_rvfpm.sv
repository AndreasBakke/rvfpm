
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
//-- Helper functions
//-----------------------
let getRow(r, addr) = r[addr]; //To pass register and address separatly (for using $past)


//-----------------------
//-- Pipeline
//-----------------------

//property prop_pipelineStep;
//    @(posedge ck) 
//    disable iff (rst || uin_rvfpm.enable) 
//    (for (int i=0; i<PIPELINE_STAGES; ++i) begin
//        
//    end);
//endproperty;

//-----------------------
//-- Operations
//-----------------------
// FMV.X.W
sequence seq_FMV_X_W_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b1110000 ) && (uin_rvfpm.instruction[14:12] == 0);
endsequence

sequence seq_FMV_X_W_out;
    ##(PIPELINE_STAGES+1) (uin_rvfpm.data_toXReg) === uin_rvfpm.registerFile[$past(uin_rvfpm.instruction[19:15], PIPELINE_STAGES+1)];
endsequence

property prop_FMV_X_W;
    @(negedge ck)
    disable iff (rst || !uin_rvfpm.enable) seq_FMV_X_W_start |-> seq_FMV_X_W_out;
endproperty


as_rvfpm_FMV_X_W: assert property (prop_FMV_X_W)
    else begin $error("Data to XReg not matching data in registerFile"); errorCnt++; end;
// FMV.X.W
sequence seq_FMV_W_X_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b1111000 ) && (uin_rvfpm.instruction[14:12] == 0);
endsequence

sequence seq_FMV_W_X_out;
    logic[31:0] instr = uin_rvfpm.instruction;
    ##(PIPELINE_STAGES+1) uin_rvfpm.registerFile[instr[11:7]] == $past(uin_rvfpm.data_fromXReg);
endsequence

property prop_FMV_W_X;
    @(negedge ck)
    disable iff (rst || !uin_rvfpm.enable) seq_FMV_W_X_start |-> seq_FMV_W_X_out;
endproperty


as_rvfpm_FMV_W_X: assert property (prop_FMV_W_X)
    else begin $error("Data from XReg not matching data in registerFile"); errorCnt++; end;

//FMIN/FMAX
let getMin(a,b) = (a<=b) ? a : b;
let getMax(a,b) = (a>b) ? a : b;

sequence seq_FMIN_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b0010100 ) && (uin_rvfpm.instruction[14:12] == 0);
endsequence

sequence seq_FMIN_res;
    logic[31:0] instr = uin_rvfpm.instruction;
    ##(PIPELINE_STAGES+1) uin_rvfpm.registerFile[instr[11:7]] === getMin(getRow($past(uin_rvfpm.registerFile), instr[19:15]), getRow($past(uin_rvfpm.registerFile), instr[24:20]));
endsequence

property prop_FMIN;
    @(negedge ck)
    disable iff(rst || !uin_rvfpm.enable) seq_FMIN_start |-> seq_FMIN_res;
endproperty

as_rvfpm_FMIN: assert property (prop_FMIN)
    else begin $error("Excpected result for FMIN not matching registerFile"); errorCnt++;end;

sequence seq_FMAX_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b0010100 ) && (uin_rvfpm.instruction[14:12] == 3'b001);
endsequence

sequence seq_FMAX_res;
    logic[31:0] instr = uin_rvfpm.instruction;
    ##(PIPELINE_STAGES+1) uin_rvfpm.registerFile[instr[11:7]] === getMax(getRow($past(uin_rvfpm.registerFile), instr[19:15]), getRow($past(uin_rvfpm.registerFile), instr[24:20]));
endsequence

property prop_FMAX;
    @(negedge ck)
    disable iff(rst || !uin_rvfpm.enable) seq_FMAX_start |-> seq_FMAX_res;
endproperty

as_rvfpm_FMAX: assert property (prop_FMAX)
    else begin $error("Excpected result for FMAX not matching registerFile"); errorCnt++;end;


//FCLASS
sequence seq_FCLASS_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b1110000 ) && (uin_rvfpm.instruction[14:12] == 3'b001);
endsequence


sequence seq_FCLASS_numbits;
    ##(PIPELINE_STAGES+1) $countones(uin_rvfpm.data_toXReg) == 1; //Verify that exactly one bit is set
endsequence

property prop_FCLASS_numbits;
    @(negedge ck)
    disable iff(rst || !uin_rvfpm.enable) seq_FCLASS_start |-> seq_FCLASS_numbits;
endproperty

as_rvfpm_CLASS_numbits: assert property (prop_FCLASS_numbits)
    else begin $error("More than one, or zero bits set during FCLASS"); errorCnt++;end;

    //How to test for the correct bit set?

//FSGNJ
sequence seq_FSGNJ_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b0010000 ) && (uin_rvfpm.instruction[14:12] == 3'b000);
endsequence

sequence seq_FSGNJ_res;
    logic[31:0] instr = uin_rvfpm.instruction; //Save executed instruction.
    ##(PIPELINE_STAGES+1) (uin_rvfpm.registerFile[instr[11:7]][30:0] === getRow($past(uin_rvfpm.registerFile), instr[19:15])[30:0]) && (uin_rvfpm.registerFile[instr[11:7]][31] === getRow($past(uin_rvfpm.registerFile),instr[24:20])[31]);
endsequence

property prop_FSGNJ;
    @(negedge ck)
    disable iff(rst || !uin_rvfpm.enable) seq_FSGNJ_start |-> seq_FSGNJ_res;
endproperty

as_rvfpm_FSGNJ: assert property (prop_FSGNJ)
    else begin $error("Register value not matching expected result for FSGNJ."); errorCnt++;end;



//FSGNJN
sequence seq_FSGNJN_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b0010000 ) && (uin_rvfpm.instruction[14:12] == 3'b001);
endsequence

sequence seq_FSGNJN_res;
    logic[31:0] instr = uin_rvfpm.instruction; //Save executed instruction.
    ##(PIPELINE_STAGES+1) (uin_rvfpm.registerFile[instr[11:7]][30:0] === getRow($past(uin_rvfpm.registerFile), instr[19:15])[30:0]) && (uin_rvfpm.registerFile[instr[11:7]][31] === !getRow($past(uin_rvfpm.registerFile),instr[24:20])[31]);
endsequence

property prop_FSGNJN;
    @(negedge ck)
    disable iff(rst || !uin_rvfpm.enable) seq_FSGNJN_start |-> seq_FSGNJN_res;
endproperty

as_rvfpm_FSGNJN: assert property (prop_FSGNJN)
    else begin $error("Register value not matching expected result for FSGNJN."); errorCnt++; end;


//FSGNJX
sequence seq_FSGNJX_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b0010000 ) && (uin_rvfpm.instruction[14:12] == 3'b010);
endsequence

sequence seq_FSGNJX_res;
    logic[31:0] instr = uin_rvfpm.instruction; //Save executed instruction.
    ##(PIPELINE_STAGES+1) (uin_rvfpm.registerFile[instr[11:7]][30:0] === getRow($past(uin_rvfpm.registerFile), instr[19:15])[30:0]) && (uin_rvfpm.registerFile[instr[11:7]][31] === (getRow($past(uin_rvfpm.registerFile), instr[19:15])[31] ^ getRow($past(uin_rvfpm.registerFile),instr[24:20])[31]));
endsequence

property prop_FSGNJX;
    @(negedge ck)
    disable iff(rst || !uin_rvfpm.enable) seq_FSGNJX_start |-> seq_FSGNJX_res;
endproperty

as_rvfpm_FSGNJX: assert property (prop_FSGNJX)
    else begin $error("Register value not matching expected result for FSGNJX."); errorCnt++;end;


// // FSUB
// sequence seq_FSUB_start;
//     (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b0000100 );
// endsequence

// sequence seq_FSUB_res;
//     logic[31:0] instr = uin_rvfpm.instruction; //Save executed instruction.
//     ##(PIPELINE_STAGES+1) uin_rvfpm.registerFile[instr[11:7]]
// endsequence


//-----------------------
//-- fcsr
//-----------------------


//-----------------------
//-- Reset
//-----------------------
//Empty pipeline

// property prop_rst_registers;
//     @(posedge ck)
//     rst |=> uin_rvfpm.pipelineIds[0] == 0;
// endproperty

// as_rvfpm_rst: assert property(prop_rst_registers)
//     else begin $error("Reset not reset"); errorCnt++;end
endmodule