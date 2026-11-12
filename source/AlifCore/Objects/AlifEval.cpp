#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Audit.h"
#include "AlifCore_Backoff.h"
#include "AlifCore_Call.h"
#include "AlifCore_Cell.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Code.h"

#include "AlifCore_Function.h"
#include "AlifCore_Instruments.h"
#include "AlifCore_Intrinsics.h"
#include "AlifCore_Long.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpcodeMetaData.h"
#include "AlifCore_Optimizer.h"
#include "AlifCore_OpcodeUtils.h"
#include "AlifCore_Errors.h"

#include "AlifCore_State.h"
#include "AlifCore_SetObject.h"
#include "AlifCore_SliceObject.h"

#include "AlifCore_Dict.h"

#include "AlifCore_Frame.h"

#include "Opcode.h"



// 62
#undef ALIF_IS_TYPE
#define ALIF_IS_TYPE(_ob, _type) \
    (ALIFOBJECT_CAST(_ob)->type == _type)

#undef ALIF_XDECREF
#define ALIF_XDECREF(_arg) \
    do { \
        AlifObject *xop = ALIFOBJECT_CAST(_arg); \
        if (xop != nullptr) { \
            ALIF_DECREF(xop); \
        } \
    } while (0)

 // 107
#undef ALIF_DECREF
#define ALIF_DECREF(_arg) \
    do { \
        AlifObject *op = ALIFOBJECT_CAST(_arg); \
        uint32_t local = alifAtomic_loadUint32Relaxed(&op->refLocal); \
        if (local == ALIF_IMMORTAL_REFCNT_LOCAL) { \
            break; \
        } \
        if (alif_isOwnedByCurrentThread(op)) { \
            local--; \
            alifAtomic_storeUint32Relaxed(&op->refLocal, local); \
            if (local == 0) { \
                alif_mergeZeroLocalRefcount(op); \
            } \
        } \
        else { \
            alif_decRefShared(op); \
        } \
    } while (0)







static AlifIntT get_exceptionHandler(AlifCodeObject*, AlifIntT, AlifIntT*, AlifIntT*, AlifIntT*); // 277
static AlifInterpreterFrame* _alifEval_framePushAndInitEx(AlifThread*, AlifStackRef, AlifObject*,
	AlifSizeT, AlifObject*, AlifObject*, AlifInterpreterFrame*); // 279


AlifIntT _alif_checkRecursiveCall(AlifThread* _thread, const char* _where) { // 305
#ifdef USE_STACKCHECK
	if (alifOS_checkStack()) {
		++_thread->cppRecursionRemaining;
		alifErr_setString(_thread, _alifExcMemoryError_, "Stack overflow");
		return -1;
	}
#endif
	if (_thread->recursionHeadroom) {
		if (_thread->cppRecursionRemaining < -50) {
			//alif_fatalError("Cannot recover from stack overflow.");
		}
	}
	else {
		if (_thread->cppRecursionRemaining <= 0) {
			_thread->recursionHeadroom++;
			//alifErr_format(_thread, _alifExcRecursionError_,
			//	"maximum recursion depth exceeded%s",
			//	where);
			_thread->recursionHeadroom--;
			++_thread->cppRecursionRemaining;
			return -1;
		}
	}
	return 0;
}


const BinaryFunc _alifEvalBinaryOps_[] = { // 307
	alifNumber_add, // NB_ADD
	alifNumber_and, // NB_AND
	alifNumber_floorDivide, // NB_FLOOR_DIVIDE
	alifNumber_lshift, // NB_LSHIFT
	nullptr, // alifNumber_matrixMultiply, // NB_MATRIX_MULTIPLY
	alifNumber_multiply, // NB_MULTIPLY
	alifNumber_remainder, // NB_REMAINDER
	alifNumber_or, // NB_OR
	_alifNumber_powerNoMod, // NB_POWER
	alifNumber_rshift, // NB_RSHIFT
	alifNumber_subtract, // NB_SUBTRACT
	alifNumber_trueDivide, // NB_TRUE_DIVIDE
	alifNumber_xor, // NB_XOR
	alifNumber_inPlaceAdd, // NB_INPLACE_ADD
	alifNumber_inPlaceAnd, // NB_INPLACE_AND
	alifNumber_inPlaceFloorDivide, // NB_INPLACE_FLOOR_DIVIDE
	alifNumber_inPlaceLshift, // NB_INPLACE_LSHIFT
	nullptr, // alifNumber_inPlaceMatrixMultiply, // NB_INPLACE_MATRIX_MULTIPLY
	alifNumber_inPlaceMultiply, // NB_INPLACE_MULTIPLY
	alifNumber_inPlaceRemainder, // NB_INPLACE_REMAINDER
	alifNumber_inPlaceOr, // NB_INPLACE_OR
	_alifNumber_inPlacePowerNoMod, // NB_INPLACE_POWER
	alifNumber_inPlaceRshift, // NB_INPLACE_RSHIFT
	alifNumber_inPlaceSubtract, // NB_INPLACE_SUBTRACT
	alifNumber_inPlaceTrueDivide, // NB_INPLACE_TRUE_DIVIDE
	alifNumber_inPlaceXor, // NB_INPLACE_XOR
};


const AlifSpecialMethod _alifSpecialMethods_[] = { // 378
	/*SPECIAL___ENTER__*/ {
		.name = &ALIF_STR(__enter__),
		.error = "الكائن '%.200s' لا يدعم "
		"مدير السياق (الوظيفة __ادخل__ مفقودة)",
	},
	/*SPECIAL___EXIT__*/ {
			.name = &ALIF_STR(__exit__),
			.error = "الكائن '%.200s' لا يدعم "
			"مدير السياق (الوظيفة __اخرج__ مفقودة)",
	},
	///*SPECIAL___AENTER__*/ {
	//		.name = &ALIF_STR(__aenter__),
	//		.error = "'%.200s' object does not support the asynchronous "
	//		"context manager protocol (missed __aenter__ method)",
	//},
	///*SPECIAL___AEXIT__*/ {
	//		.name = &ALIF_STR(__aexit__),
	//		.error = "'%.200s' object does not support the asynchronous "
	//		"context manager protocol (missed __aexit__ method)",
	//}
};

const AlifUSizeT _alifFunctionAttributeOffsets_[] = { // 396 //* alif
	0,
	offsetof(AlifFunctionObject, defaults), // MAKE_FUNCTION_DEFAULTS
	offsetof(AlifFunctionObject, kwDefaults), // MAKE_FUNCTION_KWDEFAULTS
	0,
	offsetof(AlifFunctionObject, annotations), // MAKE_FUNCTION_ANNOTATIONS
	0,0,0,
	offsetof(AlifFunctionObject, closure), // MAKE_FUNCTION_CLOSURE
	0,
	offsetof(AlifFunctionObject, annotate), // MAKE_FUNCTION_ANNOTATE
};


static AlifIntT do_raise(AlifThread*, AlifObject*, AlifObject*); // 633

AlifObject* alifEval_evalCode(AlifObject* _co,
	AlifObject* _globals, AlifObject* _locals) { // 592
	AlifThread* thread = _alifThread_get();
	if (_locals == nullptr) {
		_locals = _globals;
	}
	AlifObject* builtins = _alifDict_loadBuiltinsFromGlobals(_globals);
	if (builtins == nullptr) {
		return nullptr;
	}
	AlifFrameConstructor desc = {
		.globals = _globals,
		.builtins = builtins,
		.name = ((AlifCodeObject*)_co)->name,
		.qualname = ((AlifCodeObject*)_co)->name,
		.code = _co,
		.defaults = nullptr,
		.kwDefaults = nullptr,
		.closure = nullptr
	};
	AlifFunctionObject* func = _alifFunction_fromConstructor(&desc);
	_alif_decrefBuiltins(builtins);
	if (func == nullptr) {
		return nullptr;
	}
	AlifObject* res = _alifEval_vector(thread, func, _locals, nullptr, 0, nullptr);
	ALIF_DECREF(func);
	return res;
}



#include "AlifEvalMacros.h" // 641

AlifIntT _alif_checkRecursiveCallAlif(AlifThread* _thread) { // 643
	if (_thread->recursionHeadroom) {
		if (_thread->alifRecursionRemaining < -50) {
			//alif_fatalError("Cannot recover from Alif stack overflow.");
		}
	}
	else {
		if (_thread->alifRecursionRemaining <= 0) {
			_thread->recursionHeadroom++;
			//_alifErr_format(_thread, _alifExcRecursionError_,
			//	"maximum recursion depth exceeded");
			_thread->recursionHeadroom--;
			return -1;
		}
	}
	return 0;
}

static const AlifCodeUnit _alifInterpreterTrampolineInstructions_[] = { // 664
	/* Put a NOP at the start, so that the IP points into
	* the code, rather than before it */
	{.op{.code = NOP, .arg = 0 } },
	{.op{.code = INTERPRETER_EXIT, .arg = 0 } },  /* reached on return */
	{.op{.code = NOP, .arg = 0 } },
	{.op{.code = INTERPRETER_EXIT, .arg = 0 } },  /* reached on yield */
	{.op{.code = RESUME, .arg = RESUME_OPARG_DEPTH1_MASK | RESUME_AT_FUNC_START } }
};


AlifObject** _alifObjectArray_fromStackRefArray(AlifStackRef* _input,
	AlifSizeT _nargs, AlifObject** _scratch) { // 692
	AlifObject** result{};
	if (_nargs > MAX_STACKREF_SCRATCH) {
		// +1 in case ALIF_VECTORCALL_ARGUMENTS_OFFSET is set.
		result = (AlifObject**)alifMem_dataAlloc((_nargs + 1) * sizeof(AlifObject*));
		if (result == nullptr) {
			return nullptr;
		}
		result++;
	}
	else {
		result = _scratch;
	}
	for (AlifIntT i = 0; i < _nargs; i++) {
		result[i] = alifStackRef_asAlifObjectBorrow(_input[i]);
	}
	return result;
}


void _alifObjectArray_free(AlifObject** _array, AlifObject** _scratch) { // 713
	if (_array != _scratch) {
		alifMem_dataFree(_array);
	}
}


#define ALIF_EVAL_CPP_STACK_UNITS 2 // 724

//  هذه يجب أن تكون في ملف منفصل مع الأوامر
#ifdef TIER_TWO
#error "This file is for Tier 1 only"
#endif
#define TIER_ONE 1



AlifObject* ALIF_HOT_FUNCTION alifEval_evalFrameDefault(AlifThread* _thread,
	AlifInterpreterFrame* _frame, AlifIntT _throwflag) { // 726

#ifdef ALIF_STATS
	AlifIntT lastOpcode = 0;
#endif
	uint8_t opcode{};			/* Current opcode */
	AlifIntT oparg{};			/* Current opcode argument, if any */
#ifdef LLTRACE
	AlifIntT lltrace = 0;
#endif

	AlifInterpreterFrame  entryFrame{};

	entryFrame.executable = ALIFSTACKREF_NONE;
	entryFrame.instrPtr = (AlifCodeUnit*)_alifInterpreterTrampolineInstructions_ + 1;
	entryFrame.stackPointer = entryFrame.localsPlus;
	entryFrame.owner = FrameOwner::FRAME_OWNED_BY_CSTACK;
	entryFrame.returnOffset = 0;
	/* Push frame */
	entryFrame.previous = _thread->currentFrame;
	_frame->previous = &entryFrame;
	_thread->currentFrame = _frame;

	_thread->cppRecursionRemaining -= (ALIF_EVAL_CPP_STACK_UNITS - 1);
	if (_alif_enterRecursiveCallThread(_thread, "")) {
		_thread->cppRecursionRemaining--;
		_thread->alifRecursionRemaining--;
		goto exit_unwind;
	}

	if (_throwflag) {
		if (_alif_enterRecursiveAlif(_thread)) {
			goto exit_unwind;
		}
		_alif_instrument(_alifFrame_getCode(_frame), _thread->interpreter);
		//monitor_throw(_thread, _frame, _frame->instrPtr);
		goto resume_with_error;
	}


	AlifCodeUnit* nextInstr;
	AlifStackRef* stackPointer;



start_frame:
	if (_alif_enterRecursiveAlif(_thread)) {
		goto exit_unwind;
	}

	nextInstr = _frame->instrPtr;
resume_frame:
	stackPointer = _alifFrame_getStackPointer(_frame);

	//#ifdef LLTRACE
	//	lltrace = maybe_lltrace_resume_frame(frame, &entry_frame, GLOBALS());
	//	if (lltrace < 0) {
	//		goto exit_unwind;
	//	}
	//#endif

	DISPATCH();

	{
		/* Start instructions */
	#if !USE_COMPUTED_GOTOS
		dispatch_opcode :
		switch (opcode)
		#endif
		{
			TARGET(CACHE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				//ALIF_FATALERROR("Executing a cache.");
				printf("Executing a cache. error\n");
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_SLICE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef container{};
				AlifStackRef start{};
				AlifStackRef stop{};
				AlifStackRef res{};
				// _SPECIALIZE_BINARY_SLICE
				{
					// Placeholder until we implement BINARY_SLICE specialization
				#if ENABLE_SPECIALIZATION
					OPCODE_DEFERRED_INC(BINARY_SLICE);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _BINARY_SLICE
				{
					stop = stackPointer[-1];
					start = stackPointer[-2];
					container = stackPointer[-3];
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* slice = _alifBuildSlice_consumeRefs(alifStackRef_asAlifObjectSteal(start),
						alifStackRef_asAlifObjectSteal(stop));
					stackPointer = _alifFrame_getStackPointer(_frame);
					AlifObject* resObj{};
					// Can't use ERROR_IF() here, because we haven't
					// DECREF'ed container yet, and we still own slice.
					if (slice == nullptr) {
						resObj = nullptr;
					}
					else {
						stackPointer += -2;
						_alifFrame_setStackPointer(_frame, stackPointer);
						resObj = alifObject_getItem(alifStackRef_asAlifObjectBorrow(container), slice);
						stackPointer = _alifFrame_getStackPointer(_frame);
						ALIF_DECREF(slice);
						stackPointer += 2;
					}
					ALIFSTACKREF_CLOSE(container);
					if (resObj == nullptr) goto pop_3_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-3] = res;
				stackPointer += -2;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_SUBSCR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(BINARY_SUBSCR);
				AlifCodeUnit* thisInstr = nextInstr - 2;
				AlifStackRef container{};
				AlifStackRef sub{};
				AlifStackRef res{};
				// _SPECIALIZE_BINARY_SUBSCR
				{
					sub = stackPointer[-1];
					container = stackPointer[-2];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_binarySubscr(container, sub, nextInstr);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(BINARY_SUBSCR);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _BINARY_SUBSCR
				{
					AlifObject* containerObj = alifStackRef_asAlifObjectBorrow(container);
					AlifObject* subObj = alifStackRef_asAlifObjectBorrow(sub);
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* resObj = alifObject_getItem(containerObj, subObj);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(container);
					ALIFSTACKREF_CLOSE(sub);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP_INPLACE_ADD_UNICODE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				//INSTRUCTION_STATS(BINARY_OP_INPLACE_ADD_UNICODE);
				AlifStackRef left{};
				AlifStackRef right{};
				// _GUARD_BOTH_UNICODE
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					DEOPT_IF(!ALIFUSTR_CHECKEXACT(leftObj), BINARY_OP);
					DEOPT_IF(!ALIFUSTR_CHECKEXACT(rightObj), BINARY_OP);
				}
				/* Skip 1 cache entry */
				// _BINARY_OP_INPLACE_ADD_UNICODE
				{
				#ifndef NDEBUG
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
				#endif
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					AlifIntT nextOpArg;
				#if TIER_ONE
					nextOpArg = nextInstr->op.arg;
				#else
					nextOpArg = CURRENT_OPERAND0();
				#endif
					AlifStackRef* targetLocal = &GETLOCAL(nextOpArg);
					DEOPT_IF(!ALIFSTACKREF_IS(*targetLocal, left), BINARY_OP);
					ALIFSTACKREF_CLOSE(left);
					AlifObject* temp = alifStackRef_asAlifObjectBorrow(*targetLocal);
					alifUStr_append(&temp, rightObj);
					*targetLocal = ALIFSTACKREF_FROMALIFOBJECTSTEAL(temp);
					ALIFSTACKREF_CLOSE_SPECIALIZED(right, _alifUStr_exactDealloc);
					if (ALIFSTACKREF_ISNULL(*targetLocal)) goto pop_2_error;
				#if TIER_ONE
					// The STORE_FAST is already done. This is done here in tier one,
					// and during trace projection in tier two:
					SKIP_OVER(1);
				#endif
				}
				stackPointer += -2;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CHECK_EG_MATCH) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef excValueSt{};
				AlifStackRef matchTypeSt{};
				AlifStackRef rest{};
				AlifStackRef match{};
				matchTypeSt = stackPointer[-1];
				excValueSt = stackPointer[-2];
				AlifObject* exc_value = alifStackRef_asAlifObjectBorrow(excValueSt);
				AlifObject* match_type = alifStackRef_asAlifObjectBorrow(matchTypeSt);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT err = _alifEval_checkExceptStarTypeValid(_thread, match_type);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (err < 0) {
					ALIFSTACKREF_CLOSE(excValueSt);
					ALIFSTACKREF_CLOSE(matchTypeSt);
					if (true) goto pop_2_error;
				}
				AlifObject* matchObj = nullptr;
				AlifObject* restObj = nullptr;
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT res = _alifEval_exceptionGroupMatch(exc_value, match_type,
					&matchObj, &restObj);
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(excValueSt);
				ALIFSTACKREF_CLOSE(matchTypeSt);
				if (res < 0) goto pop_2_error;
				if (matchObj == nullptr) goto pop_2_error;
				if (!ALIF_ISNONE(matchObj)) {
					stackPointer += -2;
					_alifFrame_setStackPointer(_frame, stackPointer);
					alifErr_setHandledException(matchObj);
					stackPointer = _alifFrame_getStackPointer(_frame);
					stackPointer += 2;
				}
				rest = ALIFSTACKREF_FROMALIFOBJECTSTEAL(restObj);
				match = ALIFSTACKREF_FROMALIFOBJECTSTEAL(matchObj);
				stackPointer[-2] = rest;
				stackPointer[-1] = match;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CHECK_EXC_MATCH) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef b{};
				right = stackPointer[-1];
				left = stackPointer[-2];
				AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
				AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT err = _alifEval_checkExceptTypeValid(_thread, rightObj);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (err < 0) {
					ALIFSTACKREF_CLOSE(right);
					if (true) goto pop_1_error;
				}
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT res = alifErr_givenExceptionMatches(leftObj, rightObj);
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(right);
				b = res ? ALIFSTACKREF_TRUE : ALIFSTACKREF_FALSE;
				stackPointer[-1] = b;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CLEANUP_THROW) {
				AlifCodeUnit* const thisInstr = _frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef subIterSt{};
				AlifStackRef lastSentValSt{};
				AlifStackRef excValueSt{};
				AlifStackRef none{};
				AlifStackRef value{};
				excValueSt = stackPointer[-1];
				lastSentValSt = stackPointer[-2];
				subIterSt = stackPointer[-3];
				AlifObject* excValue = alifStackRef_asAlifObjectBorrow(excValueSt);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT matches = alifErr_givenExceptionMatches(excValue, _alifExcStopIteration_);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (matches) {
					none = ALIFSTACKREF_NONE;
					value = ALIFSTACKREF_FROMALIFOBJECTNEW(((AlifStopIterationObject*)excValue)->value);
					ALIFSTACKREF_CLOSE(subIterSt);
					ALIFSTACKREF_CLOSE(lastSentValSt);
					ALIFSTACKREF_CLOSE(excValueSt);
				}
				else {
					_alifFrame_setStackPointer(_frame, stackPointer);
					_alifErr_setRaisedException(_thread, ALIF_NEWREF(excValue));
					//monitor_reraise(_thread, _frame, thisInstr);
					stackPointer = _alifFrame_getStackPointer(_frame);
					goto exception_unwind;
				}
				stackPointer[-3] = none;
				stackPointer[-2] = value;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(DELETE_SUBSCR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef container{};
				AlifStackRef sub{};
				sub = stackPointer[-1];
				container = stackPointer[-2];
				/* del container[sub] */
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT err = alifObject_delItem(alifStackRef_asAlifObjectBorrow(container),
					alifStackRef_asAlifObjectBorrow(sub));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(container);
				ALIFSTACKREF_CLOSE(sub);
				if (err) goto pop_2_error;
				stackPointer += -2;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(END_ASYNC_FOR) {
				AlifCodeUnit* const thisInstr = _frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef awaitableSt{};
				AlifStackRef excSt{};
				excSt = stackPointer[-1];
				awaitableSt = stackPointer[-2];
				AlifObject* exc = alifStackRef_asAlifObjectBorrow(excSt);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT matches = alifErr_givenExceptionMatches(exc, _alifExcStopAsyncIteration_);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (matches) {
					ALIFSTACKREF_CLOSE(awaitableSt);
					ALIFSTACKREF_CLOSE(excSt);
				}
				else {
					ALIF_INCREF(exc);
					_alifFrame_setStackPointer(_frame, stackPointer);
					_alifErr_setRaisedException(_thread, exc);
					//monitor_reraise(_thread, _frame, thisInstr);
					stackPointer = _alifFrame_getStackPointer(_frame);
					goto exception_unwind;
				}
				stackPointer += -2;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(END_FOR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = stackPointer[-1];
				ALIFSTACKREF_CLOSE(value);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(END_SEND) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef receiver{};
				AlifStackRef value{};
				AlifStackRef val{};
				value = stackPointer[-1];
				receiver = stackPointer[-2];
				val = value;
				ALIFSTACKREF_CLOSE(receiver);
				stackPointer[-2] = val;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(FORMAT_SIMPLE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifStackRef res{};
				value = stackPointer[-1];
				AlifObject* valueObj = alifStackRef_asAlifObjectBorrow(value);
				/* If value is a UStr object, then we know the result
					* of format(value) is value itself. */
				if (!ALIFUSTR_CHECKEXACT(valueObj)) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* resObj = alifObject_format(valueObj, nullptr);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(value);
					if (resObj == nullptr) goto pop_1_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				else {
					res = value;
				}
				stackPointer[-1] = res;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(FORMAT_WITH_SPEC) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifStackRef fmtSpec{};
				AlifStackRef res{};
				fmtSpec = stackPointer[-1];
				value = stackPointer[-2];
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* resObj = alifObject_format(alifStackRef_asAlifObjectBorrow(value), alifStackRef_asAlifObjectBorrow(fmtSpec));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(value);
				ALIFSTACKREF_CLOSE(fmtSpec);
				if (resObj == nullptr) goto pop_2_error;
				res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(GET_ITER) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef iterable{};
				AlifStackRef iter{};
				iterable = stackPointer[-1];
				/* before: [obj]; after [getiter(obj)] */
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* iterObj = alifObject_getIter(alifStackRef_asAlifObjectBorrow(iterable));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(iterable);
				if (iterObj == nullptr) goto pop_1_error;
				iter = ALIFSTACKREF_FROMALIFOBJECTSTEAL(iterObj);
				stackPointer[-1] = iter;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(INTERPRETER_EXIT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef retval{};
				retval = stackPointer[-1];
				/* Restore previous frame and return. */
				_thread->currentFrame = _frame->previous;
				_thread->cppRecursionRemaining += ALIF_EVAL_CPP_STACK_UNITS;
				AlifObject *result = alifStackRef_asAlifObjectSteal(retval);
				stackPointer += -1;
				return result;
			} // ------------------------------------------------------------ //
			TARGET(LOAD_BUILD_CLASS) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef bc{};
				AlifObject* bc_o{};
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT err = alifMapping_getOptionalItem(BUILTINS(), &ALIF_ID(__buildClass__), &bc_o);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (err < 0) goto error;
				if (bc_o == nullptr) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					//_alifErr_setString(_thread, _alifExcNameError_,
					//	"__buildClass__ not found");
					stackPointer = _alifFrame_getStackPointer(_frame);
					if (true) goto error;
				}
				bc = ALIFSTACKREF_FROMALIFOBJECTSTEAL(bc_o);
				stackPointer[0] = bc;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_LOCALS) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef locals{};
				AlifObject* l = LOCALS();
				if (l == nullptr) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					_alifErr_setString(_thread, _alifExcSystemError_,
						"قيمة local فارغة");
					stackPointer = _alifFrame_getStackPointer(_frame);
					if (true) goto error;
				}
				locals = ALIFSTACKREF_FROMALIFOBJECTNEW(l);
				stackPointer[0] = locals;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(MAKE_FUNCTION) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef codeObjSt{};
				AlifStackRef func{};
				codeObjSt = stackPointer[-1];
				AlifObject* codeobj = alifStackRef_asAlifObjectBorrow(codeObjSt);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifFunctionObject* funcObj = (AlifFunctionObject*)
					alifFunction_new(codeobj, GLOBALS());
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(codeObjSt);
				if (funcObj == nullptr) goto pop_1_error;
				_alifFunction_setVersion(
					funcObj, ((AlifCodeObject*)codeobj)->version);
				func = ALIFSTACKREF_FROMALIFOBJECTSTEAL((AlifObject*)funcObj);
				stackPointer[-1] = func;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(NOP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(POP_EXCEPT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef exc_value{};
				exc_value = stackPointer[-1];
				AlifErrStackItem* exc_info = _thread->excInfo;
				_alifFrame_setStackPointer(_frame, stackPointer);
				ALIF_XSETREF(exc_info->excValue,
					ALIFSTACKREF_IS(exc_value, ALIFSTACKREF_NONE)
					? nullptr : alifStackRef_asAlifObjectSteal(exc_value));
				stackPointer = _alifFrame_getStackPointer(_frame);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(POP_TOP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = stackPointer[-1];
				ALIFSTACKREF_CLOSE(value);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(PUSH_EXC_INFO) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef exc{};
				AlifStackRef prevExc{};
				AlifStackRef newExc{};
				exc = stackPointer[-1];
				AlifErrStackItem* excInfo = _thread->excInfo;
				if (excInfo->excValue != nullptr) {
					prevExc = ALIFSTACKREF_FROMALIFOBJECTSTEAL(excInfo->excValue);
				}
				else {
					prevExc = ALIFSTACKREF_NONE;
				}
				excInfo->excValue = ALIFSTACKREF_ASALIFOBJECTNEW(exc);
				newExc = exc;
				stackPointer[-1] = prevExc;
				stackPointer[0] = newExc;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(PUSH_NULL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef res{};
				res = _alifStackRefNull_;
				stackPointer[0] = res;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(RETURN_GENERATOR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef res{};
				AlifFunctionObject* func = (AlifFunctionObject*)alifStackRef_asAlifObjectBorrow(_frame->funcObj);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifGenObject* gen = (AlifGenObject*)_alif_makeCoro(func);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (gen == nullptr) goto error;
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifInterpreterFrame* gen_frame = &gen->giIFrame;
				_frame->instrPtr++;
				_alifFrame_copy(_frame, gen_frame);
				gen->giFrameState = AlifFrameState_::Frame_Created;
				gen_frame->owner = FrameOwner::FRAME_OWNED_BY_GENERATOR;
				_alif_leaveRecursiveCallAlif(_thread);
				AlifInterpreterFrame* prev = _frame->previous;
				_alifThreadState_popFrame(_thread, _frame);
				_frame = _thread->currentFrame = prev;
				LOAD_IP(_frame->returnOffset);
				stackPointer = _alifFrame_getStackPointer(_frame);
				res = ALIFSTACKREF_FROMALIFOBJECTSTEAL((AlifObject*)gen);
				//LLTRACE_RESUME_FRAME();
				stackPointer[0] = res;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(RETURN_VALUE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef retval{};
				AlifStackRef res{};
				retval = stackPointer[-1];
				AlifStackRef temp = retval;
				stackPointer += -1;
				_alifFrame_setStackPointer(_frame, stackPointer);
				_alif_leaveRecursiveCallAlif(_thread);
				AlifInterpreterFrame* dying = _frame;
				_frame = _thread->currentFrame = dying->previous;
				_alifEval_frameClearAndPop(_thread, dying);
				stackPointer = _alifFrame_getStackPointer(_frame);
				LOAD_IP(_frame->returnOffset);
				res = temp;
				//LLTRACE_RESUME_FRAME();
				stackPointer[0] = res;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(STORE_SUBSCR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(STORE_SUBSCR);
				AlifCodeUnit* thisInstr = nextInstr - 2;
				AlifStackRef container{};
				AlifStackRef sub{};
				AlifStackRef v{};
				// _SPECIALIZE_STORE_SUBSCR
				{
					sub = stackPointer[-1];
					container = stackPointer[-2];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_storeSubscr(container, sub, nextInstr);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(STORE_SUBSCR);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _STORE_SUBSCR
				{
					v = stackPointer[-3];
					/* container[sub] = v */
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifIntT err = alifObject_setItem(alifStackRef_asAlifObjectBorrow(container),
						alifStackRef_asAlifObjectBorrow(sub), alifStackRef_asAlifObjectBorrow(v));
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(v);
					ALIFSTACKREF_CLOSE(container);
					ALIFSTACKREF_CLOSE(sub);
					if (err) goto pop_3_error;
				}
				stackPointer += -3;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(TO_BOOL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 4;
				PREDICTED(TO_BOOL);
				AlifCodeUnit* const thisInstr = nextInstr - 4;
				AlifStackRef value{};
				AlifStackRef res{};
				// _SPECIALIZE_TO_BOOL
				{
					value = stackPointer[-1];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_toBool(value, nextInstr);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(TO_BOOL);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				/* Skip 2 cache entries */
				// _TO_BOOL
				{
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifIntT err = alifObject_isTrue(alifStackRef_asAlifObjectBorrow(value));
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(value);
					if (err < 0) goto pop_1_error;
					err ? res = ALIFSTACKREF_TRUE : res = ALIFSTACKREF_FALSE;
				}
				stackPointer[-1] = res;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(UNARY_NEGATIVE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifStackRef res{};
				value = stackPointer[-1];
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* resObj = alifNumber_negative(alifStackRef_asAlifObjectBorrow(value));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(value);
				if (resObj == nullptr) goto pop_1_error;
				res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				stackPointer[-1] = res;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(UNARY_NOT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifStackRef res{};
				value = stackPointer[-1];
				ALIFSTACKREF_IS(value, ALIFSTACKREF_FALSE)
					? res = ALIFSTACKREF_TRUE : res = ALIFSTACKREF_FALSE;
				stackPointer[-1] = res;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(WITH_EXCEPT_START) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				//INSTRUCTION_STATS(WITH_EXCEPT_START);
				AlifStackRef exit_func{};
				AlifStackRef exit_self{};
				AlifStackRef lasti{};
				AlifStackRef val{};
				AlifStackRef res{};
				val = stackPointer[-1];
				lasti = stackPointer[-3];
				exit_self = stackPointer[-4];
				exit_func = stackPointer[-5];
				AlifObject *exc{}, * tb{};
				AlifObject *val_o = alifStackRef_asAlifObjectBorrow(val);
				AlifObject *exit_func_o = alifStackRef_asAlifObjectBorrow(exit_func);
				exc = ALIFEXCEPTIONINSTANCE_CLASS(val_o);
				tb = alifException_getTraceback(val_o);
				if (tb == nullptr) {
					tb = ALIF_NONE;
				}
				else {
					ALIF_DECREF(tb);
				}
				AlifObject *stack[5] = {nullptr, alifStackRef_asAlifObjectBorrow(exit_self), exc, val_o, tb};
				AlifIntT has_self = !ALIFSTACKREF_ISNULL(exit_self);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* res_o = alifObject_vectorCall(exit_func_o, stack + 2 - has_self,
					(3 + has_self) | ALIF_VECTORCALL_ARGUMENTS_OFFSET, nullptr);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (res_o == nullptr) goto error;
				res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(res_o);
				stackPointer[0] = res;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(BINARY_OP);
				AlifCodeUnit* thisInstr = nextInstr - 2;
				AlifStackRef lhs{};
				AlifStackRef rhs{};
				AlifStackRef res{};
				// _SPECIALIZE_BINARY_OP
				{
					rhs = stackPointer[-1];
					lhs = stackPointer[-2];
					uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION_FT
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_binaryOp(lhs, rhs, nextInstr, oparg, LOCALS_ARRAY);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					//OPCODE_DEFERRED_INC(BINARY_OP);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION_FT */
				}
				// _BINARY_OP
				{
					AlifObject* lhsObj = alifStackRef_asAlifObjectBorrow(lhs);
					AlifObject* rhsObj = alifStackRef_asAlifObjectBorrow(rhs);
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* resObj = _alifEvalBinaryOps_[oparg](lhsObj, rhsObj);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(lhs);
					ALIFSTACKREF_CLOSE(rhs);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BUILD_LIST) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef* values{};
				AlifStackRef list{};
				values = &stackPointer[-oparg];
				AlifObject* listObj = _alifList_fromStackRefSteal(values, oparg);
				if (listObj == nullptr) {
					stackPointer += -oparg;
					goto error;
				}
				list = ALIFSTACKREF_FROMALIFOBJECTSTEAL(listObj);
				stackPointer[-oparg] = list;
				stackPointer += 1 - oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BUILD_MAP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef* values{};
				AlifStackRef map{};
				values = &stackPointer[-oparg * 2];
				STACKREFS_TO_ALIFOBJECTS(values, oparg * 2, valuesObj);
				if (CONVERSION_FAILED(valuesObj)) {
					for (AlifIntT _i = oparg * 2; --_i >= 0;) {
						ALIFSTACKREF_CLOSE(values[_i]);
					}
					if (true) {
						stackPointer += -oparg * 2;
						goto error;
					}
				}
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* mapObj = _alifDict_fromItems(
					valuesObj, 2, valuesObj + 1, 2, oparg);
				stackPointer = _alifFrame_getStackPointer(_frame);
				STACKREFS_TO_ALIFOBJECTS_CLEANUP(valuesObj);
				for (AlifIntT _i = oparg * 2; --_i >= 0;) {
					ALIFSTACKREF_CLOSE(values[_i]);
				}
				if (mapObj == nullptr) {
					stackPointer += -oparg * 2;
					goto error;
				}
				map = ALIFSTACKREF_FROMALIFOBJECTSTEAL(mapObj);
				stackPointer[-oparg * 2] = map;
				stackPointer += 1 - oparg * 2;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BUILD_SET) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef* values{};
				AlifStackRef set{};
				values = &stackPointer[-oparg];
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* setObj = alifSet_new(nullptr);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (setObj == nullptr) {
					for (AlifIntT _i = oparg; --_i >= 0;) {
						ALIFSTACKREF_CLOSE(values[_i]);
					}
					{
						stackPointer += -oparg;
						goto error;
					}
				}
				AlifIntT err = 0;
				for (AlifIntT i = 0; i < oparg; i++) {
					if (err == 0) {
						_alifFrame_setStackPointer(_frame, stackPointer);
						err = alifSet_add(setObj, alifStackRef_asAlifObjectBorrow(values[i]));
						stackPointer = _alifFrame_getStackPointer(_frame);
					}
					ALIFSTACKREF_CLOSE(values[i]);
				}
				if (err != 0) {
					ALIF_DECREF(setObj);
					{
						stackPointer += -oparg;
						goto error;
					}
				}
				set = ALIFSTACKREF_FROMALIFOBJECTSTEAL(setObj);
				stackPointer[-oparg] = set;
				stackPointer += 1 - oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BUILD_STRING) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef* pieces{};
				AlifStackRef str{};
				pieces = &stackPointer[-oparg];
				STACKREFS_TO_ALIFOBJECTS(pieces, oparg, piecesO);
				if (CONVERSION_FAILED(piecesO)) {
					for (AlifIntT _i = oparg; --_i >= 0;) {
						ALIFSTACKREF_CLOSE(pieces[_i]);
					}
					if (true) {
						stackPointer += -oparg;
						goto error;
					}
				}
				AlifObject* strO = alifUStr_joinArray(&ALIF_STR(Empty), piecesO, oparg);
				STACKREFS_TO_ALIFOBJECTS_CLEANUP(piecesO);
				for (AlifIntT _i = oparg; --_i >= 0;) {
					ALIFSTACKREF_CLOSE(pieces[_i]);
				}
				if (strO == nullptr) {
					stackPointer += -oparg;
					goto error;
				}
				str = ALIFSTACKREF_FROMALIFOBJECTSTEAL(strO);
				stackPointer[-oparg] = str;
				stackPointer += 1 - oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BUILD_TUPLE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef* values{};
				AlifStackRef tup{};
				values = &stackPointer[-oparg];
				AlifObject* tupObj = _alifTuple_fromStackRefSteal(values, oparg);
				if (tupObj == nullptr) {
					stackPointer += -oparg;
					goto error;
				}
				tup = ALIFSTACKREF_FROMALIFOBJECTSTEAL(tupObj);
				stackPointer[-oparg] = tup;
				stackPointer += 1 - oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CALL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 4;
				PREDICTED(CALL);
				AlifCodeUnit* thisInstr = nextInstr - 4;
				AlifStackRef* callable{};
				AlifStackRef* selfOrNull{};
				AlifStackRef* args{};
				AlifStackRef* func{};
				AlifStackRef* maybeSelf{};
				AlifStackRef res{};
				// _SPECIALIZE_CALL
				{
					selfOrNull = &stackPointer[-1 - oparg];
					callable = &stackPointer[-2 - oparg];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_call(callable[0], nextInstr, oparg + !ALIFSTACKREF_ISNULL(self_or_null[0]));
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(CALL);
					ADVANCE_ADAPTIVE_COUNTER(this_instr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				/* Skip 2 cache entries */
				// _MAYBE_EXPAND_METHOD
				{
					args = &stackPointer[-oparg];
					func = &stackPointer[-2 - oparg];
					maybeSelf = &stackPointer[-1 - oparg];
					if (ALIFSTACKREF_TYPE(callable[0]) == &_alifMethodType_ and ALIFSTACKREF_ISNULL(selfOrNull[0])) {
						AlifObject* callableObj = alifStackRef_asAlifObjectBorrow(callable[0]);
						AlifObject* self = ((AlifMethodObject*)callableObj)->self;
						maybeSelf[0] = ALIFSTACKREF_FROMALIFOBJECTNEW(self);
						AlifObject* method = ((AlifMethodObject*)callableObj)->func;
						AlifStackRef temp = callable[0];
						func[0] = ALIFSTACKREF_FROMALIFOBJECTNEW(method);
						ALIFSTACKREF_CLOSE(temp);
					}
				}
				// _DO_CALL
				{
					args = &stackPointer[-oparg];
					selfOrNull = &stackPointer[-1 - oparg];
					callable = &stackPointer[-2 - oparg];
					AlifObject* callableObj = alifStackRef_asAlifObjectBorrow(callable[0]);
					// oparg counts all of the args, but *not* self:
					AlifIntT totalArgs = oparg;
					if (!ALIFSTACKREF_ISNULL(selfOrNull[0])) {
						args--;
						totalArgs++;
					}
					// Check if the call can be inlined or not
					if (ALIF_TYPE(callableObj) == &_alifFunctionType_ and
						_thread->interpreter->evalFrame == nullptr and
						((AlifFunctionObject*)callableObj)->vectorCall == alifFunction_vectorCall) {
						AlifIntT codeFlags = ((AlifCodeObject*)ALIFFUNCTION_GET_CODE(callableObj))->flags;
						AlifObject* locals = codeFlags & CO_OPTIMIZED ? nullptr : ALIF_NEWREF(ALIFFUNCTION_GET_GLOBALS(callableObj));
						_alifFrame_setStackPointer(_frame, stackPointer);
						AlifInterpreterFrame* newFrame = _alifEval_framePushAndInit(
							_thread, callable[0], locals,
							args, totalArgs, nullptr, _frame);
						stackPointer = _alifFrame_getStackPointer(_frame);
						// Manipulate stack directly since we leave using DISPATCH_INLINED().
						stackPointer += -2 - oparg;
						// The frame has stolen all the arguments from the stack,
						// so there is no need to clean them up.
						if (newFrame == nullptr) {
							goto error;
						}
						_frame->returnOffset = 4;
						DISPATCH_INLINED(newFrame);
					}
					/* Callable is not a normal Alif function */
					STACKREFS_TO_ALIFOBJECTS(args, totalArgs, argsObj);
					if (CONVERSION_FAILED(argsObj)) {
						ALIFSTACKREF_CLOSE(callable[0]);
						for (AlifIntT i = 0; i < totalArgs; i++) {
							ALIFSTACKREF_CLOSE(args[i]);
						}
						if (true) {
							stackPointer += -2 - oparg;
							goto error;
						}
					}
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* resObj = alifObject_vectorCall(
						callableObj, argsObj,
						totalArgs | ALIF_VECTORCALL_ARGUMENTS_OFFSET,
						nullptr);
					stackPointer = _alifFrame_getStackPointer(_frame);
					STACKREFS_TO_ALIFOBJECTS_CLEANUP(argsObj);
					if (opcode == INSTRUMENTED_CALL) {
						AlifObject* arg = totalArgs == 0 ?
							&_alifInstrumentationMissing_ : alifStackRef_asAlifObjectBorrow(args[0]);
						if (resObj == nullptr) {
							_alifFrame_setStackPointer(_frame, stackPointer);
							//_alifCall_instrumentationExc2(
							//	_thread, ALIF_MONITORING_EVENT_C_RAISE,
							//	_frame, this_instr, callable_o, arg);
							stackPointer = _alifFrame_getStackPointer(_frame);
						}
						else {
							_alifFrame_setStackPointer(_frame, stackPointer);
							//AlifIntT err = _alifCall_instrumentation2Args(
							//	_thread, ALIF_MONITORING_EVENT_C_RETURN,
							//	_frame, this_instr, callable_o, arg);
							//if (err < 0) {
							//	ALIF_CLEAR(res_o);
							//}
							stackPointer = _alifFrame_getStackPointer(_frame);
						}
					}
					ALIFSTACKREF_CLOSE(callable[0]);
					for (AlifIntT i = 0; i < totalArgs; i++) {
						ALIFSTACKREF_CLOSE(args[i]);
					}
					if (resObj == nullptr) {
						stackPointer += -2 - oparg;
						goto error;
					}
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				// _CHECK_PERIODIC
				{
					//ALIF_CHECK_EMSCRIPTEN_SIGNALS_PERIODICALLY();
					QSBR_QUIESCENT_STATE(_thread);
					if (alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ALIF_EVAL_EVENTS_MASK) {
						stackPointer[-2 - oparg] = res;
						stackPointer += -1 - oparg;
						_alifFrame_setStackPointer(_frame, stackPointer);
						//AlifIntT err = _alif_handlePending(_thread);
						stackPointer = _alifFrame_getStackPointer(_frame);
						//if (err != 0) goto error;
						stackPointer += 1 + oparg;
					}
				}
				stackPointer[-2 - oparg] = res;
				stackPointer += -1 - oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CALL_FUNCTION_EX) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				PREDICTED(CALL_FUNCTION_EX);
				AlifCodeUnit* const thisInstr = nextInstr - 1;
				(void)thisInstr;
				AlifStackRef func{};
				AlifStackRef callargs{};
				AlifStackRef kwargs_in = _alifStackRefNull_;
				AlifStackRef tuple{};
				AlifStackRef kwargs_out = _alifStackRefNull_;
				AlifStackRef func_st{};
				AlifStackRef callargs_st{};
				AlifStackRef kwargs_st = _alifStackRefNull_;
				AlifStackRef result{};
				// _MAKE_CALLARGS_A_TUPLE
				{
					if (oparg & 1) { kwargs_in = stackPointer[-(oparg & 1)]; }
					callargs = stackPointer[-1 - (oparg & 1)];
					func = stackPointer[-3 - (oparg & 1)];
					AlifObject* callargs_o = alifStackRef_asAlifObjectBorrow(callargs);
					if (ALIFTUPLE_CHECKEXACT(callargs_o)) {
						tuple = callargs;
					}
					else {
						_alifFrame_setStackPointer(_frame, stackPointer);
						AlifIntT err = _alifCheck_argsIterable(_thread, alifStackRef_asAlifObjectBorrow(func), callargs_o);
						stackPointer = _alifFrame_getStackPointer(_frame);
						if (err < 0) {
							goto error;
						}
						_alifFrame_setStackPointer(_frame, stackPointer);
						AlifObject* tuple_o = alifSequence_tuple(callargs_o);
						stackPointer = _alifFrame_getStackPointer(_frame);
						if (tuple_o == nullptr) {
							goto error;
						}
						ALIFSTACKREF_CLOSE(callargs);
						tuple = ALIFSTACKREF_FROMALIFOBJECTSTEAL(tuple_o);
					}
					kwargs_out = kwargs_in;
				}
				// _DO_CALL_FUNCTION_EX
				{
					kwargs_st = kwargs_out;
					callargs_st = tuple;
					func_st = func;
					AlifObject* func = alifStackRef_asAlifObjectBorrow(func_st);
					AlifObject* callargs = alifStackRef_asAlifObjectBorrow(callargs_st);
					AlifObject* kwargs = alifStackRef_asAlifObjectBorrow(kwargs_st);
					// DICT_MERGE is called before this opcode if there are kwargs.
					// It converts all dict subtypes in kwargs into regular dicts.
					AlifObject* resultObj{};
					if (opcode == INSTRUMENTED_CALL_FUNCTION_EX) {
						AlifObject* arg = ALIFTUPLE_GET_SIZE(callargs) > 0 ?
							ALIFTUPLE_GET_ITEM(callargs, 0) : &_alifInstrumentationMissing_;
						stackPointer[-1 - (oparg & 1)] = callargs_st;
						if (oparg & 1) stackPointer[-(oparg & 1)] = kwargs_st;
						_alifFrame_setStackPointer(_frame, stackPointer);
						//AlifIntT err = _alif_callInstrumentation2args(
						//	_thread, ALIF_MONITORING_EVENT_CALL,
						//	_frame, thisInstr, func, arg);
						stackPointer = _alifFrame_getStackPointer(_frame);
						//if (err) {
						//	goto error;
						//}
						_alifFrame_setStackPointer(_frame, stackPointer);
						resultObj = alifObject_call(func, callargs, kwargs);
						stackPointer = _alifFrame_getStackPointer(_frame);
						if (!ALIFFUNCTION_CHECK(func) and !ALIFMETHOD_CHECK(func)) {
							if (resultObj == nullptr) {
								_alifFrame_setStackPointer(_frame, stackPointer);
								//_alif_callInstrumentationExc2(
								//	_thread, ALIF_MONITORING_EVENT_CPP_RAISE,
								//	_frame, thisInstr, func, arg);
								stackPointer = _alifFrame_getStackPointer(_frame);
							}
							else {
								_alifFrame_setStackPointer(_frame, stackPointer);
								//AlifIntT err = _alif_callInstrumentation2args(
								//	_thread, ALIF_MONITORING_EVENT_C_RETURN,
								//	_frame, thisInstr, func, arg);
								//stackPointer = _alifFrame_getStackPointer(_frame);
								//if (err < 0) {
								//	ALIF_CLEAR(resultObj);
								//}
							}
						}
					}
					else {
						if (ALIF_TYPE(func) == &_alifFunctionType_ and
							_thread->interpreter->evalFrame == nullptr and
							((AlifFunctionObject*)func)->vectorCall == alifFunction_vectorCall) {
							AlifSizeT nargs = ALIFTUPLE_GET_SIZE(callargs);
							AlifIntT code_flags = ((AlifCodeObject*)ALIFFUNCTION_GET_CODE(func))->flags;
							AlifObject* locals = code_flags & CO_OPTIMIZED ? nullptr : ALIF_NEWREF(ALIFFUNCTION_GET_GLOBALS(func));
							stackPointer[-1 - (oparg & 1)] = callargs_st;
							if (oparg & 1) stackPointer[-(oparg & 1)] = kwargs_st;
							_alifFrame_setStackPointer(_frame, stackPointer);
							AlifInterpreterFrame* new_frame = _alifEval_framePushAndInitEx(
								_thread, func_st, locals,
								nargs, callargs, kwargs, _frame);
							stackPointer = _alifFrame_getStackPointer(_frame);
							// Need to sync the stack since we exit with DISPATCH_INLINED.
							stackPointer += -3 - (oparg & 1);
							if (new_frame == nullptr) {
								goto error;
							}
							_frame->returnOffset = 1;
							DISPATCH_INLINED(new_frame);
						}
						stackPointer[-1 - (oparg & 1)] = callargs_st;
						if (oparg & 1) stackPointer[-(oparg & 1)] = kwargs_st;
						_alifFrame_setStackPointer(_frame, stackPointer);
						resultObj = alifObject_call(func, callargs, kwargs);
						stackPointer = _alifFrame_getStackPointer(_frame);
					}
					_alifFrame_setStackPointer(_frame, stackPointer);
					ALIFSTACKREF_XCLOSE(kwargs_st);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(callargs_st);
					ALIFSTACKREF_CLOSE(func_st);
					if (resultObj == nullptr) {
						stackPointer += -3 - (oparg & 1);
						goto error;
					}
					result = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resultObj);
				}
				// _CHECK_PERIODIC
				{
					//_ALIF_CHECK_EMSCRIPTEN_SIGNALS_PERIODICALLY();
					QSBR_QUIESCENT_STATE(_thread);
					if (alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ALIF_EVAL_EVENTS_MASK) {
						stackPointer[-3 - (oparg & 1)] = result;
						stackPointer += -2 - (oparg & 1);
						_alifFrame_setStackPointer(_frame, stackPointer);
						//AlifIntT err = _alif_handlePending(_thread);
						stackPointer = _alifFrame_getStackPointer(_frame);
						//if (err != 0) goto error;
						stackPointer += 2 + (oparg & 1);
					}
				}
				stackPointer[-3 - (oparg & 1)] = result;
				stackPointer += -2 - (oparg & 1);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CALL_INTRINSIC_1) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifStackRef res{};
				value = stackPointer[-1];
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* resObj = _alifIntrinsicsUnaryFunctions_[oparg].func(_thread, alifStackRef_asAlifObjectBorrow(value));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(value);
				if (resObj == nullptr) goto pop_1_error;
				res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				stackPointer[-1] = res;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CALL_KW) {
				_frame->instrPtr = nextInstr;
				nextInstr += 4;
				PREDICTED(CALL_KW);
				AlifCodeUnit* thisInstr = nextInstr - 4;
				AlifStackRef* callable{};
				AlifStackRef* selfOrNull{};
				AlifStackRef* args{};
				AlifStackRef kwnames{};
				AlifStackRef kwnames_in{};
				AlifStackRef* func{};
				AlifStackRef* maybe_self{};
				AlifStackRef kwnames_out{};
				AlifStackRef res{};
				// _SPECIALIZE_CALL_KW
				{
					selfOrNull = &stackPointer[-2 - oparg];
					callable = &stackPointer[-3 - oparg];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_callKw(callable[0], nextInstr, oparg + !ALIFSTACKREF_ISNULL(self_or_null[0]));
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(CALL_KW);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				/* Skip 2 cache entries */
				// _MAYBE_EXPAND_METHOD_KW
				{
					kwnames_in = stackPointer[-1];
					args = &stackPointer[-1 - oparg];
					func = &stackPointer[-3 - oparg];
					maybe_self = &stackPointer[-2 - oparg];
					if (ALIFSTACKREF_TYPE(callable[0]) == &_alifMethodType_ and ALIFSTACKREF_ISNULL(selfOrNull[0])) {
						AlifObject* callable_o = alifStackRef_asAlifObjectBorrow(callable[0]);
						AlifObject* self = ((AlifMethodObject*)callable_o)->self;
						maybe_self[0] = ALIFSTACKREF_FROMALIFOBJECTNEW(self);
						AlifObject* method = ((AlifMethodObject*)callable_o)->func;
						AlifStackRef temp = callable[0];
						func[0] = ALIFSTACKREF_FROMALIFOBJECTNEW(method);
						ALIFSTACKREF_CLOSE(temp);
					}
					kwnames_out = kwnames_in;
				}
				// _DO_CALL_KW
				{
					kwnames = kwnames_out;
					args = &stackPointer[-1 - oparg];
					selfOrNull = &stackPointer[-2 - oparg];
					callable = &stackPointer[-3 - oparg];
					AlifObject* callableObj = alifStackRef_asAlifObjectBorrow(callable[0]);
					AlifObject* kwNamesObj = alifStackRef_asAlifObjectBorrow(kwnames);
					// oparg counts all of the args, but *not* self:
					AlifIntT totalArgs = oparg;
					if (!ALIFSTACKREF_ISNULL(selfOrNull[0])) {
						args--;
						totalArgs++;
					}
					AlifIntT positionalArgs = totalArgs - (AlifIntT)ALIFTUPLE_GET_SIZE(kwNamesObj);
					// Check if the call can be inlined or not
					if (ALIF_TYPE(callableObj) == &_alifFunctionType_ and
						_thread->interpreter->evalFrame == nullptr and
						((AlifFunctionObject*)callableObj)->vectorCall == alifFunction_vectorCall) {
						AlifIntT code_flags = ((AlifCodeObject*)ALIFFUNCTION_GET_CODE(callableObj))->flags;
						AlifObject* locals = code_flags & CO_OPTIMIZED ? nullptr : ALIF_NEWREF(ALIFFUNCTION_GET_GLOBALS(callableObj));
						stackPointer[-1] = kwnames;
						_alifFrame_setStackPointer(_frame, stackPointer);
						AlifInterpreterFrame* new_frame = _alifEval_framePushAndInit(
							_thread, callable[0], locals,
							args, positionalArgs, kwNamesObj, _frame);
						stackPointer = _alifFrame_getStackPointer(_frame);
						ALIFSTACKREF_CLOSE(kwnames);
						// Sync stack explicitly since we leave using DISPATCH_INLINED().
						stackPointer += -3 - oparg;
						// The frame has stolen all the arguments from the stack,
						// so there is no need to clean them up.
						if (new_frame == nullptr) {
							goto error;
						}
						_frame->returnOffset = 4;
						DISPATCH_INLINED(new_frame);
					}
					/* Callable is not a normal Alif function */
					STACKREFS_TO_ALIFOBJECTS(args, totalArgs, argsObj);
					if (CONVERSION_FAILED(argsObj)) {
						ALIFSTACKREF_CLOSE(callable[0]);
						ALIFSTACKREF_CLOSE(selfOrNull[0]);
						for (AlifIntT _i = oparg; --_i >= 0;) {
							ALIFSTACKREF_CLOSE(args[_i]);
						}
						ALIFSTACKREF_CLOSE(kwnames);
						if (true) {
							stackPointer += -3 - oparg;
							goto error;
						}
					}
					stackPointer[-1] = kwnames;
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* resObj = alifObject_vectorCall(
						callableObj, argsObj,
						positionalArgs | ALIF_VECTORCALL_ARGUMENTS_OFFSET,
						kwNamesObj);
					stackPointer = _alifFrame_getStackPointer(_frame);
					STACKREFS_TO_ALIFOBJECTS_CLEANUP(argsObj);
					if (opcode == INSTRUMENTED_CALL_KW) {
						AlifObject* arg = totalArgs == 0 ?
							&_alifInstrumentationMissing_ : alifStackRef_asAlifObjectBorrow(args[0]);
						//if (resObj == nullptr) {
						// _alifFrame_setStackPointer(_frame, stackPointer);
						//	_alifCall_instrumentationExc2(
						//		_thread, ALIF_MONITORING_EVENT_CPP_RAISE,
						//		_frame, thisInstr, callableObj, arg);
						// stackPointer = _alifFrame_getStackPointer(_frame);
						//}
						//else {
						// _alifFrame_setStackPointer(_frame, stackPointer);
						//	AlifIntT err = _alifCall_instrumentation2Args(
						//		_thread, ALIF_MONITORING_EVENT_CPP_RETURN,
						//		_frame, thisInstr, callableObj, arg);
						// stackPointer = _alifFrame_getStackPointer(_frame);
						//	if (err < 0) {
						//		ALIF_CLEAR(resObj);
						//	}
						//}
					}
					ALIFSTACKREF_CLOSE(kwnames);
					ALIFSTACKREF_CLOSE(callable[0]);
					for (int i = 0; i < totalArgs; i++) {
						ALIFSTACKREF_CLOSE(args[i]);
					}
					if (resObj == nullptr) {
						stackPointer += -3 - oparg;
						goto error;
					}
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-3 - oparg] = res;
				stackPointer += -2 - oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(COMPARE_OP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(COMPARE_OP);
				AlifCodeUnit* thisInstr = nextInstr - 2;
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef res{};
				// _SPECIALIZE_COMPARE_OP
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextOnstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_compareOp(left, right, nextInstr, oparg);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(COMPARE_OP);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _COMPARE_OP
				{
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* resObj = alifObject_richCompare(leftObj, rightObj, oparg >> 5);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(left);
					ALIFSTACKREF_CLOSE(right);
					if (resObj == nullptr) goto pop_2_error;
					if (oparg & 16) {
						stackPointer += -2;
						_alifFrame_setStackPointer(_frame, stackPointer);
						AlifIntT resBool = alifObject_isTrue(resObj);
						stackPointer = _alifFrame_getStackPointer(_frame);
						ALIF_DECREF(resObj);
						if (resBool < 0) goto error;
						resBool ? res = ALIFSTACKREF_TRUE : res = ALIFSTACKREF_FALSE;
					}
					else {
						res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
						stackPointer += -2;
					}
				}
				stackPointer[0] = res;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CONTAINS_OP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(CONTAINS_OP);
				AlifCodeUnit* thisInstr = nextInstr - 2;
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef b{};
				// _SPECIALIZE_CONTAINS_OP
				{
					right = stackPointer[-1];
					uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION_FT
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_containsOp(right, nextInstr);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					//OPCODE_DEFERRED_INC(CONTAINS_OP);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _CONTAINS_OP
				{
					left = stackPointer[-2];
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifIntT res = alifSequence_contains(rightObj, leftObj);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(left);
					ALIFSTACKREF_CLOSE(right);
					if (res < 0) goto pop_2_error;
					b = (res ^ oparg) ? ALIFSTACKREF_TRUE : ALIFSTACKREF_FALSE;
				}
				stackPointer[-2] = b;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(COPY) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef bottom{};
				AlifStackRef top{};
				bottom = stackPointer[-1 - (oparg - 1)];
				top = alifStackRef_dup(bottom);
				stackPointer[0] = top;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(COPY_FREE_VARS) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				/* Copy closure variables to free variables */
				AlifCodeObject* co = _alifFrame_getCode(_frame);
				AlifFunctionObject* func = (AlifFunctionObject*)alifStackRef_asAlifObjectBorrow(_frame->funcObj);
				AlifObject* closure = func->closure;
				AlifIntT offset = co->nLocalsPlus - oparg;
				for (AlifIntT i = 0; i < oparg; ++i) {
					AlifObject* o = ALIFTUPLE_GET_ITEM(closure, i);
					_frame->localsPlus[offset + i] = ALIFSTACKREF_FROMALIFOBJECTNEW(o);
				}
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(DELETE_FAST) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				//INSTRUCTION_STATS(DELETE_FAST);
				AlifStackRef v = GETLOCAL(oparg);
				if (ALIFSTACKREF_ISNULL(v)) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					//_alifEval_formatExcCheckArg(_thread, _alifExcUnboundLocalError_,
					//	UNBOUNDLOCAL_ERROR_MSG,
					//	alifTuple_getItem(_alifFrame_getCode(_frame)->localsPlusNames, oparg)
					//);
					stackPointer = _alifFrame_getStackPointer(_frame);
					goto error;
				}
				SETLOCAL(oparg, _alifStackRefNull_);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(DELETE_NAME) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
				AlifObject* ns = LOCALS();
				AlifIntT err{};
				if (ns == nullptr) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					_alifErr_format(_thread, _alifExcSystemError_,
						"قيمة locals اصبحت فارغة عند حذف %R", name);
					stackPointer = _alifFrame_getStackPointer(_frame);
					goto error;
				}
				_alifFrame_setStackPointer(_frame, stackPointer);
				err = alifObject_delItem(ns, name);
				stackPointer = _alifFrame_getStackPointer(_frame);
				// Can't use ERROR_IF here.
				if (err != 0) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					_alifEval_formatExcCheckArg(_thread, _alifExcNameError_,
						NAME_ERROR_MSG,
						name);
					stackPointer = _alifFrame_getStackPointer(_frame);
					goto error;
				}
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(DICT_MERGE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef callable{};
				AlifStackRef dict{};
				AlifStackRef update{};
				update = stackPointer[-1];
				dict = stackPointer[-2 - (oparg - 1)];
				callable = stackPointer[-5 - (oparg - 1)];
				AlifObject* callableObj = alifStackRef_asAlifObjectBorrow(callable);
				AlifObject* dictObj = alifStackRef_asAlifObjectBorrow(dict);
				AlifObject* updateObj = alifStackRef_asAlifObjectBorrow(update);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT err = _alifDict_mergeEx(dictObj, updateObj, 2);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (err < 0) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					//_alifEval_formatKwargsError(_thread, callableObj, updateObj);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(update);
					goto pop_1_error;
				}
				ALIFSTACKREF_CLOSE(update);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(EXTENDED_ARG) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				opcode = nextInstr->op.code;
				oparg = oparg << 8 | nextInstr->op.arg;
				PRE_DISPATCH_GOTO();
				DISPATCH_GOTO();
			} // ------------------------------------------------------------ //
			TARGET(FOR_ITER) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(FOR_ITER);
				AlifCodeUnit* const thisInstr = nextInstr - 2;
				AlifStackRef iter{};
				AlifStackRef next{};
				// _SPECIALIZE_FOR_ITER
				{
					iter = stackPointer[-1];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_forIter(iter, nextInstr, oparg);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(FOR_ITER);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _FOR_ITER
				{
					/* before: [iter]; after: [iter, iter()] *or* [] (and jump over END_FOR.) */
					AlifObject* iterObj = alifStackRef_asAlifObjectBorrow(iter);
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* nextObj = (*ALIF_TYPE(iterObj)->iterNext)(iterObj);
					stackPointer = _alifFrame_getStackPointer(_frame);
					if (nextObj == nullptr) {
						if (_alifErr_occurred(_thread)) {
							_alifFrame_setStackPointer(_frame, stackPointer);
							AlifIntT matches = _alifErr_exceptionMatches(_thread, _alifExcStopIteration_);
							stackPointer = _alifFrame_getStackPointer(_frame);
							if (!matches) {
								goto error;
							}
							_alifFrame_setStackPointer(_frame, stackPointer);
							// _alifEval_monitorRaise(_thread, _frame, thisInstr);
							_alifErr_clear(_thread);
							stackPointer = _alifFrame_getStackPointer(_frame);
						}
						/* iterator ended normally */
						ALIFSTACKREF_CLOSE(iter);
						STACK_SHRINK(1);
						/* Jump forward oparg, then skip following END_FOR and POP_TOP instruction */
						JUMPBY(oparg + 2);
						DISPATCH();
					}
					next = ALIFSTACKREF_FROMALIFOBJECTSTEAL(nextObj);
					// Common case: no jump, leave it to the code generator
				}
				stackPointer[0] = next;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(IMPORT_FROM) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef from{};
				AlifStackRef res{};
				from = stackPointer[-1];
				AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* resObj = _alifEval_importFrom(_thread, alifStackRef_asAlifObjectBorrow(from), name);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (resObj == nullptr) goto error;
				res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				stackPointer[0] = res;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(IMPORT_NAME) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef level{};
				AlifStackRef fromlist{};
				AlifStackRef res{};
				fromlist = stackPointer[-1];
				level = stackPointer[-2];
				AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* resObj = _alifEval_importName(_thread, _frame, name,
					alifStackRef_asAlifObjectBorrow(fromlist),
					alifStackRef_asAlifObjectBorrow(level));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(level);
				ALIFSTACKREF_CLOSE(fromlist);
				if (resObj == nullptr) goto pop_2_error;
				res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(IS_OP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef b{};
				right = stackPointer[-1];
				left = stackPointer[-2];
				// On free-threaded builds, objects are conditionally immortalized.
				// So their bits don't always compare equally.
				AlifIntT res = ALIF_IS(alifStackRef_asAlifObjectBorrow(left),
					alifStackRef_asAlifObjectBorrow(right)) ^ oparg;

				ALIFSTACKREF_CLOSE(left);
				ALIFSTACKREF_CLOSE(right);
				b = res ? ALIFSTACKREF_TRUE : ALIFSTACKREF_FALSE;
				stackPointer[-2] = b;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(JUMP_BACKWARD) {
				AlifCodeUnit* thisInstr = _frame->instrPtr = nextInstr;
				nextInstr += 2;
				// _CHECK_PERIODIC
				{
					QSBR_QUIESCENT_STATE(_thread);
					if (alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ALIF_EVAL_EVENTS_MASK) {
						_alifFrame_setStackPointer(_frame, stackPointer);
						//AlifIntT err = _alif_handlePending(_thread);
						stackPointer = _alifFrame_getStackPointer(_frame);
						//if (err != 0) goto error;
					}
				}
				// _JUMP_BACKWARD
				{
					uint16_t the_counter = read_u16(&thisInstr[1].cache);
					JUMPBY(-oparg);
				#ifdef ALIF_TIER2
				#if ENABLE_SPECIALIZATION
					AlifBackoffCounter counter = thisInstr[1].counter;
					if (backoff_counterTriggers(counter) and thisInstr->op.code == JUMP_BACKWARD) {
						AlifCodeUnit* start = thisInstr;
						while (oparg > 255) {
							oparg >>= 8;
							start--;
						}
						AlifExecutorObject* executor{};
						_alifFrame_setStackPointer(_frame, stackPointer);
						AlifIntT optimized = _alifOptimizer_optimize(_frame, start, stackPointer, &executor, 0);
						stackPointer = _alifFrame_getStackPointer(_frame);
						if (optimized <= 0) {
							thisInstr[1].counter = restart_backoffCounter(counter);
							if (optimized < 0) goto error;
						}
						else {
							_alifFrame_setStackPointer(_frame, stackPointer);
							thisInstr[1].counter = initial_jumpBackoffCounter();
							stackPointer = _alifFrame_getStackPointer(_frame);
							_thread->previousExecutor = ALIF_NONE;
							GOTO_TIER_TWO(executor);
						}
					}
					else {
						ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
					}
				#endif  /* ENABLE_SPECIALIZATION */
				#endif /* ALIF_TIER2 */
				}
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(JUMP_BACKWARD_NO_INTERRUPT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				JUMPBY(-oparg);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(JUMP_FORWARD) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				JUMPBY(oparg);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LIST_APPEND) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef list{};
				AlifStackRef v{};
				v = stackPointer[-1];
				list = stackPointer[-2 - (oparg - 1)];
				AlifIntT err = alifList_appendTakeRef((AlifListObject*)alifStackRef_asAlifObjectBorrow(list),
					alifStackRef_asAlifObjectSteal(v));
				if (err < 0) goto pop_1_error;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LIST_EXTEND) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef listSt{};
				AlifStackRef iterableSt{};
				iterableSt = stackPointer[-1];
				listSt = stackPointer[-2 - (oparg - 1)];
				AlifObject* list = alifStackRef_asAlifObjectBorrow(listSt);
				AlifObject* iterable = alifStackRef_asAlifObjectBorrow(iterableSt);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* none_val = _alifList_extend((AlifListObject*)list, iterable);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (none_val == nullptr) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifIntT matches = _alifErr_exceptionMatches(_thread, _alifExcTypeError_);
					stackPointer = _alifFrame_getStackPointer(_frame);
					if (matches and
						(ALIF_TYPE(iterable)->iter == nullptr and !alifSequence_check(iterable)))
					{
					 _alifFrame_setStackPointer(_frame, stackPointer);
						_alifErr_clear(_thread);
						_alifErr_format(_thread, _alifExcTypeError_,
							"القيمة بعد * يجب أن تكون قابلة للتكرار, وليس %.200s",
							ALIF_TYPE(iterable)->name);
					 stackPointer = _alifFrame_getStackPointer(_frame);
					}
					ALIFSTACKREF_CLOSE(iterableSt);
					if (true) goto pop_1_error;
				}
				ALIFSTACKREF_CLOSE(iterableSt);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_ATTR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 10;
				PREDICTED(LOAD_ATTR);
				AlifCodeUnit* thisInstr = nextInstr - 10;
				AlifStackRef owner{};
				AlifStackRef attr{};
				AlifStackRef selfOrNull = _alifStackRefNull_;
				// _SPECIALIZE_LOAD_ATTR
				{
					owner = stackPointer[-1];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg >> 1);
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_loadAttr(owner, nextInstr, name);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(LOAD_ATTR);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				/* Skip 8 cache entries */
				// _LOAD_ATTR
				{
					AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg >> 1);
					AlifObject* attrObj{};
					if (oparg & 1) {
						attrObj = nullptr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						AlifIntT is_meth = _alifObject_getMethod(alifStackRef_asAlifObjectBorrow(owner), name, &attrObj);
						stackPointer = _alifFrame_getStackPointer(_frame);
						if (is_meth) {
							selfOrNull = owner;  // Transfer ownership
						}
						else {
							ALIFSTACKREF_CLOSE(owner);
							if (attrObj == nullptr) goto pop_1_error;
							selfOrNull = _alifStackRefNull_;
						}
					}
					else {
						/* Classic, pushes one value. */
						_alifFrame_setStackPointer(_frame, stackPointer);
						attrObj = alifObject_getAttr(alifStackRef_asAlifObjectBorrow(owner), name);
						stackPointer = _alifFrame_getStackPointer(_frame);
						ALIFSTACKREF_CLOSE(owner);
						if (attrObj == nullptr) goto pop_1_error;

						/* We need to define selfOrNull on all paths */
						selfOrNull = _alifStackRefNull_;
					}
					attr = ALIFSTACKREF_FROMALIFOBJECTSTEAL(attrObj);
				}
				stackPointer[-1] = attr;
				if (oparg & 1) stackPointer[0] = selfOrNull;
				stackPointer += (oparg & 1);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_CONST) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				PREDICTED(LOAD_CONST);
				AlifStackRef value{};
				value = ALIFSTACKREF_FROMALIFOBJECTNEW(GETITEM(FRAME_CO_CONSTS, oparg));
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_DEREF) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifCellObject* cell = (AlifCellObject*)alifStackRef_asAlifObjectBorrow(GETLOCAL(oparg));
				AlifObject* value_o = alifCell_getRef(cell);
				if (value_o == nullptr) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					//_alifEval_formatExcUnbound(_thread, _alifFrame_getCode(_frame), oparg);
					stackPointer = _alifFrame_getStackPointer(_frame);
					if (true) goto error;
				}
				value = ALIFSTACKREF_FROMALIFOBJECTSTEAL(value_o);
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_FAST) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = alifStackRef_dup(GETLOCAL(oparg));
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_FAST_AND_CLEAR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = GETLOCAL(oparg);
				// do not use SETLOCAL here, it decrefs the old value
				GETLOCAL(oparg) = _alifStackRefNull_;
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_FAST_CHECK) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifStackRef valueS = GETLOCAL(oparg);
				if (ALIFSTACKREF_ISNULL(valueS)) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					//_alifEval_formatExcCheckArg(_thread, _alifExcUnboundLocalError_,
					//	UNBOUNDLOCAL_ERROR_MSG,
					//	alifTuple_getItem(_alifFrame_getCode(_frame)->localsPlusNames, oparg)
					//);
					stackPointer = _alifFrame_getStackPointer(_frame);
					if (1) goto error;
				}
				value = alifStackRef_dup(valueS);
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_GLOBAL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 5;
				PREDICTED(LOAD_GLOBAL);
				AlifCodeUnit* thisInstr = nextInstr - 5;
				AlifStackRef* res{};
				AlifStackRef null = _alifStackRefNull_;
				// _SPECIALIZE_LOAD_GLOBAL
				{
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg >> 1);
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_loadGlobal(GLOBALS(), BUILTINS(), nextInstr, name);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(LOAD_GLOBAL);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _LOAD_GLOBAL
				{
					res = &stackPointer[0];
					AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg >> 1);
					_alifFrame_setStackPointer(_frame, stackPointer);
					_alifEval_loadGlobalStackRef(GLOBALS(), BUILTINS(), name, res);
					stackPointer = _alifFrame_getStackPointer(_frame);
					if (ALIFSTACKREF_ISNULL(*res)) goto error;
					null = _alifStackRefNull_;
				}
				if (oparg & 1) stackPointer[1] = null;
				stackPointer += 1 + (oparg & 1);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_NAME) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef v{};
				AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* vObj = _alifEval_loadName(_thread, _frame, name);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (vObj == nullptr) goto error;
				v = ALIFSTACKREF_FROMALIFOBJECTSTEAL(vObj);
				stackPointer[0] = v;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_SMALL_INT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifObject *obj = (AlifObject *)&ALIFLONG_SMALL_INTS[ALIF_NSMALLNEGINTS + oparg];
				value = ALIFSTACKREF_FROMALIFOBJECTIMMORTAL(obj);
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_SPECIAL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				//INSTRUCTION_STATS(LOAD_SPECIAL);
				AlifStackRef owner{};
				AlifStackRef attr{};
				AlifStackRef self_or_null{};
				owner = stackPointer[-1];
				AlifObject* ownerObj = alifStackRef_asAlifObjectSteal(owner);
				AlifObject* name = _alifSpecialMethods_[oparg].name;
				AlifObject* selfOrNullObj{};
				stackPointer += -1;
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifObject* attrObj = _alifObject_lookupSpecialMethod(ownerObj, name, &selfOrNullObj);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (attrObj == nullptr) {
					if (!_alifErr_occurred(_thread)) {
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifErr_format(_thread, _alifExcTypeError_,
							_alifSpecialMethods_[oparg].error,
							ALIF_TYPE(ownerObj)->name);
						stackPointer = _alifFrame_getStackPointer(_frame);
					}
					goto error;
				}
				attr = ALIFSTACKREF_FROMALIFOBJECTSTEAL(attrObj);
				self_or_null = selfOrNullObj == nullptr ?
					_alifStackRefNull_ : ALIFSTACKREF_FROMALIFOBJECTSTEAL(selfOrNullObj);
				stackPointer[0] = attr;
				stackPointer[1] = self_or_null;
				stackPointer += 2;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_SUPER_ATTR) { //* review
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				//INSTRUCTION_STATS(LOAD_SUPER_ATTR);
				PREDICTED(LOAD_SUPER_ATTR);
				AlifCodeUnit* const thisInstr = nextInstr - 2;
				(void)thisInstr;
				AlifStackRef global_super_st{};
				AlifStackRef class_st{};
				AlifStackRef self_st{};
				AlifStackRef attr{};
				AlifStackRef null = _alifStackRefNull_;
				// _SPECIALIZE_LOAD_SUPER_ATTR
				{
					class_st = stackPointer[-2];
					global_super_st = stackPointer[-3];
					uint16_t counter = read_u16(&thisInstr[1].cache);
					(void)counter;
				#if ENABLE_SPECIALIZATION
					int load_method = oparg & 1;
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_loadSuperAttr(global_super_st, class_st, next_instr, load_method);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(LOAD_SUPER_ATTR);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _LOAD_SUPER_ATTR
				{
					self_st = stackPointer[-1];
					AlifObject* global_super = alifStackRef_asAlifObjectBorrow(global_super_st);
					AlifObject* class_ = alifStackRef_asAlifObjectBorrow(class_st);
					AlifObject* self = alifStackRef_asAlifObjectBorrow(self_st);
					//if (opcode == INSTRUMENTED_LOAD_SUPER_ATTR) {
					//	AlifObject *arg = oparg & 2 ? class_ : &_alifInstrumentationMissing_;
					//	_alifFrame_setStackPointer(_frame, stackPointer);
					//	int err = _alifCall_instrumentation2Args(
					//		_thread, ALIF_MONITORING_EVENT_CALL,
					//		_frame, thisInstr, global_super, arg);
					//	stackPointer = _alifFrame_getStackPointer(_frame);
					//	if (err) {
					//		ALIFSTACKREF_CLOSE(global_super_st);
					//		ALIFSTACKREF_CLOSE(class_st);
					//		ALIFSTACKREF_CLOSE(self_st);
					//		goto pop_3_error;
					//	}
					//}
					AlifObject *stack[] = {class_, self};
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* super = alifObject_vectorCall(global_super, stack, oparg & 2, nullptr);
					stackPointer = _alifFrame_getStackPointer(_frame);
					//if (opcode == INSTRUMENTED_LOAD_SUPER_ATTR) {
					//	AlifObject* arg = oparg & 2 ? class_ : &_alifInstrumentationMissing_;
					//	if (super == nullptr) {
					//		_alifFrame_setStackPointer(_frame, stackPointer);
					//		_alifCall_instrumentationExc2(
					//			_thread, ALIF_MONITORING_EVENT_C_RAISE,
					//			_frame, thisInstr, global_super, arg);
					//		stackPointer = _alifFrame_getStackPointer(_frame);
					//	}
					//	else {
					//		_alifFrame_setStackPointer(_frame, stackPointer);
					//		AlifIntT err = _alifCall_instrumentation2Args(
					//			_thread, ALIF_MONITORING_EVENT_C_RETURN,
					//			_frame, thisInstr, global_super, arg);
					//		stackPointer = _alifFrame_getStackPointer(_frame);
					//		if (err < 0) {
					//			ALIF_CLEAR(super);
					//		}
					//	}
					//}
					ALIFSTACKREF_CLOSE(global_super_st);
					ALIFSTACKREF_CLOSE(class_st);
					ALIFSTACKREF_CLOSE(self_st);
					if (super == nullptr) goto pop_3_error;
					AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg >> 2);
					stackPointer += -3;
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifObject* attr_o = alifObject_getAttr(super, name);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIF_DECREF(super);
					if (attr_o == nullptr) goto error;
					attr = ALIFSTACKREF_FROMALIFOBJECTSTEAL(attr_o);
					null = _alifStackRefNull_;
				}
				stackPointer[0] = attr;
				if (oparg & 1) stackPointer[1] = null;
				stackPointer += 1 + (oparg & 1);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(MAKE_CELL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				// "initial" is probably nullptr but not if it's an arg (or set
				// via the f_locals proxy before MAKE_CELL has run).
				AlifObject* initial = alifStackRef_asAlifObjectBorrow(GETLOCAL(oparg));
				AlifObject* cell = alifCell_new(initial);
				if (cell == nullptr) {
					goto error;
				}
				SETLOCAL(oparg, ALIFSTACKREF_FROMALIFOBJECTSTEAL(cell));
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(POP_JUMP_IF_FALSE) {
				AlifCodeUnit* thisInstr = _frame->instrPtr = nextInstr;
				nextInstr += 2;
				AlifStackRef cond{};
				/* Skip 1 cache entry */
				cond = stackPointer[-1];
				AlifIntT flag = ALIFSTACKREF_IS(cond, ALIFSTACKREF_FALSE);
				RECORD_BRANCH_TAKEN(thisInstr[1].cache, flag);
				JUMPBY(oparg * flag);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(POP_JUMP_IF_NONE) {
				AlifCodeUnit* const thisInstr = _frame->instrPtr = nextInstr;
				(void)thisInstr;
				nextInstr += 2;
				//INSTRUCTION_STATS(POP_JUMP_IF_NONE);
				AlifStackRef value{};
				AlifStackRef b{};
				AlifStackRef cond{};
				/* Skip 1 cache entry */
				// _IS_NONE
				{
					value = stackPointer[-1];
					if (ALIFSTACKREF_IS(value, ALIFSTACKREF_NONE)) {
						b = ALIFSTACKREF_TRUE;
					}
					else {
						b = ALIFSTACKREF_FALSE;
						ALIFSTACKREF_CLOSE(value);
					}
				}
				// _POP_JUMP_IF_TRUE
				{
					cond = b;
					AlifIntT flag = ALIFSTACKREF_IS(cond, ALIFSTACKREF_TRUE);
					RECORD_BRANCH_TAKEN(thisInstr[1].cache, flag);
					JUMPBY(oparg * flag);
				}
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(POP_JUMP_IF_NOT_NONE) {
				AlifCodeUnit* const thisInstr = _frame->instrPtr = nextInstr;
				(void)thisInstr;
				nextInstr += 2;
				AlifStackRef value{};
				AlifStackRef b{};
				AlifStackRef cond{};
				/* Skip 1 cache entry */
				// _IS_NONE
				{
					value = stackPointer[-1];
					if (ALIFSTACKREF_IS(value, ALIFSTACKREF_NONE)) {
						b = ALIFSTACKREF_TRUE;
					}
					else {
						b = ALIFSTACKREF_FALSE;
						ALIFSTACKREF_CLOSE(value);
					}
				}
				// _POP_JUMP_IF_FALSE
				{
					cond = b;
					AlifIntT flag = ALIFSTACKREF_IS(cond, ALIFSTACKREF_FALSE);
					RECORD_BRANCH_TAKEN(thisInstr[1].cache, flag);
					JUMPBY(oparg * flag);
				}
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(POP_JUMP_IF_TRUE) {
				AlifCodeUnit* const thisInstr = _frame->instrPtr = nextInstr;
				nextInstr += 2;
				AlifStackRef cond{};
				/* Skip 1 cache entry */
				cond = stackPointer[-1];
				AlifIntT flag = ALIFSTACKREF_IS(cond, ALIFSTACKREF_TRUE);
				RECORD_BRANCH_TAKEN(thisInstr[1].cache, flag);
				JUMPBY(oparg * flag);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(RAISE_VARARGS) {
				AlifCodeUnit* const thisInstr = _frame->instrPtr = nextInstr;
				(void)thisInstr;
				nextInstr += 1;
				AlifStackRef* args{};
				args = &stackPointer[-oparg];
				AlifObject* cause = oparg == 2 ? alifStackRef_asAlifObjectSteal(args[1]) : nullptr;
				AlifObject* exc = oparg > 0 ? alifStackRef_asAlifObjectSteal(args[0]) : nullptr;
				stackPointer += -oparg;
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT err = do_raise(_thread, exc, cause);
				stackPointer = _alifFrame_getStackPointer(_frame);
				if (err) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					//monitor_reraise(_thread, _frame, thisInstr);
					stackPointer = _alifFrame_getStackPointer(_frame);
					goto exception_unwind;
				}
				goto error;
			} // ------------------------------------------------------------ //
			TARGET(RERAISE) {
				AlifCodeUnit* const thisInstr = _frame->instrPtr = nextInstr;
				(void)thisInstr;
				nextInstr += 1;
				//INSTRUCTION_STATS(RERAISE);
				AlifStackRef* values{};
				AlifStackRef exc_st{};
				exc_st = stackPointer[-1];
				values = &stackPointer[-1 - oparg];
				AlifObject *exc = alifStackRef_asAlifObjectSteal(exc_st);
				if (oparg) {
					AlifObject *lasti = alifStackRef_asAlifObjectBorrow(values[0]);
					if (ALIFLONG_CHECK(lasti)) {
						stackPointer += -1;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_frame->instrPtr = _alifFrame_getBytecode(_frame) + alifLong_asLong(lasti);
						stackPointer = _alifFrame_getStackPointer(_frame);					}
					else {
						stackPointer += -1;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifErr_setString(_thread, _alifExcSystemError_, "lasti ليس رقم صحيح");
						stackPointer = _alifFrame_getStackPointer(_frame);
						ALIF_DECREF(exc);
						goto error;
					}
					stackPointer += 1;
				}
				stackPointer += -1;
				_alifFrame_setStackPointer(_frame, stackPointer);
				_alifErr_setRaisedException(_thread, exc);
				//monitor_reraise(_thread, _frame, this_instr);
				stackPointer = _alifFrame_getStackPointer(_frame);
				goto exception_unwind;
			} // ------------------------------------------------------------ //
			TARGET(SET_ADD) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				//INSTRUCTION_STATS(SET_ADD);
				AlifStackRef set{};
				AlifStackRef v{};
				v = stackPointer[-1];
				set = stackPointer[-2 - (oparg-1)];
				_alifFrame_setStackPointer(_frame, stackPointer);
				int err = alifSet_add(alifStackRef_asAlifObjectBorrow(set),
					alifStackRef_asAlifObjectBorrow(v));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(v);
				if (err) goto pop_1_error;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(SET_FUNCTION_ATTRIBUTE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef attrSt{};
				AlifStackRef funcIn{};
				AlifStackRef funcOut{};
				funcIn = stackPointer[-1];
				attrSt = stackPointer[-2];
				AlifObject* func = alifStackRef_asAlifObjectBorrow(funcIn);
				AlifObject* attr = alifStackRef_asAlifObjectSteal(attrSt);
				funcOut = funcIn;
				AlifUSizeT offset = _alifFunctionAttributeOffsets_[oparg];
				AlifObject** ptr = (AlifObject**)(((char*)func) + offset);
				*ptr = attr;
				stackPointer[-2] = funcOut;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(SET_UPDATE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef set{};
				AlifStackRef iterable{};
				iterable = stackPointer[-1];
				set = stackPointer[-2 - (oparg-1)];
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT err = _alifSet_update(alifStackRef_asAlifObjectBorrow(set),
					alifStackRef_asAlifObjectBorrow(iterable));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(iterable);
				if (err < 0) goto pop_1_error;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(STORE_ATTR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 5;
				PREDICTED(STORE_ATTR);
				AlifCodeUnit* thisInstr = nextInstr - 5;
				AlifStackRef owner{};
				AlifStackRef v{};
				// _SPECIALIZE_STORE_ATTR
				{
					owner = stackPointer[-1];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_storeAttr(owner, nextInstr, name);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(STORE_ATTR);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				/* Skip 3 cache entries */
				// _STORE_ATTR
				{
					v = stackPointer[-2];
					AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifIntT err = alifObject_setAttr(alifStackRef_asAlifObjectBorrow(owner),
						name, alifStackRef_asAlifObjectBorrow(v));
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(v);
					ALIFSTACKREF_CLOSE(owner);
					if (err) goto pop_2_error;
				}
				stackPointer += -2;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(STORE_DEREF) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef v{};
				v = stackPointer[-1];
				AlifCellObject* cell = (AlifCellObject*)alifStackRef_asAlifObjectBorrow(GETLOCAL(oparg));
				_alifFrame_setStackPointer(_frame, stackPointer);
				alifCell_setTakeRef(cell, alifStackRef_asAlifObjectSteal(v));
				stackPointer = _alifFrame_getStackPointer(_frame);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(STORE_FAST) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = stackPointer[-1];
				SETLOCAL(oparg, value);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(STORE_GLOBAL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef v{};
				v = stackPointer[-1];
				AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT err = alifDict_setItem(GLOBALS(), name, alifStackRef_asAlifObjectBorrow(v));
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(v);
				if (err) goto pop_1_error;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(STORE_NAME) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef v{};
				v = stackPointer[-1];
				AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
				AlifObject* ns = LOCALS();
				AlifIntT err{};
				if (ns == nullptr) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					_alifErr_format(_thread, _alifExcSystemError_,
						"متغير locals اصبح فارغ عند تخزين %R", name);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(v);
					if (true) goto pop_1_error;
				}
				if (ALIFDICT_CHECKEXACT(ns)) {
					_alifFrame_setStackPointer(_frame, stackPointer);
					err = alifDict_setItem(ns, name, alifStackRef_asAlifObjectBorrow(v));
					stackPointer = _alifFrame_getStackPointer(_frame);
				}
				else {
					_alifFrame_setStackPointer(_frame, stackPointer);
					err = alifObject_setItem(ns, name, alifStackRef_asAlifObjectBorrow(v));
					stackPointer = _alifFrame_getStackPointer(_frame);
				}
				ALIFSTACKREF_CLOSE(v);
				if (err) goto pop_1_error;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(SWAP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef bottomIn{};
				AlifStackRef bottomOut{};
				AlifStackRef topIn{};
				AlifStackRef topOut{};
				topIn = stackPointer[-1];
				bottomIn = stackPointer[-2 - (oparg - 2)];
				topOut = topIn;
				bottomOut = bottomIn;
				stackPointer[-2 - (oparg - 2)] = topOut;
				stackPointer[-1] = bottomOut;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(UNPACK_SEQUENCE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(UNPACK_SEQUENCE);
				AlifCodeUnit* thisInstr = nextInstr - 2;
				AlifStackRef seq{};
				AlifStackRef* output{};
				// _SPECIALIZE_UNPACK_SEQUENCE
				{
					seq = stackPointer[-1];
					//uint16_t counter = read_u16(&thisInstr[1].cache);
				#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifFrame_setStackPointer(_frame, stackPointer);
						_alifSpecialize_UnpackSequence(seq, nextInstr, oparg);
						stackPointer = _alifFrame_getStackPointer(_frame);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(UNPACK_SEQUENCE);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
				#endif  /* ENABLE_SPECIALIZATION */
				}
				// _UNPACK_SEQUENCE
				{
					output = &stackPointer[-1];
					AlifStackRef* top = output + oparg;
					_alifFrame_setStackPointer(_frame, stackPointer);
					AlifIntT res = _alifEval_unpackIterableStackRef(_thread, seq, oparg, -1, top);
					stackPointer = _alifFrame_getStackPointer(_frame);
					ALIFSTACKREF_CLOSE(seq);
					if (res == 0) goto pop_1_error;
				}
				stackPointer += -1 + oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(YIELD_VALUE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef retval{};
				AlifStackRef value{};
				retval = stackPointer[-1];
				_frame->instrPtr++;
				AlifGenObject* gen = _alifGen_getGeneratorFromFrame(_frame);
				gen->giFrameState = AlifFrameState_::Frame_Suspended + oparg;
				AlifStackRef temp = retval;
				stackPointer += -1;
				_alifFrame_setStackPointer(_frame, stackPointer);
				_thread->excInfo = gen->giExcState.previousItem;
				gen->giExcState.previousItem = nullptr;
				_alif_leaveRecursiveCallAlif(_thread);
				AlifInterpreterFrame* genFrame = _frame;
				_frame = _thread->currentFrame = _frame->previous;
				genFrame->previous = nullptr;
				stackPointer = _alifFrame_getStackPointer(_frame);
				LOAD_IP(1 + INLINE_CACHE_ENTRIES_SEND);
				value = temp;
				//LLTRACE_RESUME_FRAME();
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(RESUME) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				PREDICTED(RESUME);
				AlifCodeUnit* thisInstr = nextInstr - 1;
				// _LOAD_BYTECODE
				{
					if (_frame->tlbcIndex !=
						((AlifThreadImpl*)_thread)->tlbcIndex) {
						_alifFrame_setStackPointer(_frame, stackPointer);
						AlifCodeUnit* bytecode =
							_alifEval_getExecutableCode(_thread, _alifFrame_getCode(_frame));
						stackPointer = _alifFrame_getStackPointer(_frame);
						if (bytecode == nullptr) goto error;
						_alifFrame_setStackPointer(_frame, stackPointer);
						ptrdiff_t off = thisInstr - _alifFrame_getBytecode(_frame);
						stackPointer = _alifFrame_getStackPointer(_frame);
						_frame->tlbcIndex = ((AlifThreadImpl*)_thread)->tlbcIndex;
						_frame->instrPtr = bytecode + off;
						nextInstr = _frame->instrPtr;
						DISPATCH();
					}
				}
				// _MAYBE_INSTRUMENT
				{
					if (_thread->tracing == 0) {
						uintptr_t globalVersion = alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ~ALIF_EVAL_EVENTS_MASK;
						uintptr_t codeVersion = alifAtomic_loadUintptrAcquire(&_alifFrame_getCode(_frame)->instrumentationVersion);
						if (codeVersion != globalVersion) {
							_alifFrame_setStackPointer(_frame, stackPointer);
							AlifIntT err = _alif_instrument(_alifFrame_getCode(_frame), _thread->interpreter);
							stackPointer = _alifFrame_getStackPointer(_frame);
							if (err) {
								goto error;
							}
							nextInstr = thisInstr;
							DISPATCH();
						}
					}
				}
				// _QUICKEN_RESUME
				{
				#if ENABLE_SPECIALIZATION_FT
					if (_thread->tracing == 0 and thisInstr->op.code == RESUME) {
						alifAtomic_storeUint8Relaxed(&thisInstr->op.code, RESUME_CHECK);
					}
				#endif  /* ENABLE_SPECIALIZATION_FT */
				}
				// _CHECK_PERIODIC_IF_NOT_YIELD_FROM
				{
					if ((oparg & RESUME_OPARG_LOCATION_MASK) < RESUME_AFTER_YIELD_FROM) {
						//ALIF_CHECK_EMSCRIPTEN_SIGNALS_PERIODICALLY();
						QSBR_QUIESCENT_STATE(_thread); \
							if (alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ALIF_EVAL_EVENTS_MASK) {
								_alifFrame_setStackPointer(_frame, stackPointer);
								//AlifIntT err = _alif_handlePending(_thread);
								stackPointer = _alifFrame_getStackPointer(_frame);
								//if (err != 0) goto error;
							}
					}
				}
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP_ADD_FLOAT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef res{};
				// _GUARD_BOTH_FLOAT
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					DEOPT_IF(!ALIFFLOAT_CHECKEXACT(leftObj), BINARY_OP);
					DEOPT_IF(!ALIFFLOAT_CHECKEXACT(rightObj), BINARY_OP);
				}
				/* Skip 1 cache entry */
				// _BINARY_OP_ADD_FLOAT
				{
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					//STAT_INC(BINARY_OP, hit);
					double dres =
						((AlifFloatObject*)leftObj)->val +
						((AlifFloatObject*)rightObj)->val;
					AlifObject* resObj = _alifFloat_fromDoubleConsumeInputs(left, right, dres);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP_ADD_INT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				//INSTRUCTION_STATS(BINARY_OP_ADD_INT);
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef res{};
				// _GUARD_BOTH_INT
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					AlifObject *leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject *rightObj = alifStackRef_asAlifObjectBorrow(right);
					DEOPT_IF(!ALIFLONG_CHECKEXACT(leftObj), BINARY_OP);
					DEOPT_IF(!ALIFLONG_CHECKEXACT(rightObj), BINARY_OP);
				}
				/* Skip 1 cache entry */
				// _BINARY_OP_ADD_INT
				{
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					//STAT_INC(BINARY_OP, hit);
					AlifObject* resObj = _alifLong_add((AlifLongObject*)leftObj, (AlifLongObject*)rightObj);
					ALIFSTACKREF_CLOSE_SPECIALIZED(right, (Destructor)alifMem_objFree);
					ALIFSTACKREF_CLOSE_SPECIALIZED(left, (Destructor)alifMem_objFree);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP_ADD_UNICODE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef res{};
				// _GUARD_BOTH_UNICODE
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					DEOPT_IF(!ALIFUSTR_CHECKEXACT(leftObj), BINARY_OP);
					DEOPT_IF(!ALIFUSTR_CHECKEXACT(rightObj), BINARY_OP);
				}
				/* Skip 1 cache entry */
				// _BINARY_OP_ADD_UNICODE
				{
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					//STAT_INC(BINARY_OP, hit);
					AlifObject* resObj = alifUStr_concat(leftObj, rightObj);
					ALIFSTACKREF_CLOSE_SPECIALIZED(left, _alifUStr_exactDealloc);
					ALIFSTACKREF_CLOSE_SPECIALIZED(right, _alifUStr_exactDealloc);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP_MULTIPLY_FLOAT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef res{};
				// _GUARD_BOTH_FLOAT
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					DEOPT_IF(!ALIFFLOAT_CHECKEXACT(leftObj), BINARY_OP);
					DEOPT_IF(!ALIFFLOAT_CHECKEXACT(rightObj), BINARY_OP);
				}
				/* Skip 1 cache entry */
				// _BINARY_OP_MULTIPLY_FLOAT
				{
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					//STAT_INC(BINARY_OP, hit);
					double dres =
						((AlifFloatObject *)leftObj)->val *
						((AlifFloatObject *)rightObj)->val;
					AlifObject* resObj = _alifFloat_fromDoubleConsumeInputs(left, right, dres);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP_MULTIPLY_INT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				//INSTRUCTION_STATS(BINARY_OP_MULTIPLY_INT);
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef res{};
				// _GUARD_BOTH_INT
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					AlifObject *leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject *rightObj = alifStackRef_asAlifObjectBorrow(right);
					DEOPT_IF(!ALIFLONG_CHECKEXACT(leftObj), BINARY_OP);
					DEOPT_IF(!ALIFLONG_CHECKEXACT(rightObj), BINARY_OP);
				}
				/* Skip 1 cache entry */
				// _BINARY_OP_MULTIPLY_INT
				{
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					AlifObject* resObj = _alifLong_multiply((AlifLongObject*)leftObj, (AlifLongObject*)rightObj);
					ALIFSTACKREF_CLOSE_SPECIALIZED(right, (Destructor)alifMem_objFree);
					ALIFSTACKREF_CLOSE_SPECIALIZED(left, (Destructor)alifMem_objFree);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP_SUBTRACT_FLOAT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef res{};
				// _GUARD_BOTH_FLOAT
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					DEOPT_IF(!ALIFFLOAT_CHECKEXACT(leftObj), BINARY_OP);
					DEOPT_IF(!ALIFFLOAT_CHECKEXACT(rightObj), BINARY_OP);
				}
				/* Skip 1 cache entry */
				// _BINARY_OP_SUBTRACT_FLOAT
				{
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					//STAT_INC(BINARY_OP, hit);
					double dres =
						((AlifFloatObject*)leftObj)->val -
						((AlifFloatObject*)rightObj)->val;
					AlifObject* resObj = _alifFloat_fromDoubleConsumeInputs(left, right, dres);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP_SUBTRACT_INT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				//INSTRUCTION_STATS(BINARY_OP_SUBTRACT_INT);
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef res{};
				// _GUARD_BOTH_INT
				{
					right = stackPointer[-1];
					left = stackPointer[-2];
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					DEOPT_IF(!ALIFLONG_CHECKEXACT(leftObj), BINARY_OP);
					DEOPT_IF(!ALIFLONG_CHECKEXACT(rightObj), BINARY_OP);
				}
				/* Skip 1 cache entry */
				// _BINARY_OP_SUBTRACT_INT
				{
					AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
					AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
					//STAT_INC(BINARY_OP, hit);
					AlifObject* resObj = _alifLong_subtract((AlifLongObject*)leftObj,
						(AlifLongObject*)rightObj);
					ALIFSTACKREF_CLOSE_SPECIALIZED(right, (Destructor)alifMem_objFree);
					ALIFSTACKREF_CLOSE_SPECIALIZED(left, (Destructor)alifMem_objFree);
					if (resObj == nullptr) goto pop_2_error;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CONTAINS_OP_DICT) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				//INSTRUCTION_STATS(CONTAINS_OP_DICT);
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef b{};
				/* Skip 1 cache entry */
				right = stackPointer[-1];
				left = stackPointer[-2];
				AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
				AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
				DEOPT_IF(!ALIFDICT_CHECKEXACT(rightObj), CONTAINS_OP);
				//STAT_INC(CONTAINS_OP, hit);
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT res = alifDict_contains(rightObj, leftObj);
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(left);
				ALIFSTACKREF_CLOSE(right);
				if (res < 0) goto pop_2_error;
				b = (res ^ oparg) ? ALIFSTACKREF_TRUE : ALIFSTACKREF_FALSE;
				stackPointer[-2] = b;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CONTAINS_OP_SET) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				//INSTRUCTION_STATS(CONTAINS_OP_SET);
				AlifStackRef left{};
				AlifStackRef right{};
				AlifStackRef b{};
				/* Skip 1 cache entry */
				right = stackPointer[-1];
				left = stackPointer[-2];
				AlifObject* leftObj = alifStackRef_asAlifObjectBorrow(left);
				AlifObject* rightObj = alifStackRef_asAlifObjectBorrow(right);
				DEOPT_IF(!(ALIFSET_CHECKEXACT(rightObj) || ALIFFROZENSET_CHECKEXACT(rightObj)), CONTAINS_OP);
				//STAT_INC(CONTAINS_OP, hit);
				// Note: both set and frozenset use the same seq_contains method!
				_alifFrame_setStackPointer(_frame, stackPointer);
				AlifIntT res = _alifSet_contains((AlifSetObject*)rightObj, leftObj);
				stackPointer = _alifFrame_getStackPointer(_frame);
				ALIFSTACKREF_CLOSE(left);
				ALIFSTACKREF_CLOSE(right);
				if (res < 0) goto pop_2_error;
				b = (res ^ oparg) ? ALIFSTACKREF_TRUE : ALIFSTACKREF_FALSE;
				stackPointer[-2] = b;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_CONST_IMMORTAL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				//INSTRUCTION_STATS(LOAD_CONST_IMMORTAL);
				AlifStackRef value{};
				AlifObject* obj = GETITEM(FRAME_CO_CONSTS, oparg);
				value = ALIFSTACKREF_FROMALIFOBJECTIMMORTAL(obj);
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(RESUME_CHECK) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				//INSTRUCTION_STATS(RESUME_CHECK);
				uintptr_t eval_breaker = alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker);
				uintptr_t version = alifAtomic_loadUintptrAcquire(&_alifFrame_getCode(_frame)->instrumentationVersion);
				DEOPT_IF(eval_breaker != version, RESUME);
				DEOPT_IF(_frame->tlbcIndex !=
					((AlifThreadImpl*)_thread)->tlbcIndex, RESUME);
				DISPATCH();
			} // ------------------------------------------------------------ //
		}


		/* يجب أن لا يتم الوصول لهذه الحالة مطلقا.
			كل أمر يجب أن ينتهي بـ
			DISPATCH()
			او
			goto error.
		*/
		ALIF_UNREACHABLE();
		//* alif //* delete
		printf("يبدو أنه يوجد حالة غير مكتملة في مفسر اللغة،						\
		\nلغة ألف لا تزال تحت التطوير لذلك نرجو التواصل							\
		\nمع فريق تطوير اللغة على احد وسائل التواصل المدرجة في الموقع الرسمي		\
		\n www.aliflang.org  \n\n");
		exit(-7);
		//* alif

pop_4_error:
		STACK_SHRINK(1);
pop_3_error:
		STACK_SHRINK(1);
pop_2_error:
		STACK_SHRINK(1);
pop_1_error:
		STACK_SHRINK(1);
error:

		if (!_alifFrame_isIncomplete(_frame)) {
			AlifFrameObject* f = _alifFrame_getFrameObject(_frame);
			if (f != nullptr) {
				alifTraceBack_here(f);
			}
		}

		//_alifEval_monitorRaise(_thread, _frame, nextInstr - 1);

exception_unwind:
		{
			/* We can't use _frame->instrPtr here, as RERAISE may have set it */
			AlifIntT offset = INSTR_OFFSET() - 1;
			AlifIntT level, handler, lasti;
			if (get_exceptionHandler(_alifFrame_getCode(_frame), offset, &level, &handler, &lasti) == 0) {
				/* Pop remaining stack entries. */
				AlifStackRef* stackbase = _alifFrame_stackBase(_frame);
				while (stackPointer > stackbase) {
					ALIFSTACKREF_XCLOSE(POP());
				}
				_alifFrame_setStackPointer(_frame, stackPointer);
				//monitor_unwind(_thread, _frame, nextInstr - 1);
				goto exit_unwind;
			}

			AlifStackRef* new_top = _alifFrame_stackBase(_frame) + level;
			while (stackPointer > new_top) {
				ALIFSTACKREF_XCLOSE(POP());
			}
			if (lasti) {
				AlifIntT frame_lasti = ALIFINTERPRETERFRAME_LASTI(_frame);
				AlifObject* lasti = alifLong_fromLong(frame_lasti);
				if (lasti == nullptr) {
					goto exception_unwind;
				}
				PUSH(ALIFSTACKREF_FROMALIFOBJECTSTEAL(lasti));
			}

			AlifObject* exc = _alifErr_getRaisedException(_thread);
			PUSH(ALIFSTACKREF_FROMALIFOBJECTSTEAL(exc));
			nextInstr = _alifFrame_getBytecode(_frame) + handler;

			//if (monitor_handled(_thread, _frame, nextInstr, exc) < 0) {
			//	goto exception_unwind;
			//}

			/* Resume normal execution */
			DISPATCH();
		}
	}





exit_unwind:
	_alif_leaveRecursiveCallAlif(_thread);
	AlifInterpreterFrame* dying; dying = _frame;
	_frame = _thread->currentFrame = dying->previous;
	_alifEval_frameClearAndPop(_thread, dying);
	_frame->returnOffset = 0;
	if (_frame == &entryFrame) {
		_thread->currentFrame = _frame->previous;
		_thread->cppRecursionRemaining += ALIF_EVAL_CPP_STACK_UNITS;
		return nullptr;
	}

resume_with_error:
	nextInstr = _frame->instrPtr;
	stackPointer = _alifFrame_getStackPointer(_frame);
	goto error;

}


static void format_missing(AlifThread* _thread, const char* _kind,
	AlifCodeObject* _co, AlifObject* _names, AlifObject* _qualname) { // 1107
	AlifIntT err{};
	AlifSizeT len = ALIFLIST_GET_SIZE(_names);
	AlifObject* nameStr{}, * comma{}, * tail{}, * tmp{};

	switch (len) {
	case 1:
		nameStr = ALIFLIST_GET_ITEM(_names, 0);
		ALIF_INCREF(nameStr);
		break;
	case 2:
		nameStr = alifUStr_fromFormat("%U و %U",
			ALIFLIST_GET_ITEM(_names, len - 2),
			ALIFLIST_GET_ITEM(_names, len - 1));
		break;
	default:
		tail = alifUStr_fromFormat(", %U, و %U",
			ALIFLIST_GET_ITEM(_names, len - 2),
			ALIFLIST_GET_ITEM(_names, len - 1));
		if (tail == nullptr)
			return;

		err = alifList_setSlice(_names, len - 2, len, nullptr);
		if (err == -1) {
			ALIF_DECREF(tail);
			return;
		}
		comma = alifUStr_fromString(", ");
		if (comma == nullptr) {
			ALIF_DECREF(tail);
			return;
		}
		tmp = alifUStr_join(comma, _names);
		ALIF_DECREF(comma);
		if (tmp == nullptr) {
			ALIF_DECREF(tail);
			return;
		}
		nameStr = alifUStr_concat(tmp, tail);
		ALIF_DECREF(tmp);
		ALIF_DECREF(tail);
		break;
	}
	if (nameStr == nullptr)
		return;
	_alifErr_format(_thread, _alifExcTypeError_,
		"%U() مفقود %i معامل%s %s%s: %U",
		_qualname, len,
		len <= 2 ? "" : "ات", _kind,
		len <= 2 ? "" : "ات", nameStr);
	ALIF_DECREF(nameStr);
}


static void missing_arguments(AlifThread* _thread, AlifCodeObject* _co,
	AlifSizeT _missing, AlifSizeT _defcount,
	AlifStackRef* _localsPlus, AlifObject* _qualname) { // 1170
	AlifSizeT i{}, j = 0;
	AlifSizeT start{}, end{};
	AlifIntT positional = (_defcount != -1);
	const char* kind = positional ? "مكاني" : "كلمة-مفتاحية";
	AlifObject* missingNames{};

	/* Compute the names of the arguments that are missing. */
	missingNames = alifList_new(_missing);
	if (missingNames == nullptr)
		return;
	if (positional) {
		start = 0;
		end = _co->argCount - _defcount;
	}
	else {
		start = _co->argCount;
		end = start + _co->kwOnlyArgCount;
	}
	for (i = start; i < end; i++) {
		if (ALIFSTACKREF_ISNULL(_localsPlus[i])) {
			AlifObject* raw = ALIFTUPLE_GET_ITEM(_co->localsPlusNames, i);
			AlifObject* name = alifObject_repr(raw);
			if (name == nullptr) {
				ALIF_DECREF(missingNames);
				return;
			}
			ALIFLIST_SET_ITEM(missingNames, j++, name);
		}
	}
	format_missing(_thread, kind, _co, missingNames, _qualname);
	ALIF_DECREF(missingNames);
}


static void tooMany_positional(AlifThread* _thread, AlifCodeObject* _co,
	AlifSizeT _given, AlifObject* _defaults,
	AlifStackRef* _localsPlus, AlifObject* _qualname) { // 1209
	AlifIntT plural{};
	AlifSizeT kwOnlyGiven = 0;
	AlifSizeT i{};
	AlifObject* sig{}, * kwOnlySig{};
	AlifSizeT coArgCount = _co->argCount;

	/* Count missing keyword-only args. */
	for (i = coArgCount; i < coArgCount + _co->kwOnlyArgCount; i++) {
		if (alifStackRef_asAlifObjectBorrow(_localsPlus[i]) != nullptr) {
			kwOnlyGiven++;
		}
	}
	AlifSizeT defcount = _defaults == nullptr ? 0 : ALIFTUPLE_GET_SIZE(_defaults);
	if (defcount) {
		AlifSizeT atleast = coArgCount - defcount;
		plural = 1;
		sig = alifUStr_fromFormat("من %zd إلى %zd", atleast, coArgCount);
	}
	else {
		plural = (coArgCount != 1);
		sig = alifUStr_fromFormat("%zd", coArgCount);
	}
	if (sig == nullptr)
		return;
	if (kwOnlyGiven) {
		const char* format = " معامل مكاني%s (و %zd معامل مفتاحي فقط%s)";
		kwOnlySig = alifUStr_fromFormat(format,
			_given != 1 ? "ات" : "",
			kwOnlyGiven
			,kwOnlyGiven != 1 ? "ات" : "");
		if (kwOnlySig == nullptr) {
			ALIF_DECREF(sig);
			return;
		}
	}
	else {
		/* This will not fail. */
		kwOnlySig = alif_getConstant(ALIF_CONSTANT_EMPTY_STR);
	}
	_alifErr_format(_thread, _alifExcTypeError_,
		"%U() تحتاج %U معامل مكاني%s ولكن تم تمرير %zd%U %s",
		_qualname, sig, plural ? "ات" : "", _given, kwOnlySig
		/*,_given == 1 and !kwOnlyGiven ? "كان" : "كانوا"*/);
	ALIF_DECREF(sig);
	ALIF_DECREF(kwOnlySig);
}


static AlifIntT positionalOnly_passedAsKeyword(AlifThread* _tstate,
	AlifCodeObject* _co, AlifSizeT _kWCount,
	AlifObject* _kWNames, AlifObject* _qualname) { // 1267
	AlifIntT posOnlyConflicts = 0;
	AlifObject* posOnlyNames = alifList_new(0);
	if (posOnlyNames == nullptr) {
		goto fail;
	}
	for (AlifIntT k = 0; k < _co->posOnlyArgCount; k++) {
		AlifObject* posOnlyName = ALIFTUPLE_GET_ITEM(_co->localsPlusNames, k);

		for (AlifIntT k2 = 0; k2 < _kWCount; k2++) {
			AlifObject* kWName = ALIFTUPLE_GET_ITEM(_kWNames, k2);
			if (kWName == posOnlyName) {
				if (alifList_append(posOnlyNames, kWName) != 0) {
					goto fail;
				}
				posOnlyConflicts++;
				continue;
			}

			AlifIntT cmp_ = alifObject_richCompareBool(posOnlyName, kWName, ALIF_EQ);

			if (cmp_ > 0) {
				if (alifList_append(posOnlyNames, kWName) != 0) {
					goto fail;
				}
				posOnlyConflicts++;
			}
			else if (cmp_ < 0) {
				goto fail;
			}

		}
	}
	if (posOnlyConflicts) {
		AlifObject* comma = alifUStr_fromString(", ");
		if (comma == nullptr) {
			goto fail;
		}
		AlifObject* errorNames = alifUStr_join(comma, posOnlyNames);
		ALIF_DECREF(comma);
		if (errorNames == nullptr) {
			goto fail;
		}
		_alifErr_format(_tstate, _alifExcTypeError_,
			"%U() تم تمرير معاملات مكانية-فقط "
			" ك معاملات مفتاحية: '%U'",
			_qualname, errorNames);
		ALIF_DECREF(errorNames);
		goto fail;
	}

	ALIF_DECREF(posOnlyNames);
	return 0;

fail:
	ALIF_DECREF(posOnlyNames);
	return 1;

}


static inline unsigned char*
scanBack_toEntryStart(unsigned char* _p) { // 1370
	for (; (_p[0] & 128) == 0; _p--);
	return _p;
}

static inline unsigned char*
skipTo_nextEntry(unsigned char* _p, unsigned char* _end) {
	while (_p < _end and ((_p[0] & 128) == 0)) {
		_p++;
	}
	return _p;
}

#define MAX_LINEAR_SEARCH 40

static AlifIntT get_exceptionHandler(AlifCodeObject* code, AlifIntT index,
	AlifIntT* level, AlifIntT* handler, AlifIntT* lasti) { // 1387
	unsigned char* start = (unsigned char*)ALIFBYTES_AS_STRING(code->exceptiontable);
	unsigned char* end = start + ALIFBYTES_GET_SIZE(code->exceptiontable);
	if (end - start > MAX_LINEAR_SEARCH) {
		AlifIntT offset;
		parse_varint(start, &offset);
		if (offset > index) {
			return 0;
		}
		do {
			unsigned char* mid = start + ((end - start) >> 1);
			mid = scanBack_toEntryStart(mid);
			parse_varint(mid, &offset);
			if (offset > index) {
				end = mid;
			}
			else {
				start = mid;
			}

		}
		while (end - start > MAX_LINEAR_SEARCH);
	}
	unsigned char* scan = start;
	while (scan < end) {
		AlifIntT start_offset, size;
		scan = parse_varint(scan, &start_offset);
		if (start_offset > index) {
			break;
		}
		scan = parse_varint(scan, &size);
		if (start_offset + size > index) {
			scan = parse_varint(scan, handler);
			AlifIntT depthAndLasti{};
			parse_varint(scan, &depthAndLasti);
			*level = depthAndLasti >> 1;
			*lasti = depthAndLasti & 1;
			return 1;
		}
		scan = skipTo_nextEntry(scan, end);
	}
	return 0;
}


static AlifIntT initialize_locals(AlifThread* _thread, AlifFunctionObject* _func,
	AlifStackRef* _localsPlus, AlifStackRef const* _args,
	AlifSizeT _argCount, AlifObject* _kwNames) { // 1399
	AlifCodeObject* co = (AlifCodeObject*)_func->code;
	const AlifSizeT totalArgs = co->argCount + co->kwOnlyArgCount;

	AlifObject* kwdict{};
	AlifSizeT i{};
	AlifSizeT j{}, n{};
	if (co->flags & CO_VARKEYWORDS) {
		kwdict = alifDict_new();
		if (kwdict == nullptr) {
			goto failPrePositional;
		}
		i = totalArgs;
		if (co->flags & CO_VARARGS) {
			i++;
		}
		_localsPlus[i] = ALIFSTACKREF_FROMALIFOBJECTSTEAL(kwdict);
	}
	else {
		kwdict = nullptr;
	}

	if (_argCount > co->argCount) {
		n = co->argCount;
	}
	else {
		n = _argCount;
	}
	for (j = 0; j < n; j++) {
		_localsPlus[j] = _args[j];
	}

	if (co->flags & CO_VARARGS) {
		AlifObject* u = nullptr;
		if (_argCount == n) {
			u = (AlifObject*)&ALIF_SINGLETON(tupleEmpty);
		}
		else {
			u = _alifTuple_fromStackRefSteal(_args + n, _argCount - n);
		}
		if (u == nullptr) {
			goto fail_post_positional;
		}
		_localsPlus[totalArgs] = ALIFSTACKREF_FROMALIFOBJECTSTEAL(u);
	}
	else if (_argCount > n) {
		/* Too many positional args. Error is reported later */
		for (j = n; j < _argCount; j++) {
			ALIFSTACKREF_CLOSE(_args[j]);
		}
	}

	/* Handle keyword arguments */
	if (_kwNames != nullptr) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (i = 0; i < kwcount; i++) {
			AlifObject** varNames{};
			AlifObject* keyword = ALIFTUPLE_GET_ITEM(_kwNames, i);
			AlifStackRef valueStackRef = _args[i + _argCount];
			AlifSizeT j{};

			if (keyword == nullptr or !ALIFUSTR_CHECK(keyword)) {
				_alifErr_format(_thread, _alifExcTypeError_,
					"%U() الكلمات المفتاحية يجب أن تكون من نوع نص",
					_func->qualname);
				goto kw_fail;
			}

			varNames = ((AlifTupleObject*)(co->localsPlusNames))->item;
			for (j = co->posOnlyArgCount; j < totalArgs; j++) {
				AlifObject* varname = varNames[j];
				if (varname == keyword) {
					goto kw_found;
				}
			}

			/* Slow fallback, just in case */
			for (j = co->posOnlyArgCount; j < totalArgs; j++) {
				AlifObject* varname = varNames[j];
				AlifIntT cmp = alifObject_richCompareBool(keyword, varname, ALIF_EQ);
				if (cmp > 0) {
					goto kw_found;
				}
				else if (cmp < 0) {
					goto kw_fail;
				}
			}

			if (kwdict == nullptr) {

				if (co->posOnlyArgCount
					and positionalOnly_passedAsKeyword(_thread, co,
						kwcount, _kwNames,
						_func->qualname)) {
					goto kw_fail;
				}

				AlifObject* suggestionKeyword = nullptr;
				if (totalArgs > co->posOnlyArgCount) {
					AlifObject* possibleKeywords = alifList_new(totalArgs - co->posOnlyArgCount);

					if (!possibleKeywords) {
						alifErr_clear();
					}
					else {
						for (AlifSizeT k = co->posOnlyArgCount; k < totalArgs; k++) {
							ALIFLIST_SET_ITEM(possibleKeywords, k - co->posOnlyArgCount, varNames[k]);
						}

						suggestionKeyword = _alif_calculateSuggestions(possibleKeywords, keyword);
						ALIF_DECREF(possibleKeywords);
					}
				}

				if (suggestionKeyword) {
					_alifErr_format(_thread, _alifExcTypeError_,
						"%U() تم تمرير معامل مفتاحي غير صحيح '%S'. هل تقصد '%S'?",
						_func->qualname, keyword, suggestionKeyword);
					ALIF_DECREF(suggestionKeyword);
				}
				else {
					_alifErr_format(_thread, _alifExcTypeError_,
						"%U() تم تمرير معامل مفتاحي غير صحيح '%S'",
						_func->qualname, keyword);
				}

				goto kw_fail;
			}

			if (alifDict_setItem(kwdict, keyword, alifStackRef_asAlifObjectBorrow(valueStackRef)) == -1) {
				goto kw_fail;
			}
			ALIFSTACKREF_CLOSE(valueStackRef);
			continue;

kw_fail:
			for (; i < kwcount; i++) {
				ALIFSTACKREF_CLOSE(_args[i + _argCount]);
			}
			goto fail_post_args;

kw_found:
			if (alifStackRef_asAlifObjectBorrow(_localsPlus[j]) != nullptr) {
				_alifErr_format(_thread, _alifExcTypeError_,
					"%U() تم تمرير قيم متعددة للمعامل '%S'",
					_func->qualname, keyword);
				goto kw_fail;
			}
			_localsPlus[j] = valueStackRef;
		}
	}

	/* Check the number of positional arguments */
	if ((_argCount > co->argCount) and !(co->flags & CO_VARARGS)) {
		tooMany_positional(_thread, co, _argCount, _func->defaults, _localsPlus,
			_func->qualname);
		goto fail_post_args;
	}

	/* Add missing positional arguments (copy default values from defs) */
	if (_argCount < co->argCount) {
		AlifSizeT defcount = _func->defaults == nullptr ? 0 : ALIFTUPLE_GET_SIZE(_func->defaults);
		AlifSizeT m = co->argCount - defcount;
		AlifSizeT missing = 0;
		for (i = _argCount; i < m; i++) {
			if (ALIFSTACKREF_ISNULL(_localsPlus[i])) {
				missing++;
			}
		}
		if (missing) {
			missing_arguments(_thread, co, missing, defcount, _localsPlus,
				_func->qualname);
			goto fail_post_args;
		}
		if (n > m)
			i = n - m;
		else
			i = 0;
		if (defcount) {
			AlifObject** defs = &ALIFTUPLE_GET_ITEM(_func->defaults, 0);
			for (; i < defcount; i++) {
				if (alifStackRef_asAlifObjectBorrow(_localsPlus[m + i]) == nullptr) {
					AlifObject* def = defs[i];
					_localsPlus[m + i] = ALIFSTACKREF_FROMALIFOBJECTNEW(def);
				}
			}
		}
	}

	/* Add missing keyword arguments (copy default values from kwdefs) */
	if (co->kwOnlyArgCount > 0) {
		AlifSizeT missing = 0;
		for (i = co->argCount; i < totalArgs; i++) {
			if (alifStackRef_asAlifObjectBorrow(_localsPlus[i]) != nullptr)
				continue;
			AlifObject* varname = ALIFTUPLE_GET_ITEM(co->localsPlusNames, i);
			if (_func->kwDefaults != nullptr) {
				AlifObject* def{};
				if (alifDict_getItemRef(_func->kwDefaults, varname, &def) < 0) {
					goto fail_post_args;
				}
				if (def) {
					_localsPlus[i] = ALIFSTACKREF_FROMALIFOBJECTSTEAL(def);
					continue;
				}
			}
			missing++;
		}
		if (missing) {
			missing_arguments(_thread, co, missing, -1, _localsPlus,
				_func->qualname);
			goto fail_post_args;
		}
	}
	return 0;

failPrePositional:
	for (j = 0; j < _argCount; j++) {
		ALIFSTACKREF_CLOSE(_args[j]);
	}
	/* fall through */
fail_post_positional:
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (j = _argCount; j < _argCount + kwcount; j++) {
			ALIFSTACKREF_CLOSE(_args[j]);
		}
	}
	/* fall through */
fail_post_args:
	return -1;
}



static void clear_threadFrame(AlifThread* _thread, AlifInterpreterFrame* _frame) { // 1644
	_thread->cppRecursionRemaining--;
	_alifFrame_clearExceptCode(_frame);
	ALIFSTACKREF_CLEAR(_frame->executable);
	_thread->cppRecursionRemaining++;
	_alifThreadState_popFrame(_thread, _frame);
}

static void clear_genFrame(AlifThread* _thread,
	AlifInterpreterFrame* _frame) { // 1741
	AlifGenObject* gen = _alifGen_getGeneratorFromFrame(_frame);
	gen->giFrameState = AlifFrameState_::Frame_Cleared;
	_thread->excInfo = gen->giExcState.previousItem;
	gen->giExcState.previousItem = nullptr;
	_thread->cppRecursionRemaining--;
	_alifFrame_clearExceptCode(_frame);
	_alifErr_clearExcState(&gen->giExcState);
	_thread->cppRecursionRemaining++;
	_frame->previous = nullptr;
}

void _alifEval_frameClearAndPop(AlifThread* _thread, AlifInterpreterFrame* _frame) { // 1677
	if (_frame->owner == FrameOwner::FRAME_OWNED_BY_THREAD) {
		clear_threadFrame(_thread, _frame);
	}
	else {
		clear_genFrame(_thread, _frame);
	}
}

AlifInterpreterFrame* _alifEval_framePushAndInit(AlifThread* _thread,
	AlifStackRef _func, AlifObject* _locals, AlifStackRef const* _args,
	AlifUSizeT _argCount, AlifObject* _kwNames, AlifInterpreterFrame* _previous) { // 1689
	AlifFunctionObject* funcObj = (AlifFunctionObject*)alifStackRef_asAlifObjectBorrow(_func);
	AlifCodeObject* code = (AlifCodeObject*)funcObj->code;
	AlifInterpreterFrame* frame = _alifThreadState_pushFrame(_thread, code->frameSize);
	if (frame == nullptr) {
		goto fail;
	}
	_alifFrame_initialize(_thread, frame, _func, _locals, code, 0, _previous);
	if (initialize_locals(_thread, funcObj, frame->localsPlus, _args, _argCount, _kwNames)) {
		clear_threadFrame(_thread, frame);
		return nullptr;
	}
	return frame;
fail:
	/* Consume the references */
	ALIFSTACKREF_CLOSE(_func);
	ALIF_XDECREF(_locals);
	for (size_t i = 0; i < _argCount; i++) {
		ALIFSTACKREF_CLOSE(_args[i]);
	}
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			ALIFSTACKREF_CLOSE(_args[i + _argCount]);
		}
	}
	//alifErr_noMemory();
	return nullptr;
}





static AlifInterpreterFrame* _alifEvalFramePushAndInit_unTagged(AlifThread* _thread,
	AlifStackRef _func, AlifObject* _locals, AlifObject* const* _args,
	AlifUSizeT _argCount, AlifObject* _kwNames, AlifInterpreterFrame* _previous) { // 1724
	AlifUSizeT kwCount = _kwNames == nullptr ? 0 : ALIFTUPLE_GET_SIZE(_kwNames);
	AlifUSizeT totalArgCount = _argCount + kwCount;
	totalArgCount ? totalArgCount : totalArgCount = 1; //* alif
	AlifStackRef* taggedArgsBuffer = (AlifStackRef*)alifMem_dataAlloc(sizeof(AlifStackRef) * totalArgCount);
	if (taggedArgsBuffer == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	for (AlifUSizeT i = 0; i < _argCount; i++) {
		taggedArgsBuffer[i] = ALIFSTACKREF_FROMALIFOBJECTSTEAL(_args[i]);
	}
	for (AlifUSizeT i = 0; i < kwCount; i++) {
		taggedArgsBuffer[_argCount + i] = ALIFSTACKREF_FROMALIFOBJECTSTEAL(_args[_argCount + i]);
	}
	AlifInterpreterFrame* res = _alifEval_framePushAndInit(_thread, _func, _locals, (AlifStackRef const*)taggedArgsBuffer, _argCount, _kwNames, _previous);
	alifMem_dataFree(taggedArgsBuffer);
	return res;
}


AlifObject* _alifEval_vector(AlifThread* _tstate, AlifFunctionObject* _func,
	AlifObject* _locals, AlifObject* const* _args, AlifUSizeT _argCount,
	AlifObject* _kwNames) { // 1876
	ALIF_XINCREF(_locals);
	for (AlifUSizeT i = 0; i < _argCount; i++) {
		ALIF_INCREF(_args[i]);
	}
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			ALIF_INCREF(_args[i + _argCount]);
		}
	}
	AlifInterpreterFrame* frame = _alifEvalFramePushAndInit_unTagged(
		_tstate, ALIFSTACKREF_FROMALIFOBJECTNEW(_func), _locals, _args, _argCount, _kwNames, nullptr);
	if (frame == nullptr) {
		return nullptr;
	}
	return alifEval_evalFrame(_tstate, frame, 0);
}


static AlifInterpreterFrame* _alifEval_framePushAndInitEx(AlifThread* _thread,
	AlifStackRef _func, AlifObject* _locals, AlifSizeT _nargs,
	AlifObject* _callargs, AlifObject* _kwargs, AlifInterpreterFrame* _previous) { // 1815
	bool hasDict = (_kwargs != nullptr and ALIFDICT_GET_SIZE(_kwargs) > 0);
	AlifObject* kwnames = nullptr;
	AlifObject* const* newargs;
	if (hasDict) {
		newargs = _alifStack_unpackDict(_thread, ALIFTUPLE_ITEMS(_callargs), _nargs, _kwargs, &kwnames);
		if (newargs == nullptr) {
			ALIFSTACKREF_CLOSE(_func);
			goto error;
		}
	}
	else {
		newargs = &ALIFTUPLE_GET_ITEM(_callargs, 0);
		/* We need to incref all our args since the new frame steals the references. */
		for (AlifSizeT i = 0; i < _nargs; ++i) {
			ALIF_INCREF(ALIFTUPLE_GET_ITEM(_callargs, i));
		}
	}
	AlifInterpreterFrame* newFrame; newFrame = _alifEvalFramePushAndInit_unTagged(
		_thread, _func, _locals,
		newargs, _nargs, kwnames, _previous
	);
	if (hasDict) {
		_alifStack_unpackDictFreeNoDecRef(newargs, kwnames);
	}
	/* No need to decref func here because the reference has been stolen by
	   _alifEval_framePushAndInit.
	*/
	ALIF_DECREF(_callargs);
	ALIF_XDECREF(_kwargs);
	return newFrame;
error:
	ALIF_DECREF(_callargs);
	ALIF_XDECREF(_kwargs);
	return nullptr;
}

AlifObject* alifEval_evalCodeEx(AlifObject* _co, AlifObject* _globals, AlifObject* _locals,
	AlifObject* const* _args, AlifIntT _argCount,
	AlifObject* const* _kws, AlifIntT _kwCount,
	AlifObject* const* _defs, AlifIntT _defCount,
	AlifObject* _kwDefs, AlifObject* _closure) {
	AlifThread* tstate = _alifThread_get();
	AlifObject* res = nullptr;
	AlifObject* defaults = _alifTuple_fromArray(_defs, _defCount);
	if (defaults == nullptr) {
		return nullptr;
	}
	AlifObject* builtins = _alifDict_loadBuiltinsFromGlobals(_globals);
	if (builtins == nullptr) {
		ALIF_DECREF(defaults);
		return nullptr;
	}
	if (_locals == nullptr) {
		_locals = _globals;
	}
	AlifFrameConstructor constr{};
	AlifObject* kwnames = nullptr;
	AlifObject* const* allargs;
	AlifObject** newargs = nullptr;
	AlifFunctionObject* func = nullptr;
	if (_kwCount == 0) {
		allargs = _args;
	}
	else {
		kwnames = alifTuple_new(_kwCount);
		if (kwnames == nullptr) {
			goto fail;
		}
		newargs = (AlifObject**)alifMem_dataAlloc(sizeof(AlifObject*) * (_kwCount + _argCount));
		if (newargs == nullptr) {
			goto fail;
		}
		for (AlifIntT i = 0; i < _argCount; i++) {
			newargs[i] = _args[i];
		}
		for (AlifIntT i = 0; i < _kwCount; i++) {
			ALIFTUPLE_SET_ITEM(kwnames, i, ALIF_NEWREF(_kws[2 * i]));
			newargs[_argCount + i] = _kws[2 * i + 1];
		}
		allargs = newargs;
	}
	constr = {
		.globals = _globals,
		.builtins = builtins,
		.name = ((AlifCodeObject*)_co)->name,
		.qualname = ((AlifCodeObject*)_co)->name,
		.code = _co,
		.defaults = defaults,
		.kwDefaults = _kwDefs,
		.closure = _closure
	};
	func = _alifFunction_fromConstructor(&constr);
	if (func == nullptr) {
		goto fail;
	}
	//EVAL_CALL_STAT_INC(EVAL_CALL_LEGACY);
	res = _alifEval_vector(tstate, func, _locals,
		allargs, _argCount,
		kwnames);
fail:
	ALIF_XDECREF(func);
	ALIF_XDECREF(kwnames);
	alifMem_dataFree(newargs);
	_alif_decrefBuiltins(builtins);
	ALIF_DECREF(defaults);
	return res;
}

/* Logic for the raise statement (too complicated for inlining).
This *consumes* a reference count to each of its arguments. */
static AlifIntT do_raise(AlifThread* _thread,
	AlifObject* _exc, AlifObject* _cause) { // 1981
	AlifObject* type = nullptr, * value = nullptr;

	if (_exc == nullptr) {
		/* Reraise */
		AlifErrStackItem* excInfo = _alifErr_getTopMostException(_thread);
		_exc = excInfo->excValue;
		if (ALIF_ISNONE(_exc) or _exc == nullptr) {
			_alifErr_setString(_thread, _alifExcRuntimeError_,
				"لا يوجد خلل مفعل ليتم من خلاله إعادة إطلاق الخطأ");
			return 0;
		}
		ALIF_INCREF(_exc);
		_alifErr_setRaisedException(_thread, _exc);
		return 1;
	}

	/* We support the following forms of raise:
	raise
	raise <instance>
	raise <type> */

	if (ALIFEXCEPTIONCLASS_CHECK(_exc)) {
		type = _exc;
		value = _alifObject_callNoArgs(_exc);
		if (value == nullptr)
			goto raise_error;
		if (!ALIFEXCEPTIONINSTANCE_CHECK(value)) {
			_alifErr_format(_thread, _alifExcTypeError_,
				"استدعاء %R يجب أن يرجع نسخة من "
				"خطأ_اساس, وليس %R",
				type, ALIF_TYPE(value));
			goto raise_error;
		}
	}
	else if (ALIFEXCEPTIONINSTANCE_CHECK(_exc)) {
		value = _exc;
		type = ALIFEXCEPTIONINSTANCE_CLASS(_exc);
		ALIF_INCREF(type);
	}
	else {
		/* Not something you can raise.  You get an exception
		anyway, just not what you specified :-) */
		ALIF_DECREF(_exc);
		_alifErr_setString(_thread, _alifExcTypeError_,
			"الأخطاء يجب أن تستخرج من خطأ_اساس");
		goto raise_error;
	}

	if (_cause) {
		AlifObject* fixedCause{};
		if (ALIFEXCEPTIONCLASS_CHECK(_cause)) {
			fixedCause = _alifObject_callNoArgs(_cause);
			if (fixedCause == nullptr)
				goto raise_error;
			if (!ALIFEXCEPTIONINSTANCE_CHECK(fixedCause)) {
				_alifErr_format(_thread, _alifExcTypeError_,
					"استدعاء %R يجب أن يرجع نسخة من "
					"خطأ_اساس, وليس %R",
					_cause, ALIF_TYPE(fixedCause));
				goto raise_error;
			}
			ALIF_DECREF(_cause);
		}
		else if (ALIFEXCEPTIONINSTANCE_CHECK(_cause)) {
			fixedCause = _cause;
		}
		else if (ALIF_ISNONE(_cause)) {
			ALIF_DECREF(_cause);
			fixedCause = nullptr;
		}
		else {
			_alifErr_setString(_thread, _alifExcTypeError_,
				"مسببات الخطأ يجب أن تستخرج من "
				"خطأ_اساس");
			goto raise_error;
		}
		alifException_setCause(value, fixedCause);
	}

	_alifErr_setObject(_thread, type, value);
	/* _alifErr_setObject incref's its arguments */
	ALIF_DECREF(value);
	ALIF_DECREF(type);
	return 0;

raise_error:
	ALIF_XDECREF(value);
	ALIF_XDECREF(type);
	ALIF_XDECREF(_cause);
	return 0;
}


AlifIntT _alifEval_exceptionGroupMatch(AlifObject* _excValue, AlifObject* _matchType,
	AlifObject** _match, AlifObject** _rest) { // 2040
	if (ALIF_ISNONE(_excValue)) {
		*_match = ALIF_NEWREF(ALIF_NONE);
		*_rest = ALIF_NEWREF(ALIF_NONE);
		return 0;
	}

	if (alifErr_givenExceptionMatches(_excValue, _matchType)) {
		/* Full match of exc itself */
		bool is_eg = ALIFBASEEXCEPTIONGROUP_CHECK(_excValue);
		if (is_eg) {
			*_match = ALIF_NEWREF(_excValue);
		}
		else {
			/* naked exception - wrap it */
			AlifObject* excs = alifTuple_pack(1, _excValue);
			if (excs == nullptr) {
				return -1;
			}
			AlifObject* wrapped = _alifExc_createExceptionGroup("", excs);
			ALIF_DECREF(excs);
			if (wrapped == nullptr) {
				return -1;
			}
			*_match = wrapped;
		}
		*_rest = ALIF_NEWREF(ALIF_NONE);
		return 0;
	}

	/* exc_value does not match match_type.
	 * Check for partial match if it's an exception group.
	 */
	if (ALIFBASEEXCEPTIONGROUP_CHECK(_excValue)) {
		AlifObject* pair = alifObject_callMethod(_excValue, "split", "(O)",
			_matchType);
		if (pair == nullptr) {
			return -1;
		}
		*_match = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(pair, 0));
		*_rest = ALIF_NEWREF(ALIFTUPLE_GET_ITEM(pair, 1));
		ALIF_DECREF(pair);
		return 0;
	}
	/* no match */
	*_match = ALIF_NEWREF(ALIF_NONE);
	*_rest = ALIF_NEWREF(_excValue);
	return 0;
}



AlifIntT _alifEval_unpackIterableStackRef(AlifThread* _thread, AlifStackRef _vStackRef,
	AlifIntT _argCnt, AlifIntT _argCntAfter, AlifStackRef* _sp) { // 2064
	AlifIntT i = 0, j = 0;
	AlifSizeT ll = 0;
	AlifObject* it{};  /* iter(v) */
	AlifObject* w{};
	AlifObject* l = nullptr; /* variable list */

	AlifObject* v = alifStackRef_asAlifObjectBorrow(_vStackRef);

	it = alifObject_getIter(v);
	if (it == nullptr) {
		if (_alifErr_exceptionMatches(_thread, _alifExcTypeError_) and
			ALIF_TYPE(v)->iter == nullptr and !alifSequence_check(v))
		{
			_alifErr_format(_thread, _alifExcTypeError_,
				"لا يمكن فك تغليف كائن غير قابل للتكرار %.200s",
				ALIF_TYPE(v)->name);
		}
		return 0;
	}

	for (; i < _argCnt; i++) {
		w = alifIter_next(it);
		if (w == nullptr) {
			/* Iterator done, via error or exhaustion. */
			if (!_alifErr_occurred(_thread)) {
				if (_argCntAfter == -1) {
					_alifErr_format(_thread, _alifExcValueError_,
						"لا يوجد قيم كافية لفك تغليفها "
						"(من المتوقع %d, ولكن يوجد %d)",
						_argCnt, i);
				}
				else {
					_alifErr_format(_thread, _alifExcValueError_,
						"لا يوجد قيم كافية لفك تغليفها "
						"(من المتوقع على الأقل %d, ولكن يوجد %d)",
						_argCnt + _argCntAfter, i);
				}
			}
			goto Error;
		}
		*--_sp = ALIFSTACKREF_FROMALIFOBJECTSTEAL(w);
	}

	if (_argCntAfter == -1) {
		/* We better have exhausted the iterator now. */
		w = alifIter_next(it);
		if (w == nullptr) {
			if (_alifErr_occurred(_thread))
				goto Error;
			ALIF_DECREF(it);
			return 1;
		}
		ALIF_DECREF(w);

		if (ALIFLIST_CHECKEXACT(v) or ALIFTUPLE_CHECKEXACT(v)
			or ALIFDICT_CHECKEXACT(v)) {
			ll = ALIFDICT_CHECKEXACT(v) ? alifDict_size(v) : ALIF_SIZE(v);
			if (ll > _argCnt) {
				_alifErr_format(_thread, _alifExcValueError_,
					"يوجد الكثير من القيم ولا يمكن فك تغليفها (من المتوقع %d, ولكن يوجد %zd)",
					_argCnt, ll);
				goto Error;
			}
		}
		_alifErr_format(_thread, _alifExcValueError_,
			"يوجد الكثير من القيم ولا يمكن فك تغليفها (من المتوقع %d)",
			_argCnt);
		goto Error;
	}

	l = alifSequence_list(it);
	if (l == nullptr)
		goto Error;
	*--_sp = ALIFSTACKREF_FROMALIFOBJECTSTEAL(l);
	i++;

	ll = ALIFLIST_GET_SIZE(l);
	if (ll < _argCntAfter) {
		_alifErr_format(_thread, _alifExcValueError_,
			"لا يوجد قيم كافية لفك تغليفها (من المتوقع على الأقل %d, ولكن يوجد %zd)",
			_argCnt + _argCntAfter, _argCnt + ll);
		goto Error;
	}

	/* Pop the "after-variable" args off the list. */
	for (j = _argCntAfter; j > 0; j--, i++) {
		*--_sp = ALIFSTACKREF_FROMALIFOBJECTSTEAL(ALIFLIST_GET_ITEM(l, ll - j));
	}
	/* Resize the list. */
	ALIF_SET_SIZE(l, ll - _argCntAfter);
	ALIF_DECREF(it);
	return 1;

Error:
	for (; i > 0; i--, _sp++) {
		ALIFSTACKREF_CLOSE(*_sp);
	}
	ALIF_XDECREF(it);
	return 0;
}










AlifObject* _alifEval_getBuiltins(AlifThread* _thread) { // 2444
	AlifInterpreterFrame* frame = _alifThreadState_getFrame(_thread);
	if (frame != nullptr) {
		return frame->builtins;
	}
	return _thread->interpreter->builtins;
}


AlifObject* alifEval_getBuiltins() { // 2455
	AlifThread* thread = _alifThread_get();
	return _alifEval_getBuiltins(thread);
}


AlifObject* _alifEval_getFrameLocals(void) { // 2617
	AlifThread* tstate = _alifThread_get();
	AlifInterpreterFrame* current_frame = _alifThreadState_getFrame(tstate);
	if (current_frame == nullptr) {
		_alifErr_setString(tstate, _alifExcSystemError_, "لا يوجد إطار");
		return nullptr;
	}

	AlifObject* locals = _alifFrame_getLocals(current_frame);
	if (locals == nullptr) {
		return nullptr;
	}

	if (ALIFFRAMELOCALSPROXY_CHECK(locals)) {
		AlifObject* ret = alifDict_new();
		if (ret == nullptr) {
			ALIF_DECREF(locals);
			return nullptr;
		}
		if (alifDict_update(ret, locals) < 0) {
			ALIF_DECREF(ret);
			ALIF_DECREF(locals);
			return nullptr;
		}
		ALIF_DECREF(locals);
		return ret;
	}

	return locals;
}


AlifObject* alifEval_getGlobals() { // 2651
	AlifThread* thread = _alifThread_get();
	AlifInterpreterFrame* current_frame = _alifThreadState_getFrame(thread);
	if (current_frame == nullptr) {
		return nullptr;
	}
	return current_frame->globals;
}


AlifIntT alifEval_mergeCompilerFlags(AlifCompilerFlags* _cf) { // 2684
	AlifThread* thread = _alifThread_get();
	AlifInterpreterFrame* currentFrame = thread->currentFrame;
	AlifIntT result = _cf->flags != 0;

	if (currentFrame != nullptr) {
		const AlifIntT codeflags = _alifFrame_getCode(currentFrame)->flags;
		const AlifIntT compilerflags = codeflags & ALIFCF_MASK;
		if (compilerflags) {
			result = 1;
			_cf->flags |= compilerflags;
		}
	}
	return result;
}


AlifIntT _alifEval_sliceIndex(AlifObject* _v, AlifSizeT* _pi) { // 2720
	AlifThread* thread = _alifThread_get();
	if (!ALIF_ISNONE(_v)) {
		AlifSizeT x{};
		if (_alifIndex_check(_v)) {
			x = alifNumber_asSizeT(_v, nullptr);
			if (x == -1 and _alifErr_occurred(thread))
				return 0;
		}
		else {
			_alifErr_setString(thread, _alifExcTypeError_,
				"مؤشرات القطع يجب أن تكون اعداد صحيحة أو "
				"عدم أو تملك دالة __مؤشر__");
			return 0;
		}
		*_pi = x;
	}
	return 1;
}


AlifObject* _alifEval_importName(AlifThread* _thread, AlifInterpreterFrame* _frame,
	AlifObject* _name, AlifObject* _fromList, AlifObject* _level) { // 2683
	AlifObject* importFunc{};
	if (alifMapping_getOptionalItem(_frame->builtins, &ALIF_STR(__import__), &importFunc) < 0) {
		return nullptr;
	}
	if (importFunc == nullptr) {
		_alifErr_setString(_thread, _alifExcImportError_, "لم يتم العثور على __استورد__");
		return nullptr;
	}

	AlifObject* locals = _frame->locals;
	if (locals == nullptr) {
		locals = ALIF_NONE;
	}

	/* Fast path for not overloaded __import__. */
	if (_alifImport_isDefaultImportFunc(_thread->interpreter, importFunc)) {
		ALIF_DECREF(importFunc);
		AlifIntT ilevel = alifLong_asInt(_level);
		if (ilevel == -1 and _alifErr_occurred(_thread)) {
			return nullptr;
		}
		return alifImport_importModuleLevelObject(_name,
			_frame->globals, locals, _fromList, ilevel);
	}

	AlifObject* args[5] = { _name, _frame->globals, locals, _fromList, _level };
	AlifObject* res = alifObject_vectorCall(importFunc, args, 5, nullptr);
	ALIF_DECREF(importFunc);
	return res;
}

AlifObject* _alifEval_importFrom(AlifThread* _thread,
	AlifObject* _v, AlifObject* _name) { // 2722
	AlifObject* x{};
	AlifObject* fullmodname{}, * modName{}, * origin{}, * modNameOrUnknown{}, * errmsg{}, * spec{};

	if (alifObject_getOptionalAttr(_v, _name, &x) != 0) {
		return x;
	}

	if (alifObject_getOptionalAttr(_v, &ALIF_STR(__name__), &modName) < 0) {
		return nullptr;
	}
	if (modName == nullptr or !ALIFUSTR_CHECK(modName)) {
		ALIF_CLEAR(modName);
		goto error;
	}
	fullmodname = alifUStr_fromFormat("%U.%U", modName, _name);
	if (fullmodname == nullptr) {
		ALIF_DECREF(modName);
		return nullptr;
	}
	x = alifImport_getModule(fullmodname);
	ALIF_DECREF(fullmodname);
	if (x == nullptr and !_alifErr_occurred(_thread)) {
		goto error;
	}
	ALIF_DECREF(modName);
	return x;
error:
	if (modName == nullptr) {
		modNameOrUnknown = alifUStr_fromString("<unknown module name>");
		if (modNameOrUnknown == nullptr) {
			return nullptr;
		}
	}
	else {
		modNameOrUnknown = modName;
	}

	origin = nullptr;
	if (alifObject_getOptionalAttr(_v, &ALIF_STR(__spec__), &spec) < 0) {
		ALIF_DECREF(modNameOrUnknown);
		return nullptr;
	}
	if (spec == nullptr) {
		errmsg = alifUStr_fromFormat(
			"cannot import name %R from %R (unknown location)",
			_name, modNameOrUnknown
		);
		goto done_with_errmsg;
	}
	if (_alifModuleSpec_getFileOrigin(spec, &origin) < 0) {
		goto done;
	}

	AlifIntT isPossiblyShadowing; isPossiblyShadowing = _alifModule_isPossiblyShadowing(origin);
	if (isPossiblyShadowing < 0) {
		goto done;
	}
	AlifIntT isPossiblyShadowingStdlib; isPossiblyShadowingStdlib = 0;
	if (isPossiblyShadowing) {
		AlifObject* stdlibModules = alifSys_getObject("stdlib_module_names");
		if (stdlibModules and ALIFANYSET_CHECK(stdlibModules)) {
			isPossiblyShadowingStdlib = alifSet_contains(stdlibModules, modNameOrUnknown);
			if (isPossiblyShadowingStdlib < 0) {
				goto done;
			}
		}
	}

	if (isPossiblyShadowingStdlib) {
		errmsg = alifUStr_fromFormat(
			"cannot import name %R from %R "
			"(consider renaming %R since it has the same "
			"name as the standard library module named %R "
			"and prevents importing that standard library module)",
			_name, modNameOrUnknown, origin, modNameOrUnknown
		);
	}
	else {
		AlifIntT rc = _alifModuleSpec_isInitializing(spec);
		if (rc < 0) {
			goto done;
		}
		else if (rc > 0) {
			if (isPossiblyShadowing) {
				// For non-stdlib modules, only mention the possibility of
				// shadowing if the module is being initialized.
				errmsg = alifUStr_fromFormat(
					"cannot import name %R from %R "
					"(consider renaming %R if it has the same name "
					"as a library you intended to import)",
					_name, modNameOrUnknown, origin
				);
			}
			else if (origin) {
				errmsg = alifUStr_fromFormat(
					"cannot import name %R from partially initialized module %R "
					"(most likely due to a circular import) (%S)",
					_name, modNameOrUnknown, origin
				);
			}
			else {
				errmsg = alifUStr_fromFormat(
					"cannot import name %R from partially initialized module %R "
					"(most likely due to a circular import)",
					_name, modNameOrUnknown
				);
			}
		}
		else {
			if (origin) {
				errmsg = alifUStr_fromFormat(
					"cannot import name %R from %R (%S)",
					_name, modNameOrUnknown, origin
				);
			}
			else {
				errmsg = alifUStr_fromFormat(
					"cannot import name %R from %R (unknown location)",
					_name, modNameOrUnknown
				);
			}
		}
	}

done_with_errmsg:
	//_alifErr_setImportErrorWithNameFrom(errmsg, modName, origin, _name);
	ALIF_DECREF(errmsg);

done:
	ALIF_XDECREF(origin);
	ALIF_XDECREF(spec);
	ALIF_DECREF(modNameOrUnknown);
	return nullptr;
}

// 2861 
#define CANNOT_CATCH_MSG "catching classes that do not inherit from "\
                         "BaseException is not allowed"

#define CANNOT_EXCEPT_STAR_EG "catching ExceptionGroup with except* "\
                              "is not allowed. Use except instead."

AlifIntT _alifEval_checkExceptTypeValid(AlifThread* _thread, AlifObject* _right) { // 2867
	if (ALIFTUPLE_CHECK(_right)) {
		AlifSizeT i{}, length{};
		length = ALIFTUPLE_GET_SIZE(_right);
		for (i = 0; i < length; i++) {
			AlifObject* exc = ALIFTUPLE_GET_ITEM(_right, i);
			if (!ALIFEXCEPTIONCLASS_CHECK(exc)) {
				_alifErr_setString(_thread, _alifExcTypeError_,
					CANNOT_CATCH_MSG);
				return -1;
			}
		}
	}
	else {
		if (!ALIFEXCEPTIONCLASS_CHECK(_right)) {
			_alifErr_setString(_thread, _alifExcTypeError_,
				CANNOT_CATCH_MSG);
			return -1;
		}
	}
	return 0;
}

AlifIntT _alifEval_checkExceptStarTypeValid(AlifThread* _thread, AlifObject* _right) { // 2892
	if (_alifEval_checkExceptTypeValid(_thread, _right) < 0) {
		return -1;
	}

	/* reject except *ExceptionGroup */

	AlifIntT isSubclass = 0;
	if (ALIFTUPLE_CHECK(_right)) {
		AlifSizeT length = ALIFTUPLE_GET_SIZE(_right);
		for (AlifSizeT i = 0; i < length; i++) {
			AlifObject* exc = ALIFTUPLE_GET_ITEM(_right, i);
			isSubclass = alifObject_isSubclass(exc, _alifExcBaseExceptionGroup_);
			if (isSubclass < 0) {
				return -1;
			}
			if (isSubclass) {
				break;
			}
		}
	}
	else {
		isSubclass = alifObject_isSubclass(_right, _alifExcBaseExceptionGroup_);
		if (isSubclass < 0) {
			return -1;
		}
	}
	if (isSubclass) {
		_alifErr_setString(_thread, _alifExcTypeError_,
			CANNOT_EXCEPT_STAR_EG);
		return -1;
	}
	return 0;
}


AlifIntT _alifCheck_argsIterable(AlifThread* _thread, AlifObject* _func, AlifObject* _args) { // 2951
	if (ALIF_TYPE(_args)->iter == nullptr and !alifSequence_check(_args)) {
		_alifErr_clear(_thread);
		AlifObject* funcstr = _alifObject_functionStr(_func);
		if (funcstr != nullptr) {
			_alifErr_format(_thread, _alifExcTypeError_,
				"%U المعامل بعد * يجب أن يكون قابل للتكرار, وليس %.200s",
				funcstr, ALIF_TYPE(_args)->name);
			ALIF_DECREF(funcstr);
		}
		return -1;
	}
	return 0;
}


void _alifEval_loadGlobalStackRef(AlifObject* _globals, AlifObject* _builtins,
	AlifObject* _name, AlifStackRef* _writeto) { // 3225
	AlifObject* res{};
	if (ALIFDICT_CHECKEXACT(_globals) and ALIFDICT_CHECKEXACT(_builtins)) {
		_alifDict_loadGlobalStackRef((AlifDictObject*)_globals,
			(AlifDictObject*)_builtins, _name, _writeto);
		if (ALIFSTACKREF_ISNULL(*_writeto) and !alifErr_occurred()) {
			_alifEval_formatExcCheckArg(ALIFTHREADSTATE_GET(), _alifExcNameError_,
				NAME_ERROR_MSG, _name);
		}
	}
	else {
		AlifObject* res{};
		if (alifMapping_getOptionalItem(_globals, _name, &res) < 0) {
			*_writeto = _alifStackRefNull_;
			return;
		}
		if (res == nullptr) {
			/* namespace 2: builtins */
			if (alifMapping_getOptionalItem(_builtins, _name, &res) < 0) {
				*_writeto = _alifStackRefNull_;
				return;
			}
			if (res == nullptr) {
				_alifEval_formatExcCheckArg(
					ALIFTHREADSTATE_GET(), _alifExcNameError_,
					NAME_ERROR_MSG, _name);
			}
		}
		*_writeto = ALIFSTACKREF_FROMALIFOBJECTSTEAL(res);
	}
}


void _alifEval_formatExcCheckArg(AlifThread* _thread, AlifObject* _exc,
	const char* _formatStr, AlifObject* _obj) { // 3093
	const char* objStr{};

	if (!_obj)
		return;

	objStr = alifUStr_asUTF8(_obj);
	if (!objStr)
		return;

	_alifErr_format(_thread, _exc, _formatStr, objStr);

	if (_exc == _alifExcNameError_) {
		AlifObject* _exc = alifErr_getRaisedException();
		if (alifErr_givenExceptionMatches(_exc, _alifExcNameError_)) {
			if (((AlifNameErrorObject*)_exc)->name == nullptr) {
				(void)alifObject_setAttr(_exc, &ALIF_STR(Name), _obj);
			}
		}
		alifErr_setRaisedException(_exc);
	}
}


AlifObject* _alifEval_loadName(AlifThread* _thread,
	AlifInterpreterFrame* _frame, AlifObject* _name) { // 3133

	AlifObject* value{};
	if (_frame->locals == nullptr) {
		_alifErr_setString(_thread, _alifExcSystemError_,
			"اسماء_محلية فارغة");
		return nullptr;
	}
	if (alifMapping_getOptionalItem(_frame->locals, _name, &value) < 0) {
		return nullptr;
	}
	if (value != nullptr) {
		return value;
	}
	if (alifDict_getItemRef(_frame->globals, _name, &value) < 0) {
		return nullptr;
	}
	if (value != nullptr) {
		return value;
	}
	if (alifMapping_getOptionalItem(_frame->builtins, _name, &value) < 0) {
		return nullptr;
	}
	if (value == nullptr) {
		_alifEval_formatExcCheckArg(
			_thread, _alifExcNameError_,
			NAME_ERROR_MSG, _name);
	}
	return value;
}
