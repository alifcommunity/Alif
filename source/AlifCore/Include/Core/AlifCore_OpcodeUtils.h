#pragma once


#include "OpcodeIDs.h"






// 19
#define IS_BLOCK_PUSH_OPCODE(_opcode) \
        ((_opcode) == SETUP_FINALLY or \
         (_opcode) == SETUP_WITH or \
         (_opcode) == SETUP_CLEANUP)



// 24
#define HAS_TARGET(_opcode) \
        (OPCODE_HAS_JUMP(_opcode) or IS_BLOCK_PUSH_OPCODE(_opcode))










 // 68
#define RESUME_AT_FUNC_START 0
#define RESUME_AFTER_YIELD 1
#define RESUME_AFTER_YIELD_FROM 2
#define RESUME_AFTER_AWAIT 3
