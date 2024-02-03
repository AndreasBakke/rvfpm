/*  rvfpm - 2024
  Andreas S. Bakke

  Description
  Predecoder for the eXtension Issue interface
  Handles acceptance of offloaded instructions from the CPU
*/

#include <cstdint>
#include "fpu_decode.h"
#include "fpu_pipe.h"

typedef struct packed {
    bool          accept;     // Is the offloaded instruction (id) accepted by the coprocessor?
    bool          writeback;  // Will the coprocessor perform a writeback in the core to rd?
    bool          dualwrite;  // Will the coprocessor perform a dual writeback in the core to rd and rd+1?
    unsigned int  dualread;   // Will the coprocessor require dual reads from rs1\rs2\rs3 and rs1+1\rs2+1\rs3+1?
    bool          loadstore;  // Is the offloaded instruction a load/store instruction?
    bool          ecswrite ;  // Will the coprocessor write the Extension Context Status in mstatus?
    bool          exc;        // Can the offloaded instruction possibly cause a synchronous exception in the coprocessor itself?
  } x_issue_resp_t;

class FpuPredecoder {
  private:
    bool accept;
    unsigned int current_decode_id;
    x_issue_resp_t resp;
    bool& fpuReady;

  public:
  FpuPredecoder(bool& fpuReady);
  ~FpuPredecoder();

  void predecodeInstruction(uint32_t instruction, unsigned int id);
  void pollPredecoderResult(bool& accept_ref, x_issue_resp_t& resp_ref);
};