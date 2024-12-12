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










AlifObject* ALIF_HOT_FUNCTION alifEval_evalFrameDefault(AlifThread* _thread,
	AlifInterpreterFrame* _frame, AlifIntT _throwflag) { // 726


}


static AlifIntT initialize_locals(AlifThread* _thread, AlifFunctionObject* _func,
	AlifStackRef* _localsPlus, AlifStackRef const* _args,
	AlifSizeT _argCount, AlifObject* _kwNames) { // 1399
	AlifCodeObject* co = (AlifCodeObject*)_func->code;
	const AlifSizeT totalArgs = co->argCount + co->kwOnlyArgCount;

	AlifObject* kwdict{};
	AlifSizeT i{};
	if (co->flags & CO_VARKEYWORDS) {
		kwdict = alifDict_new();
		if (kwdict == nullptr) {
			goto fail_pre_positional;
		}
		i = totalArgs;
		if (co->flags & CO_VARARGS) {
			i++;
		}
		_localsPlus[i] = alifStackRef_fromAlifObjectSteal(kwdict);
	}
	else {
		kwdict = nullptr;
	}

	AlifSizeT j{}, n{};
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
		_localsPlus[totalArgs] = ALIFSTACKREF_FROMPYOBJECTSTEAL(u);
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

						suggestionKeyword = _alif_CalculateSuggestions(possibleKeywords, keyword);
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
			if (alifStackRef_asPyObjectBorrow(_localsPlus[j]) != nullptr) {
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
					_localsPlus[m + i] = alifStackRef_fromAlifObjectNew(def);
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
					_localsPlus[i] = alifStackRef_fromAlifObjectSteal(def);
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

fail_pre_positional:
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
	AlifStackRef* taggedArgsBuffer = (AlifStackRef*)alifMem_dataAlloc(sizeof(AlifStackRef) * totalArgCount);
	if (taggedArgsBuffer == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	for (AlifUSizeT i = 0; i < _argCount; i++) {
		taggedArgsBuffer[i] = ALIFSTACKREF_FROMPYOBJECTSTEAL(_args[i]);
	}
	for (AlifUSizeT i = 0; i < kwCount; i++) {
		taggedArgsBuffer[_argCount + i] = ALIFSTACKREF_FROMPYOBJECTSTEAL(_args[_argCount + i]);
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
