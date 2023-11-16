// Interface for riscv floating point verification tool
// TODO: Connect with cpp model. Where to add pipelines?
// Zfinx?
// 
//Idea: We can implement each stage in cpp. Connect the stages in a SV top module. Then add interface?
//Weird to find a place to parameterize number of cycles delay. Except if we only want to return it to a output?


// It needs to actually function with pipelineing. eg.  8 executions can be pipelined and get correct results after each clock.
// Not as simple  as  just delaying output.

//Should we just provide fpu_rf.cpp, fpu_instr.cpp, fp_number and handle pipelineing in c++?
// Or have fpu_topp.cpp and just wrap that for verilog. so that we import a whole fpu?
// What makes it easiest for expanding with zfinx?

// TODO: extract these definitions to separate parameter file

`define FLEN 32
`define XLEN 32
`define NUM_FPU_REGS 32


module in_rvfpm #(
    parameter NUM_REGS          = 32,


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
    input logic [31:0] instruction,
    input logic [X_ID_WIDTH-1:0] id,
    input logic [31:0] data_fromXreg, //Todo: when does this data need to be present in the pipeline?
    input logic [31:0] data_fromMem,

    //TODO: if ZFinx - have operands as inputs, and output

    output logic [31:0] data_toXreg,
    output logic [31:0] data_toMem,
    output logic toXreg_valid, //valid flags for outputs
    output logic toMem_valid,
    output logic id_out,
    output logic fpu_ready //Indicate stalls
);
    //-----------------------
    //-- DPI-C Imports
    //-----------------------
    import "DPI-C" function chandle create_fpu_model(input int pipelineStages, input int rfDepth);
    import "DPI-C" function void operation(
        input chandle fpu_ptr,
        input logic[31:0] instruction,
        input logic[X_ID_WIDTH-1:0] id,
        input int fromXReg,
        input real fromMem,
        output logic[X_ID_WIDTH-1:0] id_out,
        output real toMem,
        output logic[31:0] toXreg,
        output logic pipelineFull
        );
    import "DPI-C" function void reset_fpu(input chandle fpu_ptr);
    import "DPI-C" function void destroy_fpu(input chandle fpu_ptr);

    //-----------------------
    //-- Local parameters
    //-----------------------
    logic pipelineFull; //status signal
    //-----------------------
    //-- Initialization
    //-----------------------
    chandle fpu;
    initial begin
        fpu= create_fpu_model(PIPELINE_STAGES, NUM_REGS);
    end


    always @(posedge ck) begin: la_main
        if (rst) begin
            reset_fpu(fpu);
        end
        else if (enable) begin //TODO: if implemented as coprosessor, follow CORE-V-XIF conventions
            operation(fpu, instruction, id, data_fromXreg, data_fromMem, id_out, data_toMem, data_toXreg, pipelineFull);
        end begin
        end
    end

    always_comb begin
        fpu_ready <= pipelineFull;
    end


endmodule;