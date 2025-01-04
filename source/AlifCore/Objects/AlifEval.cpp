#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Backoff.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Code.h"

#include "AlifCore_Function.h"
#include "AlifCore_Instruments.h"

#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpcodeMetaData.h"
#include "AlifCore_Optimizer.h"
#include "AlifCore_OpcodeUtils.h"

#include "AlifCore_State.h"


#include "AlifCore_Dict.h"

#include "AlifCore_Frame.h"

#include "Opcode.h"















AlifIntT alif_checkRecursiveCall(AlifThread* _thread, const char* _where) { // 305
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






AlifObject* alifEval_evalCode(AlifObject* _co,
	AlifObject* _globals, AlifObject* _locals) { // 592
	AlifThread* thread = _alifThread_get();
	if (_locals == nullptr) {
		_locals = _globals;
	}
	AlifObject* builtins = _alifEval_builtinsFromGlobals(thread, _globals); // borrowed ref
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
	if (func == nullptr) {
		return nullptr;
	}
	AlifObject* res = alifEval_vector(thread, func, _locals, nullptr, 0, nullptr);
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

	entryFrame.executable = ALIF_NONE;
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
		//goto exit_unwind;
	}

	//if (_throwflag) {
	//	if (_alif_enterRecursiveAlif(_thread)) {
	//		goto exit_unwind;
	//	}
	//	_alif_instrument(_alifFrame_getCode(_frame), _thread->interpreter);
	//	monitor_throw(_thread, _frame, _frame->instrPtr);
	//	goto resume_with_error;
	//}


	AlifCodeUnit* nextInstr{};
	AlifStackRef* stackPointer{};



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
			TARGET(END_FOR) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = stackPointer[-1];
				alifStackRef_close(value);
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
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(alifObject_format(valueObj, nullptr));
					alifStackRef_close(value);
					//if (ALIFSTACKREF_ISNULL(res)) goto pop_1_error;
				}
				else {
					res = value;
				}
				stackPointer[-1] = res;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(GET_ITER) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef iterable{};
				AlifStackRef iter{};
				iterable = stackPointer[-1];
				/* before: [obj]; after [getiter(obj)] */
				iter = ALIFSTACKREF_FROMALIFOBJECTSTEAL(alifObject_getIter(alifStackRef_asAlifObjectBorrow(iterable)));
				alifStackRef_close(iterable);
				//if (ALIFSTACKREF_ISNULL(iter)) goto pop_1_error;
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
				return alifStackRef_asAlifObjectSteal(retval);
			} // ------------------------------------------------------------ //
			TARGET(LOAD_BUILD_CLASS) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef bc{};
				AlifObject* bc_o{};
				if (alifMapping_getOptionalItem(BUILTINS(), &ALIF_ID(__buildClass__), &bc_o) < 0)
				{ /*goto error;*/ }
				if (bc_o == nullptr) {
					//_alifErr_setString(_thread, _alifExcNameError_,
					//	"__build_class__ not found");
					//if (true) goto error;
				}
				bc = ALIFSTACKREF_FROMALIFOBJECTSTEAL(bc_o);
				stackPointer[0] = bc;
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
				AlifFunctionObject* funcObj = (AlifFunctionObject*)
					alifFunction_new(codeobj, GLOBALS());
				alifStackRef_close(codeObjSt);
				if (funcObj == nullptr) {
					//goto error;
				}
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
			TARGET(POP_TOP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = stackPointer[-1];
				alifStackRef_close(value);
				stackPointer += -1;
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
			TARGET(RETURN_VALUE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef retval{};
				AlifStackRef res{};
				retval = stackPointer[-1];
				stackPointer += -1;
				_alifFrame_setStackPointer(_frame, stackPointer);
				_alif_leaveRecursiveCallAlif(_thread);
				AlifInterpreterFrame* dying = _frame;
				_frame = _thread->currentFrame = dying->previous;
				_alifEval_frameClearAndPop(_thread, dying);
				LOAD_SP();
				LOAD_IP(_frame->returnOffset);
				res = retval;
				//LLTRACE_RESUME_FRAME();
				stackPointer[0] = res;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(TO_BOOL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 4;
				PREDICTED(TO_BOOL);
				AlifCodeUnit* thisInstr = nextInstr - 4;
				AlifStackRef value{};
				AlifStackRef res{};
				// _SPECIALIZE_TO_BOOL
				value = stackPointer[-1];
				{
					uint16_t counter = read_u16(&thisInstr[1].cache);
#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifSpecialize_toBool(value, nextInstr);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(TO_BOOL);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
#endif  /* ENABLE_SPECIALIZATION */
				}
				/* Skip 2 cache entries */
				// _TO_BOOL
				{
					AlifIntT err = alifObject_isTrue(alifStackRef_asAlifObjectBorrow(value));
					alifStackRef_close(value);
					//if (err < 0) goto pop_1_error;
					err ? res = ALIFSTACKREF_TRUE : res = ALIFSTACKREF_FALSE;
				}
				stackPointer[-1] = res;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(BINARY_OP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(BINARY_OP);
				AlifCodeUnit* this_instr = nextInstr - 2;
				AlifStackRef lhs{};
				AlifStackRef rhs{};
				AlifStackRef res{};
				// _SPECIALIZE_BINARY_OP
				rhs = stackPointer[-1];
				lhs = stackPointer[-2];
				{
					uint16_t counter = read_u16(&this_instr[1].cache);
#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = this_instr;
						_alifSpecialize_binaryOp(lhs, rhs, nextInstr, oparg, LOCALS_ARRAY);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(BINARY_OP);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
#endif  /* ENABLE_SPECIALIZATION */
				}
				// _BINARY_OP
				{
					AlifObject* lhsObj = alifStackRef_asAlifObjectBorrow(lhs);
					AlifObject* rhsObj = alifStackRef_asAlifObjectBorrow(rhs);
					AlifObject* resObj = _alifEvalBinaryOps_[oparg](lhsObj, rhsObj);
					alifStackRef_close(lhs);
					alifStackRef_close(rhs);
					//if (resObj == nullptr) goto pop_2_error;
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
					//goto error;
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
						alifStackRef_close(values[_i]);
					}
					if (true) {
						stackPointer += -oparg * 2;
						//goto error;
					}
				}
				AlifObject* mapObj = _alifDict_fromItems(
					valuesObj, 2, valuesObj + 1, 2, oparg);
				STACKREFS_TO_ALIFOBJECTS_CLEANUP(valuesObj);
				for (AlifIntT _i = oparg * 2; --_i >= 0;) {
					alifStackRef_close(values[_i]);
				}
				if (mapObj == nullptr) {
					stackPointer += -oparg * 2;
					//goto error;
				}
				map = ALIFSTACKREF_FROMALIFOBJECTSTEAL(mapObj);
				stackPointer[-oparg * 2] = map;
				stackPointer += 1 - oparg * 2;
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
						alifStackRef_close(pieces[_i]);
					}
					if (true) {
						stackPointer += -oparg;
						//goto error;
					}
				}
				AlifObject* strO = alifUStr_joinArray(&ALIF_STR(Empty), piecesO, oparg);
				STACKREFS_TO_ALIFOBJECTS_CLEANUP(piecesO);
				for (AlifIntT _i = oparg; --_i >= 0;) {
					alifStackRef_close(pieces[_i]);
				}
				if (strO == nullptr) {
					stackPointer += -oparg;
					//goto error;
				}
				str = ALIFSTACKREF_FROMALIFOBJECTSTEAL(strO);
				stackPointer[-oparg] = str;
				stackPointer += 1 - oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(CALL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 4;
				PREDICTED(CALL);
				AlifCodeUnit* this_instr = nextInstr - 4;
				AlifStackRef callable{};
				AlifStackRef selfOrNull{};
				AlifStackRef* args{};
				AlifStackRef func{};
				AlifStackRef maybe_self{};
				AlifStackRef res{};
				// _SPECIALIZE_CALL
				selfOrNull = stackPointer[-1 - oparg];
				callable = stackPointer[-2 - oparg];
				{
					uint16_t counter = read_u16(&this_instr[1].cache);
#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = this_instr;
						_alifSpecialize_call(callable, nextInstr, oparg + !ALIFSTACKREF_ISNULL(self_or_null));
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(CALL);
					ADVANCE_ADAPTIVE_COUNTER(this_instr[1].counter);
#endif  /* ENABLE_SPECIALIZATION */
				}
				/* Skip 2 cache entries */
				// _MAYBE_EXPAND_METHOD
				args = &stackPointer[-oparg];
				{
					args = &stackPointer[-oparg];
					if (ALIFSTACKREF_TYPE(callable) == &_alifMethodType_ and ALIFSTACKREF_ISNULL(selfOrNull)) {
						AlifObject* callable_o = alifStackRef_asAlifObjectBorrow(callable);
						AlifObject* self = ((AlifMethodObject*)callable_o)->self;
						maybe_self = ALIFSTACKREF_FROMALIFOBJECTNEW(self);
						stackPointer[-1 - oparg] = maybe_self;
						AlifObject* method = ((AlifMethodObject*)callable_o)->func;
						func = ALIFSTACKREF_FROMALIFOBJECTNEW(method);
						stackPointer[-2 - oparg] = func;
						/* Make sure that callable and all args are in memory */
						args[-2] = func;
						args[-1] = maybe_self;
						alifStackRef_close(callable);
					}
					else {
						func = callable;
						maybe_self = selfOrNull;
					}
				}
				// _DO_CALL
				selfOrNull = maybe_self;
				callable = func;
				{
					AlifObject* callableObj = alifStackRef_asAlifObjectBorrow(callable);
					// oparg counts all of the args, but *not* self:
					AlifIntT totalArgs = oparg;
					if (!ALIFSTACKREF_ISNULL(selfOrNull)) {
						args--;
						totalArgs++;
					}
					// Check if the call can be inlined or not
					if (ALIF_TYPE(callableObj) == &_alifFunctionType_ and
						_thread->interpreter->evalFrame == nullptr and
						((AlifFunctionObject*)callableObj)->vectorCall == alifFunction_vectorCall)
					{
						AlifIntT codeFlags = ((AlifCodeObject*)ALIFFUNCTION_GET_CODE(callableObj))->flags;
						AlifObject* locals = codeFlags & CO_OPTIMIZED ? nullptr : ALIF_NEWREF(ALIFFUNCTION_GET_GLOBALS(callableObj));
						AlifInterpreterFrame* newFrame = _alifEval_framePushAndInit(
							_thread, (AlifFunctionObject*)alifStackRef_asAlifObjectSteal(callable), locals,
							args, totalArgs, nullptr, _frame
						);
						// Manipulate stack directly since we leave using DISPATCH_INLINED().
						STACK_SHRINK(oparg + 2);
						// The frame has stolen all the arguments from the stack,
						// so there is no need to clean them up.
						if (newFrame == nullptr) {
							//goto error;
						}
						_frame->returnOffset = (uint16_t)(nextInstr - this_instr);
						DISPATCH_INLINED(newFrame);
					}
					/* Callable is not a normal Alif function */
					STACKREFS_TO_ALIFOBJECTS(args, totalArgs, argsObj);
					if (CONVERSION_FAILED(argsObj)) {
						alifStackRef_close(callable);
						for (AlifIntT i = 0; i < totalArgs; i++) {
							alifStackRef_close(args[i]);
						}
						if (true) {
							stackPointer += -2 - oparg;
							//goto error;
						}
					}
					AlifObject* resObj = alifObject_vectorCall(
						callableObj, argsObj,
						totalArgs | ALIF_VECTORCALL_ARGUMENTS_OFFSET,
						nullptr);
					STACKREFS_TO_ALIFOBJECTS_CLEANUP(argsObj);
					if (opcode == INSTRUMENTED_CALL) {
						AlifObject* arg = totalArgs == 0 ?
							&_alifInstrumentationMissing_ : alifStackRef_asAlifObjectBorrow(args[0]);
						if (resObj == nullptr) {
							//_alifCall_instrumentationExc2(
							//	_thread, ALIF_MONITORING_EVENT_C_RAISE,
							//	_frame, this_instr, callable_o, arg);
						}
						else {
							//AlifIntT err = _alifCall_instrumentation2Args(
							//	_thread, ALIF_MONITORING_EVENT_C_RETURN,
							//	_frame, this_instr, callable_o, arg);
							//if (err < 0) {
							//	ALIF_CLEAR(res_o);
							//}
						}
					}
					alifStackRef_close(callable);
					for (AlifIntT i = 0; i < totalArgs; i++) {
						alifStackRef_close(args[i]);
					}
					if (resObj == nullptr) {
						stackPointer += -2 - oparg;
						//goto error;
					}
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				// _CHECK_PERIODIC
				{
					//ALIF_CHECK_EMSCRIPTEN_SIGNALS_PERIODICALLY();
					QSBR_QUIESCENT_STATE(_thread); \
						if (alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ALIF_EVAL_EVENTS_MASK) {
							//AlifIntT err = _alif_handlePending(_thread);
							//if (err != 0) {
							//	stackPointer[-2 - oparg] = res;
							//	stackPointer += -1 - oparg;
							//	goto error;
							//}
						}
				}
				stackPointer[-2 - oparg] = res;
				stackPointer += -1 - oparg;
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
				right = stackPointer[-1];
				left = stackPointer[-2];
				{
					uint16_t counter = read_u16(&thisInstr[1].cache);
#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextOnstr = thisInstr;
						_alifSpecialize_compareOp(left, right, nextInstr, oparg);
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
					AlifObject* resObj = alifObject_richCompare(leftObj, rightObj, oparg >> 5);
					alifStackRef_close(left);
					alifStackRef_close(right);
					//if (res_o == nullptr) goto pop_2_error;
					if (oparg & 16) {
						AlifIntT resBool = alifObject_isTrue(resObj);
						ALIF_DECREF(resObj);
						//if (res_bool < 0) goto pop_2_error;
						resBool ? res = ALIFSTACKREF_TRUE : res = ALIFSTACKREF_FALSE;
					}
					else {
						res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
					}
				}
				stackPointer[-2] = res;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(FOR_ITER) {
				_frame->instrPtr = nextInstr;
				nextInstr += 2;
				PREDICTED(FOR_ITER);
				AlifCodeUnit* thisInstr = nextInstr - 2;
				AlifStackRef iter{};
				AlifStackRef next{};
				// _SPECIALIZE_FOR_ITER
				iter = stackPointer[-1];
				{
					uint16_t counter = read_u16(&thisInstr[1].cache);
					(void)counter;
#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifSpecialize_forIter(iter, nextInstr, oparg);
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
					AlifObject* nextObj = (*ALIF_TYPE(iterObj)->iterNext)(iterObj);
					if (nextObj == nullptr) {
						next = _alifStackRefNull_;
						//if (_alifErr_occurred(_thread)) {
						//	AlifIntT matches = _alifErr_exceptionMatches(_thread, _alifExcStopIteration_);
						//	if (!matches) {
						//		goto error;
						//	}
						//	_alifEval_monitorRaise(_thread, _frame, thisInstr);
						//	_alifErr_clear(_thread);
						//}
						/* iterator ended normally */
						alifStackRef_close(iter);
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
			TARGET(JUMP_BACKWARD) {
				AlifCodeUnit* thisInstr = _frame->instrPtr = nextInstr;
				nextInstr += 2;
				// _CHECK_PERIODIC
				{
					QSBR_QUIESCENT_STATE(_thread); \
						if (alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ALIF_EVAL_EVENTS_MASK) {
							//AlifIntT err = _alif_handlePending(_thread);
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
						AlifIntT optimized = _alifOptimizer_optimize(_frame, start, stackPointer, &executor, 0);
						if (optimized < 0) goto error;
						if (optimized) {
							_thread->previousExecutor = ALIF_NONE;
							GOTO_TIER_TWO(executor);
						}
						else {
							thisInstr[1].counter = restart_backoffCounter(counter);
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
			TARGET(JUMP_FORWARD) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				JUMPBY(oparg);
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
				AlifObject* none_val = _alifList_extend((AlifListObject*)list, iterable);
				if (none_val == nullptr) {
					//AlifIntT matches = _alifErr_exceptionMatches(_thread, _alifExcTypeError_);
					//if (matches and
					//	(ALIF_TYPE(iterable)->iter == nullptr and !alifSequence_check(iterable)))
					//{
					//	_alifErr_clear(_thread);
					//	_alifErr_format(_thread, _alifExcTypeError_,
					//		"Value after * must be an iterable, not %.200s",
					//		ALIF_TYPE(iterable)->name);
					//}
					alifStackRef_close(iterableSt);
					//if (true) goto pop_1_error;
				}
				alifStackRef_close(iterableSt);
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
				AlifStackRef self_or_null = _alifStackRefNull_;
				// _SPECIALIZE_LOAD_ATTR
				owner = stackPointer[-1];
				{
					uint16_t counter = read_u16(&thisInstr[1].cache);
#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg >> 1);
						nextInstr = thisInstr;
						_alifSpecialize_loadAttr(owner, nextInstr, name);
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
						AlifIntT is_meth = _alifObject_getMethod(alifStackRef_asAlifObjectBorrow(owner), name, &attrObj);
						if (is_meth) {
							self_or_null = owner;  // Transfer ownership
						}
						else {
							alifStackRef_close(owner);
							//if (attrObj == nullptr) goto pop_1_error;
							self_or_null = _alifStackRefNull_;
						}
					}
					else {
						/* Classic, pushes one value. */
						attrObj = alifObject_getAttr(alifStackRef_asAlifObjectBorrow(owner), name);
						alifStackRef_close(owner);
						//if (attrObj == nullptr) goto pop_1_error;
					}
					attr = ALIFSTACKREF_FROMALIFOBJECTSTEAL(attrObj);
				}
				stackPointer[-1] = attr;
				if (oparg & 1) stackPointer[0] = self_or_null;
				stackPointer += (oparg & 1);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_CONST) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = ALIFSTACKREF_FROMALIFOBJECTNEW(GETITEM(FRAME_CO_CONSTS, oparg));
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
			TARGET(LOAD_GLOBAL) {
				_frame->instrPtr = nextInstr;
				nextInstr += 5;
				PREDICTED(LOAD_GLOBAL);
				AlifCodeUnit* thisInstr = nextInstr - 5;
				AlifStackRef res{};
				AlifStackRef null = _alifStackRefNull_;
				// _SPECIALIZE_LOAD_GLOBAL
				{
					uint16_t counter = read_u16(&thisInstr[1].cache);
#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg >> 1);
						nextInstr = thisInstr;
						_alifSpecialize_loadGlobal(GLOBALS(), BUILTINS(), nextInstr, name);
						DISPATCH_SAME_OPARG();
					}
					OPCODE_DEFERRED_INC(LOAD_GLOBAL);
					ADVANCE_ADAPTIVE_COUNTER(thisInstr[1].counter);
#endif  /* ENABLE_SPECIALIZATION */
				}
				// _LOAD_GLOBAL
				{
					AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg >> 1);
					AlifObject* resObj = _alifEval_loadGlobal(GLOBALS(), BUILTINS(), name);
					//if (resObj == nullptr) goto error;
					null = _alifStackRefNull_;
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(resObj);
				}
				stackPointer[0] = res;
				if (oparg & 1) stackPointer[1] = null;
				stackPointer += 1 + (oparg & 1);
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(LOAD_NAME) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef v{};
				AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
				AlifObject* vObj = _alifEval_loadName(_thread, _frame, name);
				//if (vObj == nullptr) goto error;
				v = ALIFSTACKREF_FROMALIFOBJECTSTEAL(vObj);
				stackPointer[0] = v;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(POP_JUMP_IF_FALSE) {
				AlifCodeUnit* thisInstr = _frame->instrPtr = nextInstr;
				nextInstr += 2;
				AlifStackRef cond{};
				/* Skip 1 cache entry */
				cond = stackPointer[-1];
				AlifIntT flag = ALIFSTACKREF_IS(cond, ALIFSTACKREF_FALSE);
#if ENABLE_SPECIALIZATION
				this_instr[1].cache = (thisInstr[1].cache << 1) | flag;
#endif
				JUMPBY(oparg * flag);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(POP_JUMP_IF_TRUE) {
				AlifCodeUnit* thisInstr = _frame->instrPtr = nextInstr;
				nextInstr += 2;
				AlifStackRef cond{};
				/* Skip 1 cache entry */
				cond = stackPointer[-1];
				AlifIntT flag = ALIFSTACKREF_IS(cond, ALIFSTACKREF_TRUE);
#if ENABLE_SPECIALIZATION
				thisInstr[1].cache = (thisInstr[1].cache << 1) | flag;
#endif
				JUMPBY(oparg * flag);
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(RETURN_CONST) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifStackRef retval{};
				AlifStackRef res{};
				// _LOAD_CONST
				{
					value = ALIFSTACKREF_FROMALIFOBJECTNEW(GETITEM(FRAME_CO_CONSTS, oparg));
					stackPointer[0] = value;
				}
				// _RETURN_VALUE
				retval = value;
				{
					_alifFrame_setStackPointer(_frame, stackPointer);
					_alif_leaveRecursiveCallAlif(_thread);
					AlifInterpreterFrame* dying = _frame;
					_frame = _thread->currentFrame = dying->previous;
					_alifEval_frameClearAndPop(_thread, dying);
					LOAD_SP();
					LOAD_IP(_frame->returnOffset);
					res = retval;
					//LLTRACE_RESUME_FRAME();
				}
				stackPointer[0] = res;
				stackPointer += 1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(SET_FUNCTION_ATTRIBUTE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef attrSt{};
				AlifStackRef funcSt{};
				funcSt = stackPointer[-1];
				attrSt = stackPointer[-2];
				AlifObject* func = alifStackRef_asAlifObjectBorrow(funcSt);
				AlifObject* attr = alifStackRef_asAlifObjectBorrow(attrSt);
				AlifFunctionObject* funcObj = (AlifFunctionObject*)func;
				switch (oparg) {
				case MAKE_FUNCTION_CLOSURE:
					funcObj->closure = attr;
					break;
				case MAKE_FUNCTION_ANNOTATIONS:
					funcObj->annotations = attr;
					break;
				case MAKE_FUNCTION_KWDEFAULTS:
					funcObj->kwDefaults = attr;
					break;
				case MAKE_FUNCTION_DEFAULTS:
					funcObj->defaults = attr;
					break;
				case MAKE_FUNCTION_ANNOTATE:
					funcObj->annotate = attr;
					break;
				default:
					ALIF_UNREACHABLE();
				}
				stackPointer[-2] = funcSt;
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
			TARGET(STORE_NAME) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef v{};
				v = stackPointer[-1];
				AlifObject* name = GETITEM(FRAME_CO_NAMES, oparg);
				AlifObject* ns = LOCALS();
				AlifIntT err{};
				if (ns == nullptr) {
					//_alifErr_format(_thread, _alifExcSystemError_,
					//	"no locals found when storing %R", name);
					alifStackRef_close(v);
					//if (true) goto pop_1_error;
				}
				if (ALIFDICT_CHECKEXACT(ns))
					err = alifDict_setItem(ns, name, alifStackRef_asAlifObjectBorrow(v));
				else
					err = alifObject_setItem(ns, name, alifStackRef_asAlifObjectBorrow(v));
				alifStackRef_close(v);
				//if (err) goto pop_1_error;
				stackPointer += -1;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(SWAP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef bottom{};
				AlifStackRef top{};
				top = stackPointer[-1];
				bottom = stackPointer[-2 - (oparg - 2)];
				stackPointer[-2 - (oparg - 2)] = top;
				stackPointer[-1] = bottom;
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
				seq = stackPointer[-1];
				{
					uint16_t counter = read_u16(&thisInstr[1].cache);
#if ENABLE_SPECIALIZATION
					if (ADAPTIVE_COUNTER_TRIGGERS(counter)) {
						nextInstr = thisInstr;
						_alifSpecialize_UnpackSequence(seq, nextInstr, oparg);
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
					AlifIntT res = _alifEval_unpackIterableStackRef(_thread, seq, oparg, -1, top);
					alifStackRef_close(seq);
					//if (res == 0) goto pop_1_error;
				}
				stackPointer += -1 + oparg;
				DISPATCH();
			} // ------------------------------------------------------------ //
			TARGET(RESUME) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				PREDICTED(RESUME);
				AlifCodeUnit* thisInstr = nextInstr - 1;
				// _MAYBE_INSTRUMENT
				{
					if (_thread->tracing == 0) {
						uintptr_t globalVersion = alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ~ALIF_EVAL_EVENTS_MASK;
						uintptr_t codeVersion = alifAtomic_loadUintptrAcquire(&_alifFrame_getCode(_frame)->instrumentationVersion);
						if (codeVersion != globalVersion) {
							AlifIntT err = _alif_instrument(_alifFrame_getCode(_frame), _thread->interpreter);
							if (err) {
								//goto error;
							}
							nextInstr = thisInstr;
							DISPATCH();
						}
					}
				}
				// _QUICKEN_RESUME
				{
#if ENABLE_SPECIALIZATION
					if (_thread->tracing == 0 and thisInstr->op.code == RESUME) {
						alifAtomic_storeUint8Relaxed(thisInstr->op.code, RESUME_CHECK);
					}
#endif  /* ENABLE_SPECIALIZATION */
				}
				// _CHECK_PERIODIC_IF_NOT_YIELD_FROM
				{
					if ((oparg & RESUME_OPARG_LOCATION_MASK) < RESUME_AFTER_YIELD_FROM) {
						//ALIF_CHECK_EMSCRIPTEN_SIGNALS_PERIODICALLY();
						QSBR_QUIESCENT_STATE(_thread); \
							if (alifAtomic_loadUintptrRelaxed(&_thread->evalBreaker) & ALIF_EVAL_EVENTS_MASK) {
								//AlifIntT err = _alif_handlePending(_thread);
								//if (err != 0) goto error;
							}
					}
				}
				DISPATCH();
			} // ------------------------------------------------------------ //
		}
	}

exit_unwind:
	//_alif_leaveRecursiveCallAlif(_thread);
	//AlifInterpreterFrame* dying = _frame;
	//_frame = _thread->currentFrame = dying->previous;
	//_alifEval_frameClearAndPop(_thread, dying);
	//_frame->returnOffset = 0;
	//if (_frame == &entryFrame) {
	//	_thread->currentFrame = _frame->previous;
	//	_thread->cppRecursionRemaining += ALIF_EVAL_CPP_STACK_UNITS;
	//	return nullptr;
	//}



	return ALIF_NONE; // alif
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
		nameStr = alifUStr_fromFormat("%U and %U",
			ALIFLIST_GET_ITEM(_names, len - 2),
			ALIFLIST_GET_ITEM(_names, len - 1));
		break;
	default:
		tail = alifUStr_fromFormat(", %U, and %U",
			ALIFLIST_GET_ITEM(_names, len - 2),
			ALIFLIST_GET_ITEM(_names, len - 1));
		if (tail == NULL)
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
	//_alifErr_format(_thread, _alifExcTypeError_,
	//	"%U() missing %i required %s argument%s: %U",
	//	_qualname, len, _kind,
	//	len == 1 ? "" : "s", nameStr);
	ALIF_DECREF(nameStr);
}


static void missing_arguments(AlifThread* _thread, AlifCodeObject* _co,
	AlifSizeT _missing, AlifSizeT _defcount,
	AlifStackRef* _localsPlus, AlifObject* _qualname) { // 1170
	AlifSizeT i{}, j = 0;
	AlifSizeT start{}, end{};
	AlifIntT positional = (_defcount != -1);
	const char* kind = positional ? "positional" : "keyword-only";
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
		sig = alifUStr_fromFormat("from %zd to %zd", atleast, coArgCount);
	}
	else {
		plural = (coArgCount != 1);
		sig = alifUStr_fromFormat("%zd", coArgCount);
	}
	if (sig == nullptr)
		return;
	if (kwOnlyGiven) {
		const char* format = " positional argument%s (and %zd keyword-only argument%s)";
		kwOnlySig = alifUStr_fromFormat(format,
			_given != 1 ? "s" : "",
			kwOnlyGiven,
			kwOnlyGiven != 1 ? "s" : "");
		if (kwOnlySig == nullptr) {
			ALIF_DECREF(sig);
			return;
		}
	}
	else {
		/* This will not fail. */
		kwOnlySig = alifUStr_fromString("");
	}
	//_alifErr_format(_thread, _alifExcTypeError_,
	//	"%U() takes %U positional argument%s but %zd%U %s given",
	//	_qualname, sig, plural ? "s" : "", _given, kwOnlySig,
	//	_given == 1 and !kwOnlyGiven ? "was" : "were");
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
		//_alifErr_format(tstate, _alifExcTypeError_,
			//"%U() got some positional-only arguments passed"
			//" as keyword arguments: '%U'",
			//qualname, error_names);
		ALIF_DECREF(errorNames);
		goto fail;
	}

	ALIF_DECREF(posOnlyNames);
	return 0;

fail:
	ALIF_DECREF(posOnlyNames);
	return 1;

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
			u = alifTuple_fromStackRefSteal(_args + n, _argCount - n);
		}
		if (u == nullptr) {
			goto fail_post_positional;
		}
		_localsPlus[totalArgs] = ALIFSTACKREF_FROMALIFOBJECTSTEAL(u);
	}
	else if (_argCount > n) {
		/* Too many positional args. Error is reported later */
		for (j = n; j < _argCount; j++) {
			alifStackRef_close(_args[j]);
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
				//_alifErr_format(_thread, _alifExcTypeError_,
				//	"%U() keywords must be strings",
				//	_func->qualname);
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
						_func->qualname))
				{
					goto kw_fail;
				}

				AlifObject* suggestionKeyword = nullptr;
				if (totalArgs > co->posOnlyArgCount) {
					AlifObject* possibleKeywords = alifList_new(totalArgs - co->posOnlyArgCount);

					if (!possibleKeywords) {
						//alifErr_clear();
					}
					else {
						for (AlifSizeT k = co->posOnlyArgCount; k < totalArgs; k++) {
							ALIFLIST_SET_ITEM(possibleKeywords, k - co->posOnlyArgCount, varNames[k]);
						}

						//suggestionKeyword = _alif_calculateSuggestions(possibleKeywords, keyword);
						ALIF_DECREF(possibleKeywords);
					}
				}

				if (suggestionKeyword) {
					//_alifErr_format(_thread, _alifExcTypeError_,
					//	"%U() got an unexpected keyword argument '%S'. Did you mean '%S'?",
					//	_func->qualname, keyword, suggestionKeyword);
					ALIF_DECREF(suggestionKeyword);
				}
				else {
					//_alifErr_format(_thread, _alifExcTypeError_,
					//	"%U() got an unexpected keyword argument '%S'",
					//	_func->qualname, keyword);
				}

				goto kw_fail;
			}

			if (alifDict_setItem(kwdict, keyword, alifStackRef_asAlifObjectBorrow(valueStackRef)) == -1) {
				goto kw_fail;
			}
			alifStackRef_close(valueStackRef);
			continue;

		kw_fail:
			for (; i < kwcount; i++) {
				alifStackRef_close(_args[i + _argCount]);
			}
			goto fail_post_args;

		kw_found:
			if (alifStackRef_asAlifObjectBorrow(_localsPlus[j]) != nullptr) {
				//_alifErr_format(_thread, _alifExcTypeError_,
				//	"%U() got multiple values for argument '%S'",
				//	_func->qualname, keyword);
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
		alifStackRef_close(_args[j]);
	}
	/* fall through */
fail_post_positional:
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (j = _argCount; j < _argCount + kwcount; j++) {
			alifStackRef_close(_args[j]);
		}
	}
	/* fall through */
fail_post_args:
	return -1;
}



static void clear_threadFrame(AlifThread* _thread, AlifInterpreterFrame* _frame) { // 1644
	_thread->cppRecursionRemaining--;
	_alifFrame_clearExceptCode(_frame);
	ALIF_DECREF(_frame->executable);
	_thread->cppRecursionRemaining++;
	_alifThreadState_popFrame(_thread, _frame);
}

void _alifEval_frameClearAndPop(AlifThread* _thread, AlifInterpreterFrame* _frame) { // 1677
	if (_frame->owner == FrameOwner::FRAME_OWNED_BY_THREAD) {
		clear_threadFrame(_thread, _frame);
	}
	else {
		//clear_genFrame(_thread, _frame);
	}
}

AlifInterpreterFrame* _alifEval_framePushAndInit(AlifThread* _thread,
	AlifFunctionObject* _func, AlifObject* _locals, AlifStackRef const* _args,
	AlifUSizeT _argCount, AlifObject* _kwNames, AlifInterpreterFrame* _previous) { // 1689
	AlifCodeObject* code = (AlifCodeObject*)_func->code;
	AlifInterpreterFrame* frame = _alifThreadState_pushFrame(_thread, code->frameSize);
	if (frame == nullptr) {
		goto fail;
	}
	_alifFrame_initialize(frame, _func, _locals, code, 0, _previous);
	if (initialize_locals(_thread, _func, frame->localsPlus, _args, _argCount, _kwNames)) {
		clear_threadFrame(_thread, frame);
		return nullptr;
	}
	return frame;
fail:
	/* Consume the references */
	ALIF_DECREF(_func);
	ALIF_XDECREF(_locals);
	for (size_t i = 0; i < _argCount; i++) {
		alifStackRef_close(_args[i]);
	}
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			alifStackRef_close(_args[i + _argCount]);
		}
	}
	//alifErr_noMemory();
	return nullptr;
}





static AlifInterpreterFrame* _alifEvalFramePushAndInit_unTagged(AlifThread* _thread,
	AlifFunctionObject* _func, AlifObject* _locals, AlifObject* const* _args,
	AlifUSizeT _argCount, AlifObject* _kwNames, AlifInterpreterFrame* _previous) { // 1724
#if defined(ALIF_GIL_DISABLED)
	AlifUSizeT kwCount = _kwNames == nullptr ? 0 : ALIFTUPLE_GET_SIZE(_kwNames);
	AlifUSizeT totalArgCount = _argCount + kwCount;
	totalArgCount ? totalArgCount : totalArgCount = 1; // alif
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
#else
	return _alifEval_framePushAndInit(_thread, _func, _locals, (AlifStackRef const*)_args, _argCount, _kwNames, _previous);
#endif
}





AlifObject* alifEval_vector(AlifThread* _tstate, AlifFunctionObject* _func,
	AlifObject* _locals, AlifObject* const* _args, AlifUSizeT _argCount,
	AlifObject* _kwNames) { // 1794
	ALIF_INCREF(_func);
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
		_tstate, _func, _locals, _args, _argCount, _kwNames, nullptr);
	if (frame == nullptr) {
		return nullptr;
	}
	return alifEval_evalFrame(_tstate, frame, 0);
}








AlifIntT _alifEval_unpackIterableStackRef(AlifThread* _thread, AlifStackRef _vStackRef,
	AlifIntT _argCnt, AlifIntT _argCntAfter, AlifStackRef* _sp) { // 2064
	AlifIntT i = 0, j = 0;
	AlifSizeT ll = 0;
	AlifObject* it;  /* iter(v) */
	AlifObject* w{};
	AlifObject* l = nullptr; /* variable list */

	AlifObject* v = alifStackRef_asAlifObjectBorrow(_vStackRef);

	it = alifObject_getIter(v);
	if (it == nullptr) {
		//if (_alifErr_exceptionMatches(_thread, _alifExcTypeError_) and
		//	ALIF_TYPE(v)->iter == nullptr and !alifSequence_check(v))
		//{
		//	_alifErr_format(_thread, _alifExcTypeError_,
		//		"cannot unpack non-iterable %.200s object",
		//		ALIF_TYPE(v)->name);
		//}
		return 0;
	}

	for (; i < _argCnt; i++) {
		w = alifIter_next(it);
		if (w == nullptr) {
			/* Iterator done, via error or exhaustion. */
			//if (!_alifErr_occurred(_thread)) {
			//	if (_argCntAfter == -1) {
			//		_alifErr_format(_thread, _alifExcValueError_,
			//			"not enough values to unpack "
			//			"(expected %d, got %d)",
			//			_argCnt, i);
			//	}
			//	else {
			//		_alifErr_format(_thread, _alifExcValueError_,
			//			"not enough values to unpack "
			//			"(expected at least %d, got %d)",
			//			_argCnt + _argCntAfter, i);
			//	}
			//}
			goto Error;
		}
		*--_sp = ALIFSTACKREF_FROMALIFOBJECTSTEAL(w);
	}

	if (_argCntAfter == -1) {
		/* We better have exhausted the iterator now. */
		w = alifIter_next(it);
		if (w == nullptr) {
			//if (_alifErr_occurred(_thread))
			//	goto Error;
			ALIF_DECREF(it);
			return 1;
		}
		ALIF_DECREF(w);
		//_alifErr_format(_thread, _alifExcValueError_,
		//	"too many values to unpack (expected %d)",
		//	_argCnt);
		goto Error;
	}

	l = alifSequence_list(it);
	if (l == nullptr)
		goto Error;
	*--_sp = ALIFSTACKREF_FROMALIFOBJECTSTEAL(l);
	i++;

	ll = ALIFLIST_GET_SIZE(l);
	if (ll < _argCntAfter) {
		//_alifErr_format(_thread, _alifExcValueError_,
		//	"not enough values to unpack (expected at least %d, got %zd)",
		//	_argCnt + _argCntAfter, _argCnt + ll);
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
		alifStackRef_close(*_sp);
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






AlifObject* alifEval_getGlobals() { // 2557
	AlifThread* thread = _alifThread_get();
	AlifInterpreterFrame* current_frame = _alifThreadState_getFrame(thread);
	if (current_frame == nullptr) {
		return nullptr;
	}
	return current_frame->globals;
}


AlifObject* _alifEval_loadGlobal(AlifObject* _globals,
	AlifObject* _builtins, AlifObject* _name) { // 3073
	AlifObject* res{};
	if (ALIFDICT_CHECKEXACT(_globals) and ALIFDICT_CHECKEXACT(_builtins)) {
		res = _alifDict_loadGlobal((AlifDictObject*)_globals,
			(AlifDictObject*)_builtins, _name);
		if (res == nullptr /*and !alifErr_occurred()*/) {
			//_alifEval_formatExcCheckArg(ALIFTHREADSTATE_GET(), _alifExcNameError_,
			//	NAME_ERROR_MSG, _name);
		}
	}
	else {
		if (alifMapping_getOptionalItem(_globals, _name, &res) < 0) {
			return nullptr;
		}
		if (res == nullptr) {
			/* namespace 2: builtins */
			if (alifMapping_getOptionalItem(_builtins, _name, &res) < 0) {
				return nullptr;
			}
			if (res == nullptr) {
				//_alifEval_formatExcCheckArg(
				//	ALIFTHREADSTATE_GET(), _alifExcNameError_,
				//	NAME_ERROR_MSG, _name);
			}
		}
	}
	return res;
}


AlifObject* _alifEval_loadName(AlifThread* _thread,
	AlifInterpreterFrame* _frame, AlifObject* _name) { // 3133

	AlifObject* value{};
	if (_frame->locals == nullptr) {
		//_alifErr_setString(_thread, _alifExcSystemError_,
		//	"no locals found");
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
		//_alifEval_formatExcCheckArg(
		//	_thread, _alifExcNameError_,
		//	NAME_ERROR_MSG, _name);
	}
	return value;
}
