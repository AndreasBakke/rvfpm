/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model.
  Package for types 
*/

package pa_rvfpm;

  typedef struct packed { //from in_xif.sv
    logic       accept;     // Is the offloaded instruction (id) accepted by the coprocessor?
    logic       writeback;  // Will the coprocessor perform a writeback in the core to rd?
    logic       dualwrite;  // Will the coprocessor perform a dual writeback in the core to rd and rd+1?
    logic [2:0] dualread;   // Will the coprocessor require dual reads from rs1\rs2\rs3 and rs1+1\rs2+1\rs3+1?
    logic       loadstore;  // Is the offloaded instruction a load/store instruction?
    logic       ecswrite ;  // Will the coprocessor write the Extension Context Status in mstatus?
    logic       exc;        // Can the offloaded instruction possibly cause a synchronous exception in the coprocessor itself?
  } x_issue_resp_t;


endpackage : pa_rvfpm