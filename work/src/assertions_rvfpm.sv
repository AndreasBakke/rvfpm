//TODO: Use opcode and stuff like this to detect what kind of instructions, and enforce the best assertion.
    uin_rvfpm.instruction[31:20] = 0; //imm
    uin_rvfpm.instruction[19:15] = 0; //rs1 (base)
    uin_rvfpm.instruction[14:12] = 0; //W
    uin_rvfpm.instruction[11:7] = 0;  //rd (dest)
    uin_rvfpm.instruction[6:0] = 7'b0000111;  //OPCODE


//And use the parameters for when to check (like in testPR writing to fromMem)
