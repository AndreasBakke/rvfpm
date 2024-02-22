/*  rvfpm - 2024
  Andreas S. Bakke

  Description:
  RISC-V Floating Point Unit Model.
  Package for types
*/
package pa_rvfpm;
  typedef struct packed {
    logic [31:0]  instr;
    logic [1:0]   mode;
    logic [pa_defines::X_ID_WIDTH-1:0]  id;
    logic [pa_defines::X_NUM_RS  -1:0][pa_defines::X_RFR_WIDTH-1:0] rs;
    logic [pa_defines::X_NUM_RS  -1:0]  rs_valid;
    logic [ 5:0]  ecs;
    logic         ecs_valid;
  } x_issue_req_t;

  typedef struct packed {
    logic       accept;
    logic       writeback;
    logic       dualwrite;
    logic [2:0] dualread;
    logic       loadstore;
    logic       ecswrite ;
    logic       exc;
  } x_issue_resp_t;

  typedef struct packed {
    logic [pa_defines::X_ID_WIDTH -1:0] id;
    logic [pa_defines::X_MEM_WIDTH-1:0] rdata;
    logic err;
    logic dbg;
  } x_mem_result_t;
endpackage : pa_rvfpm