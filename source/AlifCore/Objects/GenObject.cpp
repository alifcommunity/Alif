#include "alif.h"

#include "AlifCore_Call.h"         
#include "AlifCore_Eval.h"        
#include "AlifCore_Frame.h"        
#include "AlifCore_FreeList.h"
#include "AlifCore_GC.h"           
#include "AlifCore_ModSupport.h"   
#include "AlifCore_Object.h"       
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_Errors.h"
#include "AlifCore_State.h"



// 26
#define ALIFGEN_CAST(_op) \
    ALIF_CAST(AlifGenObject*, (_op))
#define ALIFCOROOBJECT_CAST(_op) \
     ALIF_CAST(AlifCoroObject*, (_op)))
#define ALIFASYNCGENOBJECT_CAST(_op) \
    ALIF_CAST(AlifAsyncGenObject*, (_op))



static const char* _nonInitCoroMsg_ = "can't send non-None value to a "
"just-started coroutine"; // 35




static void gen_dealloc(AlifObject* self) { // 137
	AlifGenObject* gen = ALIFGEN_CAST(self);

	ALIFOBJECT_GC_UNTRACK(gen);

	if (gen->giWeakRefList != nullptr)
		alifObject_clearWeakRefs(self);

	ALIFOBJECT_GC_TRACK(self);

	if (alifObject_callFinalizerFromDealloc(self))
		return;                     /* resurrected.  :( */

	ALIFOBJECT_GC_UNTRACK(self);
	if (ALIFASYNCGEN_CHECKEXACT(gen)) {
		ALIF_CLEAR(((AlifAsyncGenObject*)gen)->agOriginOrFinalizer);
	}
	if (ALIFCORO_CHECKEXACT(gen)) {
		ALIF_CLEAR(((AlifCoroObject*)gen)->crOriginOrFinalizer);
	}
	if (gen->giFrameState != AlifFrameState_::Frame_Cleared) {
		AlifInterpreterFrame* frame = &gen->giIFrame;
		gen->giFrameState = AlifFrameState_::Frame_Cleared;
		frame->previous = nullptr;
		_alifFrame_clearExceptCode(frame);
		_alifErr_clearExcState(&gen->giExcState);
	}
	ALIFSTACKREF_CLEAR(gen->giIFrame.executable);
	ALIF_CLEAR(gen->giName);
	ALIF_CLEAR(gen->giQualname);

	alifObject_gcDel(gen);
}


static AlifSendResult gen_sendEx2(AlifGenObject* gen, AlifObject* arg,
	AlifObject** presult, AlifIntT exc, AlifIntT closing) { // 177
	AlifThread* thread = _alifThread_get();
	AlifInterpreterFrame* frame = &gen->giIFrame;

	*presult = nullptr;
	if (gen->giFrameState == AlifFrameState_::Frame_Created and arg and arg != ALIF_NONE) {
		const char* msg = "can't send non-None value to a "
			"just-started generator";
		if (ALIFCORO_CHECKEXACT(gen)) {
			msg = _nonInitCoroMsg_;
		}
		else if (ALIFASYNCGEN_CHECKEXACT(gen)) {
			msg = "can't send non-None value to a "
				"just-started async generator";
		}
		alifErr_setString(_alifExcTypeError_, msg);
		return AlifSendResult::AlifGen_Error;
	}
	if (gen->giFrameState == AlifFrameState_::Frame_Executing) {
		const char* msg = "generator already executing";
		if (ALIFCORO_CHECKEXACT(gen)) {
			msg = "coroutine already executing";
		}
		else if (ALIFASYNCGEN_CHECKEXACT(gen)) {
			msg = "async generator already executing";
		}
		alifErr_setString(_alifExcValueError_, msg);
		return AlifSendResult::AlifGen_Error;
	}
	if (FRAME_STATE_FINISHED(gen->giFrameState)) {
		if (ALIFCORO_CHECKEXACT(gen) && !closing) {
			//alifErr_setString(
			//	_alifExcRuntimeError_,
			//	"cannot reuse already awaited coroutine");
		}
		else if (arg and !exc) {
			*presult = ALIF_NEWREF(ALIF_NONE);
			return AlifSendResult::AlifGen_Return;
		}
		return AlifSendResult::AlifGen_Error;
	}

	AlifObject* argObj = arg ? arg : ALIF_NONE;
	_alifFrame_stackPush(frame, ALIFSTACKREF_FROMALIFOBJECTNEW(argObj));

	AlifErrStackItem* prevExcInfo = thread->excInfo;
	gen->giExcState.previousItem = prevExcInfo;
	thread->excInfo = &gen->giExcState;

	if (exc) {
		//_alifErr_chainStackItem();
	}

	gen->giFrameState = AlifFrameState_::Frame_Executing;
	AlifObject* result = alifEval_evalFrame(thread, frame, exc);

	if (result) {
		if (FRAME_STATE_SUSPENDED(gen->giFrameState)) {
			*presult = result;
			return AlifSendResult::AlifGen_Next;
		}
		if (result == ALIF_NONE and !ALIFASYNCGEN_CHECKEXACT(gen) && !arg) {
			ALIF_CLEAR(result);
		}
	}
	else {
	}

	*presult = result;
	return result ? AlifSendResult::AlifGen_Return : AlifSendResult::AlifGen_Error;
}




static AlifObject* gen_iterNext(AlifObject* _self) { // 605
	AlifGenObject* gen = ALIFGEN_CAST(_self);

	AlifObject* result{};
	if (gen_sendEx2(gen, nullptr, &result, 0, 0) == AlifSendResult::AlifGen_Return) {
		if (result != ALIF_NONE) {
			_alifGen_setStopIterationValue(result);
		}
		ALIF_CLEAR(result);
	}
	return result;
}


AlifIntT _alifGen_setStopIterationValue(AlifObject* _value) { // 627
	AlifObject* e{};

	if (_value == nullptr or
		(!ALIFTUPLE_CHECK(_value) and !ALIFEXCEPTIONINSTANCE_CHECK(_value))) {
		alifErr_setObject(_alifExcStopIteration_, _value);
		return 0;
	}
	e = alifObject_callOneArg(_alifExcStopIteration_, _value);
	if (e == nullptr) {
		return -1;
	}
	alifErr_setObject(_alifExcStopIteration_, e);
	ALIF_DECREF(e);
	return 0;
}




AlifTypeObject _alifGenType_ = { // 847
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مولد",
	.basicSize = offsetof(AlifGenObject, giIFrame.localsPlus),
	.itemSize = sizeof(AlifObject*),
	/* methods */
	.dealloc = gen_dealloc,
	//&_genAsAsync_,                              /* tp_as_async */
	//.repr = gen_repr,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = gen_traverse,
	.weakListOffset = offsetof(AlifGenObject, giWeakRefList),
	.iter = alifObject_selfIter,
	.iterNext = gen_iterNext,
	//.methods = _genMethods_,
	//.members = gen_memberlist,
	//.getSet = gen_getsetlist,
	//.finalize = _alifGen_finalize,
};

static AlifObject* make_gen(AlifTypeObject* type, AlifFunctionObject* func) { // 900
	AlifCodeObject* code = (AlifCodeObject*)func->code;
	AlifIntT slots = _alifFrame_numSlotsForCodeObject(code);
	AlifGenObject* gen = ALIFOBJECT_GC_NEWVAR(AlifGenObject, type, slots);
	if (gen == nullptr) {
		return nullptr;
	}
	gen->giFrameState = AlifFrameState_::Frame_Cleared;
	gen->giWeakRefList = NULL;
	gen->giExcState.excValue = NULL;
	gen->giExcState.previousItem = NULL;
	gen->giName = ALIF_NEWREF(func->name);
	gen->giQualname = ALIF_NEWREF(func->qualname);
	ALIFOBJECT_GC_TRACK(gen);
	return (AlifObject*)gen;
}

static AlifObject* compute_crOrigin(AlifIntT, AlifInterpreterFrame*); // 922

AlifObject* _alif_makeCoro(AlifFunctionObject* _func) { // 924
	AlifIntT coro_flags = ((AlifCodeObject*)_func->code)->flags &
		(CO_GENERATOR | CO_COROUTINE | CO_ASYNC_GENERATOR);
	if (coro_flags == CO_GENERATOR) {
		return make_gen(&_alifGenType_, _func);
	}
	if (coro_flags == CO_ASYNC_GENERATOR) {
		AlifAsyncGenObject* ag{};
		ag = (AlifAsyncGenObject*)make_gen(&_alifAsyncGenType_, _func);
		if (ag == nullptr) {
			return nullptr;
		}
		ag->agOriginOrFinalizer = nullptr;
		ag->agClosed = 0;
		ag->agHooksInited = 0;
		ag->agRunningAsync = 0;
		return (AlifObject*)ag;
	}

	AlifObject* coro = make_gen(&_alifCoroType_, _func);
	if (!coro) {
		return nullptr;
	}
	AlifThread* thread = _alifThread_get();
	AlifIntT originDepth = thread->coroutineOriginTrackingDepth;

	if (originDepth == 0) {
		((AlifCoroObject*)coro)->crOriginOrFinalizer = nullptr;
	}
	else {
		AlifInterpreterFrame* frame = thread->currentFrame;
		frame = _alifFrame_getFirstComplete(frame->previous);
		AlifObject* cr_origin = compute_crOrigin(originDepth, frame);
		((AlifCoroObject*)coro)->crOriginOrFinalizer = cr_origin;
		if (!cr_origin) {
			ALIF_DECREF(coro);
			return nullptr;
		}
	}
	return coro;
}






AlifTypeObject _alifCoroType_ = { // 1207
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "الإجراء_الفرعي",
	.basicSize = offsetof(AlifCoroObject, crIFrame.localsPlus),
	.itemSize = sizeof(AlifObject*),
	/* methods */
	//.dealloc = gen_dealloc,
	//&_coroAsAsync_,                             /* tp_as_async */
	//.repr = coro_repr,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = gen_traverse,
	.weakListOffset = offsetof(AlifCoroObject, crWeakRefList),
	//.methods = _coroMethods_,
	//.members = _coroMemberList_,
	//.getSet = _coroGetSetList_,
	//.finalize = _alifGen_Finalize,
};







static AlifObject* compute_crOrigin(AlifIntT _originDepth,
	AlifInterpreterFrame* _currentFrame) { // 1354
	AlifInterpreterFrame* frame = _currentFrame;
	/* First count how many frames we have */
	AlifIntT frameCount = 0;
	for (; frame and frameCount < _originDepth; ++frameCount) {
		frame = _alifFrame_getFirstComplete(frame->previous);
	}

	/* Now collect them */
	AlifObject* crOrigin = alifTuple_new(frameCount);
	if (crOrigin == nullptr) {
		return nullptr;
	}
	frame = _currentFrame;
	for (AlifIntT i = 0; i < frameCount; ++i) {
		AlifCodeObject* code = _alifFrame_getCode(frame);
		AlifIntT line = alifUnstable_interpreterFrameGetLine(frame);
		AlifObject* frameinfo = alif_buildValue("OiO", code->filename, line,
			code->name);
		if (!frameinfo) {
			ALIF_DECREF(crOrigin);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(crOrigin, i, frameinfo);
		frame = _alifFrame_getFirstComplete(frame->previous);
	}

	return crOrigin;
}








AlifTypeObject _alifAsyncGenType_ = { // 1638
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مولد_متزامن",
	.basicSize = offsetof(AlifAsyncGenObject, agIFrame.localsPlus),
	.itemSize = sizeof(AlifObject*),
	/* methods */
	//.dealloc = gen_dealloc,
	//&_asyncGenAsAsync_,                        /* tp_as_async */
	//.repr = async_genRepr,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = async_genTraverse,
	.weakListOffset = offsetof(AlifAsyncGenObject, agWeakRefList),
	//.methods = _asyncGenMethods_,
	//.members = _asyncGenMemberList_,
	//.getSet = _asyncGenGetSetList_,
	//.finalize = _alifGen_finalize,
};
