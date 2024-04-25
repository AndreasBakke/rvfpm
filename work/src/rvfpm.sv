/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model.
  Implemented in C++ (fpu_top) and interfaced using sv and DPI-C.
*/


`include "config.svh"
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
  import "DPI-C" function void resolve_forwards(input chandle fpu_ptr);
  import "DPI-C" function int unsigned getRFContent(input chandle fpu_ptr, input int addr);
  import "DPI-C" function void add_accepted_instruction(input chandle fpu_ptr, input int instr, input int unsigned id, input int unsigned operand_a, input int unsigned operand_b, input int unsigned operand_c, input int unsigned mode, input logic commit_valid, input int unsigned commit_id, input logic commit_kill);
  import "DPI-C" function void reset_predecoder(input chandle fpu_ptr);
  import "DPI-C" function void predecode_instruction(input chandle fpu_ptr, input int instr, input int unsigned id, output logic accept, output logic loadstore, output logic writeback, output logic use_rs_a, output logic use_rs_b, output logic use_rs_c);
  import "DPI-C" function void commit_instruction(input chandle fpu_ptr, input int unsigned id, input logic kill);
  import "DPI-C" function void executeStep(input chandle fpu_ptr);
  import "DPI-C" function void poll_memory_request(input chandle fpu_ptr, output logic mem_valid, output int unsigned id, output int unsigned addr, output int unsigned wdata, output logic last, output int unsigned size, output int unsigned mode, output logic we);
  import "DPI-C" function void write_sv_state(input chandle fpu_ptr, input logic mem_ready, input logic result_ready);
  import "DPI-C" function void poll_res(input chandle fpu_ptr, output logic result_valid, output int unsigned id, output int unsigned data, output int unsigned rd, output logic we, output int unsigned ecswe, output int unsigned ecsdata); //TODO: add remaining signals in interface
  import "DPI-C" function void write_memory_result(input chandle fpu_ptr, input int unsigned id, input int unsigned rdata, input logic err, input logic dbg);
  import "DPI-C" function void writebackStep(input chandle fpu_ptr);
  import "DPI-C" function void memoryStep(input chandle fpu_ptr);
  import "DPI-C" function void reset_memory_request(input chandle fpu_ptr, input int unsigned id);
  import "DPI-C" function void reset_result(input chandle fpu_ptr, input int unsigned id);

  //-----------------------
  //-- Local parameters
  //-----------------------
  logic fpu_ready_s; //status signal
  logic [X_NUM_RS-1:0] use_rs_i; //Which operands are used
  x_mem_result_t mem_res;
  //-----------------------
  //-- Initialization
  //-----------------------
  chandle fpu;
  initial begin
    fpu = create_fpu_model(PIPELINE_STAGES, QUEUE_DEPTH, NUM_F_REGS);
  end

  assign fpu_ready = fpu_ready_s;

  logic accept_s, loadstore_s, writeback_s;
  assign issue_resp.accept = accept_s;
  assign issue_resp.writeback = writeback_s;
  assign issue_resp.dualwrite = 0;
  assign issue_resp.dualread = 0;
  assign issue_resp.loadstore = loadstore_s;
  assign issue_resp.ecswrite = 0;
  assign issue_resp.exc = 0;

  assign mem_req.be = 'hF;
  assign mem_req.attr = 0;
  assign mem_req.spec = 1;

  logic [31:0] result_ecswe_full, result_ecsdata_full;
  logic[31:0] result_id_full, result_rd_full;
  assign result.ecsdata = result_ecsdata_full[5:0];
  assign result.ecswe = result_ecswe_full[2:0];
  assign result.id = result_id_full[X_ID_WIDTH-1:0];
  assign result.rd = result_rd_full[4:0];
  assign result.exc = 0;
  assign result.exccode = 0;
  assign result.err = 0;
  assign result.dbg = 0;

  //Need to switch byte order, first for the whole struct, then for each part. Only for incoming structs. Outgoing structs need to be passed part by part
  // assign issue_resp= {<< {issue_resp_s}};
  assign mem_res = mem_result;
  logic[31:0] mem_id_full, mem_req_size_full, mem_req_mode_full;
  assign mem_req.id = mem_id_full[X_ID_WIDTH-1:0];
  assign mem_req.size = mem_req_size_full[2:0];
  assign mem_req.mode = mem_req_mode_full[1:0];

  `ifndef ZFINX
    logic [FLEN-1:0] registerFile[NUM_F_REGS]; //For verification
  `endif

  always_ff @(posedge ck or negedge rst) begin: la_main
    if (!rst) begin
      $display("--- %t: Resetting FPU ---", $time);
      reset_fpu(fpu);
      poll_ready(fpu, fpu_ready_s);
    end
    else if (enable) begin //Call clocked functions
      clock_event(fpu);
      poll_ready(fpu, fpu_ready_s);
      if (mem_valid && mem_ready) begin
        reset_memory_request(fpu, mem_req.id);
      end
      if (result_valid && result_ready) begin
        reset_result(fpu, result_id_full);
      end
    end
  end

  always_ff @(negedge ck) begin
    if (enable) begin
      write_sv_state(fpu, mem_ready, result_ready);
      if(accept_s) begin
        add_accepted_instruction(fpu, issue_req.instr, issue_req.id, issue_req.rs[0], issue_req.rs[1], issue_req.rs[2], issue_req.mode, commit_valid, commit.id, commit.commit_kill);
      end
      if (mem_result_valid) begin
        write_memory_result(fpu, mem_result.id, mem_result.rdata, mem_result.err, mem_result.dbg);
      end
      executeStep(fpu);
      memoryStep(fpu);
      writebackStep(fpu);
      if(FORWARDING == 1) begin
        resolve_forwards(fpu);
      end
      poll_memory_request(fpu, mem_valid, mem_id_full, mem_req.addr, mem_req.wdata, mem_req.last, mem_req_size_full, mem_req_mode_full, mem_req.we);
      poll_res(fpu, result_valid, result_id_full, result.data, result_rd_full, result.we, result_ecswe_full, result_ecsdata_full);
    end
  end

  always_latch begin
    issue_ready = fpu_ready_s;
    if (issue_valid && fpu_ready_s) begin
      predecode_instruction(fpu, issue_req.instr, issue_req.id, accept_s, loadstore_s, writeback_s, use_rs_i[0], use_rs_i[1], use_rs_i[2]);
    end else begin
      accept_s = 0;
      loadstore_s = 0;
      writeback_s = 0;
    end

    if (commit_valid) begin
      commit_instruction(fpu, commit.id,  commit.commit_kill); //ibex tries to commit before accepted
    end

    write_sv_state(fpu, mem_ready, result_ready);
  end

  always @(posedge ck) begin
    `ifndef ZFINX
    for (int i = 0; i < NUM_F_REGS; ++i) begin
      assign registerFile[i] = getRFContent(fpu, i); // Use assign to connect wire to array index
    end
  `endif
  end

endmodule;

