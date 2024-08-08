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
	AlifObject* extraLocals;   /* Dict for locals set by users using f_locals, could be nullptr */
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

static inline AlifCodeObject* alifFrame_getCode(AlifInterpreterFrame* _f) { 
	return (AlifCodeObject*)_f->executable;
}

static inline void alifFrame_stackPush(AlifInterpreterFrame* _f, AlifObject* _value) { 
	_f->localsPlus[_f->stacktop] = _value;
	_f->stacktop++;
}

#define FRAME_SPECIALS_SIZE ((AlifIntT)((sizeof(AlifInterpreterFrame)-1)/sizeof(AlifObject *)))

static inline void alifFrame_initialize(AlifInterpreterFrame* _frame, AlifFunctionObject* _func,
	AlifObject* _locals, AlifCodeObject* _code, AlifIntT _nullLocalsFrom) { 
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
		_frame->localsPlus[i] = nullptr;
	}
}

static inline bool alifFrame_isIncomplete(AlifInterpreterFrame* _frame) { 
	if (_frame->owner == FRAME_OWNED_BY_CSTACK) {
		return true;
	}
	return _frame->owner != FRAME_OWNED_BY_GENERATOR and
		_frame->instrPtr < ALIFCODE_CODE(alifFrame_getCode(_frame)) + alifFrame_getCode(_frame)->firstTraceable;
}

static inline AlifInterpreterFrame* alifFrame_getFirstComplete(AlifInterpreterFrame* _frame) { 
	while (_frame and alifFrame_isIncomplete(_frame)) {
		_frame = _frame->previous;
	}
	return _frame;
}

static inline AlifInterpreterFrame* alifThread_getFrame(AlifThread* _thread) { 
	return alifFrame_getFirstComplete(_thread->currentFrame);
}

static inline AlifObject** alifFrame_getStackPointer(AlifInterpreterFrame* _frame)
{
	AlifObject** sp = _frame->localsPlus + _frame->stacktop;
	_frame->stacktop = -1;
	return sp;
}

static inline bool alifThread_hasStackSpace(AlifThread* _thread, AlifIntT _size) { 
	return _thread->dataStackTop != nullptr and _size < _thread->dataStackLimit - _thread->dataStackTop;
}



extern AlifInterpreterFrame* alifThread_pushFrame(AlifThread*, AlifUSizeT);


AlifInterpreterFrame* alifEvalFrame_initAndPush(AlifThread*, AlifFunctionObject*,
	AlifObject*, AlifObject* const*, AlifUSizeT, AlifObject*);
