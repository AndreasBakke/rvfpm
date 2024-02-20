/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model.
  Implemented in C++ (fpu_top) and interfaced using sv and DPI-C.
  Number of pipeline stages are parameterized. Note: Execute (and dependency on data_fromMem and data_fromXreg) is last stage in pipeline
  Further work: fully parameterized pipeline. More fp types. Standardized interface (CORE-V-XIF)
*/


`define NUM_FPU_REGS 32
`include "pa_rvfpm.sv"
import pa_rvfpm::*;


module rvfpm #(
  parameter NUM_REGS          = pa_rvfpm::NUM_REGS,
  parameter XLEN              = pa_rvfpm::XLEN,
  //System parameters
  //Pipeline parameters
  parameter PIPELINE_STAGES   = 4,
  parameter QUEUE_DEPTH       = 0, //Size of operation queue
  parameter FORWARDING        = 0, //Set to 1 to enable forwarding, not implemented
  parameter OUT_OF_ORDER      = 0, //Set to 1 to enable out of order execution, not implemented

  //CORE-V-XIF parameters for coprocessor
  parameter X_NUM_RS          = pa_rvfpm::X_NUM_RS, //Read ports //TODO: not used
  parameter X_ID_WIDTH        = pa_rvfpm::X_ID_WIDTH,
  parameter X_MEM_WIDTH       = pa_rvfpm::FLEN, //TODO: dependent on extension
  parameter X_RFR_WIDTH       = pa_rvfpm::FLEN, //Read acces width //TODO: not used
  parameter X_RFW_WIDTH       = pa_rvfpm::FLEN, //Write acces width //TODO: not used
  parameter X_MISA            = pa_rvfpm::X_MISA, //TODO: not used
  parameter X_ECS_XS          = pa_rvfpm::X_ECS_XS,        //TODO: not used
  parameter X_DUALREAD        = pa_rvfpm::X_DUALREAD, //TODO: not implemented
  parameter X_DUALWRITE       = pa_rvfpm::X_DUALWRITE //TODO: not implemented
)

(
  input logic ck,
  input logic rst,
  input logic enable,
  output logic fpu_ready, //0 if not accepting instructions

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
  import "DPI-C" function void poll_predecoder_result(input chandle fpu_ptr, output x_issue_resp_t resp);
  import "DPI-C" function void predecode_instruction(input chandle fpu_ptr, input int instr, input int unsigned id);
  import "DPI-C" function void poll_mem_req(input chandle fpu_ptr, output logic mem_valid, output int unsigned id, output int unsigned addr, output int unsigned wdata);
  import "DPI-C" function void write_mem_res(input chandle fpu_ptr, input logic mem_result_valid, input int unsigned id, input int unsigned rdata, input logic err, input logic dbg);

  //Something to issue response from predecoder
  //-----------------------
  //-- Local parameters
  //-----------------------
  logic fpu_ready_s; //status signal
  logic [X_ID_WIDTH-1:0] fpu_accept_id; //ID of accepted instruction
  logic fpu_accept; //Acceptance signal
  logic new_instruction_accepted; //Indicates that a new instruction is accepted
  logic issue_transaction_active; //Indicates that an issue transaction is active
  x_issue_resp_t issue_resp; //To recieve issue response
  x_issue_resp_t issue_resp_r; //reversed bit order
  x_mem_result_t mem_res;
  //-----------------------
  //-- Initialization
  //-----------------------
  assign new_instruction_accepted = xif_issue_if.issue_valid && xif_issue_if.issue_ready && xif_issue_if.issue_resp.accept; //Signal that a new instruction is accepted
  chandle fpu;
  initial begin
    fpu = create_fpu_model(PIPELINE_STAGES, QUEUE_DEPTH, NUM_REGS);
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
      reset_fpu(fpu);
    end
    else if (enable) begin
      //Call clocked functions
      clock_event(fpu, fpu_ready_s);
      poll_predecoder_result(fpu, issue_resp);
      poll_mem_req(fpu, xif_mem_if.mem_valid, xif_mem_if.mem_req.id, xif_mem_if.mem_req.addr, xif_mem_if.mem_req.wdata);
      write_mem_res(fpu, xif_mem_result_if.mem_result_valid, mem_res.id, mem_res.rdata, mem_res.err, mem_res.dbg);

      // add_accepted_instruction(fpu, instruction, id); //This can be async! Handle in predecoder when accepted

      if (xif_issue_if.issue_valid) begin
        xif_issue_if.issue_ready = fpu_ready_s ; //TODO: and if RS_valid is true for all used operands (find out in predecoder) (Could predictivly do it in case of ZFINX aswell)
      end else begin
        xif_issue_if.issue_ready = 0;
        //fpu_operation(fpu, instruction, id, data_fromXReg, data_fromMem, id_out, data_toMem, data_toXReg, pipelineFull, toMem_valid, toXReg_valid);
      end

      if (issue_transaction_active && new_instruction_accepted) begin
        add_accepted_instruction(fpu, xif_issue_if.issue_req.instr, xif_issue_if.issue_req.id, xif_issue_if.issue_req.rs[0], xif_issue_if.issue_req.rs[1], xif_issue_if.issue_req.rs[2]);

        //TODO: Add operands from register-interace
        //Reset predecodercontent
      end
    end
  end


  always_comb begin
    assign issue_transaction_active = xif_issue_if.issue_valid && xif_issue_if.issue_ready;
    if (issue_transaction_active) begin
      predecode_instruction(fpu, xif_issue_if.issue_req.instr, xif_issue_if.issue_req.id); //TODO: add issue_transaction_active?
    end
  end


endmodule;