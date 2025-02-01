#include "alif.h"

#include "FrameObject.h"
#include "AlifCore_Code.h"
#include "AlifCore_Frame.h"
#include "AlifCore_Object.h"

#include "Opcode.h"





AlifFrameObject* _alifFrame_makeAndSetFrameObject(AlifInterpreterFrame* _frame) { // 21
	//AlifObject* exc = alifErr_getRaisedException();

	AlifFrameObject* f = _alifFrame_newNoTrack(_alifFrame_getCode(_frame));
	if (f == nullptr) {
		//ALIF_XDECREF(exc);
		return nullptr;
	}
	//alifErr_setRaisedException(exc);
	f->frame = _frame;
	_frame->frameObj = f;
	return f;
}



static void take_ownership(AlifFrameObject* _f, AlifInterpreterFrame* _frame) { // 49
	AlifSizeT size = ((char*)_frame->stackPointer) - (char*)_frame;
	memcpy((AlifInterpreterFrame*)_f->frameData, _frame, size);
	_frame = (AlifInterpreterFrame*)_f->frameData;
	_frame->stackPointer = (AlifStackRef*)(((char*)_frame) + size);
	_frame->executable = alifStackRef_dup(_frame->executable);
	_f->frame = _frame;
	_frame->owner = FrameOwner::FRAME_OWNED_BY_FRAME_OBJECT;
	if (_alifFrame_isIncomplete(_frame)) {
		AlifCodeObject* code = _alifFrame_getCode(_frame);
		_frame->instrPtr = ALIFCODE_CODE(code) + code->firstTraceable + 1;
	}

	AlifInterpreterFrame* prev = _alifFrame_getFirstComplete(_frame->previous);
	_frame->previous = nullptr;
	if (prev) {
		AlifFrameObject* back = _alifFrame_getFrameObject(prev);
		if (back == nullptr) {
			//alifErr_clear();
		}
		else {
			_f->back = (AlifFrameObject*)ALIF_NEWREF(back);
		}
	}
	if (!ALIFOBJECT_GC_IS_TRACKED((AlifObject*)_f)) {
		ALIFOBJECT_GC_TRACK((AlifObject*)_f);
	}
}


void _alifFrame_clearLocals(AlifInterpreterFrame* _frame) { // 91
	AlifStackRef* sp = _frame->stackPointer;
	AlifStackRef* locals = _frame->localsPlus;
	_frame->stackPointer = locals;
	while (sp > locals) {
		sp--;
		ALIFSTACKREF_XCLOSE(*sp);
	}
	ALIF_CLEAR(_frame->locals);
}


void _alifFrame_clearExceptCode(AlifInterpreterFrame* _frame) { // 105
	if (_frame->frameObj) {
		AlifFrameObject* f = _frame->frameObj;
		_frame->frameObj = nullptr;
		if (ALIF_REFCNT(f) > 1) {
			take_ownership(f, _frame);
			ALIF_DECREF(f);
			return;
		}
		ALIF_DECREF(f);
	}
	_alifFrame_clearLocals(_frame);
	ALIFSTACKREF_CLEAR(_frame->funcObj);
}
