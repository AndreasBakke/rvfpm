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
    STYPE dec_instr = {.instr = newOp.instr}; //For step-by-step - comparrison purposes we dont use addrFrom (So the data is present for flw aswell)
    newOp.data.bitpattern = registerFile.read(dec_instr.parts.rs2).bitpattern;
    addMemoryRequest(newOp);
  }

  if(QUEUE_DEPTH>0){
    fpu_pipeline.addOpToQueue(newOp);
  }
  else {
    fpu_pipeline.setWaitingOp(newOp); //set waitingOp (if queue=0, this will be empty given the instruction is accepted
  }
}


//--------------------
// Hazards
//--------------------
bool Controller::hasSameTarget(FpuPipeObj first, FpuPipeObj last){
  if (last.isEmpty() || first.isEmpty()) {return false;}
  if (first.toMem) {
    if (first.addrFrom.front() == last.addrTo){
      return true;
    }
    return false;
  }
  if(first.fromMem){ //No overlap if we are reading from memory
    return false;
  }
  for (int i = 0; i < first.addrFrom.size(); i++){
    return true;
  }
  return false;
}

void Controller::detectHazards(){
  #ifdef CTRL_RAW
  fpu_pipeline.at(WRITEBACK_STEP).stalledByCtrl = 0;
  for(int i=WRITEBACK_STEP+1; i<=std::max(EXECUTE_STEP, MEMORY_STEP); i++){
    fpu_pipeline.at(i).stalledByCtrl = 0;

    if (fpu_pipeline.at(i).isEmpty() || fpu_pipeline.at(i).addrTo == 999 ){continue;}

    if(i != EXECUTE_STEP && i != MEMORY_STEP){continue;}//no need to stall if nothing is done

    //Forward data if the difference is one step
    if (hasSameTarget(fpu_pipeline.at(i), fpu_pipeline.at(WRITEBACK_STEP))){
      fpu_pipeline.at(i).stalledByCtrl = 1;
    }
  }
  #endif

  #ifdef CTRL_WAW
  //Also a non-issue if we dont reorder operations. Then it is intended by the user i guess...
    //Check for WAW hazards
  #endif
  #ifdef CTRL_WAR
  //Only necessary if we reorder operations to write over results before a previous operation is done
  //Check for WAR
  //Pretty much only stall FLW operations beeing added to wb if the target register is read from further down the line!
  #endif
}

#ifdef FORWARDING
void Controller::resolveForwards(){ //Currently only resolves memory operations that have not done a mem-rquest due to the stall.
  for (int i = WRITEBACK_STEP; i < fpu_pipeline.getNumStages(); i++){
    if (fpu_pipeline.at(i).stalledByCtrl){
        FpuPipeObj& op = fpu_pipeline.at(i);
        #ifdef CTRL_RAW
          if (op.toMem && !op.added_to_mem_queue && op.addrFrom.front() == fpu_pipeline.fw_addr){
            op.data.bitpattern = fpu_pipeline.fw_data.bitpattern;
            op.stalledByCtrl = 0;
            addMemoryRequest(op);
          }
        #endif
        #ifdef CTRL_WAR

        #endif
    }
  }
  if (QUEUE_DEPTH > 0){
    for (int i = 0; i < fpu_pipeline.getQueueDepth(); i++){
      if (fpu_pipeline.at_queue(i).stalledByCtrl){
        FpuPipeObj& op = fpu_pipeline.at_queue(i);
        #ifdef CTRL_RAW
          if (op.toMem && !op.added_to_mem_queue && op.addrFrom.front() == fpu_pipeline.fw_addr){
            op.data.bitpattern = fpu_pipeline.fw_data.bitpattern;
            op.stalledByCtrl = 0;
            addMemoryRequest(op);
          }
        #endif
        #ifdef CTRL_WAR

        #endif
      }
    }
  }
  else {
    if (fpu_pipeline.getWaitingOp().stalledByCtrl){
      FpuPipeObj op = fpu_pipeline.getWaitingOp();
      if (op.toMem && !op.added_to_mem_queue && op.addrFrom.front() == fpu_pipeline.fw_addr){
          op.data.bitpattern = fpu_pipeline.fw_data.bitpattern;
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

//--------------------
// Memory interface
//--------------------
void Controller::addMemoryRequest(FpuPipeObj& op){
  x_mem_req_t mem_req_s = {};
  mem_req_s.id = op.id;
  mem_req_s.addr = op.toMem ? op.addrTo : op.addrFrom.front();
  mem_req_s.wdata = op.data.bitpattern;
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