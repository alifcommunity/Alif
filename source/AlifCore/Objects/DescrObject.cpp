#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_Tuple.h"






static AlifObject* method_vectorCallO(AlifObject* _func, AlifObject* const* _args,
	AlifUSizeT _nargsf, AlifObject* _kwnames) { // 452
	AlifThread* _thread = _alifThread_get();
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	if (method_checkArgs(_func, _args, nargs, _kwnames)) {
		return nullptr;
	}
	if (nargs != 2) {
		AlifObject* funcstr = _alifObject_functionStr(_func);
		if (funcstr != nullptr) {
			//alifErr_format(_alifExcTypeError_,
			//	"%U takes exactly one argument (%zd given)",
			//	funcstr, nargs - 1);
			ALIF_DECREF(funcstr);
		}
		return nullptr;
	}
	AlifCPPFunction meth = (AlifCPPFunction)method_enterCall(_thread, _func);
	if (meth == nullptr) {
		return nullptr;
	}
	AlifObject* result = ALIFCPPFUNCTION_TRAMPOLINECALL(meth, _args[0], _args[1]);
	_alif_leaveRecursiveCallThread(_thread);
	return result;
}





AlifTypeObject _alifMethodDescrType_ = { // 716
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "معرف_صفة",
	.basicSize = sizeof(AlifMethodDescrObject),
	//.dealloc = descr_dealloc,
	.vectorCallOffset = offsetof(AlifMethodDescrObject, vectorCall),
	//.repr = method_repr,
	//.call = alifVectorCall_call,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_METHOD_DESCRIPTOR,
	//.traverse = descr_traverse,
	//.methods = descr_methods,
	//.members = descr_members,
	//.getSet = method_getset,
	//.descrGet = method_get,
};


AlifTypeObject _alifClassMethodDescrType_ = { // 756
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "معرف_صفة_صنف",
	.basicSize = sizeof(AlifMethodDescrObject),
	//descr_dealloc,                              /* dealloc */
	//method_repr,                                /* repr */
	//classmethoddescr_call,                      /* call */
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = descr_traverse,
	//descr_members,                              /* members */
	//method_getset,                              /* getset */
	//classmethod_get,                            /* descr_get */
};



static AlifDescrObject* descr_new(AlifTypeObject* descrtype, AlifTypeObject* type, const char* name) { // 905
	AlifDescrObject* descr{};

	descr = (AlifDescrObject*)alifType_genericAlloc(descrtype, 0);
	if (descr != nullptr) {
		alifObject_setDeferredRefcount((AlifObject*)descr);
		descr->type = (AlifTypeObject*)ALIF_XNEWREF(type);
		descr->name = alifUStr_internFromString(name);
		if (descr->name == nullptr) {
			ALIF_SETREF(descr, nullptr);
		}
		else {
			descr->qualname = nullptr;
		}
	}
	return descr;
}


AlifObject* alifDescr_newMethod(AlifTypeObject* type, AlifMethodDef* method) { // 925
	VectorCallFunc vectorcall{};
	switch (method->flags & (METHOD_VARARGS | METHOD_FASTCALL | METHOD_NOARGS |
		METHOD_O | METHOD_KEYWORDS | METHOD_METHOD))
	{
	case METHOD_VARARGS:
		//vectorcall = method_vectorCallVarArgs;
		break;
	case METHOD_VARARGS | METHOD_KEYWORDS:
		//vectorcall = method_vectorCallVarArgsKeywords;
		break;
	case METHOD_FASTCALL:
		//vectorcall = method_vectorCallFastCall;
		break;
	case METHOD_FASTCALL | METHOD_KEYWORDS:
		//vectorcall = method_vectorCallFastCallKeywords;
		break;
	case METHOD_NOARGS:
		//vectorcall = method_vectorCallNoArgs;
		break;
	case METHOD_O:
		vectorcall = method_vectorCallO;
		break;
	case METHOD_METHOD | METHOD_FASTCALL | METHOD_KEYWORDS:
		//vectorcall = method_vectorCallFastCallKeywordsMethod;
		break;
	default:
		//alifErr_format(_alifExcSystemError_,
		//	"%s() method: bad call flags", method->name);
		return nullptr;
	}

	AlifMethodDescrObject* descr{};

	descr = (AlifMethodDescrObject*)descr_new(&_alifMethodDescrType_,
		type, method->name);
	if (descr != nullptr) {
		descr->method = method;
		descr->vectorCall = vectorcall;
	}
	return (AlifObject*)descr;
}



AlifObject* alifDescr_newClassMethod(AlifTypeObject* _type, AlifMethodDef* _method) { // 971
	AlifMethodDescrObject* descr{};

	descr = (AlifMethodDescrObject*)descr_new(&_alifClassMethodDescrType_,
		_type, _method->name);
	if (descr != nullptr)
		descr->method = _method;
	return (AlifObject*)descr;
}




AlifIntT alifDescr_isData(AlifObject* _ob) { // 1028
	return ALIF_TYPE(_ob)->descrSet != nullptr;
}
