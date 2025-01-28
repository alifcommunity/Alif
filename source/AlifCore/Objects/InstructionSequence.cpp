#include "alif.h"

#include "AlifCore_Compile.h"
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_OpcodeMetaData.h"


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
		_alifCompiler_ensureArrayLargeEnough(_seq->used + 1,
			(void**)&_seq->instrs, &_seq->allocated,
			INITIAL_INSTR_SEQUENCE_SIZE, sizeof(Instruction)));
	return _seq->used++;
}


AlifJumpTargetLabel _alifInstructionSequence_newLabel(InstrSequence* _seq) { // 50
	AlifJumpTargetLabel lbl = { ++_seq->nextFreeLabel };
	return lbl;
}


AlifIntT _alifInstructionSequence_useLabel(InstrSequence* _seq, AlifIntT _lbl) { // 57
	AlifIntT oldSize = _seq->labelMapSize;
	RETURN_IF_ERROR(
		_alifCompiler_ensureArrayLargeEnough(_lbl,
			(void**)&_seq->labelMap,
			&_seq->labelMapSize,
			INITIAL_INSTR_SEQUENCE_LABELS_MAP_SIZE,
			sizeof(AlifIntT)));

	for (AlifIntT i = oldSize; i < _seq->labelMapSize; i++) {
		_seq->labelMap[i] = -111;  /* something weird, for debugging */
	}
	_seq->labelMap[_lbl] = _seq->used; /* label refers to the next instruction */
	return SUCCESS;
}

AlifIntT _alifInstructionSequence_applyLabelMap(InstrSequence* _instrs) { // 75
	if (_instrs->labelMap == nullptr) {
		return SUCCESS;
	}
	for (AlifIntT i = 0; i < _instrs->used; i++) {
		Instruction* instr = &_instrs->instrs[i];
		if (HAS_TARGET(instr->opcode)) {
			instr->oparg = _instrs->labelMap[instr->oparg];
		}
		AlifExceptHandlerInfo* hi = &instr->exceptHandlerInfo;
		if (hi->label >= 0) {
			hi->label = _instrs->labelMap[hi->label];
		}
	}
	alifMem_dataFree(_instrs->labelMap);
	_instrs->labelMap = nullptr;
	_instrs->labelMapSize = 0;
	return SUCCESS;
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

AlifIntT _alifInstructionSequence_insertInstruction(InstrSequence* _seq, AlifIntT _pos,
	AlifIntT _opcode, AlifIntT _oparg, Location _loc) { // 122
	AlifIntT lastIdx = instrSequence_nextInst(_seq);
	RETURN_IF_ERROR(lastIdx);
	for (AlifIntT i = lastIdx - 1; i >= _pos; i--) {
		_seq->instrs[i + 1] = _seq->instrs[i];
	}
	Instruction* ci = &_seq->instrs[_pos];
	ci->opcode = _opcode;
	ci->oparg = _oparg;
	ci->loc = _loc;

	/* fix the labels map */
	for (AlifIntT lbl = 0; lbl < _seq->labelMapSize; lbl++) {
		if (_seq->labelMap[lbl] >= _pos) {
			_seq->labelMap[lbl]++;
		}
	}
	return SUCCESS;
}


AlifIntT _alifInstructionSequence_addNested(InstrSequence* _seq,
	InstrSequence* _nested) { // 146
	if (_seq->nested == nullptr) {
		_seq->nested = alifList_new(0);
		if (_seq->nested == nullptr) {
			return ERROR;
		}
	}
	if (alifList_append(_seq->nested, (AlifObject*)_nested) < 0) {
		return ERROR;
	}
	return SUCCESS;
}



void alifInstructionSequence_fini(InstrSequence* _seq) { // 161
	ALIF_XDECREF(_seq->nested);

	alifMem_dataFree(_seq->labelMap);
	_seq->labelMap = nullptr;

	alifMem_dataFree(_seq->instrs);
	_seq->instrs = nullptr;
}



static AlifInstructionSequence* instSeq_create(void) { // 178
	AlifInstructionSequence* seq_{};
	seq_ = ALIFOBJECT_GC_NEW(AlifInstructionSequence, &_alifInstructionSequenceType_);
	if (seq_ == nullptr) {
		return nullptr;
	}
	seq_->instrs = nullptr;
	seq_->allocated = 0;
	seq_->used = 0;
	seq_->nextFreeLabel = 0;
	seq_->labelMap = nullptr;
	seq_->labelMapSize = 0;
	seq_->nested = nullptr;

	alifObject_gcTrack(seq_);
	return seq_;
}



AlifObject* _alifInstructionSequence_new() { // 197
	AlifInstructionSequence* seq = instSeq_create();
	if (seq == nullptr) {
		return nullptr;
	}
	return (AlifObject*)seq;
}




static void instSeq_dealloc(AlifInstructionSequence* _seq) { // 390
	alifObject_gcUnTrack(_seq);
	ALIF_TRASHCAN_BEGIN(_seq, instSeq_dealloc)
	alifInstructionSequence_fini(_seq);
	alifObject_gcDel(_seq);
	ALIF_TRASHCAN_END
}


AlifTypeObject _alifInstructionSequenceType_ = { // 414
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "تعليمات_تتالي",
	.basicSize = sizeof(AlifInstructionSequence),
	.dealloc = (Destructor)instSeq_dealloc,
	.getAttro = alifObject_genericGetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};
