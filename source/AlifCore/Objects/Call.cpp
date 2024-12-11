#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_Call.h"
#include "AlifCore_Dict.h"
#include "AlifCore_State.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Tuple.h"
#include "AlifCore_Object.h"
#include "AlifCore_Eval.h"

//#define ALIFVECTORCALL_ARGUMENTS_OFFSET \
//    ((size_t)1 << (8 * sizeof(size_t) - 1))
//
//AlifObject* alif_checkFunctionResult(AlifThread* _thread,
//	AlifObject* _callable, AlifObject* _result, const wchar_t* _where) { 
//
//	if (_result == nullptr) {
//		//if (!alifErr_occurred(_thread)) {
//		//	if (_callable)
//		//		alifErr_format(_thread, alifExcSystemError,
//		//			"%R returned nullptr without setting an exception", _callable);
//		//	else
//		//		alifErr_format(_thread, alifExcSystemError,
//		//			"%s returned nullptr without setting an exception", _where);
//		//	return nullptr;
//		//}
//	}
//	else {
//		//if (alifErr_occurred(_thread)) {
//		//	ALIF_DECREF(_result);
//
//		//	if (_callable) {
//		//		alifErr_formatFromCauseThread(
//		//			_thread, alifExcSystemError,
//		//			"%R returned a result with an exception set", _callable);
//		//	}
//		//	else {
//		//		alifErr_formatFromCauseTstate(
//		//			_thread, alifExcSystemError,
//		//			"%s returned a result with an exception set", _where);
//		//	}
//		//	return nullptr;
//		//}
//	}
//	return _result;
//}
//
//static void object_isNotCallable(AlifThread* _thread, AlifObject* _callable) {
//	if (ALIF_IS_TYPE(_callable, &_alifModuleType_)) {
//		AlifObject* name = alifModule_getNameObject(_callable);
//		if (name == nullptr) {
//			//alifErr_clear(_thread);
//			goto basic_type_error;
//		}
//		AlifObject* attr;
//		int res = alifObject_getOptionalAttr(_callable, name, &attr);
//		if (res < 0) {
//			//alifErr_clear(_thread);
//		}
//		else if (res > 0 and alifCallable_check(attr)) {
//			//alifErr_format(_thread, alifExcTypeError,
//			//	"'%.200s' object is not callable. "
//			//	"Did you mean: '%U.%U(...)'?",
//			//	ALIF_TYPE(_callable)->name_, name, name);
//			ALIF_DECREF(attr);
//			ALIF_DECREF(name);
//			return;
//		}
//		ALIF_XDECREF(attr);
//		ALIF_DECREF(name);
//	}
//basic_type_error:
//	//alifErr_format(_thread, alifExcTypeError, "'%.200s' object is not callable", ALIF_TYPE(_callable)->name_);
//	return; //
//}
//
//AlifObject* alifObject_makeTpCall(AlifThread* _thread, AlifObject* _callable,
//	AlifObject* const* _args, AlifSizeT _nargs, AlifObject* _keywords) { 
//
//	TernaryFunc call = ALIF_TYPE(_callable)->call_;
//	if (call == nullptr) {
//		object_isNotCallable(_thread, _callable);
//		return nullptr;
//	}
//
//	AlifObject* argstuple = alifSubTuple_fromArray(_args, _nargs);
//	if (argstuple == nullptr) {
//		return nullptr;
//	}
//
//	AlifObject* kwdict;
//	if (_keywords == nullptr or ALIFDICT_CHECK(_keywords)) {
//		kwdict = _keywords;
//	}
//	else {
//		if (ALIFTUPLE_GET_SIZE(_keywords)) {
//			kwdict = alifStack_asDict(_args + _nargs, _keywords);
//			if (kwdict == nullptr) {
//				ALIF_DECREF(argstuple);
//				return nullptr;
//			}
//		}
//		else {
//			_keywords = kwdict = nullptr;
//		}
//	}
//
//	AlifObject* result = nullptr;
//	if (alif_enterRecursiveCallThread(_thread, L" بينما يتم إستدعاء كائن ألف") == 0)
//	{
//		result = ALIFCFUNCTIONWITHKEYWORDS_TRAMPOLINECALL((AlifCFunctionWithKeywords)call, _callable, argstuple, kwdict);
//		alif_leaveRecursiveCallThread(_thread);
//	}
//
//	ALIF_DECREF(argstuple);
//	if (kwdict != _keywords) {
//		ALIF_DECREF(kwdict);
//	}
//
//	return alif_checkFunctionResult(_thread, _callable, result, nullptr);
//}



VectorCallFunc alifVectorCall_function(AlifObject* _callable) { // 256
	return _alifVectorCall_functionInline(_callable);
}

//AlifObject* alifObject_vectorCall(AlifObject* callable, AlifObject* const* args,
//	AlifUSizeT nargsf, AlifObject* kwnames)
//{
//	AlifThread* thread = alifThread_get();
//	return alifObject_vectorCallThread(thread, callable,
//		args, nargsf, kwnames);
//}

//AlifObject* alifObject_callOneArg(AlifObject* func, AlifObject* arg) { 
//
//	AlifObject* _args[2];
//	AlifObject** args = _args + 1; 
//	args[0] = arg;
//	AlifThread* thread = alifThread_get();
//	size_t nargsf = 1 | ALIF_VECTORCALL_ARGUMENTS_OFFSET;
//	return alifObject_vectorCallThread(thread, func, args, nargsf, nullptr);
//}


AlifObject* alifFunction_vectorCall(AlifObject* _func, AlifObject* const* _stack,
	AlifUSizeT _nArgsF, AlifObject* _kwNames) { // 402

	AlifFunctionObject* f_ = (AlifFunctionObject*)_func;
	AlifSizeT nArgs = ALIFVECTORCALL_NARGS(_nArgsF);
	AlifThread* tstate = _alifThread_get();
	if (((AlifCodeObject*)f_->code)->flags & CO_OPTIMIZED) {
		return alifEval_vector(tstate, f_, nullptr, _stack, nArgs, _kwNames);
	}
	else {
		return alifEval_vector(tstate, f_, f_->globals, _stack, nArgs, _kwNames);
	}
}


//AlifObject* alifStack_asDict(AlifObject* const* _values, AlifObject* _kwNames) { 
//	AlifSizeT nkwargs;
//
//	nkwargs = ALIFTUPLE_GET_SIZE(_kwNames);
//	return alifDict_fromItems(&ALIFTUPLE_GET_ITEM(_kwNames, 0), 1, _values, 1, nkwargs);
//}
//
//static AlifObject* alifVectorCall_callSub(VectorCallFunc func,
//    AlifObject* callable, AlifObject* tuple, AlifObject* kwArgs)
//{
//
//    int64_t nargs = ((AlifTupleObject*)tuple)->_base_.size_;
//
//    /* Fast path for no keywords */
//    if (kwArgs == nullptr || ((AlifDictObject*)kwArgs)->used == 0) {
//        return func(callable, ((AlifTupleObject*)tuple)->items_, nargs, nullptr);
//    }
//
//    /* Convert arguments & call */
//    AlifObject* const* args{};
//    AlifObject* kwNames{};
//    args = alifStack_unpackDict(
//        ((AlifTupleObject*)tuple)->items_, nargs,
//        kwArgs, &kwNames);
//    if (args == nullptr) {
//        return nullptr;
//    }
//    AlifObject* result = func(callable, args,
//        nargs | ALIFVECTORCALL_ARGUMENTS_OFFSET, kwNames);
//    //alifStack_unpackDict_free(args, nargs, kwNames);
//
//    return result;
//}
//
//AlifObject* alifVectorCall_call(AlifObject* callable, AlifObject* tuple, AlifObject* kwArgs)
//{
//
//    int64_t offset = callable->type_->vectorCallOffset;
//    if (offset <= 0) {
//        //_PyErr_Format(tstate, PyExc_TypeError,
//        //    "'%.200s' object does not support vectorcall",
//        //    Py_TYPE(callable)->tp_name);
//        return nullptr;
//    }
//
//    VectorCallFunc func{};
//    memcpy(&func, (char*)callable + offset, sizeof(func));
//    if (func == nullptr) {
//        //_PyErr_Format(tstate, PyExc_TypeError,
//        //    "'%.200s' object does not support vectorcall",
//        //    Py_TYPE(callable)->tp_name);
//        return nullptr;
//    }
//
//    return alifVectorCall_callSub(func, callable, tuple, kwArgs);
//}
//
//AlifObject* const* alifStack_unpackDict(AlifObject* const* args, int64_t nArgs,
//    AlifObject* kwArgs, AlifObject** p_kwnames)
//{
//
//    int64_t nKwArgs = ((AlifDictObject*)kwArgs)->used;
//
//    int64_t maxnargs = INT64_MAX / sizeof(args[0]) - 1;
//    if (nArgs > maxnargs - nKwArgs) {
//        return nullptr;
//    }
//
//    AlifObject** stack = (AlifObject**)alifMem_objAlloc((1 + nArgs + nKwArgs) * sizeof(args[0]));
//    if (stack == nullptr) {
//        return nullptr;
//    }
//
//    AlifObject* kwNames = alifNew_tuple(nKwArgs);
//    if (kwNames == nullptr) {
//        alifMem_objFree(stack);
//        return nullptr;
//    }
//
//    stack++; 
//
//    for (int64_t i = 0; i < nArgs; i++) {
//        stack[i] = args[i];
//    }
//
//    AlifObject** kwStack = stack + nArgs;
//
//	AlifSizeT pos = 0, i = 0;
//    AlifObject* key{}, * value{};
//    unsigned long keys_are_strings = ALIFTPFLAGS_USTR_SUBCLASS;
//    while (alifDict_next(kwArgs, &pos, &key, &value)) {
//        keys_are_strings &= key->type_->flags_;
//        ((AlifTupleObject*)kwNames)->items_[i] = key;
//        kwStack[i] = value;
//        i++;
//    }
//
//    //if (!keys_are_strings) {
//    //    _PyErr_SetString(tstate, PyExc_TypeError,
//    //        "keywords must be strings");
//    //    _PyStack_UnpackDict_Free(stack, nargs, kwNames);
//    //    return nullptr;
//    //}
//
//    *p_kwnames = kwNames;
//    return stack;
//}
//
//void alifStack_unpackDict_free(AlifObject* const* stack, int64_t nArgs,
//    AlifObject* kwNames)
//{
//    int64_t n = ((AlifTupleObject*)kwNames)->_base_.size_ +nArgs;
//    //for (int64_t i = 0; i < n; i++) {
//    //    ALIF_DECREF(stack[i]);
//    //}
//    //alifStack_unpackDict_freeNoDecRef(stack, kwNames);
//}
//
//void alifStack_unpackDict_freeNoDecRef(AlifObject* const* stack, AlifObject* kwNames)
//{
//    alifMem_objFree((AlifObject**)stack - 1);
//    //ALIF_DECREF(kwNames);
//}
