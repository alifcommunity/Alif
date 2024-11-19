#include "alif.h"

#include "AlifCore_Compile.h"
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_OpcodeMetadata.h"


 // 16
typedef AlifInstruction Instruction;
typedef AlifInstructionSequence InstrSequence;
typedef AlifSourceLocation Location;

 // 20
#define INITIAL_INSTR_SEQUENCE_SIZE 100
#define INITIAL_INSTR_SEQUENCE_LABELS_MAP_SIZE 10


 // 25
#undef SUCCESS
#undef ERROR
#define SUCCESS 0
#define ERROR -1

#define RETURN_IF_ERROR(_x)  \
    if ((_x) == -1) {        \
        return ERROR;       \
    }



static AlifIntT instrSequence_nextInst(InstrSequence * _seq) { // 35
	RETURN_IF_ERROR(
		_alifCompile_ensureArrayLargeEnough(_seq->used + 1,
			(void**)&_seq->instrs, &_seq->allocated,
			INITIAL_INSTR_SEQUENCE_SIZE, sizeof(Instruction)));
	return _seq->used++;
}



 // 102
#define MAX_OPCODE 511

AlifIntT _alifInstructionSequence_addOp(InstrSequence * _seq,
	AlifIntT _opcode, AlifIntT _oparg, Location _loc) { // 104

	AlifIntT idx = instrSequence_nextInst(_seq);
	RETURN_IF_ERROR(idx);
	Instruction* ci = &_seq->instrs[idx];
	ci->opcode = _opcode;
	ci->oparg = _oparg;
	ci->loc = _loc;
	return SUCCESS;
}










AlifObject* _alifInstructionSequence_new() { // 197
	AlifInstructionSequence* seq = instSeq_create();
	if (seq == nullptr) {
		return nullptr;
	}
	return (AlifObject*)seq;
}
