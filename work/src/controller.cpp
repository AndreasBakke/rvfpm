/*  rvfpm - 2024
  Andreas S. Bakke

  Description

*/

#include "controller.h"

Controller::Controller(FpuRf& rf, FpuPipeline& pipe, bool& ready) : registerFile(rf), fpu_pipeline(pipe), fpuReady(ready){
}

Controller::~Controller(){};

void Controller::reset(){
  mem_valid = false;
  mem_ready = false;
  mem_req = {};
}


void Controller::addAcceptedInstruction(uint32_t instruction, unsigned int id, unsignedType operand_a, unsignedType operand_b, unsignedType operand_c, unsigned int mode, bool commit_valid, unsigned int commit_id, bool commit_kill){ //and other necessary inputs (should be somewhat close to in_xif type)
  FpuPipeObj newOp = decodeOp(instruction, id, operand_a, operand_b, operand_c, mode);
  if (newOp == fpu_pipeline.at(0) && !fpu_pipeline.at(0).isEmpty()){
    return;
  }

  if (commit_valid && commit_id == newOp.id){
    if (commit_kill){
      newOp = {};
    } else {
      newOp.speculative = 0;
    }
  }

  //Check if we are encountering a RAW Hazard if we ask for mem immediatly
  #ifdef CTRL_RAW
    if (newOp.toMem) {
      //Check if the register to store is used further ahead in the pipeline
      for (int i = 0; i < fpu_pipeline.getNumStages(); i++){
        if (hasSameTarget(newOp, fpu_pipeline.at(i))){
          newOp.stalledByCtrl = 1;
        }
      }
      #ifdef INCLUDE_QUEUE
      for (int i = 0; i < fpu_pipeline.getQueueDepth(); i++){
        if (hasSameTarget(newOp, fpu_pipeline.at_queue(i))){
          newOp.stalledByCtrl = 1;
        }
      }
      #endif
    }
  #endif

  if ((newOp.toMem || newOp.fromMem) && !newOp.stalledByCtrl && !newOp.speculative){
    STYPE dec_instr = {.instr = newOp.instr};
    newOp.data = registerFile.read(dec_instr.parts.rs2);
    addMemoryRequest(newOp);
  }

  if(QUEUE_DEPTH>0){
    fpu_pipeline.addOpToQueue(newOp);
  }
  else {
    fpu_pipeline.setWaitingOp(newOp); //set waitingOp (if queue=0), this will be empty given the instruction is accepted
  }
}


//--------------------
// Hazards
//--------------------
bool Controller::hasSameTarget(FpuPipeObj op_to_check, FpuPipeObj op2){
  if (op2.isEmpty() || op_to_check.isEmpty()) {return false;}
  if (op_to_check.toMem) {
    if (op_to_check.addrFrom.front() == op2.addrTo && !op2.toMem){ //Only need to check in one direction
      return true;
    }
    if(op_to_check.addrTo == op2.addrFrom.front() && op2.fromMem){
      return true;
    } 

  } else if(op_to_check.fromMem){ //Make sure we have written to mem before reading from mem.
    if (op_to_check.addrFrom.front() == op2.addrTo){
      return true;
    }
    for (unsigned int addrFrom : op2.addrFrom){
      if (op_to_check.addrTo == addrFrom){
        return true;
      }
    }

  } else { //If using a register later written to:
    for (unsigned int addrFrom : op_to_check.addrFrom){
      if (addrFrom == op2.addrTo && !op_to_check.fromXReg && !op2.toXReg){
        return true;
      }
    }
    for (unsigned int addrFrom : op2.addrFrom){ //if execute is before memory.
      if(op_to_check.addrTo == addrFrom){
        return true;
      }
    }
  }
  return false;
}

void Controller::detectHazards(){
  #ifdef CTRL_RAW
  fpu_pipeline.at(WRITEBACK_STEP).stalledByCtrl = 0;
  for(int i=WRITEBACK_STEP+1; i<=std::max(EXECUTE_STEP, MEMORY_STEP); i++){
    fpu_pipeline.at(i).stalledByCtrl = 0;

    if (fpu_pipeline.at(i).isEmpty()){continue;}

    //Stall operation if the target register is the same as the one we are writing to
    if (hasSameTarget(fpu_pipeline.at(i), fpu_pipeline.at(WRITEBACK_STEP))){
      fpu_pipeline.at(i).stalledByCtrl = 1;
    }
    
    if (i < MEMORY_STEP && hasSameTarget(fpu_pipeline.at(MEMORY_STEP), fpu_pipeline.at(i))){
      fpu_pipeline.at(MEMORY_STEP).stalledByCtrl = 1;
    }
    if (i < EXECUTE_STEP && hasSameTarget(fpu_pipeline.at(EXECUTE_STEP), fpu_pipeline.at(i))){
      fpu_pipeline.at(EXECUTE_STEP).stalledByCtrl = 1;
    }
    if (i > MEMORY_STEP && hasSameTarget(fpu_pipeline.at(i), fpu_pipeline.at(MEMORY_STEP))){
      fpu_pipeline.at(i).stalledByCtrl = 1;
    }
    if (i > EXECUTE_STEP && hasSameTarget(fpu_pipeline.at(i), fpu_pipeline.at(EXECUTE_STEP))){
      fpu_pipeline.at(i).stalledByCtrl = 1;
    }
  }
  #endif

  #ifdef CTRL_WAW
    // Not implemted as OOO is not implemented - Thought to be handled by CPU
  #endif
  #ifdef CTRL_WAR
    // Not implemted as OOO is not implemented - Thought to be handled by CPU
  #endif
}

#ifdef FORWARDING
void Controller::resolveForwards(){
  for (int i = WRITEBACK_STEP; i < fpu_pipeline.getNumStages(); i++){
    if (fpu_pipeline.at(i).stalledByCtrl){
        FpuPipeObj& op = fpu_pipeline.at(i);
        #ifdef CTRL_RAW
          if (op.toMem && !op.added_to_mem_queue && op.addrFrom.front() == fpu_pipeline.fw_addr){ //Fw data if address is correct
            op.data = fpu_pipeline.fw_data;
            op.stalledByCtrl = 0;
            addMemoryRequest(op);
            fpu_pipeline.fw_addr = 0xbeefcafe; //Reset
            fpu_pipeline.fw_data = 0xBABECAFE; //reset
            return;
          }
          else if (!op.fromXReg && !op.fromMem) {
            for (unsigned int addrFrom : op.addrFrom){
              if (addrFrom == fpu_pipeline.fw_addr){
                op.fw_data = fpu_pipeline.fw_data;
                op.fw_addr = fpu_pipeline.fw_addr;
                op.stalledByCtrl = 0;
                op.useFwData = 1;
                fpu_pipeline.fw_addr = 0xcafecafe; //Reset
                fpu_pipeline.fw_data = 0xBABECAFE; //res4636et
                return;
              }
            }
          }
        #endif
        #ifdef CTRL_WAR

        #endif
    }
  }
  if (QUEUE_DEPTH > 0){ //also check the queue
    for (int i = 0; i < fpu_pipeline.getQueueDepth(); i++){
      if (fpu_pipeline.at_queue(i).stalledByCtrl){
        FpuPipeObj& op = fpu_pipeline.at_queue(i);
        #ifdef CTRL_RAW
          if (op.toMem && !op.added_to_mem_queue && op.addrFrom.front() == fpu_pipeline.fw_addr){
            op.data = fpu_pipeline.fw_data;
            op.stalledByCtrl = 0;
            addMemoryRequest(op);
            fpu_pipeline.fw_addr = 0xcafecafe; //Reset
            fpu_pipeline.fw_data = 0xBABECAFE; //reset
            return;
          }
        #endif
        #ifdef CTRL_WAR

        #endif
      }
    }
  }
  else { //Check waitingOp if no queue is used
    if (fpu_pipeline.getWaitingOp().stalledByCtrl){
      FpuPipeObj op = fpu_pipeline.getWaitingOp();
      if (op.toMem && !op.added_to_mem_queue && op.addrFrom.front() == fpu_pipeline.fw_addr){
          op.data = fpu_pipeline.fw_data;
          op.stalledByCtrl = 0;
          addMemoryRequest(op);
          fpu_pipeline.setWaitingOp(op);
        }
    }
  }
}
#endif

//--------------------
// Commit interface
//--------------------
void Controller::commitInstruction(unsigned int id, bool kill){
  fpu_pipeline.commitInstruction(id, kill);
};



//--------------------
// Memory interface
//--------------------
void Controller::addMemoryRequest(FpuPipeObj& op){ //Add to mem_req_queue
  x_mem_req_t mem_req_s = {};
  mem_req_s.id = op.id;
  mem_req_s.addr = op.toMem ? op.addrTo : op.addrFrom.front();
  mem_req_s.wdata = op.data;
  mem_req_s.last = 1;
  mem_req_s.size = op.size;
  mem_req_s.mode = op.mode;
  mem_req_s.we = op.toMem ? 1 : 0;
  fpu_pipeline.mem_req_queue.push_back(mem_req_s);
  op.added_to_mem_queue = 1;
}

void Controller::pollMemoryRequest(bool& mem_valid, x_mem_req_t& mem_req){
  mem_valid = !fpu_pipeline.mem_req_queue.empty();
  if (mem_valid) {
    mem_req = fpu_pipeline.mem_req_queue.front();
  }

};

void Controller::resetMemoryRequest(unsigned int id){
  if (id == fpu_pipeline.mem_req_queue.front().id){
    fpu_pipeline.mem_req_queue.pop_front();
  }
};


void Controller::writeMemoryResponse(bool mem_ready, bool exc, unsigned int exccode, bool dbg){
  this->mem_ready = mem_ready;
  if (exc) {
    std::cerr << "Exception in memory request - id: " << this->mem_req.id << std::endl;
    std::cout << "Exception code: " << exccode << std::endl;
  }
};

void Controller::writeMemoryResult(unsigned int id, uint32_t rdata, bool err, bool dbg) {
  if (err) {
    std::cerr << "Error in memory result - id: " << id << std::endl;
    return;
  }
  for (int i = 0; i < fpu_pipeline.getNumStages(); i++) {
    if (fpu_pipeline.at(i).id == id && !fpu_pipeline.at(i).isEmpty() && !fpu_pipeline.at(i).mem_result_valid && (fpu_pipeline.at(i).toMem || fpu_pipeline.at(i).fromMem)) {
      fpu_pipeline.at(i).mem_result_valid = 1;
      fpu_pipeline.at(i).mem_result = rdata;
      return;
    }
  }

  if (fpu_pipeline.getWaitingOp().id == id && !fpu_pipeline.getWaitingOp().isEmpty() && !fpu_pipeline.getWaitingOp().mem_result_valid && (fpu_pipeline.getWaitingOp().toMem || fpu_pipeline.getWaitingOp().fromMem)) {
    FpuPipeObj tmpOp = fpu_pipeline.getWaitingOp();
    tmpOp.mem_result_valid = 1;
    tmpOp.mem_result = rdata;
    if (tmpOp.toMem){ //If the store operation is done before reaching the pipeline. Remove it to save time
      fpu_pipeline.addResult(tmpOp);
      tmpOp = {};
    }
    fpu_pipeline.setWaitingOp(tmpOp);
    return;
  }
  for (int i = 0; i < fpu_pipeline.getQueueDepth(); i++) {
    if (fpu_pipeline.at_queue(i).id == id && !fpu_pipeline.at_queue(i).isEmpty() && !fpu_pipeline.at_queue(i).mem_result_valid && (fpu_pipeline.at_queue(i).toMem || fpu_pipeline.at_queue(i).fromMem)) {
      fpu_pipeline.at_queue(i).mem_result_valid = 1;
      fpu_pipeline.at_queue(i).mem_result = rdata;
      return;
    }
  }
}


void Controller::pollResult(bool& result_valid_ptr, x_result_t& result_ptr){
  result_valid_ptr = !fpu_pipeline.result_queue.empty();
  if(result_valid_ptr) {
    result_ptr = fpu_pipeline.result_queue.front();
  }
}

void Controller::resetResult(unsigned int id){
  if(id == fpu_pipeline.result_queue.front().id){
    fpu_pipeline.result_queue.pop_front();
  }
}