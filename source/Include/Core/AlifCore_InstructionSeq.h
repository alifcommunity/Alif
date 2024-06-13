#pragma once

#include "AlifCore_SymTable.h"


class AlifInstruction {
public:
	AlifIntT opCode{};
	AlifIntT opArg{};

	SourceLocation loc{};

	AlifIntT target{};
	AlifIntT offset{};
};

class InstructionSequence {
public:
	ALIFOBJECT_HEAD;
	AlifInstruction* instructions{};
	AlifIntT allocated{};
	AlifIntT used{};
	AlifIntT nextFreeLable{};
	AlifIntT* labelMap{};
	AlifIntT labelMapSize{};
	AlifObject* nested{};
};

class JumpTargetLable {
public:
	AlifIntT id{};
};


AlifObject* alifInstructionSequance_new();

AlifIntT alifInstructionSequence_useLabel(InstructionSequence*, AlifIntT);
AlifIntT alifInstructionSequence_addOp(InstructionSequence*, AlifIntT, AlifIntT, SourceLocation);

AlifIntT alifInstructionSeq_applyLableMap(InstructionSequence*);

AlifIntT alifInstructionSequence_addNested(InstructionSequence*, InstructionSequence*);

extern AlifTypeObject _alifInstructionSeqType_;