#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_Long.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"



static void notify_funcWatchers(AlifInterpreter* _interp, AlifFunctionWatchEvent _event,
	AlifFunctionObject* _func, AlifObject* _newValue) { // 25
	uint8_t bits = _interp->activeFuncWatchers;
	AlifIntT i_ = 0;
	while (bits) {
		if (bits & 1) {
			AlifFunctionWatchCallback cb_ = _interp->funcWatchers[i_];
		
			if (cb_(_event, _func, _newValue) < 0) {
				//alifErr_formatUnraisable(
					//"Exception ignored in %s watcher callback for function %U at %p",
					//func_eventName(event), func->funcQualname, func);
			}
		}
		i_++;
		bits >>= 1;
	}
}

static inline void handle_funcEvent(AlifFunctionWatchEvent _event, AlifFunctionObject* _func,
	AlifObject* _newValue) { // 48
	AlifInterpreter* interp = alifInterpreter_get();
	if (interp->activeFuncWatchers) {
		notify_funcWatchers(interp, _event, _func, _newValue);
	}
	switch (_event) {
	case AlifFunctionWatchEvent::AlifFunction_Event_Modify_Code:
	case AlifFunctionWatchEvent::AlifFunction_Event_Modify_Defaults:
	case AlifFunctionWatchEvent::AlifFunction_Event_Modify_KWDefaults:
		RARE_EVENT_INTERP_INC(interp, funcModification);
		break;
	default:
		break;
	}
}


AlifFunctionObject* _alifFunction_fromConstructor(AlifFrameConstructor* _constr) { // 103
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

AlifObject* alifFunction_newWithQualName(AlifObject* _code,
	AlifObject* _globals, AlifObject* _qualname) { // 140
	ALIF_INCREF(_globals);

	AlifFunctionObject* op{}; //* alif

	AlifThread* thread = _alifThread_get();

	AlifCodeObject* codeObj = (AlifCodeObject*)ALIF_NEWREF(_code);

	AlifObject* name = ALIF_NEWREF(codeObj->name);

	if (!_qualname) {
		_qualname = codeObj->qualname;
	}
	ALIF_INCREF(_qualname);

	AlifObject* consts = codeObj->consts;
	AlifObject* doc{};
	if (alifTuple_size(consts) >= 1) {
		doc = alifTuple_getItem(consts, 0);
		if (!ALIFUSTR_CHECK(doc)) {
			doc = ALIF_NONE;
		}
	}
	else {
		doc = ALIF_NONE;
	}
	ALIF_INCREF(doc);

	// __module__: Use globals['__name__'] if it exists, or nullptr.
	AlifObject* module{};
	AlifObject* builtins = nullptr;
	if (alifDict_getItemRef(_globals, &ALIF_ID(__name__), &module) < 0) {
		goto error;
	}

	builtins = _alifEval_builtinsFromGlobals(thread, _globals); // borrowed ref
	if (builtins == nullptr) {
		goto error;
	}
	ALIF_INCREF(builtins);

	op = ALIFOBJECT_GC_NEW(AlifFunctionObject, &_alifFunctionType_);
	if (op == nullptr) {
		goto error;
	}
	/* Note: No failures from this point on, since func_dealloc() does not
	   expect a partially-created object. */

	op->globals = _globals;
	op->builtins = builtins;
	op->name = name;
	op->qualname = _qualname;
	op->code = (AlifObject*)codeObj;
	op->defaults = nullptr;    // No default positional arguments
	op->kwDefaults = nullptr;  // No default keyword arguments
	op->closure = nullptr;
	op->doc = doc;
	op->dict = nullptr;
	op->weakRefList = nullptr;
	op->module = module;
	op->annotations = nullptr;
	op->annotate = nullptr;
	op->typeParams = nullptr;
	op->vectorCall = alifFunction_vectorCall;
	op->version = 0;
	if ((codeObj->flags & CO_NESTED) == 0) {
		alifObject_setDeferredRefcount((AlifObject*)op);
	}
	ALIFOBJECT_GC_TRACK(op);
	handle_funcEvent(AlifFunctionWatchEvent::AlifFunction_Event_Create, op, nullptr);
	return (AlifObject*)op;

error:
	ALIF_DECREF(_globals);
	ALIF_DECREF(codeObj);
	ALIF_DECREF(name);
	ALIF_DECREF(_qualname);
	ALIF_DECREF(doc);
	ALIF_XDECREF(module);
	ALIF_XDECREF(builtins);
	return nullptr;
}




void _alifFunction_setVersion(AlifFunctionObject* _func,
	uint32_t _version) { // 290
	_func->version = _version;
}


AlifObject* alifFunction_new(AlifObject* _code, AlifObject* _globals) { // 370
	return alifFunction_newWithQualName(_code, _globals, nullptr);
}



static AlifIntT func_clear(AlifObject* _self) { // 1019
	AlifFunctionObject* op = ALIFFUNCTION_CAST(_self);
	_alifFunction_setVersion(op, 0);
	ALIF_CLEAR(op->globals);
	ALIF_CLEAR(op->builtins);
	ALIF_CLEAR(op->module);
	ALIF_CLEAR(op->defaults);
	ALIF_CLEAR(op->kwDefaults);
	ALIF_CLEAR(op->doc);
	ALIF_CLEAR(op->dict);
	ALIF_CLEAR(op->closure);
	ALIF_CLEAR(op->annotations);
	ALIF_CLEAR(op->annotate);
	ALIF_CLEAR(op->typeParams);
	ALIF_SETREF(op->name, &ALIF_STR(Empty));
	ALIF_SETREF(op->qualname, &ALIF_STR(Empty));
	return 0;
}

static void func_dealloc(AlifObject* _self) { // 1044
	AlifFunctionObject* op = ALIFFUNCTION_CAST(_self);
	ALIF_SET_REFCNT(op, 1);
	handle_funcEvent(AlifFunctionWatchEvent::AlifFunction_Event_Destroy, op, nullptr);
	if (ALIF_REFCNT(op) > 1) {
		ALIF_SET_REFCNT(op, ALIF_REFCNT(op) - 1);
		return;
	}
	ALIF_SET_REFCNT(op, 0);
	ALIFOBJECT_GC_UNTRACK(op);
	if (op->weakRefList != nullptr) {
		alifObject_clearWeakRefs((AlifObject*)op);
	}
	_alifFunction_setVersion(op, 0);
	(void)func_clear((AlifObject*)op);
	// These aren't cleared by func_clear().
	ALIF_DECREF(op->code);
	ALIF_DECREF(op->name);
	ALIF_DECREF(op->qualname);
	alifObject_gcDel(op);
}








AlifTypeObject _alifFunctionType_ = { // 1105
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "دالة",
	.basicSize = sizeof(AlifFunctionObject),
	.dealloc = func_dealloc,
	.vectorCallOffset = offsetof(AlifFunctionObject, vectorCall),

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_METHOD_DESCRIPTOR,
};


class ClassMethod { // 1248
public:
	ALIFOBJECT_HEAD{};
	AlifObject* callable{};
	AlifObject* dict{};
};

// 1268
#define ALIFCLASSMETHOD_CAST(_cm) \
     ALIF_CAST(ClassMethod*, _cm))


AlifTypeObject _alifClassMethodType_ = { // 1395
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "صفة_صنف",
	.basicSize = sizeof(ClassMethod),
	//.dealloc = cm_dealloc,
	//.repr = cm_repr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = cm_traverse,
	//cm_clear,                   
	//.members = cm_memberlist, 
	//.getSet = cm_getsetlist,
	//.descrGet = cm_descr_get,
	.dictOffset = offsetof(ClassMethod, dict),
	//.init = cm_init,
	.alloc = alifType_genericAlloc,
	//.new_ = alifType_genericNew,
	.free = alifObject_gcDel,
};


AlifObject* alifClassMethod_new(AlifObject* _callable) { // 1437
	ClassMethod* cm = (ClassMethod*)
		alifType_genericAlloc(&_alifClassMethodType_, 0);
	if (cm != nullptr) {
		cm->callable = ALIF_NEWREF(_callable);
	}
	return (AlifObject*)cm;
}



class StaticMethod { // 1467
public:
	ALIFOBJECT_HEAD{};
	AlifObject* callable{};
	AlifObject* dict{};
};



AlifTypeObject _alifStaticMethodType_ = { // 1615
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "صفة_ساكنة",
	.basicSize = sizeof(StaticMethod),
	//sm_dealloc,
	//sm_repr,
	//sm_call,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC,
	//sm_traverse,
	//sm_clear,
	//sm_memberlist,
	//sm_getsetlist,
	//sm_descr_get,
	.dictOffset = offsetof(StaticMethod, dict),
	//sm_init,
	.alloc = alifType_genericAlloc,
	//alifType_genericNew,
	.free = alifObject_gcDel,
};



AlifObject* alifStaticMethod_new(AlifObject* callable) { // 1657
	StaticMethod* sm = (StaticMethod*)
		alifType_genericAlloc(&_alifStaticMethodType_, 0);
	if (sm != nullptr) {
		sm->callable = ALIF_NEWREF(callable);
	}
	return (AlifObject*)sm;
}
