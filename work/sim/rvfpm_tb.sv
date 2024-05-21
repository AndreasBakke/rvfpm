/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  rvfpm testbench.
  Prerequisites: Compile c++ model using make sharedLib (macSL not tested/verified)
  Then run vsim using -sv_lib bin/lib_rvfpm -novopt work.rvfpm_tb -suppress 12110
*/
`timescale 1ns/1ps
`include "../src/config.svh"
import in_xif::*;
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
  parameter TB_X_RFR_WIDTH       = `XLEN; //Read acces width
  parameter TB_X_RFW_WIDTH       = `XLEN; //Write acces width
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
    .rst(!uin_rvfpm.rst),
    .enable(uin_rvfpm.enable),
    .fpu_ready(uin_rvfpm.fpu_ready),
    // Issue Interface
    .issue_valid      (uin_rvfpm.issue_valid),
    .issue_ready      (uin_rvfpm.issue_ready),
    .issue_req        (uin_rvfpm.issue_req),
    .issue_resp       (uin_rvfpm.issue_resp),

    // Commit Interface
    .commit_valid     (uin_rvfpm.commit_valid),
    .commit           (uin_rvfpm.commit),

    // Memory Request/Response Interface
    .mem_valid        (uin_rvfpm.mem_valid),
    .mem_ready        (uin_rvfpm.mem_ready),
    .mem_req          (uin_rvfpm.mem_req),
    .mem_resp         (uin_rvfpm.mem_resp),

    // Memory Result Interface
    .mem_result_valid (uin_rvfpm.mem_result_valid),
    .mem_result       (uin_rvfpm.mem_result),

    // Result Interface
    .result_valid     (uin_rvfpm.result_valid),
    .result_ready     (uin_rvfpm.result_ready),
    .result           (uin_rvfpm.result)
  );
  import "DPI-C" function int unsigned getRFContent(input chandle fpu_ptr, input int addr);
  import "DPI-C" function int unsigned getPipeStageId(input chandle fpu_ptr, input int stage);
  import "DPI-C" function int unsigned getQueueStageId(input chandle fpu_ptr, input int stage);
  import "DPI-C" function int unsigned getWaitingOpId(input chandle fpu_ptr);

  always @(posedge uin_rvfpm.ck or negedge uin_rvfpm.ck) begin
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
      uin_rvfpm.waitingOpId = getWaitingOpId(dut.fpu);
    `endif


    uin_rvfpm.result_ready = 1;
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
    .uin_rvfpm(uin_rvfpm)
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
    $display("------------------------------------");
    $display("------------------------------------");
    $display("");
    $display("    Simulation finished, errors: %0d", errorCnt);
    $display("    Total time: %t", $time);
    $display("");
    $display("------------------------------------");
    $display("------------------------------------");
  endfunction

endmodule