#pragma once


#include "OpcodeIDs.h"



#define MAX_REAL_OPCODE 254 // 13



// 19
#define IS_BLOCK_PUSH_OPCODE(_opcode) \
        ((_opcode) == SETUP_FINALLY or \
         (_opcode) == SETUP_WITH or \
         (_opcode) == SETUP_CLEANUP)



// 24
#define HAS_TARGET(_opcode) \
        (OPCODE_HAS_JUMP(_opcode) or IS_BLOCK_PUSH_OPCODE(_opcode))



// 27
#define IS_TERMINATOR_OPCODE(_opcode) \
        (OPCODE_HAS_JUMP(_opcode) or IS_SCOPE_EXIT_OPCODE(_opcode))

// 41
#define IS_UNCONDITIONAL_JUMP_OPCODE(_opcode) \
        ((_opcode) == JUMP or \
         (_opcode) == JUMP_NO_INTERRUPT or \
         (_opcode) == JUMP_FORWARD or \
         (_opcode) == JUMP_BACKWARD or \
         (_opcode) == JUMP_BACKWARD_NO_INTERRUPT)

// 48
#define IS_SCOPE_EXIT_OPCODE(_opcode) \
        ((_opcode) == RETURN_VALUE or \
         (_opcode) == RETURN_CONST or \
         (_opcode) == RAISE_VARARGS or \
         (_opcode) == RERAISE)









 // 68
#define RESUME_AT_FUNC_START 0
#define RESUME_AFTER_YIELD 1
#define RESUME_AFTER_YIELD_FROM 2
#define RESUME_AFTER_AWAIT 3



 // 73
#define RESUME_OPARG_LOCATION_MASK 0x3
#define RESUME_OPARG_DEPTH1_MASK 0x4
