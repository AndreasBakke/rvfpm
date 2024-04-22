/*  rvfpm - 2024
  Andreas S. Bakke

  Description
  Predecoder for the eXtension Issue interface
  Handles acceptance of offloaded instructions from the CPU
*/

#include <cstdint>
#include "fpu_decode.h"
#include "fpu_pipe.h"
#include "xif_config.h"



class FpuPredecoder {
  private:
    unsigned int current_decode_id;
    x_issue_resp_t resp;
    bool& fpuReady;
    bool use_rs_i[3];

  public:
  FpuPredecoder(bool& fpuReady);
  ~FpuPredecoder();

  void predecodeInstruction(uint32_t instruction, unsigned int id, bool& accept, bool& loadstore, bool& writeback, bool& use_rs_a, bool& use_rs_b, bool& use_rs_c);
  void reset();
};

//75866375 past 75866375
