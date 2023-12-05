
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

sequence seq_FMV_X_W_start;
    (uin_rvfpm.instruction[6:0] == 7'b1010011) && (uin_rvfpm.instruction[31:25] == 7'b1110000 ) && (uin_rvfpm.instruction[14:12] == 0);
endsequence

sequence seq_FMV_X_W_out;
    ##PIPELINE_STAGES $bitstoshortreal(uin_rvfpm.data_toXReg) === uin_rvfpm.registerFile[$past(uin_rvfpm.instruction[19:15])];
endsequence

property prop_FMV_X_W;
    @(posedge ck)
    disable iff (rst || !uin_rvfpm.enable) seq_FMV_X_W_start |-> seq_FMV_X_W_out;
endproperty

property testprop;
	@(posedge ck)
	0;
endproperty

as_rvfpm_FMV_X_W: assert property (prop_FMV_X_W)
    else begin $error("Data to XReg not matching data in registerFile"); errorCnt++; 
    $display($bitstoshortreal(uin_rvfpm.data_toXReg));
    $display(uin_rvfpm.registerFile[$past(uin_rvfpm.instruction[19:15])]);
    $display($bitstoshortreal(uin_rvfpm.data_toXReg) === uin_rvfpm.registerFile[$past(uin_rvfpm.instruction[19:15])]);



    // $error("Got: %s, but expected, %s", (string($bitstoreal(uin_rvfpm.data_toXReg)), string(uin_rvfpm.registerFile[$past(uin_rvfpm.instruction[24:20])])))
    end;



//-----------------------
//-- fcsr
//-----------------------


//-----------------------
//-- Reset
//-----------------------


endmodule