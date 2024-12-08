#pragma once

#include "OpcodeIDs.h"


 // 20
#define IS_PSEUDO_INSTR(_op)  ( \
    ((_op) == LOAD_CLOSURE) or \
    ((_op) == STORE_FAST_MAYBE_NULL) or \
    ((_op) == JUMP) or \
    ((_op) == JUMP_NO_INTERRUPT) or \
    ((_op) == SETUP_FINALLY) or \
    ((_op) == SETUP_CLEANUP) or \
    ((_op) == SETUP_WITH) or \
    ((_op) == POP_BLOCK) or \
    0)


extern AlifIntT _alifOpcode_numPopped(AlifIntT _opcode, AlifIntT _oparg); // 32









extern AlifIntT _alifOpcode_numPushed(AlifIntT _opcode, AlifIntT _oparg); // 487








#define HAS_CONST_FLAG (2) // 959


#define HAS_JUMP_FLAG (8) // 961


#define HAS_EVAL_BREAK_FLAG (64) // 964


#define OPCODE_HAS_CONST(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_CONST_FLAG)) // 974

#define OPCODE_HAS_JUMP(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_JUMP_FLAG)) // 976


#define OPCODE_HAS_EVAL_BREAK(_op) (_alifOpcodeOpcodeMetadata_[_op].flags & (HAS_EVAL_BREAK_FLAG)) // 979






class OpcodeMetadata { // 998
public:
	uint8_t validEntry{};
	int8_t instrFormat{};
	int16_t flags{};
};

extern const OpcodeMetadata _alifOpcodeOpcodeMetadata_[264]; // 1004




extern const uint8_t _alifOpcodeCaches_[256]; // 1647



extern const uint8_t _alifOpcodeDeopt_[256]; // 1673
