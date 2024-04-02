/*
  rvfpm - 2023
  Andreas S. Bakke

  Description:
  CORE-V-XIF interface
  Adapted from https://github.com/openhwgroup/cv32e40x/blob/master/rtl/cv32e40x_if_xif.sv
  LICENSE:

  Copyright 2021 TU Wien

  This file, and derivatives thereof are licensed under the
  Solderpad License, Version 2.0 (the "License");
  Use of this file means you agree to the terms and conditions
  of the license and are in full compliance with the License.
  You may obtain a copy of the License at

  https://solderpad.org/licenses/SHL-2.0/

  Unless required by applicable law or agreed to in writing, software
  and hardware implementations thereof
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESSED OR IMPLIED.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

`include "defines.svh"
package in_xif;
  parameter NUM_F_REGS        = `NUM_F_REGS;
  parameter XLEN              = `XLEN;
  parameter FLEN              = `FLEN;
  //System parameters

  //Pipeline parameters
  parameter PIPELINE_STAGES   = `NUM_PIPELINE_STAGES;
  parameter QUEUE_DEPTH       = `QUEUE_DEPTH; //Size of operation queue
  parameter FORWARDING        = `FORWARDING; //Set to 1 to enable forwarding; not implemented
  parameter OUT_OF_ORDER      = `OOO; //Set to 1 to enable out of order execution; not implemented

  //CORE-V-XIF parameters for coprocessor
  parameter X_NUM_RS          = `X_NUM_RS; //Read ports
  parameter X_ID_WIDTH        = `X_ID_WIDTH;
  parameter X_MEM_WIDTH       = `FLEN; //TODO: dependent on extension
  parameter X_RFR_WIDTH       = `FLEN; //Read acces width
  parameter X_RFW_WIDTH       = `FLEN; //Write acces width
  parameter X_MISA            = `X_MISA; //TODO: not used
  parameter X_ECS_XS          = `X_ECS_XS;        //TODO: not used


  typedef struct packed {
    logic [          31:0]                  instr;     // Offloaded instruction
    logic [           1:0]                  mode;      // Privilege level
    logic [X_ID_WIDTH-1:0]                  id;        // Identification of the offloaded instruction
    logic [X_NUM_RS  -1:0][X_RFR_WIDTH-1:0] rs;        // Register file source operands for the offloaded instruction
    logic [X_NUM_RS  -1:0]                  rs_valid;  // Validity of the register file source operand(s)
    logic [           5:0]                  ecs;       // Extension Context Status ({mstatus.xs, mstatus.fs, mstatus.vs})
    logic                                   ecs_valid; // Validity of the Extension Context Status
  } x_issue_req_t;

  typedef struct packed {
    logic       accept;     // Is the offloaded instruction (id) accepted by the coprocessor?
    logic       writeback;  // Will the coprocessor perform a writeback in the core to rd?
    logic       dualwrite;  // Will the coprocessor perform a dual writeback in the core to rd and rd+1?
    logic       dualread;   // Will the coprocessor require dual reads from rs1\rs2\rs3 and rs1+1\rs2+1\rs3+1?
    logic       loadstore;  // Is the offloaded instruction a load/store instruction?
    logic       ecswrite ;  // Will the coprocessor write the Extension Context Status in mstatus?
    logic       exc;        // Can the offloaded instruction possibly cause a synchronous exception in the coprocessor itself?
  } x_issue_resp_t;

  typedef struct packed {
    logic [X_ID_WIDTH-1:0] id;          // Identification of the offloaded instruction
    logic                  commit_kill; // Shall an offloaded instruction be killed?
  } x_commit_t;

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
    logic       exc;      // Did the memory request cause a synchronous exception?
    logic [5:0] exccode;  // Exception code
    logic       dbg;      // Did the memory request cause a debug trigger match with ``mcontrol.timing`` = 0?
  } x_mem_resp_t;

  typedef struct packed {
    logic [X_ID_WIDTH -1:0] id;     // Identification of the offloaded instruction
    logic [X_MEM_WIDTH-1:0] rdata;  // Read data of a read memory transaction
    logic                   err;    // Did the instruction cause a bus error?
    logic                   dbg;    // Did the read data cause a debug trigger match with ``mcontrol.timing`` = 0?
  } x_mem_result_t;

  typedef struct packed {
    logic [X_ID_WIDTH      -1:0] id;      // Identification of the offloaded instruction
    logic [X_RFW_WIDTH     -1:0] data;    // Register file write data value(s)
    logic [                 4:0] rd;      // Register file destination address(es)
    logic [X_RFW_WIDTH/XLEN-1:0] we;      // Register file write enable(s)
    logic [                 2:0] ecswe;   // Write enables for {mstatus.xs, mstatus.fs, mstatus.vs}
    logic [                 5:0] ecsdata; // Write data value for {mstatus.xs, mstatus.fs, mstatus.vs}
    logic                        exc;     // Did the instruction cause a synchronous exception?
    logic [                 5:0] exccode; // Exception code
    logic                        dbg;     // Did the instruction cause a debug trigger match with ``mcontrol.timing`` = 0?
    logic                        err;     // Did the instruction cause a bus error?
  } x_result_t;

  // Issue interface
  logic               issue_valid;
  logic               issue_ready;
  x_issue_req_t       issue_req;
  x_issue_resp_t      issue_resp;

  // Commit interface
  logic               commit_valid;
  x_commit_t          commit;

  // Memory (request/response) interface
  logic               mem_valid;
  logic               mem_ready;
  x_mem_req_t         mem_req;
  x_mem_resp_t        mem_resp;

  // Memory result interface
  logic               mem_result_valid;
  x_mem_result_t      mem_result;

  // Result interface
  logic               result_valid;
  logic               result_ready;
  x_result_t          result;


endpackage : in_xif

