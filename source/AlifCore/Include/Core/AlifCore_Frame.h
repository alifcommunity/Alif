#pragma once




#include "AlifCore_Code.h"
#include "AlifCore_StackRef.h"






class Frame { // 21
public:
	ALIFOBJECT_HEAD;
	AlifFrameObject* back{};
	AlifInterpreterFrame* frame{};
	AlifObject* trace{};
	AlifIntT lineno{};
	char traceLines{};
	char traceOpcodes{};
	AlifObject* extraLocals{};
	AlifObject* localsCache{};
	AlifObject* frameData[1]{};
};



extern AlifFrameObject* _alifFrame_newNoTrack(AlifCodeObject*); // 38



enum FrameOwner { // 55
	FRAME_OWNED_BY_THREAD = 0,
	FRAME_OWNED_BY_GENERATOR = 1,
	FRAME_OWNED_BY_FRAME_OBJECT = 2,
	FRAME_OWNED_BY_CSTACK = 3,
};






class AlifInterpreterFrame { // 62
public:
	AlifObject* executable{};
	AlifInterpreterFrame* previous{};
	AlifObject* funcObj{};
	AlifObject* globals{};
	AlifObject* builtins{};
	AlifObject* locals{};
	AlifFrameObject* frameObj{};
	AlifCodeUnit* instrPtr{};
	AlifStackRef* stackPointer{};
	uint16_t returnOffset{};
	char owner{};
	/* Locals and stack */
	AlifStackRef localsPlus[1]{};
};




static inline AlifCodeObject* _alifFrame_getCode(AlifInterpreterFrame* _f) { // 81
	return (AlifCodeObject*)_f->executable;
}



#define FRAME_SPECIALS_SIZE ((AlifIntT)((sizeof(AlifInterpreterFrame)-1)/sizeof(AlifObject*))) // 107




static inline void _alifFrame_initialize(AlifInterpreterFrame* _frame,
	AlifFunctionObject* func, AlifObject* _locals, AlifCodeObject* code,
	AlifIntT _nullLocalsFrom, AlifInterpreterFrame* _previous) { // 144

	_frame->previous = _previous;
	_frame->funcObj = (AlifObject*)func;
	_frame->executable = ALIF_NEWREF(code);
	_frame->builtins = func->builtins;
	_frame->globals = func->globals;
	_frame->locals = _locals;
	_frame->stackPointer = _frame->localsPlus + code->nLocalsPlus;
	_frame->frameObj = nullptr;
	_frame->instrPtr = ALIFCODE_CODE(code);
	_frame->returnOffset = 0;
	_frame->owner = FrameOwner::FRAME_OWNED_BY_THREAD;

	for (AlifIntT i = _nullLocalsFrom; i < code->nLocalsPlus; i++) {
		_frame->localsPlus[i] = _alifStackRefNull_;
	}

#ifdef ALIF_GIL_DISABLED
	for (AlifIntT i = code->nLocalsPlus; i < code->nLocalsPlus + code->stackSize; i++) {
		_frame->localsPlus[i] = _alifStackRefNull_;
	}
#endif
}



static inline AlifStackRef* _alifFrame_getStackPointer(AlifInterpreterFrame* _frame) { // 188
	AlifStackRef* sp = _frame->stackPointer;
	_frame->stackPointer = nullptr;
	return sp;
}


static inline void _alifFrame_setStackPointer(AlifInterpreterFrame* _frame,
	AlifStackRef* _stackPointer) { // 197
	_frame->stackPointer = _stackPointer;
}




static inline bool _alifFrame_isIncomplete(AlifInterpreterFrame* _frame) { // 212
	if (_frame->owner == FrameOwner::FRAME_OWNED_BY_CSTACK) {
		return true;
	}
	return _frame->owner != FrameOwner::FRAME_OWNED_BY_GENERATOR and
		_frame->instrPtr < ALIFCODE_CODE(_alifFrame_getCode(_frame)) + _alifFrame_getCode(_frame)->firstTraceable;
}

static inline AlifInterpreterFrame* _alifFrame_getFirstComplete(AlifInterpreterFrame* _frame) { // 222
	while (_frame and _alifFrame_isIncomplete(_frame)) {
		_frame = _frame->previous;
	}
	return _frame;
}


static inline AlifInterpreterFrame* _alifThreadState_getFrame(AlifThread* _thread) { // 231
	return _alifFrame_getFirstComplete(_thread->currentFrame);
}

AlifFrameObject* _alifFrame_makeAndSetFrameObject(AlifInterpreterFrame*); // 240

static inline AlifFrameObject* _alifFrame_getFrameObject(AlifInterpreterFrame* _frame) { // 245

	AlifFrameObject* res = _frame->frameObj;
	if (res != nullptr) {
		return res;
	}
	return _alifFrame_makeAndSetFrameObject(_frame);
}


void _alifFrame_clearExceptCode(AlifInterpreterFrame*); // 269

static inline bool _alifThreadState_hasStackSpace(AlifThread* _tState, AlifIntT _size) { // 282
	return _tState->dataStackTop != nullptr and
		_size < _tState->dataStackLimit - _tState->dataStackTop;
}


extern AlifInterpreterFrame* _alifThreadState_pushFrame(AlifThread*, AlifUSizeT); // 294

void _alifThreadState_popFrame(AlifThread*, AlifInterpreterFrame*); // 296




AlifInterpreterFrame* _alifEval_framePushAndInit(AlifThread* _thread, AlifFunctionObject*,
	AlifObject*, AlifStackRef const*, AlifUSizeT, AlifObject*, AlifInterpreterFrame*); // 347
