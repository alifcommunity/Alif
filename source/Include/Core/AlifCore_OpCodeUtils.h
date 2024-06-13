#pragma once


#include "OpCodeIDs.h"










#define ISBLOCK_PUSH_OPCODE(_opCode) \
        ((_opCode) == SETUP_FINALLY or \
         (_opCode) == SETUP_WITH or \
         (_opCode) == SETUP_CLEANUP)


#define HAS_TARGET(_opCode) (OPCODE_HAS_JUMP(_opCode) or ISBLOCK_PUSH_OPCODE(_opCode))

#define IS_TERMINATOR_OPCODE(_opCode) (OPCODE_HAS_JUMP(_opCode) or ISSCOPE_EXIT_OPCODE(_opCode))






#define ISUNCONDITIONAL_JUMP_OPCODE(_opCode) \
        ((_opCode) == JUMP or \
         (_opCode) == JUMP_NO_INTERRUPT or \
         (_opCode) == JUMP_FORWARD or \
         (_opCode) == JUMP_BACKWARD or \
         (_opCode) == JUMP_BACKWARD_NO_INTERRUPT)

#define ISSCOPE_EXIT_OPCODE(_opCode) \
        ((_opCode) == RETURN_VALUE or \
         (_opCode) == RETURN_CONST or \
         (_opCode) == RAISE_VARARGS or \
         (_opCode) == RERAISE)






#define RESUME_ATFUNC_START 0