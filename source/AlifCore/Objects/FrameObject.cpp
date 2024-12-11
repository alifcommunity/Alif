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









AlifFunctionObject* _alifFunction_fromConstructor(AlifFrameConstructor* _constr) { // 110
	AlifObject* module{};
	if (alifDict_getItemRef(_constr->globals, &ALIF_ID(__name__), &module) < 0) {
		return nullptr;
	}

	AlifFunctionObject* op_ = ALIFOBJECT_GC_NEW(AlifFunctionObject, &_alifFunctionType_);
	if (op_ == nullptr) {
		ALIF_XDECREF(module);
		return nullptr;
	}
	op_->globals = ALIF_NEWREF(_constr->globals);
	op_->builtins = ALIF_NEWREF(_constr->builtins);
	op_->name = ALIF_NEWREF(_constr->name);
	op_->qualname = ALIF_NEWREF(_constr->qualname);
	op_->code = ALIF_NEWREF(_constr->code);
	op_->defaults = ALIF_XNEWREF(_constr->defaults);
	op_->kwDefaults = ALIF_XNEWREF(_constr->kwDefaults);
	op_->closure = ALIF_XNEWREF(_constr->closure);
	op_->doc = ALIF_NEWREF(ALIF_NONE);
	op_->dict = nullptr;
	op_->weakRefList = nullptr;
	op_->module = module;
	op_->annotations = nullptr;
	op_->annotate = nullptr;
	op_->typeParams = nullptr;
	op_->vectorCall = alifFunction_vectorCall;
	op_->version = 0;

	ALIFOBJECT_GC_TRACK(op_);
	handle_funcEvent(AlifFunction_Event_Create, op_, nullptr);
	return op_;
}







AlifTypeObject _alifFunctionType_ = { // 1105
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "function",
	.basicSize = sizeof(AlifFunctionObject),
};









AlifObject* _alifEval_builtinsFromGlobals(AlifThread* _thread, AlifObject* _globals) { // 2131
	AlifObject* builtins = alifDict_getItemWithError(_globals, &ALIF_ID(__builtins__));
	if (builtins) {
		if (ALIFMODULE_CHECK(builtins)) {
			builtins = alifModule_getDict(builtins);
		}
		return builtins;
	}
	//if (alifErr_occurred()) {
	//	return nullptr;
	//}

	return alifEval_getBuiltins(_thread);
}

