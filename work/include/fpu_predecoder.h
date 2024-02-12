/*  rvfpm - 2024
  Andreas S. Bakke

  Description
  Predecoder for the eXtension Issue interface
  Handles acceptance of offloaded instructions from the CPU
*/

#include <cstdint>
#include "fpu_decode.h"
#include "fpu_pipe.h"
#include "fpu_config.h"



class FpuPredecoder {
  private:
    unsigned int current_decode_id;
    x_issue_resp_t resp;
    bool& fpuReady;

  public:
  FpuPredecoder(bool& fpuReady);
  ~FpuPredecoder();

  void predecodeInstruction(uint32_t instruction, unsigned int id);
  //TODO: add some logic that resets internal state of predecoder when not used
  void pollPredecoderResult(x_issue_resp_t& resp_ref);
};