/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model.
  Implemented in C++ (fpu_top) and interfaced using sv and DPI-C.
*/


`include "defines.svh"
`include "pa_rvfpm.sv"
import pa_rvfpm::*;
import in_xif::*;

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
  input  logic issue_valid,
  output logic issue_ready,
  input  x_issue_req_t issue_req,
  output x_issue_resp_t issue_resp,

  // Commit Interface
  input  logic commit_valid,
  input  x_commit_t commit,

  // Memory Eequest/Response Interface
  output logic mem_valid,
  input  logic mem_ready,
  output x_mem_req_t mem_req,
  input  x_mem_resp_t mem_resp,

  // Memory Result Interface
  input  logic mem_result_valid,
  input  x_mem_result_t mem_result,

  // Result Interface
  output logic result_valid,
  input  logic result_ready,
  output x_result_t result


);
  //-----------------------
  //-- DPI-C Imports
  //-----------------------
  import "DPI-C" function chandle create_fpu_model(input int pipelineStages, input int queueDepth, input int rfDepth);
  import "DPI-C" function void reset_fpu(input chandle fpu_ptr);
  import "DPI-C" function void clock_event(input chandle fpu_ptr);
  import "DPI-C" function void poll_ready(input chandle fpu_ptr, output logic fpu_ready);
  import "DPI-C" function void destroy_fpu(input chandle fpu_ptr);
  import "DPI-C" function int unsigned getRFContent(input chandle fpu_ptr, input int addr);
  import "DPI-C" function void add_accepted_instruction(input chandle fpu_ptr, input int instr, input int unsigned id, input int unsigned operand_a, input int unsigned operand_b, input int unsigned operand_c, input int unsigned mode, input logic commit_valid, input int unsigned commit_id, input logic commit_kill);
  import "DPI-C" function void reset_predecoder(input chandle fpu_ptr);
  import "DPI-C" function void predecode_instruction(input chandle fpu_ptr, input int instr, input int unsigned id, output x_issue_resp_t resp, output logic use_rs_a, output logic use_rs_b, output logic use_rs_c);
  import "DPI-C" function void commit_instruction(input chandle fpu_ptr, input int unsigned id, input logic kill);
  import "DPI-C" function void poll_mem_req(input chandle fpu_ptr, output logic mem_valid, output int unsigned id, output int unsigned addr, output int unsigned wdata, output logic last, output int unsigned size, output int unsigned mode);
  import "DPI-C" function void write_sv_state(input chandle fpu_ptr, input logic mem_ready, input logic mem_result_valid, input int unsigned id, input int unsigned rdata, input logic err, input logic dbg, input logic result_ready);
  import "DPI-C" function void poll_res(input chandle fpu_ptr, output logic result_valid, output int unsigned id, output int unsigned data, output int unsigned rd); //TODO: add remaining signals in interface


  //-----------------------
  //-- Local parameters
  //-----------------------
  logic fpu_ready_s; //status signal
  logic [X_NUM_RS-1:0] use_rs_i; //Which operands are used
  logic new_instruction_accepted; //Indicates that a new instruction is accepted
  x_issue_resp_t issue_resp_s; //To recieve issue response
  x_mem_result_t mem_res;
  //-----------------------
  //-- Initialization
  //-----------------------
  assign new_instruction_accepted = issue_valid && issue_ready && issue_resp.accept; //Signal that a new instruction is accepted
  chandle fpu;
  initial begin
    fpu = create_fpu_model(PIPELINE_STAGES, QUEUE_DEPTH, NUM_F_REGS);
  end

  assign fpu_ready = fpu_ready_s;

  // assign mem_req.mode = 3; //TODO: Set to 0 for now
  assign mem_req.we = 0; //TODO: Set to 0 for now
  // assign mem_req.size = 7; //TODO: Set to 7 for now
  assign mem_req.be = 'hF; //TODO: Set to F for now
  assign mem_req.attr = 0; //TODO: Set to 0 for now
  assign mem_req.spec = 1; //TODO: Set to 0 for now

  assign result.we = 0; //TODO: Set to 0 for now
  assign result.ecsdata = 0; //TODO: Set to 0 for now
  assign result.ecswe = 0;
  assign result.exc = 0; //TODO: Set to 0 for now
  assign result.exccode = 0; //TODO: Set to 0 for now
  assign result.err = 0; //TODO: Set to 0 for now
  assign result.dbg = 0; //TODO: Set to 0 for now

  //Need to switch byte order, first for the whole struct, then for each part. Only for incoming structs. Outgoing structs need to be passed part by part
  assign issue_resp= {<< {issue_resp_s}};
  assign mem_res = mem_result;
  logic[31:0] mem_id_full;
  assign mem_req.id = mem_id_full[X_ID_WIDTH-1:0];
  logic[31:0] mem_req_size_full;
  assign mem_req.size = mem_req_size_full[2:0];
  logic[31:0] mem_req_mode_full;
  assign mem_req.mode = mem_req_mode_full[1:0];

  logic[31:0] result_id_full;
  assign result.id = result_id_full[X_ID_WIDTH-1:0];
  logic[31:0] result_rd_full;
  assign result.rd = result_rd_full[4:0];

  always_ff @(posedge ck or negedge rst) begin: la_main
    if (!rst) begin
      $display("--- %t: Resetting FPU ---", $time);
      reset_fpu(fpu);
      fpu_ready_s <= 0;
    end
    else if (enable) begin //Call clocked functions
      // poll_ready(fpu, fpu_ready_s);
      write_sv_state(fpu, mem_ready, mem_result_valid, mem_res.id, mem_res.rdata, mem_res.err, mem_res.dbg, result_ready);
      clock_event(fpu);
      poll_ready(fpu, fpu_ready_s);

    end
  end


  always_latch begin
    issue_ready = fpu_ready_s;
    if (issue_valid && fpu_ready_s) begin
      predecode_instruction(fpu, issue_req.instr, issue_req.id, issue_resp_s, use_rs_i[0], use_rs_i[1], use_rs_i[2]);
    end else begin
      issue_resp_s = 0;
    end
    if (new_instruction_accepted) begin
      add_accepted_instruction(fpu, issue_req.instr, issue_req.id, issue_req.rs[0], issue_req.rs[1], issue_req.rs[2], issue_req.mode, commit_valid, commit.id, commit.commit_kill); //TODO: We want this to be done before the pipeline step to improve speed. Can it be done combinatorially?
      poll_mem_req(fpu, mem_valid, mem_id_full, mem_req.addr, mem_req.wdata, mem_req.last, mem_req_size_full, mem_req_mode_full);
      poll_res(fpu, result_valid, result_id_full, result.data, result_rd_full);
    end
    if (commit_valid) begin
      commit_instruction(fpu, commit.id,  commit.commit_kill);
    end
    poll_res(fpu, result_valid, result_id_full, result.data, result_rd_full);
    poll_mem_req(fpu, mem_valid, mem_id_full, mem_req.addr, mem_req.wdata, mem_req.last, mem_req_size_full, mem_req_mode_full);

  end


endmodule;
