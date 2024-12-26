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

	AlifFunctionObject* op{}; // alif

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



static AlifIntT func_clear(AlifFunctionObject* _op) { // 1019
	_alifFunction_setVersion(_op, 0);
	ALIF_CLEAR(_op->globals);
	ALIF_CLEAR(_op->builtins);
	ALIF_CLEAR(_op->module);
	ALIF_CLEAR(_op->defaults);
	ALIF_CLEAR(_op->kwDefaults);
	ALIF_CLEAR(_op->doc);
	ALIF_CLEAR(_op->dict);
	ALIF_CLEAR(_op->closure);
	ALIF_CLEAR(_op->annotations);
	ALIF_CLEAR(_op->annotate);
	ALIF_CLEAR(_op->typeParams);
	ALIF_SETREF(_op->name, &ALIF_STR(Empty));
	ALIF_SETREF(_op->qualname, &ALIF_STR(Empty));
	return 0;
}

static void func_dealloc(AlifFunctionObject* _op) { // 1044
	ALIF_SET_REFCNT(_op, 1);
	handle_funcEvent(AlifFunctionWatchEvent::AlifFunction_Event_Destroy, _op, nullptr);
	if (ALIF_REFCNT(_op) > 1) {
		ALIF_SET_REFCNT(_op, ALIF_REFCNT(_op) - 1);
		return;
	}
	ALIF_SET_REFCNT(_op, 0);
	ALIFOBJECT_GC_UNTRACK(_op);
	if (_op->weakRefList != nullptr) {
		alifObject_clearWeakRefs((AlifObject*)_op);
	}
	_alifFunction_setVersion(_op, 0);
	(void)func_clear(_op);
	// These aren't cleared by func_clear().
	ALIF_DECREF(_op->code);
	ALIF_DECREF(_op->name);
	ALIF_DECREF(_op->qualname);
	alifObject_gcDel(_op);
}








AlifTypeObject _alifFunctionType_ = { // 1105
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "دالة",
	.basicSize = sizeof(AlifFunctionObject),
	.dealloc = (Destructor)func_dealloc,
	.vectorCallOffset = offsetof(AlifFunctionObject, vectorCall),

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_HAVE_VECTORCALL |
	ALIF_TPFLAGS_METHOD_DESCRIPTOR,
};




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
	//(Destructor)sm_dealloc,
	//(reprfunc)sm_repr,                          /* repr */
	//sm_call,                                    /* call */
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC,
	//staticmethod_doc,                           /* doc */
	//(TraverseProc)sm_traverse,                  /* traverse */
	//(Inquiry)sm_clear,                          /* clear */
	//sm_memberlist,							  /* members */
	//sm_getsetlist,                              /* getset */
	//sm_descr_get,                               /* descr_get */
	.dictOffset = offsetof(StaticMethod, dict),
	//sm_init,                                    /* init */
	.alloc = alifType_genericAlloc,
	//alifType_genericNew,                          /* new */
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
