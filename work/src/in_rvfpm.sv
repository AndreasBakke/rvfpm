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


//Pseudocode:
signal result
operation(opcode, fromInt, FromXreg, result)

module in_rvfpm #(
    parameter NUM_REGS = `NUM_FPU_REGS,
    parameter PIPELINE_STAGES = `FPU_PIPE_STAGES
)
 
(
    input clk;
    input reset_n;
    //TODO: expand for other formats to correct num of bits.
    input [31:0] instruction;
    input [31:0] data_fromXreg;
    input [31:0] data_fromMem;

    //TODO: if ZFinx - have operands as inputs, and output

    output [31:0] data_toXreg;
    output [31:0] data_toMem;
    output flag;//TODO: what flags? Should we split in the interface?
);

    void* fpu_model;

    initial begin
        fpu_model = create_fpu_model(PIPELINE_STAGES, NUM_REGS);
    end

    always @(posedge clk) begin
        if (reset) begin
            resetFPU(fpu);
        end
        else begin
            operation(fpu_model, data_fromXreg, data_fromMem, data_toMem, data_toXreg, flags_out);
        end
    end
        fpu->operation(instruction, fromXReg, fromMem, toMem, toXreg, flags_out);


endmodule;