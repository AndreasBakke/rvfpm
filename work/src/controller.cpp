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



void Controller::addAcceptedInstruction(uint32_t instruction, unsigned int id, unsigned int operand_a, unsigned int operand_b, unsigned int operand_c, unsigned int mode, bool commit_valid, unsigned int commit_id, bool commit_kill){ //and other necessary inputs (should be somewhat close to in_xif type)
  FpuPipeObj newOp = decodeOp(instruction, id, operand_a, operand_b, operand_c, mode);
  if (newOp.id == fpu_pipeline.at(0).id && !fpu_pipeline.at(0).isEmpty()){
    return;
  }

 //Need to include newOp in some way so the following: can be stled for FSW
 //We can just look through the pipeline?
 //And then in the memory step, check if op.mem_result is set OR the op is in mem_req_queue.
  //Go through all queue stages and pipeline stages and check if the addrTo is the same as the addrFrom of the newOp
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
  if ((newOp.toMem || newOp.fromMem) && !newOp.stalledByCtrl){
    x_mem_req_t mem_req_s = {};
    mem_req_s.id = newOp.id;
    mem_req_s.addr = newOp.toMem ? newOp.addrTo : newOp.addrFrom.front();
    mem_req_s.wdata = newOp.toMem ?  registerFile.read(newOp.addrFrom.front()).bitpattern: 0;
    mem_req_s.last = 1;
    mem_req_s.size = newOp.size;
    mem_req_s.mode = newOp.mode;
    mem_req_s.we = newOp.toMem ? 1 : 0;
    mem_req_queue.push_back(mem_req_s);
    newOp.added_to_mem_queue = 1;
  }

  if (commit_valid && commit_id == newOp.id){
    if (commit_kill){
      newOp = {};
    } else {
      newOp.speculative = 0;
    }
  }

  if (fpu_pipeline.getQueueDepth() > 0){
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
      std::cout << "store" <<std::endl;
      return true;
    }
    return false;
  }
  if(first.fromMem){ //No overlap if we are reading from memory
    return false;
  }
  for (int i = 0; i < first.addrFrom.size(); i++){
    std::cout << "other" <<std::endl;
    return true;
  }
  return false;
}

void Controller::detectHazards(){
  #ifdef CTRL_RAW
  for(int i=WRITEBACK_STEP+1; i<std::max(EXECUTE_STEP, MEMORY_STEP); i++){
    fpu_pipeline.at(i).stalledByCtrl = 0;

    if (fpu_pipeline.at(i).isEmpty() || fpu_pipeline.at(i).addrTo == 0){continue;}

    if(i != EXECUTE_STEP && i != MEMORY_STEP){continue;}//no need to stall if nothing is done

    //Forward data if the difference is one step
    if (hasSameTarget(fpu_pipeline.at(i), fpu_pipeline.at(WRITEBACK_STEP))){
      fpu_pipeline.at(i).stalledByCtrl = 1;
    }
    if(i == EXECUTE_STEP && hasSameTarget(fpu_pipeline.at(i), fpu_pipeline.at(MEMORY_STEP))){
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
  #endif
}

// void Controller::forwardData(){
//   #ifdef FORWARDING
//   //if execute done, and the operation at EXECUTE_STEP-1 has the same addrFROM as addrTO of the operation at EXECUTE_STEP, forward data
//   //Also need to check if we should forward to the queue. (In case execute is the first stpe of the pipeline.)
//   #endif
// }




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
    if (fpu_pipeline.at(i).id == id && !fpu_pipeline.at(i).isEmpty()) {
      fpu_pipeline.at(i).mem_result_valid = 1;
      fpu_pipeline.at(i).mem_result = rdata;
      return;
    }
  }

  if (fpu_pipeline.getWaitingOp().id == id && !fpu_pipeline.getWaitingOp().isEmpty()) {
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
    if (fpu_pipeline.at_queue(i).id == id && !fpu_pipeline.at_queue(i).isEmpty()) {
      fpu_pipeline.at_queue(i).mem_result_valid = 1;
      fpu_pipeline.at_queue(i).mem_result = rdata;
      if (fpu_pipeline.at_queue(i).toMem){ //If the store operation is done before reaching the pipeline. Remove it to save time
        fpu_pipeline.addResult(fpu_pipeline.at_queue(i));
        fpu_pipeline.at_queue(i) = {};
      }
      return;
    }
  }
}

void Controller::pollMemoryRequest(bool& mem_valid, x_mem_req_t& mem_req){
  mem_valid = !mem_req_queue.empty();
  if (mem_valid) {
    mem_req = mem_req_queue.front();
  }

};

void Controller::resetMemoryRequest(unsigned int id){
  if (id == mem_req_queue.front().id){
    mem_req_queue.pop_front();
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