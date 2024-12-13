#include "alif.h"

#include "FrameObject.h"
#include "AlifCore_code.h"
#include "AlifCore_frame.h"
#include "AlifCore_Object.h"

#include "Opcode.h"












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
	ALIF_DECREF(_frame->funcObj);
}
