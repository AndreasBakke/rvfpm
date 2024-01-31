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


module rvfpm #(
  parameter NUM_REGS          = 32,
  parameter XLEN              = `XLEN,
  //System parameters
  parameter COPROC            = 0, //Set to 1 to function as coprocessor, will act as a HW accelerator if not
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

  //For HW accelerator use
  `ifndef COPROC
    input logic[31:0] instruction,
    input logic [X_ID_WIDTH-1:0] id,
    output logic [X_ID_WIDTH-1:0] id_out,

    input logic[XLEN-1:0] data_fromXReg, //Todo: when does this data need to be present in the pipeline?
    input int unsigned data_fromMem, //Todo: use logic[FLEN-1:0] instead?
    output logic[XLEN-1:0] data_toXReg,
    output int unsigned  data_toMem,
    output logic toXReg_valid, //valid flags for outputs
    output logic toMem_valid,
    output logic fpu_ready //0 if not accepting instructions

  //eXtension interface for coprocessor
  `else
    in_xif.coproc_compressed xif_compressed_if,
    in_xif.coproc_issue xif_issue_if,
    in_xif.coproc_commit xif_commit_if,
    in_xif.coproc_mem  xif_mem_if,
    in_xif.coproc_mem_result xif_mem_result_if,
    in_xif.coproc_result xif_result_if
  `endif

);
  //-----------------------
  //-- DPI-C Imports
  //-----------------------
  import "DPI-C" function chandle create_fpu_model(input int pipelineStages, input int queueStages, input int rfDepth);
  import "DPI-C" function void fpu_operation(
    input chandle fpu_ptr,
    input int unsigned instruction,
    input int unsigned id,
    input int unsigned fromXReg,
    input int unsigned fromMem,
    output int unsigned id_out,
    output int unsigned toMem,
    output int unsigned toXReg,
    output logic pipelineFull,
    output logic toMem_valid,
    output logic toXReg_valid
    );
  import "DPI-C" function void reset_fpu(input chandle fpu_ptr);
  import "DPI-C" function void destroy_fpu(input chandle fpu_ptr);
  import "DPI-C" function int unsigned getRFContent(input chandle fpu_ptr, input int addr);

  //-----------------------
  //-- Local parameters
  //-----------------------
  logic pipelineFull; //status signal
  //-----------------------
  //-- Initialization
  //-----------------------
  chandle fpu;
  initial begin
    fpu = create_fpu_model(PIPELINE_STAGES, QUEUE_DEPTH, NUM_REGS);
  end


  always_ff @(posedge ck) begin: la_main
    if (rst) begin
      reset_fpu(fpu);
    end
    else if (enable) begin //TODO: if implemented as coprosessor, follow CORE-V-XIF conventions
      //TODO:
      //Call clocked functions, and seperate fpu_operation into something else. We need to accept instructions.
      fpu_operation(fpu, instruction, id, data_fromXReg, data_fromMem, id_out, data_toMem, data_toXReg, pipelineFull, toMem_valid, toXReg_valid);
    end begin
    end
  end

  always_comb begin
    fpu_ready <= !pipelineFull;
  end


endmodule;