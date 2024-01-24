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
    //Pipeline parameters
  parameter PIPELINE_STAGES   = 4,
    //CORE-V-XIF parameters
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
  //TODO: expand for other formats to correct num of bits.
  input logic[31:0] instruction,
  input logic [X_ID_WIDTH-1:0] id,
  input logic[XLEN-1:0] data_fromXReg, //Todo: when does this data need to be present in the pipeline?
  input int unsigned data_fromMem, //Todo: use logic[FLEN-1:0] instead?

  //TODO: if ZFinx - have operands as inputs, and output

  output logic[XLEN-1:0] data_toXReg,
  output int unsigned  data_toMem,
  output logic toXReg_valid, //valid flags for outputs
  output logic toMem_valid,
  output logic [X_ID_WIDTH-1:0] id_out,
  output logic fpu_ready //Indicate stalls
);
  //-----------------------
  //-- DPI-C Imports
  //-----------------------
  import "DPI-C" function chandle create_fpu_model(input int pipelineStages, input int rfDepth);
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
    fpu = create_fpu_model(PIPELINE_STAGES, NUM_REGS);
  end


  always_ff @(posedge ck) begin: la_main
    if (rst) begin
      reset_fpu(fpu);
    end
    else if (enable) begin //TODO: if implemented as coprosessor, follow CORE-V-XIF conventions
      fpu_operation(fpu, instruction, id, data_fromXReg, data_fromMem, id_out, data_toMem, data_toXReg, pipelineFull, toMem_valid, toXReg_valid);
    end begin
    end
  end

  always_comb begin
    fpu_ready <= !pipelineFull;
  end


endmodule;