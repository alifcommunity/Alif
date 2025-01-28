#pragma once

#include "AlifCore_SymTable.h"




class AlifExceptHandlerInfo { // 15
public:
	AlifIntT label{};
	AlifIntT startDepth{};
	AlifIntT preserveLastI{};
};

class AlifInstruction { // 21
public:
	AlifIntT opcode{};
	AlifIntT oparg{};
	AlifSourceLocation loc{};
	AlifExceptHandlerInfo exceptHandlerInfo{};

	/* Temporary fields, used by the assembler and in instr_sequence_to_cfg */
	AlifIntT target{};
	AlifIntT offset{};
};

class AlifInstructionSequence { // 32
public:
	ALIFOBJECT_HEAD;
	AlifInstruction* instrs{};
	AlifIntT allocated{};
	AlifIntT used{};

	AlifIntT nextFreeLabel{}; /* next free label id */

	/* Map of a label id to instruction offset (index into s_instrs).
	 * If labelmap is nullptr, then each label id is the offset itself.
	 */
	AlifIntT* labelMap{};
	AlifIntT labelMapSize{};

	/* AlifList of instruction sequences of nested functions */
	AlifObject* nested{};
};

class AlifJumpTargetLabel { // 50
public:
	AlifIntT id{};
};

#define NO_LABEL AlifJumpTargetLabel(-1)

#define SAME_JUMP_TARGET_LABEL(_l1, _l2) ((_l1).id == (_l2).id)
#define IS_JUMP_TARGET_LABEL(_l) (!SAME_JUMP_TARGET_LABEL(_l, NO_LABEL))

typedef AlifInstructionSequence InstrSequence; //* alif
typedef AlifSourceLocation Location; //* alif

AlifObject* _alifInstructionSequence_new(); // 54
AlifIntT _alifInstructionSequence_useLabel(InstrSequence*, AlifIntT); // 56
AlifIntT _alifInstructionSequence_addOp(InstrSequence*, AlifIntT, AlifIntT, Location); // 57


AlifJumpTargetLabel _alifInstructionSequence_newLabel(InstrSequence*); // 60
AlifIntT _alifInstructionSequence_applyLabelMap(InstrSequence* ); // 61
AlifIntT _alifInstructionSequence_insertInstruction(InstrSequence*, AlifIntT,
	AlifIntT, AlifIntT, Location); // 62

AlifIntT _alifInstructionSequence_addNested(InstrSequence*, InstrSequence*); // 64
void alifInstructionSequence_fini(InstrSequence*); // 65
extern AlifTypeObject _alifInstructionSequenceType_; // 67
