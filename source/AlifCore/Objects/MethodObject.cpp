#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"



static AlifObject* cfunction_vectorCallFastCallKeywords(AlifObject*,
	AlifObject* const*, AlifUSizeT, AlifObject*); // 20










AlifObject* alifCPPMethod_new(AlifMethodDef* _ml,
	AlifObject* _self, AlifObject* _module, AlifTypeObject* _cls) { // 44
	/* Figure out correct vectorcall function to use */
	VectorCallFunc vectorCall{};
	switch (_ml->flags & (METHOD_VARARGS | METHOD_FASTCALL | METHOD_NOARGS |
		METHOD_O | METHOD_KEYWORDS | METHOD_METHOD))
	{
	case METHOD_VARARGS:
	case METHOD_VARARGS | METHOD_KEYWORDS:
		vectorCall = nullptr;
		break;
	case METHOD_FASTCALL:
		//vectorCall = cfunction_vectorCallFastCall;
		break;
	case METHOD_FASTCALL | METHOD_KEYWORDS:
		vectorCall = cfunction_vectorCallFastCallKeywords;
		break;
	case METHOD_NOARGS:
		//vectorCall = cfunction_vectorCallNoArgs;
		break;
	case METHOD_O:
		//vectorCall = cfunction_vectorCallO;
		break;
	case METHOD_METHOD | METHOD_FASTCALL | METHOD_KEYWORDS:
		//vectorCall = cfunction_vectorCallFastCallKeyWordsMethod;
		break;
	default:
		//alifErr_format(_alifExcSystemError_,
		//	"%s() method: bad call flags", ml->name);
		return nullptr;
	}

	AlifCPPFunctionObject* op_ = nullptr;

	if (_ml->flags & METHOD_METHOD) {
		if (!_cls) {
			//alifErr_setString(_alifExcSystemError_,
			//	"attempting to create AlifCMethod with a METH_METHOD "
			//	"flag but no class");
			return nullptr;
		}
		AlifCPPMethodObject* om = ALIFOBJECT_GC_NEW(AlifCPPMethodObject, &_alifCPPMethodType_);
		if (om == nullptr) {
			return nullptr;
		}
		om->class_ = (AlifTypeObject*)ALIF_NEWREF(_cls);
		op_ = (AlifCPPFunctionObject*)om;
	}
	else {
		if (_cls) {
			//alifErr_setString(_alifExcSystemError_,
			//	"attempting to create AlifCFunction with class "
			//	"but no METH_METHOD flag");
			return nullptr;
		}
		op_ = ALIFOBJECT_GC_NEW(AlifCPPFunctionObject, &_alifCPPFunctionType_);
		if (op_ == nullptr) {
			return nullptr;
		}
	}

	op_->weakRefList = nullptr;
	op_->ml_ = _ml;
	op_->self = ALIF_XNEWREF(_self);
	op_->module_ = ALIF_XNEWREF(_module);
	op_->vectorCall = vectorCall;
	ALIFOBJECT_GC_TRACK(op_);
	return (AlifObject*)op_;
}












AlifTypeObject _alifCPPFunctionType_ = { // 332
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "دالة_او_صفة_مضمنة",
	.basicSize = sizeof(AlifCPPFunctionObject),
	.itemSize = 0,

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_HAVE_VECTORCALL,
};



AlifTypeObject _alifCPPMethodType_ = { // 368
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "صفة_مضمنة",
	.basicSize = sizeof(AlifCPPMethodObject),
	.base = &_alifCPPFunctionType_,
};




typedef void (*FuncPtr)(void);

static inline FuncPtr cfunction_enterCall(AlifThread* _thread, AlifObject* _func) { // 401
	if (_alif_enterRecursiveCallThread(_thread, " while calling a Alif object")) {
		return nullptr;
	}
	return (FuncPtr)ALIFCPPFUNCTION_GET_FUNCTION(_func);
}




static AlifObject* cfunction_vectorCallFastCallKeywords(AlifObject* _func,
	AlifObject* const* _args, AlifUSizeT _nargsf, AlifObject* _kwnames) { // 430
	AlifThread* tstate = _alifThread_get();
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	AlifCPPFunctionFastWithKeywords meth = (AlifCPPFunctionFastWithKeywords)
		cfunction_enterCall(tstate, _func);
	if (meth == nullptr) {
		return nullptr;
	}
	AlifObject* result = meth(ALIFCPPFUNCTION_GET_SELF(_func), _args, nargs, _kwnames);
	_alif_leaveRecursiveCallThread(tstate);
	return result;
}
