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
} _PyInterpreterFrame;




static inline AlifCodeObject* _alifFrame_getCode(AlifInterpreterFrame* _f) { // 81
	return (AlifCodeObject*)_f->executable;
}



#define FRAME_SPECIALS_SIZE ((AlifIntT)((sizeof(AlifInterpreterFrame)-1)/sizeof(AlifObject*))) // 107













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
