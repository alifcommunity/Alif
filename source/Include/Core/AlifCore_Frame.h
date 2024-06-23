#pragma once

#include "AlifCore_Code.h"

class AlifFrameObject {
public:
	ALIFOBJECT_HEAD;
	AlifFrameObject* back;      /* previous frame, or nullptr */
	class AlifInterpreterFrame* frame; /* points to the frame data */
	AlifObject* trace;          /* Trace function */
	int lineNo;               /* Current line number. Only valid if non-zero */
	char traceLines;         /* Emit per-line trace events? */
	char traceOpCodes;       /* Emit per-opcode trace events? */
	AlifObject* extraLocals;   /* Dict for locals set by users using f_locals, could be NULL */
	/* The frame data, if this frame object owns the frame */
	AlifObject* frameData[1];
};





class AlifInterpreterFrame {
public:
	AlifObject* executable; /* Strong reference (code object or None) */
	AlifInterpreterFrame* previous;
	AlifObject* funcobj; /* Strong reference. Only valid if not on C stack */
	AlifObject* globals; /* Borrowed reference. Only valid if not on C stack */
	AlifObject* builtins; /* Borrowed reference. Only valid if not on C stack */
	AlifObject* locals; /* Strong reference, may be nullptr. Only valid if not on C stack */
	AlifFrameObject* frameObj; /* Strong reference, may be nullptr. Only valid if not on C stack */
	AlifCodeUnit* instrPtr; /* Instruction currently executing (or about to begin) */
	int stacktop;  /* Offset of TOS from localsplus  */
	uint16_t returnOffset;  /* Only relevant during a function call */
	char owner;
	/* Locals and stack */
	AlifObject* localsPlus[1];
};

enum FrameOwner {
	FRAME_OWNED_BY_THREAD = 0,
	FRAME_OWNED_BY_GENERATOR ,
	FRAME_OWNED_BY_FRAME_OBJECT,
	FRAME_OWNED_BY_CSTACK,
};

static inline AlifCodeObject* alifFrame_getCode(AlifInterpreterFrame* _f) { // 76
	return (AlifCodeObject*)_f->executable;
}

static inline void alifFrame_stackPush(AlifInterpreterFrame* _f, AlifObject* _value) { // 97
	_f->localsPlus[_f->stacktop] = _value;
	_f->stacktop++;
}

#define FRAME_SPECIALS_SIZE ((AlifIntT)((sizeof(AlifInterpreterFrame)-1)/sizeof(AlifObject *)))

static inline void alifFrame_initialize(AlifInterpreterFrame* _frame, AlifFunctionObject* _func,
	AlifObject* _locals, AlifCodeObject* _code, AlifIntT _nullLocalsFrom) { // 129
	_frame->funcobj = (AlifObject*)_func;
	_frame->executable = ALIF_NEWREF(_code);
	_frame->builtins = _func->funcBuiltins;
	_frame->globals = _func->funcGlobals;
	_frame->locals = _locals;
	_frame->stacktop = _code->nLocalsPlus;
	_frame->frameObj = nullptr;
	_frame->instrPtr = ALIFCODE_CODE(_code);
	_frame->returnOffset = 0;
	_frame->owner = FrameOwner::FRAME_OWNED_BY_THREAD;

	for (int i = _nullLocalsFrom; i < _code->nLocalsPlus; i++) {
		_frame->localsPlus[i] = NULL;
	}
}

static inline AlifObject** alifFrame_getStackPointer(AlifInterpreterFrame* _frame)
{
	AlifObject** sp = _frame->localsPlus + _frame->stacktop;
	_frame->stacktop = -1;
	return sp;
}

static inline bool alifThread_hasStackSpace(AlifThread* _thread, AlifIntT _size) { // 254
	return _thread->dataStackTop != nullptr and _size < _thread->dataStackLimit - _thread->dataStackTop;
}



extern AlifInterpreterFrame* alifThread_pushFrame(AlifThread*, AlifUSizeT);


AlifInterpreterFrame* alifEvalFrame_initAndPush(AlifThread*, AlifFunctionObject*,
	AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*);
