/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  rvfpm testbench.
  Prerequisites: Compile c++ model using make sharedLib (macSL not tested/verified)
  Then run vsim using -sv_lib bin/lib_rvfpm -novopt work.rvfpm_tb -suppress 12110
*/
`timescale 1ns/1ps
`include "../src/defines.svh"
module rvfpm_tb;
  //-----------------------
  //-- Parameters
  //-----------------------
  //System parameters
  parameter TB_NUM_F_REGS        = `NUM_F_REGS;
  parameter TB_XLEN              = `XLEN;
  parameter TB_FLEN              = `FLEN;
  //System parameters

  //Pipeline parameters
  parameter TB_PIPELINE_STAGES   = `NUM_PIPELINE_STAGES;
  parameter TB_QUEUE_DEPTH       = `QUEUE_DEPTH; //Size of operation queue
  parameter TB_FORWARDING        = `FORWARDING; //Set to 1 to enable forwarding; not implemented
  parameter TB_OUT_OF_ORDER      = `OOO; //Set to 1 to enable out of order execution; not implemented

  //CORE-V-XIF parameters for coprocessor
  parameter TB_X_NUM_RS          = `X_NUM_RS; //Read ports
  parameter TB_X_ID_WIDTH        = `X_ID_WIDTH;
  parameter TB_X_MEM_WIDTH       = `FLEN; //TODO: dependent on extension
  parameter TB_X_RFR_WIDTH       = `FLEN; //Read acces width
  parameter TB_X_RFW_WIDTH       = `FLEN; //Write acces width
  parameter TB_X_MISA            = `X_MISA; //TODO: not used
  parameter TB_X_ECS_XS          = `X_ECS_XS;        //TODO: not used

  //Clock
  localparam time ck_period = 40ns;

  //-----------------------
  //-- Declarations
  //-----------------------
  int errorCnt; //errorCount
  //Test interface
  inTest_rvfpm #(
    .X_ID_WIDTH(TB_X_ID_WIDTH),
    .NUM_F_REGS(TB_NUM_F_REGS),
    .PIPELINE_STAGES(TB_PIPELINE_STAGES),
    .QUEUE_DEPTH(TB_QUEUE_DEPTH),
    .XLEN(TB_XLEN)
  ) uin_rvfpm ();


  //eXtension interface
  in_xif #(
    .XLEN(TB_XLEN),
    .FLEN(TB_FLEN),
    .X_NUM_RS(TB_X_NUM_RS),
    .X_ID_WIDTH(TB_X_ID_WIDTH),
    .X_MEM_WIDTH(TB_X_MEM_WIDTH),
    .X_RFR_WIDTH(TB_X_RFR_WIDTH),
    .X_RFW_WIDTH(TB_X_RFW_WIDTH),
    .X_MISA(TB_X_MISA),
    .X_ECS_XS(TB_X_ECS_XS)
  ) uin_xif ();
  //-----------------------
  //-- Clk gen
  //-----------------------
  initial begin
    uin_rvfpm.ck=0;
    forever begin
      #(ck_period/2);
      uin_rvfpm.ck=!uin_rvfpm.ck;
    end
  end

  //-----------------------
  //-- DUT
  //-----------------------

  rvfpm #(
    .FORWARDING(TB_FORWARDING),
    .OUT_OF_ORDER(TB_OUT_OF_ORDER),
    .NUM_F_REGS(TB_NUM_F_REGS),
    .PIPELINE_STAGES(TB_PIPELINE_STAGES),
    .QUEUE_DEPTH(TB_QUEUE_DEPTH),
    .X_ID_WIDTH(TB_X_ID_WIDTH),
    .XLEN(TB_XLEN)
  ) dut (
    .ck(uin_rvfpm.ck),
    .rst(uin_rvfpm.rst),
    .enable(uin_rvfpm.enable),
    .fpu_ready(uin_rvfpm.fpu_ready),
    .xif_issue_if(uin_xif.coproc_issue),
    .xif_commit_if(uin_xif.coproc_commit),
    .xif_mem_if(uin_xif.coproc_mem),
    .xif_mem_result_if(uin_xif.coproc_mem_result),
    .xif_result_if(uin_xif.coproc_result)
  );
  import "DPI-C" function int unsigned getRFContent(input chandle fpu_ptr, input int addr);
  import "DPI-C" function int unsigned getPipeStageId(input chandle fpu_ptr, input int stage);
  import "DPI-C" function int unsigned getQueueStageId(input chandle fpu_ptr, input int stage);

  always @(posedge uin_rvfpm.ck) begin
    //Get entire rf for verification`
    `ifndef ZFINX
    for (int i=0; i < TB_NUM_F_REGS; ++i) begin
      uin_rvfpm.registerFile[i] = getRFContent(dut.fpu, i);
    end
    `endif

    //Get entire pipeline for verification
    `ifdef INCLUDE_PIPELINE
      for (int i=0; i < TB_PIPELINE_STAGES; ++i) begin
        uin_rvfpm.pipelineIds[i] = getPipeStageId(dut.fpu, i);
      end
    `endif
    `ifdef INCLUDE_QUEUE
      //Get entire queue for verification
      for (int i=0; i < TB_QUEUE_DEPTH; ++i) begin
        uin_rvfpm.queueIds[i] = getQueueStageId(dut.fpu, i);
      end
    `endif


    if (uin_rvfpm.speculative_ids.size() > 0) begin
      uin_xif.commit_valid = 1;
      uin_xif.commit.id = uin_rvfpm.speculative_ids.pop_front();
      uin_xif.commit.commit_kill = 0;
      @(posedge uin_rvfpm.ck);
      uin_xif.commit_valid = 0;
      uin_xif.commit = {};
    end

    uin_xif.result_ready = 0;
    if (uin_xif.result_valid) begin
      uin_xif.result_ready = 1;
    end
  end

  //-----------------------
  //-- Assertions
  //-----------------------
  assertions_rvfpm #(
    .NUM_F_REGS(TB_NUM_F_REGS),
    .PIPELINE_STAGES(TB_PIPELINE_STAGES)
  ) u_assertions_rvfpm (
    .uin_rvfpm(uin_rvfpm)
  );

  //-----------------------
  //-- Test Program
  //-----------------------
  testPr_rvfpm #(
    .NUM_F_REGS(TB_NUM_F_REGS),
    .PIPELINE_STAGES(TB_PIPELINE_STAGES),
    .X_ID_WIDTH(TB_X_ID_WIDTH)
  ) u_testPr(
    .uin_rvfpm(uin_rvfpm),
    .uin_xif(uin_xif)
  );

  //-----------------------
  //-- Result
  //-----------------------
  assign errorCnt = uin_rvfpm.errorCntAssertions;

  final begin
    printResult();
  end

  function void printResult;
    $display("");
    $display("");
    $display($time, "ns: ");
    $display("------------------------------------");
    $display("------------------------------------");
    $display("");
    $display("Simulation finished, errors: $0d", errorCnt);
    $display("");
    $display("------------------------------------");
    $display("------------------------------------");
  endfunction

endmodule