#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Backoff.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Code.h"

#include "AlifCore_Function.h"

#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpcodeMetaData.h"
#include "AlifCore_Optimizer.h"
#include "AlifCore_OpcodeUtils.h"

#include "AlifCore_State.h"



#include "AlifCore_Frame.h"

















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
	if (alif_enterRecursiveCallTstate(_thread, "")) {
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
			TARGET(FORMAT_SIMPLE) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				AlifStackRef res{};
				value = stackPointer[-1];
				AlifObject* value_o = alifStackRef_asAlifObjectBorrow(value);
				/* If value is a UStr object, then we know the result
				 * of format(value) is value itself. */
				if (!ALIFUSTR_CHECKEXACT(value_o)) {
					res = ALIFSTACKREF_FROMALIFOBJECTSTEAL(alifObject_format(value_o, nullptr));
					alifStackRef_close(value);
					//if (ALIFSTACKREF_ISNULL(res)) goto pop_1_error;
				}
				else {
					res = value;
				}
				stackPointer[-1] = res;
				DISPATCH();
			}
			TARGET(NOP) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				DISPATCH();
			}
			TARGET(LOAD_CONST) {
				_frame->instrPtr = nextInstr;
				nextInstr += 1;
				AlifStackRef value{};
				value = ALIFSTACKREF_FROMALIFOBJECTNEW(GETITEM(FRAME_CO_CONSTS, oparg));
				stackPointer[0] = value;
				stackPointer += 1;
				DISPATCH();
			}
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
			}


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



	return ALIF_NONE;
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
















AlifObject* _alifEval_getBuiltins(AlifThread* _thread) { // 2444
	AlifInterpreterFrame* frame = _alifThreadState_getFrame(_thread);
	if (frame != nullptr) {
		return frame->builtins;
	}
	return _thread->interpreter->builtins;
}
