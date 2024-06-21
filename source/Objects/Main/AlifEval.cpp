#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_AlifEval.h"
#include "AlifCore_Code.h"
#include "AlifCore_Object.h"
#include "AlifCore_OpCodeData.h"
#include "AlifCore_OpCodeUtils.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Function.h"
#include "AlifCore_Integer.h"
#include "AlifCore_Tuple.h"

#include "AlifCore_Frame.h"

#include "OpCode.h"


AlifObject* alifEval_evalCode(AlifObject* _co) { // 572
	AlifThread* thread = alifThread_get();
	//if (_locals == nullptr) {
	//	_locals = _globals;
	//}
	//AlifObject* builtins = alifEval_builtinsFromGlobals(thread, _globals);
	//if (builtins == nullptr) return nullptr;

	AlifFrameConstructor desc = {
		nullptr,
		nullptr,
		((AlifCodeObject*)_co)->name,
		((AlifCodeObject*)_co)->name,
		_co,
		nullptr,
		nullptr,
		nullptr
	};
	AlifFunctionObject* func = alifFunction_fromConstructor(&desc);
	if (func == nullptr) return nullptr;

	AlifObject* res = alifEval_vector(thread, func, nullptr, nullptr, 0, nullptr);
	ALIF_DECREF(func);
	return res;
}


static AlifIntT positionalOnly_passedAsKeyword(AlifThread* _thread, AlifCodeObject* _co,
	AlifSizeT _kwCount, AlifObject* _kwNames, AlifObject* qualname) { // 1290

	int posOnlyConflicts = 0;
	AlifObject* posOnlyNames = alifNew_list(0);
	if (posOnlyNames == nullptr) goto fail;
	
	for (int k = 0; k < _co->posOnlyArgCount; k++) {
		AlifObject* posOnlyName = ALIFTUPLE_GET_ITEM(_co->localsPlusNames, k);

		for (int k2 = 0; k2 < _kwCount; k2++) {
			AlifObject* kwName = ALIFTUPLE_GET_ITEM(_kwNames, k2);
			if (kwName == posOnlyName) {
				if (alifList_append(posOnlyNames, kwName) != 0) {
					goto fail;
				}
				posOnlyConflicts++;
				continue;
			}

			int cmp = alifObject_richCompareBool(posOnlyName, kwName, ALIF_EQ);

			if (cmp > 0) {
				if (alifList_append(posOnlyNames, kwName) != 0) {
					goto fail;
				}
				posOnlyConflicts++;
			}
			else if (cmp < 0) {
				goto fail;
			}

		}
	}
	if (posOnlyConflicts) {
		AlifObject* comma = alifUStr_fromString(L", ");
		if (comma == nullptr) {
			goto fail;
		}
		AlifObject* errorNames = alifUStr_join(comma, posOnlyNames);
		ALIF_DECREF(comma);
		if (errorNames == nullptr) {
			goto fail;
		}
		// error
		ALIF_DECREF(errorNames);
		goto fail;
	}

	ALIF_DECREF(posOnlyNames);
	return 0;

fail:
	ALIF_XDECREF(posOnlyNames);
	return 1;

}


static AlifIntT initialize_locals(AlifThread* _thread, AlifFunctionObject* _func,
	AlifObject** _localsPlus, AlifObject* const* _args, AlifSizeT _argCount, AlifObject* _kwNames) { // 1422

	AlifCodeObject* co = (AlifCodeObject*)_func->funcCode;
	const AlifSizeT totalArgs = co->args + co->kwOnlyArgCount;

	AlifObject* kwDict{};
	AlifSizeT i{};
	if (co->flags & CO_VARKEYWORDS) {
		kwDict = alifNew_dict();
		if (kwDict == nullptr) {
			goto fail_pre_positional;
		}
		i = totalArgs;
		if (co->flags & CO_VARARGS) {
			i++;
		}
		_localsPlus[i] = kwDict;
	}
	else {
		kwDict = nullptr;
	}

	AlifSizeT j, n;
	if (_argCount > co->args) {
		n = co->args;
	}
	else {
		n = _argCount;
	}
	for (j = 0; j < n; j++) {
		AlifObject* x = _args[j];
		_localsPlus[j] = x;
	}

	if (co->flags & CO_VARARGS) {
		AlifObject* u = nullptr;
		if (_argCount == n) {
			u = (AlifObject*)&ALIF_SINGLETON(tupleEmpty);
		}
		else {
			u = alifTuple_fromArraySteal(_args + n, _argCount - n);
		}
		if (u == nullptr) {
			goto fail_post_positional;
		}
		_localsPlus[totalArgs] = u;
	}
	else if (_argCount > n) {
		for (j = n; j < _argCount; j++) {
			ALIF_DECREF(_args[j]);
		}
	}

	if (_kwNames != nullptr) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (i = 0; i < kwcount; i++) {
			AlifObject** co_varnames;
			AlifObject* keyword = ALIFTUPLE_GET_ITEM(_kwNames, i);
			AlifObject* value = _args[i + _argCount];
			AlifSizeT j;

			if (keyword == nullptr or !ALIFUSTR_CHECK(keyword)) {
				// error
				goto kw_fail;
			}

			co_varnames = ((AlifTupleObject*)(co->localsPlusNames))->items_;
			for (j = co->posOnlyArgCount; j < totalArgs; j++) {
				AlifObject* varname = co_varnames[j];
				if (varname == keyword) {
					goto kw_found;
				}
			}

			for (j = co->posOnlyArgCount; j < totalArgs; j++) {
				AlifObject* varname = co_varnames[j];
				int cmp = alifObject_richCompareBool(keyword, varname, ALIF_EQ);
				if (cmp > 0) {
					goto kw_found;
				}
				else if (cmp < 0) {
					goto kw_fail;
				}
			}

			if (kwDict == nullptr) {

				if (co->posOnlyArgCount and positionalOnly_passedAsKeyword(_thread, co,
						kwcount, _kwNames,
						_func->funcQualname))
				{
					goto kw_fail;
				}

				AlifObject* suggestion_keyword = nullptr;
				if (totalArgs > co->posOnlyArgCount) {
					AlifObject* possible_keywords = alifNew_list(totalArgs - co->posOnlyArgCount);

					if (!possible_keywords) {
						//error
					}
					else {
						for (AlifSizeT k = co->posOnlyArgCount; k < totalArgs; k++) {
							ALIFLIST_SETITEM(possible_keywords, k - co->posOnlyArgCount, co_varnames[k]);
						}

						//suggestion_keyword = alif_calculateSuggestions(possible_keywords, keyword);
						ALIF_DECREF(possible_keywords);
					}
				}

				if (suggestion_keyword) {
					// error
					ALIF_DECREF(suggestion_keyword);
				}
				else {
					// error
				}

				goto kw_fail;
			}

			//if (alifDict_setItem(kwDict, keyword, value) == -1) {
			//	goto kw_fail;
			//}
			ALIF_DECREF(value);
			continue;

		kw_fail:
			for (; i < kwcount; i++) {
				AlifObject* value = _args[i + _argCount];
				ALIF_DECREF(value);
			}
			goto fail_post_args;

		kw_found:
			if (_localsPlus[j] != nullptr) {
				// errro
				goto kw_fail;
			}
			_localsPlus[j] = value;
		}
	}

	if ((_argCount > co->args) and !(co->flags & CO_VARARGS)) {
		//too_many_positional(_thread, co, _argCount, _func->funcDefaults, _localsPlus, _func->funcQualname);
		goto fail_post_args;
	}

	if (_argCount < co->args) {
		AlifSizeT defcount = _func->funcDefaults == nullptr ? 0 : ALIFTUPLE_GET_SIZE(_func->funcDefaults);
		AlifSizeT m = co->args - defcount;
		AlifSizeT missing = 0;
		for (i = _argCount; i < m; i++) {
			if (_localsPlus[i] == nullptr) {
				missing++;
			}
		}
		if (missing) {
			//missing_arguments(_thread, co, missing, defcount, _localsPlus, _func->funcQualname);
			goto fail_post_args;
		}
		if (n > m)
			i = n - m;
		else
			i = 0;
		if (defcount) {
			AlifObject** defs = &ALIFTUPLE_GET_ITEM(_func->funcDefaults, 0);
			for (; i < defcount; i++) {
				if (_localsPlus[m + i] == nullptr) {
					AlifObject* def = defs[i];
					_localsPlus[m + i] = ALIF_NEWREF(def);
				}
			}
		}
	}

	if (co->kwOnlyArgCount > 0) {
		AlifSizeT missing = 0;
		for (i = co->args; i < totalArgs; i++) {
			if (_localsPlus[i] != nullptr)
				continue;
			AlifObject* varname = ALIFTUPLE_GET_ITEM(co->localsPlusNames, i);
			if (_func->funcKwdefaults != nullptr) {
				AlifObject* def;
				if (alifDict_getItemRef(_func->funcKwdefaults, varname, &def) < 0) {
					goto fail_post_args;
				}
				if (def) {
					_localsPlus[i] = def;
					continue;
				}
			}
			missing++;
		}
		if (missing) {
			//missing_arguments(_thread, co, missing, -1, _localsPlus, _func->funcQualname);
			goto fail_post_args;
		}
	}
	return 0;

fail_pre_positional:
	for (j = 0; j < _argCount; j++) {
		ALIF_DECREF(_args[j]);
	}

fail_post_positional:
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (j = _argCount; j < _argCount + kwcount; j++) {
			ALIF_DECREF(_args[j]);
		}
	}

fail_post_args:
	return -1;
}

static void clear_threadFrame(AlifThread* _thread, AlifInterpreterFrame* _frame)
{
	_thread->recursionRemaining--;
	//alifFrame_clearExceptCode(_frame);
	//ALIF_DECREF(_frame->f_executable);
	_thread->recursionRemaining++;
	//alifThread_popFrame(_thread, _frame);
}

AlifInterpreterFrame* alifEvalFrame_initAndPush(AlifThread* _thread, AlifFunctionObject* _func,
	AlifObject* _locals, AlifObject* const* _args, AlifUSizeT _argCount, AlifObject* _kwNames) { // 1715

	AlifCodeObject* code = (AlifCodeObject*)_func->funcCode;
	AlifInterpreterFrame* frame = alifThread_pushFrame(_thread, code->frameSize);
	if (frame == nullptr) goto fail;

	alifFrame_initialize(frame, _func, _locals, code, 0);
	if (initialize_locals(_thread, _func, frame->localsPlus, _args, _argCount, _kwNames)) {
		clear_threadFrame(_thread, frame);
		return nullptr;
	}
	return frame;
fail:
	ALIF_DECREF(_func);
	//ALIF_XDECREF(_locals);
	for (size_t i = 0; i < _argCount; i++) {
		ALIF_DECREF(_args[i]);
	}
	if (_kwNames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwNames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			ALIF_DECREF(_args[i + _argCount]);
		}
	}
	// memory error
	return nullptr;
}


AlifObject* alifEval_vector(AlifThread* _thread, AlifFunctionObject* _func, AlifObject* _locals,
	AlifObject* const* _args, AlifUSizeT _argcount, AlifObject* _kwnames) { // 1793

	ALIF_INCREF(_func);
	//ALIF_XINCREF(_locals);
	for (AlifUSizeT i = 0; i < _argcount; i++) {
		ALIF_INCREF(_args[i]);
	}
	if (_kwnames) {
		AlifSizeT kwcount = ALIFTUPLE_GET_SIZE(_kwnames);
		for (AlifSizeT i = 0; i < kwcount; i++) {
			ALIF_INCREF(_args[i + _argcount]);
		}
	}
	AlifInterpreterFrame* frame = alifEvalFrame_initAndPush(
		_thread, _func, _locals, _args, _argcount, _kwnames);
	if (frame == nullptr) return nullptr;

	//return alifEval_evalFrame(_thread, frame, 0);
}
