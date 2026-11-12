#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_DescrObject.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_Tuple.h"


// 73 // alifCore_EmscriptenTrampoline
#define DESCR_SET_TRAMPOLINE_CALL(set, obj, value, closure) \
    (set)((obj), (value), (closure))

#define DESCR_GET_TRAMPOLINE_CALL(get, obj, closure) \
    (get)((obj), (closure))



static AlifObject* descr_name(AlifDescrObject *descr) { // 32
	if (descr->name != nullptr and ALIFUSTR_CHECK(descr->name))
		return descr->name;
	return nullptr;
}


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


static AlifObject* member_get(AlifObject* self, AlifObject* obj, AlifObject* type) { // 161
	AlifMemberDescrObject* descr = (AlifMemberDescrObject*)self;
	if (obj == nullptr) {
		return ALIF_NEWREF(descr);
	}
	if (descr_check((AlifDescrObject*)descr, obj) < 0) {
		return nullptr;
	}

	if (descr->member->flags & ALIF_AUDIT_READ) {
		//if (alifSys_audit("object.__getattr__", "Os",
		//	obj ? obj : ALIF_NONE, descr->member->name) < 0) {
		//	return nullptr;
		//}
	}

	return alifMember_getOne((char*)obj, descr->member);
}


static AlifObject* getset_get(AlifObject* self, AlifObject* obj, AlifObject* type) { // 182
	AlifGetSetDescrObject* descr = (AlifGetSetDescrObject*)self;
	if (obj == nullptr) {
		return ALIF_NEWREF(descr);
	}
	if (descr_check((AlifDescrObject*)descr, obj) < 0) {
		return nullptr;
	}
	if (descr->getSet->get != nullptr)
		return DESCR_GET_TRAMPOLINE_CALL(
			descr->getSet->get, obj, descr->getSet->closure);
	alifErr_format(_alifExcAttributeError_,
		"الصفة '%V' للكائن '%.100s' غير قابلة للقراءة",
		descr_name((AlifDescrObject*)descr), "?",
		ALIFDESCR_TYPE(descr)->name);
	return nullptr;
}


static AlifIntT descr_setCheck(AlifDescrObject* descr,
	AlifObject* obj, AlifObject* value) { // 215
	if (!ALIFOBJECT_TYPECHECK(obj, descr->type)) {
		alifErr_format(_alifExcTypeError_,
			"الواصف '%V' للكائنات '%.100s' "
			"لا يمكن تطبيقه على الكائن '%.100s'",
			descr_name(descr), "?",
			descr->type->name,
			ALIF_TYPE(obj)->name);
		return -1;
	}
	return 0;
}

static AlifIntT member_set(AlifObject* self, AlifObject* obj, AlifObject* value) { // 231
	AlifMemberDescrObject* descr = (AlifMemberDescrObject*)self;
	if (descr_setCheck((AlifDescrObject*)descr, obj, value) < 0) {
		return -1;
	}
	return alifMember_setOne((char*)obj, descr->member, value);
}


static AlifIntT getset_set(AlifObject* self, AlifObject* obj, AlifObject* value) { // 241
	AlifGetSetDescrObject* descr = (AlifGetSetDescrObject*)self;
	if (descr_setCheck((AlifDescrObject*)descr, obj, value) < 0) {
		return -1;
	}
	if (descr->getSet->set != nullptr) {
		return DESCR_SET_TRAMPOLINE_CALL(
			descr->getSet->set, obj, value,
			descr->getSet->closure);
	}
	alifErr_format(_alifExcAttributeError_,
		"الصفة '%V' للكائن '%.100s' غير قابلة للكتابة",
		descr_name((AlifDescrObject*)descr), "?",
		ALIFDESCR_TYPE(descr)->name);
	return -1;
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
	AlifObject* argstuple = _alifTuple_fromArray(args + 1, nargs - 1);
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
	AlifObject* argstuple = _alifTuple_fromArray(args + 1, nargs - 1);
	if (argstuple == nullptr) {
		return nullptr;
	}
	AlifObject* result = nullptr;
	AlifObject* kwdict = nullptr;
	if (kwnames != nullptr and ALIFTUPLE_GET_SIZE(kwnames) > 0) {
		kwdict = _alifStack_asDict(args + nargs, kwnames);
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
	.name = "معرف_وظيفة",
	.basicSize = sizeof(AlifMethodDescrObject),
	//.dealloc = descr_dealloc,
	.vectorCallOffset = offsetof(AlifMethodDescrObject, vectorCall),
	.call = alifVectorCall_call,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_METHOD_DESCRIPTOR,
	.descrGet = method_get,
};


AlifTypeObject _alifClassMethodDescrType_ = { // 756
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "معرف_وظيفة_صنف",
	.basicSize = sizeof(AlifMethodDescrObject),
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	.descrGet = classMethod_get,
};

AlifTypeObject _alifMemberDescrType_ = { // 793
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "واصف_عنصر",
	.basicSize = sizeof(AlifMemberDescrObject),
	//.dealloc = descr_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,

	.descrGet = member_get,
	.descrSet = member_set,
};

AlifTypeObject _alifGetSetDescrType_ = { // 830
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "واصف_جلب_ضبط",
	.basicSize = sizeof(AlifGetSetDescrObject),
	//.dealloc = descr_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	.descrGet = getset_get,
	.descrSet = getset_set,                                
};

AlifTypeObject _alifWrapperDescrType_ = { // 867
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "واصف_الغلاف",
	.basicSize = sizeof(AlifWrapperDescrObject),
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_METHOD_DESCRIPTOR,
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
		alifErr_format(_alifExcSystemError_,
			"الدالة %s(): استدعاء أعلام غير صحيحة", method->name);
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


AlifObject* alifDescr_newMember(AlifTypeObject* _type, AlifMemberDef* _member) { // 983
	AlifMemberDescrObject* descr{};

	if (_member->flags & ALIF_RELATIVE_OFFSET) {
		alifErr_setString(
			_alifExcSystemError_,
			"alifDescr_newMember مستخدم مع ALIF_RELATIVE_OFFSET");
		return nullptr;
	}
	descr = (AlifMemberDescrObject*)descr_new(&_alifMemberDescrType_,
		_type, _member->name);
	if (descr != nullptr)
		descr->member = _member;
	return (AlifObject*)descr;
}

AlifObject* alifDescr_newGetSet(AlifTypeObject* _type, AlifGetSetDef* _getset) { // 1001
	AlifGetSetDescrObject* descr{};

	descr = (AlifGetSetDescrObject*)descr_new(&_alifGetSetDescrType_,
		_type, _getset->name);
	if (descr != nullptr)
		descr->getSet = _getset;
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














static AlifObject* property_copy(AlifObject*, AlifObject*, AlifObject*, AlifObject*); // 1556


static AlifMemberDef _propertyMembers_[] = { // 1559
	{"fget", ALIF_T_OBJECT, offsetof(PropertyObject, propGet), ALIF_READONLY},
	{"fset", ALIF_T_OBJECT, offsetof(PropertyObject, propSet), ALIF_READONLY},
	{"fdel", ALIF_T_OBJECT, offsetof(PropertyObject, propDel), ALIF_READONLY},
	{"__doc__",  ALIF_T_OBJECT, offsetof(PropertyObject, propDoc), 0},
	{0}
};



static AlifObject* property_getter(AlifObject* _self,
	AlifObject* _getter) { // 1571
	return property_copy(_self, _getter, nullptr, nullptr);
}


static AlifMethodDef _propertyMethods_[] = { // 1622
	{"getter", property_getter, METHOD_O},
	//{"setter", property_setter, METHOD_O},
	//{"deleter", property_deleter, METHOD_O},
	//{"__set_name__", property_setName, METHOD_VARARGS},
	{0}
};



static AlifIntT property_name(PropertyObject* _prop, AlifObject** _name) { // 1645
	if (_prop->propName != nullptr) {
		*_name = ALIF_NEWREF(_prop->propName);
		return 1;
	}
	if (_prop->propGet == nullptr) {
		*_name = nullptr;
		return 0;
	}
	return alifObject_getOptionalAttr(_prop->propGet, &ALIF_STR(__name__), _name);
}

static AlifObject* property_descrGet(AlifObject* _self,
	AlifObject* _obj, AlifObject* _type) { // 1659
	if (_obj == nullptr or _obj == ALIF_NONE) {
		return ALIF_NEWREF(_self);
	}

	PropertyObject* gs = (PropertyObject*)_self;
	if (gs->propGet == nullptr) {
		AlifObject* propname{};
		if (property_name(gs, &propname) < 0) {
			return nullptr;
		}
		AlifObject* qualname = alifType_getQualName(ALIF_TYPE(_obj));
		if (propname != nullptr and qualname != nullptr) {
			alifErr_format(_alifExcAttributeError_,
				"الخاصية %R من الكائن %R لا تملك (جالب)",
				propname,
				qualname);
		}
		else if (qualname != nullptr) {
			alifErr_format(_alifExcAttributeError_,
				"خاصية الكائن %R لا تملك (جالب)",
				qualname);
		}
		else {
			alifErr_setString(_alifExcAttributeError_,
				"الخاصية لا تملك (جالب)");
		}
		ALIF_XDECREF(propname);
		ALIF_XDECREF(qualname);
		return nullptr;
	}

	return alifObject_callOneArg(gs->propGet, _obj);
}



static AlifObject* property_copy(AlifObject* _old,
	AlifObject* _get, AlifObject* _set, AlifObject* _del) { // 1761
	PropertyObject* pold = (PropertyObject*)_old;
	AlifObject* new_{}, * type{}, * doc{};

	type = alifObject_type(_old);
	if (type == nullptr)
		return nullptr;

	if (_get == nullptr or _get == ALIF_NONE) {
		_get = pold->propGet ? pold->propGet : ALIF_NONE;
	}
	if (_set == nullptr or _set == ALIF_NONE) {
		_set = pold->propSet ? pold->propSet : ALIF_NONE;
	}
	if (_del == nullptr or _del == ALIF_NONE) {
		_del = pold->propDel ? pold->propDel : ALIF_NONE;
	}
	if (pold->getterDoc and _get != ALIF_NONE) {
		/* make _init use __doc__ from getter */
		doc = ALIF_NONE;
	}
	else {
		doc = pold->propDoc ? pold->propDoc : ALIF_NONE;
	}

	new_ = alifObject_callFunctionObjArgs(type, _get, _set, _del, doc, nullptr);
	ALIF_DECREF(type);
	if (new_ == nullptr)
		return nullptr;

	if (alifObject_typeCheck((new_), &_alifPropertyType_)) {
		ALIF_XSETREF(((PropertyObject*)new_)->propName, ALIF_XNEWREF(pold->propName));
	}
	return new_;
}


static AlifIntT property_initImpl(PropertyObject* self, AlifObject* fget,
	AlifObject* fset, AlifObject* fdel, AlifObject* doc) { // 1835
	if (fget == ALIF_NONE)
		fget = nullptr;
	if (fset == ALIF_NONE)
		fset = nullptr;
	if (fdel == ALIF_NONE)
		fdel = nullptr;

	ALIF_XSETREF(self->propGet, ALIF_XNEWREF(fget));
	ALIF_XSETREF(self->propSet, ALIF_XNEWREF(fset));
	ALIF_XSETREF(self->propDel, ALIF_XNEWREF(fdel));
	ALIF_XSETREF(self->propDoc, nullptr);
	ALIF_XSETREF(self->propName, nullptr);

	self->getterDoc = 0;
	AlifObject* propDoc = nullptr;

	if (doc != nullptr and doc != ALIF_NONE) {
		propDoc = ALIF_XNEWREF(doc);
	}
	/* if no docstring given and the getter has one, use that one */
	else if (fget != nullptr) {
		int rc = alifObject_getOptionalAttr(fget, &ALIF_ID(__doc__), &propDoc);
		if (rc < 0) {
			return rc;
		}
		if (propDoc == ALIF_NONE) {
			propDoc = nullptr;
			ALIF_DECREF(ALIF_NONE);
		}
		if (propDoc != nullptr) {
			self->getterDoc = 1;
		}
	}

	if (ALIF_IS_TYPE(self, &_alifPropertyType_)) {
		ALIF_XSETREF(self->propDoc, propDoc);
	}
	else {

		if (propDoc == nullptr) {
			propDoc = ALIF_NEWREF(ALIF_NONE);
		}
		AlifIntT err = alifObject_setAttr(
			(AlifObject*)self, &ALIF_ID(__doc__), propDoc);
		ALIF_DECREF(propDoc);
		if (err < 0) {
			if (!self->getterDoc and
				alifErr_exceptionMatches(_alifExcAttributeError_)) {
				alifErr_clear();
				return 0;
			}
			else {
				return -1;
			}
		}
	}

	return 0;
}



#include "clinic/DescrObject.cpp.h" // 1993



AlifTypeObject _alifPropertyType_ = { // 2038
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "خاصية",
	.basicSize = sizeof(PropertyObject),
	/* methods */
	//.dealloc = property_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE,
	//.traverse = property_traverse,
	//.clear = property_clear,
	.methods = _propertyMethods_,
	.members = _propertyMembers_,
	//.getSet = property_getSetList,
	.descrGet = property_descrGet,
	//.descrSet = property_descrSet,
	.init = property_init,
	.alloc = alifType_genericAlloc,
	.new_ = alifType_genericNew,
	.free = alifObject_gcDel,
};










