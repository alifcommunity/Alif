#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_Tuple.h"





static AlifIntT descr_check(AlifDescrObject* descr, AlifObject* obj) { // 78
	if (!ALIFOBJECT_TYPECHECK(obj, descr->type)) {
		//alifErr_format(_alifExcTypeError_,
		//	"descriptor '%V' for '%.100s' objects "
		//	"doesn't apply to a '%.100s' object",
		//	descr_name((AlifDescrObject*)descr), "?",
		//	descr->type->name,
		//	ALIF_TYPE(obj)->name);
		return -1;
	}
	return 0;
}

static AlifObject* classMethod_get(AlifObject* self, AlifObject* obj, AlifObject* type) { // 93
	AlifMethodDescrObject* descr = (AlifMethodDescrObject*)self;
	/* Ensure a valid type.  Class methods ignore obj. */
	if (type == nullptr) {
		if (obj != nullptr)
			type = (AlifObject*)ALIF_TYPE(obj);
		else {
			/* Wot - no type?! */
			//alifErr_format(_alifExcTypeError_,
			//	"descriptor '%V' for type '%.100s' "
			//	"needs either an object or a type",
			//	descr_name((AlifDescrObject*)descr), "?",
			//	ALIFDESCR_TYPE(descr)->name);
			return nullptr;
		}
	}
	if (!ALIFTYPE_CHECK(type)) {
		//alifErr_format(_alifExcTypeError_,
		//	"descriptor '%V' for type '%.100s' "
		//	"needs a type, not a '%.100s' as arg 2",
		//	descr_name((AlifDescrObject*)descr), "?",
		//	ALIFDESCR_TYPE(descr)->name,
		//	ALIF_TYPE(type)->name);
		return nullptr;
	}
	if (!alifType_isSubType((AlifTypeObject*)type, ALIFDESCR_TYPE(descr))) {
		//alifErr_format(_alifExcTypeError_,
		//	"descriptor '%V' requires a subtype of '%.100s' "
		//	"but received '%.100s'",
		//	descr_name((AlifDescrObject*)descr), "?",
		//	ALIFDESCR_TYPE(descr)->name,
		//	((AlifTypeObject*)type)->name);
		return nullptr;
	}
	AlifTypeObject* cls = nullptr;
	if (descr->method->flags & METHOD_METHOD) {
		cls = descr->common.type;
	}
	return alifCPPMethod_new(descr->method, type, nullptr, cls);
}

static AlifObject* method_get(AlifObject* self, AlifObject* obj, AlifObject* type) { // 136
	AlifMethodDescrObject* descr = (AlifMethodDescrObject*)self;
	if (obj == nullptr) {
		return ALIF_NEWREF(descr);
	}
	if (descr_check((AlifDescrObject*)descr, obj) < 0) {
		return nullptr;
	}
	if (descr->method->flags & METHOD_METHOD) {
		if (ALIFTYPE_CHECK(type)) {
			return alifCPPMethod_new(descr->method, obj, nullptr, descr->common.type);
		}
		else {
			//alifErr_format(_alifExcTypeError_,
			//	"descriptor '%V' needs a type, not '%s', as arg 2",
			//	descr_name((AlifDescrObject*)descr),
			//	ALIF_TYPE(type)->name);
			return nullptr;
		}
	}
	else {
		return ALIFCPPFUNCTION_NEWEX(descr->method, obj, nullptr);
	}
}



static inline AlifIntT method_checkArgs(AlifObject* func,
	AlifObject* const* args, AlifSizeT nargs, AlifObject* kwnames) { // 265
	if (nargs < 1) {
		AlifObject* funcstr = _alifObject_functionStr(func);
		if (funcstr != nullptr) {
			//alifErr_format(_alifExcTypeError_,
			//	"unbound method %U needs an argument", funcstr);
			ALIF_DECREF(funcstr);
		}
		return -1;
	}
	AlifObject* self = args[0];
	if (descr_check((AlifDescrObject*)func, self) < 0) {
		return -1;
	}
	if (kwnames and ALIFTUPLE_GET_SIZE(kwnames)) {
		AlifObject* funcstr = _alifObject_functionStr(func);
		if (funcstr != nullptr) {
			//alifErr_format(_alifExcTypeError_,
			//	"%U takes no keyword arguments", funcstr);
			ALIF_DECREF(funcstr);
		}
		return -1;
	}
	return 0;
}


typedef void (*FuncPtr)(void); // 294

static inline FuncPtr method_enterCall(AlifThread* _thread, AlifObject* _func) { // 296
	if (_alif_enterRecursiveCallThread(_thread, " while calling a Alif object")) {
		return nullptr;
	}
	return (FuncPtr)((AlifMethodDescrObject*)_func)->method->method;
}


static AlifObject* method_vectorCallVarArgs(AlifObject* func,
	AlifObject* const* args, AlifUSizeT nargsf, AlifObject* kwnames) { // 306
	AlifThread* thread = _alifThread_get();
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);
	if (method_checkArgs(func, args, nargs, kwnames)) {
		return nullptr;
	}
	AlifObject* argstuple = alifTuple_fromArray(args + 1, nargs - 1);
	if (argstuple == nullptr) {
		return nullptr;
	}
	AlifCPPFunction meth = (AlifCPPFunction)method_enterCall(thread, func);
	if (meth == nullptr) {
		ALIF_DECREF(argstuple);
		return nullptr;
	}
	AlifObject* result = ALIFCPPFUNCTION_TRAMPOLINECALL(
		meth, args[0], argstuple);
	ALIF_DECREF(argstuple);
	_alif_leaveRecursiveCallThread(thread);
	return result;
}


static AlifObject* method_vectorCallVarArgsKeywords(AlifObject* func,
	AlifObject* const* args, AlifUSizeT nargsf, AlifObject* kwnames) { // 331
	AlifThread* tstate = _alifThread_get();
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);

	AlifCPPFunctionWithKeywords meth{}; //* alif

	if (method_checkArgs(func, args, nargs, nullptr)) {
		return nullptr;
	}
	AlifObject* argstuple = alifTuple_fromArray(args + 1, nargs - 1);
	if (argstuple == nullptr) {
		return nullptr;
	}
	AlifObject* result = nullptr;
	AlifObject* kwdict = nullptr;
	if (kwnames != nullptr and ALIFTUPLE_GET_SIZE(kwnames) > 0) {
		kwdict = alifStack_asDict(args + nargs, kwnames);
		if (kwdict == nullptr) {
			goto exit;
		}
	}
	meth = (AlifCPPFunctionWithKeywords)
		method_enterCall(tstate, func);
	if (meth == nullptr) {
		goto exit;
	}
	result = ALIFCPPFUNCTIONWITHKEYWORDS_TRAMPOLINECALL(
		meth, args[0], argstuple, kwdict);
	_alif_leaveRecursiveCallThread(tstate);
exit:
	ALIF_DECREF(argstuple);
	ALIF_XDECREF(kwdict);
	return result;
}


static AlifObject* method_vectorCallFastCallKeywordsMethod(AlifObject* _func,
	AlifObject* const* _args, AlifUSizeT _nargsf, AlifObject* _kwnames) { // 367
	AlifThread* thread = _alifThread_get();
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	if (method_checkArgs(_func, _args, nargs, nullptr)) {
		return nullptr;
	}
	AlifCPPMethod meth = (AlifCPPMethod)method_enterCall(thread, _func);
	if (meth == nullptr) {
		return nullptr;
	}
	AlifObject* result = meth(_args[0],
		((AlifMethodDescrObject*)_func)->common.type,
		_args + 1, nargs - 1, _kwnames);
	_alif_leaveRecursiveCall();
	return result;
}


static AlifObject* method_vectorCallFastCall(AlifObject* _func,
	AlifObject* const* _args, AlifUSizeT _nargsf, AlifObject* _kwnames) { // 387
	AlifThread* thread = _alifThread_get();
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	if (method_checkArgs(_func, _args, nargs, _kwnames)) {
		return nullptr;
	}
	AlifCPPFunctionFast meth = (AlifCPPFunctionFast)
		method_enterCall(thread, _func);
	if (meth == nullptr) {
		return nullptr;
	}
	AlifObject* result = meth(_args[0], _args + 1, nargs - 1);
	_alif_leaveRecursiveCallThread(thread);
	return result;
}


static AlifObject* method_vectorCallFastCallKeywords(AlifObject* _func,
	AlifObject* const* _args, AlifUSizeT _nargsf, AlifObject* _kwnames) { // 406
	AlifThread* thread = _alifThread_get();
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	if (method_checkArgs(_func, _args, nargs, nullptr)) {
		return nullptr;
	}
	AlifCPPFunctionFastWithKeywords meth = (AlifCPPFunctionFastWithKeywords)
		method_enterCall(thread, _func);
	if (meth == nullptr) {
		return nullptr;
	}
	AlifObject* result = meth(_args[0], _args + 1, nargs - 1, _kwnames);
	_alif_leaveRecursiveCallThread(thread);
	return result;
}


static AlifObject* method_vectorCallNoArgs(AlifObject* _func,
	AlifObject* const* _args, AlifUSizeT _nargsf, AlifObject* _kwnames) { // 425
	AlifThread* thread = _alifThread_get();
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	if (method_checkArgs(_func, _args, nargs, _kwnames)) {
		return nullptr;
	}
	if (nargs != 1) {
		AlifObject* funcstr = _alifObject_functionStr(_func);
		if (funcstr != nullptr) {
			//alifErr_format(_alifExcTypeError_,
			//	"%U takes no arguments (%zd given)", funcstr, nargs - 1);
			ALIF_DECREF(funcstr);
		}
		return nullptr;
	}
	AlifCPPFunction meth = (AlifCPPFunction)method_enterCall(thread, _func);
	if (meth == nullptr) {
		return nullptr;
	}
	AlifObject* result = ALIFCPPFUNCTION_TRAMPOLINECALL(meth, _args[0], nullptr);
	_alif_leaveRecursiveCallThread(thread);
	return result;
}


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
	.descrGet = method_get,
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
	.descrGet = classMethod_get,
};





AlifTypeObject _alifWrapperDescrType_ = { // 867
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "واصف_الغلاف",
	.basicSize = sizeof(AlifWrapperDescrObject),
	//.dealloc = descr_dealloc,
	//.repr = wrapperDescr_repr,
	//.call = wrapperDescr_call,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_METHOD_DESCRIPTOR,
	//.traverse = descr_traverse,
	//.methods = descr_methods,
	//.members = descr_members,
	//.getSet = wrapperDescr_getset,
	//.descrGet = wrapperDescr_get,
};




static AlifDescrObject* descr_new(AlifTypeObject* descrtype,
	AlifTypeObject* type, const char* name) { // 905
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
		vectorcall = method_vectorCallVarArgs;
		break;
	case METHOD_VARARGS | METHOD_KEYWORDS:
		vectorcall = method_vectorCallVarArgsKeywords;
		break;
	case METHOD_FASTCALL:
		vectorcall = method_vectorCallFastCall;
		break;
	case METHOD_FASTCALL | METHOD_KEYWORDS:
		vectorcall = method_vectorCallFastCallKeywords;
		break;
	case METHOD_NOARGS:
		vectorcall = method_vectorCallNoArgs;
		break;
	case METHOD_O:
		vectorcall = method_vectorCallO;
		break;
	case METHOD_METHOD | METHOD_FASTCALL | METHOD_KEYWORDS:
		vectorcall = method_vectorCallFastCallKeywordsMethod;
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



AlifObject* alifDescr_newWrapper(AlifTypeObject* type,
	WrapperBase* base, void* wrapped) { // 1013
	AlifWrapperDescrObject* descr{};

	descr = (AlifWrapperDescrObject*)descr_new(&_alifWrapperDescrType_,
		type, base->name);
	if (descr != nullptr) {
		descr->base = base;
		descr->wrapped = wrapped;
	}
	return (AlifObject*)descr;
}


AlifIntT alifDescr_isData(AlifObject* _ob) { // 1028
	return ALIF_TYPE(_ob)->descrSet != nullptr;
}
