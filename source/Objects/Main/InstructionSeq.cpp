#include "alif.h"

#include "AlifCore_Compile.h"
#include "AlifCore_OpCodeUtils.h"
#include "AlifCore_OpCodeData.h"


//typedef AlifInstruction AlifInstruction;
//typedef InstructionSequance InstructionSequance;
//typedef SourceLocation SourceLocation;



#define INITIAL_INSTRSEQUANCE_SIZE 100
#define INITIAL_INSTRSEQUENCE_LABELS_MAPSIZE 10


static AlifIntT instrSequance_nextInst(InstructionSequence* _seq) { // 35

	if (alifCompile_ensureArraySpace(
        _seq->used + 1,
        (void**)&_seq->instructions,
        &_seq->allocated,
        INITIAL_INSTRSEQUANCE_SIZE,
        sizeof(AlifInstruction)) == -1) return -1;

	return _seq->used++;
}

AlifIntT alifInstructionSequence_useLabel(InstructionSequence* _seq, AlifIntT _label) { // 57
    int old_size = _seq->labelMapSize;
    if(alifCompile_ensureArraySpace(_label,
            (void**)&_seq->labelMap,
            &_seq->labelMapSize,
            INITIAL_INSTRSEQUENCE_LABELS_MAPSIZE,
            sizeof(AlifIntT)) == -1) return -1;

    for (int i = old_size; i < _seq->labelMapSize; i++) {
        _seq->labelMap[i] = -111;  /* something weird, for debugging */
    }
    _seq->labelMap[_label] = _seq->used; /* label refers to the next instruction */
    return 1;
}

AlifIntT alifInstructionSeq_applyLableMap(InstructionSequence* _instr) { // 75

    if (_instr->labelMap == nullptr) return 1;

    for (AlifIntT i = 0; i < _instr->used; i++) {
        AlifInstruction* instr = &_instr->instructions[i];
        if (HAS_TARGET(instr->opCode)) {
            instr->opArg = _instr->labelMap[instr->opArg];
        }
        /* except handler here */
    }
    alifMem_dataFree(_instr->labelMap);
    _instr->labelMap = nullptr;
    _instr->labelMapSize = 0;
    return 1;
}


#define MAX_OPCODE 511

AlifIntT alifInstructionSequence_addOp(InstructionSequence* _seq,
	AlifIntT _opCode, AlifIntT _opArg, SourceLocation _loc) { // 104

	AlifIntT idx = instrSequance_nextInst(_seq);
	if (idx == -1) return -1;

	AlifInstruction* ci = &_seq->instructions[idx];
	ci->opCode = _opCode;
	ci->opArg = _opArg;
	ci->loc = _loc;

	return 1;
}

AlifIntT alifInstructionSequence_addNested(InstructionSequence* _seq,
    InstructionSequence* _nested) { // 146

    if (_seq->nested == nullptr) {
        _seq->nested = alifNew_list(0);
        if (_seq->nested == nullptr) {
            return -1;
        }
    }
    if (alifList_append(_seq->nested, (AlifObject*)_nested) < 0) {
        return -1;
    }
    return 1;
}

AlifObject* alifInstructionSequance_new() { // 197
	// short from inst_seq_create()

	InstructionSequence* seq = ALIFOBJECT_GC_NEW(InstructionSequence, &_alifInstructionSeqType_);
	if (seq == nullptr) return nullptr;

	seq->instructions = nullptr;
	seq->allocated = 0;
	seq->used = 0;
	seq->nextFreeLable = 0;
	seq->labelMap = nullptr;
	seq->labelMapSize = 0;
	seq->nested = nullptr;

	//alifObject_gcTrack(seq);
	return (AlifObject*)seq;
}






// need fix
AlifTypeObject _alifInstructionSeqType_ = {
    //ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
    ALIFVAROBJECT_HEAD_INIT(&_alifInstructionSeqType_, 0) // temp
    L"InstructionSequence",
    sizeof(InstructionSequence),
    0,
    (Destructor)0, /*tp_dealloc*/
    0,                  /*tp_vectorcall_offset*/
    0,                  /*tp_getattr*/
    0,                  /*tp_setattr*/
    0,                  /*tp_as_async*/
    0,                  /*tp_repr*/
    0,                  /*tp_as_number*/
    0,                  /*tp_as_sequence*/
    0,                  /*tp_as_mapping*/
    0,                  /* tp_hash */
    0,                  /* tp_call */
    0,                  /* tp_str */
    0,  /* tp_getattro */
    0,                  /* tp_setattro */
    0,                  /* tp_as_buffer */
    L".",/* tp_flags */
    0,                    /* tp_doc */
    nullptr,        /* tp_traverse */
    nullptr,                /* tp_clear */
    0,                                      /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */
    0,                       /* tp_methods */
    0,                    /* tp_members */
    0,                    /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,                                      /* tp_descr_get */
    0,                                      /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    0,                                      /* tp_init */
    0,                                      /* tp_alloc */
    0,                           /* tp_new */
};
