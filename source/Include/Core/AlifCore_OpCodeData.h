#pragma once

#include "OpCodeIDs.h"





enum InstructionFormat { // 902
    INSTR_FMT_IB = 1,
    INSTR_FMT_IBC = 2,
    INSTR_FMT_IBC00 = 3,
    INSTR_FMT_IBC000 = 4,
    INSTR_FMT_IBC00000000 = 5,
    INSTR_FMT_IX = 6,
    INSTR_FMT_IXC = 7,
    INSTR_FMT_IXC00 = 8,
    INSTR_FMT_IXC000 = 9,
};



#define HAS_ARG_FLAG (1) // 918
#define HAS_CONST_FLAG (2) // 919

#define HAS_JUMP_FLAG (8)

#define HAS_EVAL_BREAK_FLAG (64)

#define OPCODE_HAS_CONST(_op) (alifOpCodeData[_op].flags & (HAS_CONST_FLAG)) // 934

#define OPCODE_HAS_JUMP(_op) (alifOpCodeData[_op].flags & (HAS_JUMP_FLAG)) // 936

#define OPCODE_HAS_EVAL_BREAK(_op) (alifOpCodeData[_op].flags & (HAS_EVAL_BREAK_FLAG))








class OpCodeData { // 958
public:
    uint8_t validEntry;
    int8_t instrFormat;
    int16_t flags;
}; 


extern const OpCodeData alifOpCodeData[256];
#ifdef NEED_OPCODE_DATA
extern const OpCodeData alifOpCodeData[256] = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {TRUE, InstructionFormat::INSTR_FMT_IB, HAS_ARG_FLAG | HAS_JUMP_FLAG}, // JUMB_FORWARD -79-
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {TRUE, InstructionFormat::INSTR_FMT_IB, HAS_ARG_FLAG | HAS_CONST_FLAG}, // RETURN_CONST -103-



};

#endif