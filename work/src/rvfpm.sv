/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model.
  Implemented in C++ (fpu_top) and interfaced using sv and DPI-C.
  Number of pipeline stages are parameterized. Note: Execute (and dependency on data_fromMem and data_fromXreg) is last stage in pipeline
  Further work: fully parameterized pipeline. More fp types. Standardized interface (CORE-V-XIF)
*/


`include "defines.svh"
`include "pa_rvfpm.sv"
import pa_rvfpm::*;


module rvfpm #(
  parameter NUM_F_REGS        = `NUM_F_REGS,
  parameter XLEN              = `XLEN,
  //System parameters

  //Pipeline parameters
  parameter PIPELINE_STAGES   = `NUM_PIPELINE_STAGES,
  parameter QUEUE_DEPTH       = `QUEUE_DEPTH, //Size of operation queue
  parameter FORWARDING        = `FORWARDING, //Set to 1 to enable forwarding, not implemented
  parameter OUT_OF_ORDER      = `OOO, //Set to 1 to enable out of order execution, not implemented

  //CORE-V-XIF parameters for coprocessor
  parameter X_NUM_RS          = `X_NUM_RS, //Read ports
  parameter X_ID_WIDTH        = `X_ID_WIDTH,
  parameter X_MEM_WIDTH       = `FLEN, //TODO: dependent on extension
  parameter X_RFR_WIDTH       = `FLEN, //Read acces width
  parameter X_RFW_WIDTH       = `FLEN, //Write acces width
  parameter X_MISA            = `X_MISA, //TODO: not used
  parameter X_ECS_XS          = `X_ECS_XS        //TODO: not used
)

(
  input logic ck,
  input logic rst,
  input logic enable,
  output logic fpu_ready,

  //eXtension interface for coprocessor
  in_xif.coproc_issue xif_issue_if,
  in_xif.coproc_commit xif_commit_if,
  in_xif.coproc_mem  xif_mem_if,
  in_xif.coproc_mem_result xif_mem_result_if,
  in_xif.coproc_result xif_result_if

);
  //-----------------------
  //-- DPI-C Imports
  //-----------------------
  import "DPI-C" function chandle create_fpu_model(input int pipelineStages, input int queueDepth, input int rfDepth);
  import "DPI-C" function void reset_fpu(input chandle fpu_ptr);
  import "DPI-C" function void clock_event(input chandle fpu_ptr, output logic fpu_ready);
  import "DPI-C" function void destroy_fpu(input chandle fpu_ptr);
  import "DPI-C" function int unsigned getRFContent(input chandle fpu_ptr, input int addr);
  import "DPI-C" function void add_accepted_instruction(input chandle fpu_ptr, input int instr, input int unsigned id, input int unsigned operand_a, input int unsigned operand_b, input int unsigned operand_c);
  import "DPI-C" function void reset_predecoder(input chandle fpu_ptr);
  import "DPI-C" function void poll_predecoder_result(input chandle fpu_ptr, output x_issue_resp_t resp, output logic use_rs_a, output logic use_rs_b, output logic use_rs_c);
  import "DPI-C" function void predecode_instruction(input chandle fpu_ptr, input int instr, input int unsigned id);
  import "DPI-C" function void commit_instruction(input chandle fpu_ptr, input int unsigned id, input logic kill);
  import "DPI-C" function void poll_mem_req(input chandle fpu_ptr, output logic mem_valid, output int unsigned id, output int unsigned addr, output int unsigned wdata);
  import "DPI-C" function void write_sv_state(input chandle fpu_ptr, input logic mem_ready, input logic mem_result_valid, input int unsigned id, input int unsigned rdata, input logic err, input logic dbg, input logic result_ready);
  import "DPI-C" function void poll_res(input chandle fpu_ptr, output logic result_valid, output int unsigned id, output int unsigned data, output int unsigned rd); //TODO: add remaining signals in interface


  //-----------------------
  //-- Local parameters
  //-----------------------
  logic fpu_ready_s; //status signal
  logic [X_NUM_RS-1:0] use_rs_i; //Which operands are used
  logic new_instruction_accepted; //Indicates that a new instruction is accepted
  x_issue_resp_t issue_resp; //To recieve issue response
  x_mem_result_t mem_res;
  //-----------------------
  //-- Initialization
  //-----------------------
  assign new_instruction_accepted = xif_issue_if.issue_valid && xif_issue_if.issue_ready && xif_issue_if.issue_resp.accept; //Signal that a new instruction is accepted
  chandle fpu;
  initial begin
    fpu = create_fpu_model(PIPELINE_STAGES, QUEUE_DEPTH, NUM_F_REGS);
  end

  assign fpu_ready = fpu_ready_s;
  assign xif_mem_if.mem_req.mode = 0; //TODO: Set to 0 for now
  assign xif_mem_if.mem_req.we = 0; //TODO: Set to 0 for now
  assign xif_mem_if.mem_req.size = 0; //TODO: Set to 0 for now
  assign xif_mem_if.mem_req.be = 0; //TODO: Set to 0 for now
  assign xif_mem_if.mem_req.attr = 0; //TODO: Set to 0 for now
  assign xif_mem_if.mem_req.last = 1; //TODO: Set to 1 for now
  assign xif_mem_if.mem_req.spec = 0; //TODO: Set to 0 for now

  //Need to switch byte order, first for the whole struct, then for each part. Only for incoming structs. Outgoing structs need to be passed part by part
  assign xif_issue_if.issue_resp = {<< {issue_resp}};
  assign mem_res = xif_mem_result_if.mem_result;


  always_ff @(posedge ck) begin: la_main
    if (rst) begin
      $display("--- %t: Resetting FPU ---", $time);
      reset_fpu(fpu);
      fpu_ready_s <= 0;
      //Reset the rest of the signals aswell

    end
    else if (enable) begin
      //Call clocked functions

      write_sv_state(fpu, xif_mem_if.mem_ready, xif_mem_result_if.mem_result_valid, mem_res.id, mem_res.rdata, mem_res.err, mem_res.dbg, xif_result_if.result_ready);
      clock_event(fpu, fpu_ready_s);
      poll_predecoder_result(fpu, issue_resp, use_rs_i[0], use_rs_i[1], use_rs_i[2]);
      poll_mem_req(fpu, xif_mem_if.mem_valid, xif_mem_if.mem_req.id, xif_mem_if.mem_req.addr, xif_mem_if.mem_req.wdata); //TODO: should this be polled more often to more closely resemble internal signals?
      poll_res(fpu, xif_result_if.result_valid, xif_result_if.result.id, xif_result_if.result.data, xif_result_if.result.rd); //TODO: add remaining signals in interface


      if (xif_issue_if.issue_valid) begin
        xif_issue_if.issue_ready = fpu_ready_s
        & ((use_rs_i[0] & xif_issue_if.issue_req.rs_valid[0]) | !use_rs_i[0])
        & ((use_rs_i[1] & xif_issue_if.issue_req.rs_valid[1]) | !use_rs_i[1])
        & ((use_rs_i[2] & xif_issue_if.issue_req.rs_valid[2]) | !use_rs_i[2]) ; //TODO: and if RS_valid is true for all used operands (find out in predecoder) (Could predictivly do it in case of ZFINX aswell)
      end else begin
        xif_issue_if.issue_ready = 0;
      end
      if (new_instruction_accepted && fpu_ready_s) begin
        add_accepted_instruction(fpu, xif_issue_if.issue_req.instr, xif_issue_if.issue_req.id, xif_issue_if.issue_req.rs[0], xif_issue_if.issue_req.rs[1], xif_issue_if.issue_req.rs[2]);
        reset_predecoder(fpu); //or something
      end

    end
  end


  always_comb begin
    if (xif_issue_if.issue_valid && fpu_ready_s) begin
      predecode_instruction(fpu, xif_issue_if.issue_req.instr, xif_issue_if.issue_req.id); //TODO: add issue_transaction_active?
    end
    if (xif_commit_if.commit_valid) begin
      commit_instruction(fpu, xif_commit_if.commit.id,  xif_commit_if.commit.commit_kill);
    end
  end


endmodule