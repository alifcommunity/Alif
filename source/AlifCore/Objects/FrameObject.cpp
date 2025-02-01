#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Code.h"
#include "AlifCore_Function.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpcodeMetaData.h"

#include "FrameObject.h"
#include "AlifCore_Frame.h"
#include "Opcode.h"










AlifTypeObject _alifFrameType_ = { // 1758
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "إطار",
	.basicSize = offsetof(AlifFrameObject, frameData) +
	offsetof(AlifInterpreterFrame, localsPlus),
	.itemSize = sizeof(AlifObject*),
	//.dealloc = (Destructor)frame_dealloc,
	//(ReprFunc)frame_repr,
	.getAttro = alifObject_genericGetAttr,
	.setAttro = alifObject_genericSetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//(TraverseProc)frame_traverse,
	//(Inquiry)frame_tpClear,
	//frame_memberList,
	//frame_getSetList,
};





AlifFrameObject* _alifFrame_newNoTrack(AlifCodeObject* _code) { // 1802
	AlifIntT slots = _code->nLocalsPlus + _code->stackSize;
	AlifFrameObject* f = ALIFOBJECT_GC_NEWVAR(AlifFrameObject, &_alifFrameType_, slots);
	if (f == nullptr) {
		return nullptr;
	}
	f->back = nullptr;
	f->trace = nullptr;
	f->traceLines = 1;
	f->traceOpcodes = 0;
	f->lineno = 0;
	f->extraLocals = nullptr;
	f->localsCache = nullptr;
	return f;
}











AlifObject* _alifEval_builtinsFromGlobals(AlifThread* _thread, AlifObject* _globals) { // 2131
	AlifObject* builtins = alifDict_getItemWithError(_globals, &ALIF_ID(__builtins__));
	if (builtins) {
		if (ALIFMODULE_CHECK(builtins)) {
			builtins = alifModule_getDict(builtins);
		}
		return builtins;
	}
	if (alifErr_occurred()) {
		return nullptr;
	}

	return _alifEval_getBuiltins(_thread);
}

