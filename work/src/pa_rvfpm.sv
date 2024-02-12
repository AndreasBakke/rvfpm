/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model.
  Package for types
*/

package pa_rvfpm;
  parameter NUM_REGS          = 32;
  parameter XLEN              = 32;
  parameter FLEN              = 32;

  //CORE-V-XIF parameters for coprocessor
  parameter X_NUM_RS          = 2; //Read ports //TODO: not used
  parameter X_ID_WIDTH        = 4;
  parameter X_MEM_WIDTH       = 32; //TODO: dependent on extension
  parameter X_RFR_WIDTH       = 32; //Read acces width //TODO: not used
  parameter X_RFW_WIDTH       = 32; //Write acces width //TODO: not used
  parameter X_MISA            = 'h0000_0000; //TODO: not used
  parameter X_ECS_XS          = 2'b0;        //TODO: not used
  parameter X_DUALREAD        = 0; //TODO: not implemented
  parameter X_DUALWRITE       = 0; //TODO: not implemented

  typedef struct packed {
    logic [          31:0]                  instr;     // Offloaded instruction
    logic [           1:0]                  mode;      // Privilege level
    logic [X_ID_WIDTH-1:0]                  id;        // Identification of the offloaded instruction
    logic [X_NUM_RS  -1:0][X_RFR_WIDTH-1:0] rs;        // Register file source operands for the offloaded instruction
    logic [X_NUM_RS  -1:0]                  rs_valid;  // Validity of the register file source operand(s)
    logic [           5:0]                  ecs;       // Extension Context Status ({mstatus.xs, mstatus.fs, mstatus.vs})
    logic                                   ecs_valid; // Validity of the Extension Context Status
  } x_issue_req_t;

  typedef struct packed { //from in_xif.sv
    logic       accept;     // Is the offloaded instruction (id) accepted by the coprocessor?
    logic       writeback;  // Will the coprocessor perform a writeback in the core to rd?
    logic       dualwrite;  // Will the coprocessor perform a dual writeback in the core to rd and rd+1?
    logic [2:0] dualread;   // Will the coprocessor require dual reads from rs1\rs2\rs3 and rs1+1\rs2+1\rs3+1?
    logic       loadstore;  // Is the offloaded instruction a load/store instruction?
    logic       ecswrite ;  // Will the coprocessor write the Extension Context Status in mstatus?
    logic       exc;        // Can the offloaded instruction possibly cause a synchronous exception in the coprocessor itself?
  } x_issue_resp_t;

  typedef struct packed {
    logic [X_ID_WIDTH   -1:0] id;    // Identification of the offloaded instruction
    logic [             31:0] addr;  // Virtual address of the memory transaction
    logic [              1:0] mode;  // Privilege level
    logic                     we;    // Write enable of the memory transaction
    logic [              2:0] size;  // Size of the memory transaction
    logic [X_MEM_WIDTH/8-1:0] be;    // Byte enables for memory transaction
    logic [              1:0] attr;  // Memory transaction attributes
    logic [X_MEM_WIDTH  -1:0] wdata; // Write data of a store memory transaction
    logic                     last;  // Is this the last memory transaction for the offloaded instruction?
    logic                     spec;  // Is the memory transaction speculative?
  } x_mem_req_t;

  typedef struct packed {
    logic [X_ID_WIDTH -1:0] id;     // Identification of the offloaded instruction
    logic [X_MEM_WIDTH-1:0] rdata;  // Read data of a read memory transaction
    logic                   err;    // Did the instruction cause a bus error?
    logic                   dbg;    // Did the read data cause a debug trigger match with ``mcontrol.timing`` = 0?
  } x_mem_result_t;

endpackage : pa_rvfpm