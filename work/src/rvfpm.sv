/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model.
  Implemented in C++ (fpu_top) and interfaced using sv and DPI-C.
  Number of pipeline stages are parameterized. Note: Execute (and dependency on data_fromMem and data_fromXreg) is last stage in pipeline
  Further work: fully parameterized pipeline. More fp types. Standardized interface (CORE-V-XIF)
*/


`define FLEN 32
`define XLEN 32
`define NUM_FPU_REGS 32
`define COPROC 0
`include "pa_rvfpm.sv"
import pa_rvfpm::*;


module rvfpm #(
  parameter NUM_REGS          = 32,
  parameter XLEN              = `XLEN,
  //System parameters
  parameter COPROC            = `COPROC, //Set to 1 to function as coprocessor, will act as a HW unit if not
  //Pipeline parameters
  parameter PIPELINE_STAGES   = 4,
  parameter QUEUE_DEPTH       = 0, //Size of operation queue
  parameter FORWARDING        = 0, //Set to 1 to enable forwarding, not implemented
  parameter OUT_OF_ORDER      = 0, //Set to 1 to enable out of order execution, not implemented

  //CORE-V-XIF parameters for coprocessor
  parameter X_NUM_RS          = 2, //Read ports //TODO: not used
  parameter X_ID_WIDTH        = 4,
  parameter X_MEM_WIDTH       = `FLEN, //TODO: dependent on extension
  parameter X_RFR_WIDTH       = `FLEN, //Read acces width //TODO: not used
  parameter X_RFW_WIDTH       = `FLEN, //Write acces width //TODO: not used
  parameter X_MISA            = 'h0000_0000, //TODO: not used
  parameter X_ECS_XS          = 2'b0,        //TODO: not used
  parameter X_DUALREAD        = 0, //TODO: not implemented
  parameter X_DUALWRITE       = 0 //TODO: not implemented

)

(
  input logic ck,
  input logic rst,
  input logic enable,

  input logic[31:0] instruction,
  input logic [X_ID_WIDTH-1:0] id,
  output logic [X_ID_WIDTH-1:0] id_out,

  input logic[XLEN-1:0] data_fromXReg,
  input int unsigned data_fromMem,
  output logic[XLEN-1:0] data_toXReg,
  output int unsigned  data_toMem,
  output logic toXReg_valid, //valid flags for outputs
  output logic toMem_valid,
  output logic fpu_ready, //0 if not accepting instructions

//eXtension interface for coprocessor
  in_xif.coproc_compressed xif_compressed_if,
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
  import "DPI-C" function void clock_event(input chandle fpu_ptr);
  import "DPI-C" function void destroy_fpu(input chandle fpu_ptr);
  import "DPI-C" function int unsigned getRFContent(input chandle fpu_ptr, input int addr);
  import "DPI-C" function void add_accepted_instruction(input chandle fpu_ptr, input int instr, input int unsigned id);
  import "DPI-C" function void poll_predecoder_result(input chandle fpu_ptr, output logic accept, output x_issue_resp_t resp);
  import "DPI-C" function void predecode_instruction(input chandle fpu_ptr, input int instr, input int unsigned id);
  //Something to issue response from predecoder
  //-----------------------
  //-- Local parameters
  //-----------------------
  logic fpu_ready_s; //status signal
  logic [X_ID_WIDTH-1:0] fpu_accept_id; //ID of accepted instruction
  logic fpu_accept; //Acceptance signal
  logic new_instruction_accepted; //Indicates that a new instruction is accepted
  logic issue_transaction_active; //Indicates that an issue transaction is active
  //-----------------------
  //-- Initialization
  //-----------------------
  chandle fpu;
  initial begin
    fpu = create_fpu_model(PIPELINE_STAGES, QUEUE_DEPTH, NUM_REGS);
  end
  assign fpu_ready_s = 1;

  //issue ready er egentlig bare at fpu er klar til å motta en forespørsel om ofloaded instruction
  //Så set til 1 så lenge fpu er ready

  always_ff @(posedge ck) begin: la_main
    if (rst) begin
      reset_fpu(fpu);
    end
    else if (enable) begin
      //Call clocked functions
      clock_event(fpu);
      // add_accepted_instruction(fpu, instruction, id); //This can be async! Handle in predecoder when accepted
      
      if (xif_issue_if.issue_valid) begin
        xif_issue_if.issue_ready = fpu_ready_s; //if it is actually ready.
      end else begin
        xif_issue_if.issue_ready = 0;
        //fpu_operation(fpu, instruction, id, data_fromXReg, data_fromMem, id_out, data_toMem, data_toXReg, pipelineFull, toMem_valid, toXReg_valid);
      end

      if (issue_transaction_active && new_instruction_accepted) begin
        add_accepted_instruction(fpu, xif_issue_if.issue_req.instr, xif_issue_if.issue_req.id);
      end

      if (issue_transaction_active && !new_instruction_accepted) begin
        //Do not accept instruction
      end

    end
  end


  always_comb begin
    assign issue_transaction_active = xif_issue_if.issue_valid && xif_issue_if.issue_ready;
    if (issue_transaction_active) begin
      predecode_instruction(fpu, xif_issue_if.issue_req.instr, xif_issue_if.issue_req.id);
      poll_predecoder_result(fpu, fpu_accept, xif_issue_if.issue_resp);
      assign xif_issue_if.issue_resp.accept = xif_issue_if.issue_valid && fpu_accept:  1'b0; //Accept instruction if valid
      assign new_instruction_accepted = xif_issue_if.issue_valid && xif_issue_if.issue_ready && xif_issue_if.issue_resp.accept; //Signal that a new instruction is accepted
    end
  end


endmodule;