#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_Call.h"
#include "AlifCore_Dict.h"
#include "AlifCore_State.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Tuple.h"
#include "AlifCore_Object.h"
#include "AlifCore_Eval.h"



static AlifObject* null_error(AlifThread* _tstate) { // 13
	//if (!alifErr_occurred(_tstate)) {
		//alifErr_setString(_tstate, _alifExcSystemError_,
			//"null argument to internal routine");
	//}
	return nullptr;
}

AlifObject* _alif_checkFunctionResult(AlifThread* _thread,
	AlifObject* _callable, AlifObject* _result, const wchar_t* _where) { // 24

	if (_result == nullptr) {
		//if (!alifErr_occurred(_thread)) {
		//	if (_callable)
		//		alifErr_format(_thread, alifExcSystemError,
		//			"%R returned nullptr without setting an exception", _callable);
		//	else
		//		alifErr_format(_thread, alifExcSystemError,
		//			"%s returned nullptr without setting an exception", _where);
		//	return nullptr;
		//}
	}
	else {
		//if (alifErr_occurred(_thread)) {
		//	ALIF_DECREF(_result);

		//	if (_callable) {
		//		alifErr_formatFromCauseThread(
		//			_thread, alifExcSystemError,
		//			"%R returned a result with an exception set", _callable);
		//	}
		//	else {
		//		alifErr_formatFromCauseTstate(
		//			_thread, alifExcSystemError,
		//			"%s returned a result with an exception set", _where);
		//	}
		//	return nullptr;
		//}
	}
	return _result;
}

AlifObject* _alifObject_vectorCallDictThread(AlifThread* _thread, AlifObject* _callable,
	AlifObject* const* _args, AlifUSizeT _nargsf, AlifObject* _kwargs) { // 110
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);

	VectorCallFunc func = alifVectorCall_function(_callable);
	if (func == nullptr) {
		/* Use call instead */
		return alifObject_makeTpCall(_thread, _callable, _args, nargs, _kwargs);
	}

	AlifObject* res{};
	if (_kwargs == nullptr or ALIFDICT_GET_SIZE(_kwargs) == 0) {
		res = func(_callable, _args, _nargsf, nullptr);
	}
	else {
		AlifObject* kwnames{};
		AlifObject* const* newargs{};
		newargs = _alifStack_unpackDict(_thread,
			_args, nargs,
			_kwargs, &kwnames);
		if (newargs == nullptr) {
			return nullptr;
		}
		res = func(_callable, newargs,
			nargs | ALIF_VECTORCALL_ARGUMENTS_OFFSET, kwnames);
		_alifStack_unpackDictFree(newargs, nargs, kwnames);
	}
	return _alif_checkFunctionResult(_thread, _callable, res, nullptr);
}

AlifObject* alifObject_vectorCallDict(AlifObject* _callable, AlifObject* const* _args,
	AlifUSizeT _nargsf, AlifObject* _kwargs) { // 154
	AlifThread* tstate = _alifThread_get();
	return _alifObject_vectorCallDictThread(tstate, _callable, _args, _nargsf, _kwargs);
}

static void object_isNotCallable(AlifThread* _thread, AlifObject* _callable) { // 163
	if (ALIF_IS_TYPE(_callable, &_alifModuleType_)) {
		AlifObject* name = alifModule_getNameObject(_callable);
		if (name == nullptr) {
			//alifErr_clear(_thread);
			goto basic_type_error;
		}
		AlifObject* attr{};
		AlifIntT res = alifObject_getOptionalAttr(_callable, name, &attr);
		if (res < 0) {
			//alifErr_clear(_thread);
		}
		else if (res > 0 and alifCallable_check(attr)) {
			//alifErr_format(_thread, alifExcTypeError,
			//	"'%.200s' object is not callable. "
			//	"Did you mean: '%U.%U(...)'?",
			//	ALIF_TYPE(_callable)->name_, name, name);
			ALIF_DECREF(attr);
			ALIF_DECREF(name);
			return;
		}
		ALIF_XDECREF(attr);
		ALIF_DECREF(name);
	}
basic_type_error:
	//alifErr_format(_thread, alifExcTypeError, "'%.200s' object is not callable", ALIF_TYPE(_callable)->name_);
	return; //* alif
}

AlifObject* alifObject_makeTpCall(AlifThread* _thread, AlifObject* _callable,
	AlifObject* const* _args, AlifSizeT _nargs, AlifObject* _keywords) { // 200

	TernaryFunc call = ALIF_TYPE(_callable)->call;
	if (call == nullptr) {
		object_isNotCallable(_thread, _callable);
		return nullptr;
	}

	AlifObject* argsTuple = alifTuple_fromArray(_args, _nargs);
	if (argsTuple == nullptr) {
		return nullptr;
	}

	AlifObject* kWDict{};
	if (_keywords == nullptr or ALIFDICT_CHECK(_keywords)) {
		kWDict = _keywords;
	}
	else {
		if (ALIFTUPLE_GET_SIZE(_keywords)) {
			kWDict = alifStack_asDict(_args + _nargs, _keywords);
			if (kWDict == nullptr) {
				ALIF_DECREF(argsTuple);
				return nullptr;
			}
		}
		else {
			_keywords = kWDict = nullptr;
		}
	}

	AlifObject* result = nullptr;
	if (_alif_enterRecursiveCallThread(_thread, " while calling a Alif object") == 0)
	{
		result = ALIFCPPFUNCTIONWITHKEYWORDS_TRAMPOLINECALL((AlifCPPFunctionWithKeywords)call, _callable, argsTuple, kWDict);
		_alif_leaveRecursiveCallThread(_thread);
	}

	ALIF_DECREF(argsTuple);
	if (kWDict != _keywords) {
		ALIF_DECREF(kWDict);
	}

	return _alif_checkFunctionResult(_thread, _callable, result, nullptr);
}



VectorCallFunc alifVectorCall_function(AlifObject* _callable) { // 256
	return _alifVectorCall_functionInline(_callable);
}

static AlifObject* _alifVectorCall_call(AlifThread* tstate, VectorCallFunc func,
	AlifObject* callable, AlifObject* tuple, AlifObject* kwargs) { // 263

	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(tuple);

	/* Fast path for no keywords */
	if (kwargs == nullptr or ALIFDICT_GET_SIZE(kwargs) == 0) {
		return func(callable, ALIFTUPLE_ITEMS(tuple), nargs, nullptr);
	}

	/* Convert arguments & call */
	AlifObject* const* args;
	AlifObject* kwnames{};
	args = _alifStack_unpackDict(tstate,
		ALIFTUPLE_ITEMS(tuple), nargs,
		kwargs, &kwnames);
	if (args == nullptr) {
		return nullptr;
	}
	AlifObject* result = func(callable, args,
		nargs | ALIF_VECTORCALL_ARGUMENTS_OFFSET, kwnames);
	_alifStack_unpackDictFree(args, nargs, kwnames);

	return _alif_checkFunctionResult(tstate, callable, result, nullptr);
}

AlifObject* alifObject_vectorCall(AlifObject* _callable, AlifObject* const* _args,
	AlifUSizeT _nArgsF, AlifObject* _kwNames) { // 322
	AlifThread* thread = _alifThread_get();
	return alifObject_vectorCallThread(thread, _callable,
		_args, _nArgsF, _kwNames);
}

AlifObject* _alifObject_call(AlifThread* _thread, AlifObject* _callable,
	AlifObject* _args, AlifObject* _kwargs) { // 332
	TernaryFunc call{};
	AlifObject* result{};

	VectorCallFunc vector_func = alifVectorCall_function(_callable);
	if (vector_func != nullptr) {
		return _alifVectorCall_call(_thread, vector_func, _callable, _args, _kwargs);
	}
	else {
		call = ALIF_TYPE(_callable)->call;
		if (call == nullptr) {
			object_isNotCallable(_thread, _callable);
			return nullptr;
		}

		if (_alif_enterRecursiveCallThread(_thread, " while calling a Alif object")) {
			return nullptr;
		}

		result = (*call)(_callable, _args, _kwargs);

		_alif_leaveRecursiveCallThread(_thread);

		return _alif_checkFunctionResult(_thread, _callable, result, nullptr);
	}
}

AlifObject* alifObject_call(AlifObject* callable,
	AlifObject* args, AlifObject* kwargs) { // 369
	AlifThread* thread = _alifThread_get();
	return _alifObject_call(thread, callable, args, kwargs);
}

AlifObject* alifObject_callOneArg(AlifObject* _func, AlifObject* _arg) { // 386

	AlifObject* _args[2]{};
	AlifObject** args = _args + 1;
	args[0] = _arg;
	AlifThread* thread = _alifThread_get();
	AlifUSizeT nArgsF = 1 | ALIF_VECTORCALL_ARGUMENTS_OFFSET;
	return alifObject_vectorCallThread(thread, _func, args, nArgsF, nullptr);
}


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


AlifObject* _alifObject_callPrepend(AlifThread* _thread, AlifObject* _callable,
	AlifObject* _obj, AlifObject* _args, AlifObject* _kwargs) { // 477

	AlifObject* small_stack[ALIF_FASTCALL_SMALL_STACK]{};
	AlifObject** stack{};

	AlifSizeT argcount = ALIFTUPLE_GET_SIZE(_args);
	if (argcount + 1 <= (AlifSizeT)ALIF_ARRAY_LENGTH(small_stack)) {
		stack = small_stack;
	}
	else {
		stack = (AlifObject**)alifMem_dataAlloc((argcount + 1) * sizeof(AlifObject*));
		if (stack == nullptr) {
			//alifErr_noMemory();
			return nullptr;
		}
	}

	/* use borrowed references */
	stack[0] = _obj;
	memcpy(&stack[1],
		ALIFTUPLE_ITEMS(_args),
		argcount * sizeof(AlifObject*));

	AlifObject* result = _alifObject_vectorCallDictThread(_thread, _callable,
		stack, argcount + 1,
		_kwargs);
	if (stack != small_stack) {
		alifMem_dataFree(stack);
	}
	return result;
}


static AlifObject* alifObject_callFunctionVa(AlifThread* _thread, AlifObject* _callable,
	const char* _format, va_list _va) { // 517
	AlifObject* smallStack[ALIF_FASTCALL_SMALL_STACK]{};
	const AlifSizeT smallStackLen = ALIF_ARRAY_LENGTH(smallStack);
	AlifObject** stack{};
	AlifSizeT nargs{}, i{};
	AlifObject* result{};

	if (_callable == nullptr) {
		return null_error(_thread);
	}

	if (!_format or !*_format) {
		return _alifObject_callNoArgsThread(_thread, _callable);
	}

	stack = _alif_vaBuildStack(smallStack, smallStackLen,
		_format, _va, &nargs);
	if (stack == nullptr) {
		return nullptr;
	}
	if (nargs == 1 and ALIFTUPLE_CHECK(stack[0])) {
		AlifObject* args = stack[0];
		result = alifObject_vectorCallThread(_thread, _callable,
			ALIFTUPLE_ITEMS(args),
			ALIFTUPLE_GET_SIZE(args),
			nullptr);
	}
	else {
		result = alifObject_vectorCallThread(_thread, _callable,
			stack, nargs, nullptr);
	}

	for (i = 0; i < nargs; ++i) {
		ALIF_DECREF(stack[i]);
	}
	if (stack != smallStack) {
		alifMem_dataFree(stack);
	}
	return result;
}

static AlifObject* call_method(AlifThread* _tstate, AlifObject* _callable, const char* _format, va_list _va) { // 616
	if (!alifCallable_check(_callable)) {
		//alifErr_format(tstate, _alifExcTypeError_,
			//"attribute of type '%.200s' is not callable",
			//ALIF_TYPE(callable)->name);
		return nullptr;
	}

	return alifObject_callFunctionVa(_tstate, _callable, _format, _va);
}

AlifObject* alifObject_callMethod(AlifObject* _obj,
	const char* _name, const char* _format, ...) { // 630
	AlifThread* thread = _alifThread_get();

	if (_obj == nullptr or _name == nullptr) {
		return null_error(thread);
	}

	AlifObject* callable = alifObject_getAttrString(_obj, _name);
	if (callable == nullptr) {
		return nullptr;
	}

	va_list va_{};
	va_start(va_, _format);
	AlifObject* retval = call_method(thread, callable, _format, va_);
	va_end(va_);

	ALIF_DECREF(callable);
	return retval;
}


AlifObject* _alifObject_callMethod(AlifObject* obj, AlifObject* name,
	const char* format, ...) { // 678
	AlifThread* tstate = _alifThread_get();
	if (obj == nullptr or name == nullptr) {
		return null_error(tstate);
	}

	AlifObject* callable = alifObject_getAttr(obj, name);
	if (callable == nullptr) {
		return nullptr;
	}

	va_list va{};
	va_start(va, format);
	AlifObject* retval = call_method(tstate, callable, format, va);
	va_end(va);

	ALIF_DECREF(callable);
	return retval;
}



static AlifObject* object_vacall(AlifThread* _thread, AlifObject* _base,
	AlifObject* _callable, va_list _vargs) { // 765
	AlifObject* small_stack[ALIF_FASTCALL_SMALL_STACK];
	AlifObject** stack{};
	AlifSizeT nargs{};
	AlifObject* result{};
	AlifSizeT i{};
	va_list countva{};

	if (_callable == nullptr) {
		return null_error(_thread);
	}

	/* Count the number of arguments */
	va_copy(countva, _vargs);
	nargs = _base ? 1 : 0;
	while (1) {
		AlifObject* arg = va_arg(countva, AlifObject*);
		if (arg == nullptr) {
			break;
		}
		nargs++;
	}
	va_end(countva);

	/* Copy arguments */
	if (nargs <= (AlifSizeT)ALIF_ARRAY_LENGTH(small_stack)) {
		stack = small_stack;
	}
	else {
		stack = (AlifObject**)alifMem_dataAlloc(nargs * sizeof(stack[0]));
		if (stack == nullptr) {
			//alifErr_noMemory();
			return nullptr;
		}
	}

	i = 0;
	if (_base) {
		stack[i++] = _base;
	}

	for (; i < nargs; ++i) {
		stack[i] = va_arg(_vargs, AlifObject*);
	}

	/* Call the function */
	result = alifObject_vectorCallThread(_thread, _callable, stack, nargs, nullptr);

	if (stack != small_stack) {
		alifMem_dataFree(stack);
	}
	return result;
}



AlifObject* alifObject_vectorCallMethod(AlifObject* _name, AlifObject* const* _args,
	AlifUSizeT _nargsf, AlifObject* _kwnames) { // 828

	AlifThread* thread = _alifThread_get();
	AlifObject* callable = nullptr;

	AlifIntT unbound = _alifObject_getMethod(_args[0], _name, &callable);
	if (callable == nullptr) {
		return nullptr;
	}

	if (unbound) {
		_nargsf &= ~ALIF_VECTORCALL_ARGUMENTS_OFFSET;
	}
	else {
		_args++;
		_nargsf--;
	}
	AlifObject* result = alifObject_vectorCallThread(thread, callable,
		_args, _nargsf, _kwnames);
	ALIF_DECREF(callable);
	return result;
}


AlifObject* alifObject_callMethodObjArgs(AlifObject* _obj, AlifObject* _name, ...) { // 863
	AlifThread* tstate = _alifThread_get();
	if (_obj == nullptr or _name == nullptr) {
		return null_error(tstate);
	}

	AlifObject* callable = nullptr;
	AlifIntT isMethod = _alifObject_getMethod(_obj, _name, &callable);
	if (callable == nullptr) {
		return nullptr;
	}
	_obj = isMethod ? _obj : nullptr;

	va_list vargs;
	va_start(vargs, _name);
	AlifObject* result = object_vacall(tstate, _obj, callable, vargs);
	va_end(vargs);

	ALIF_DECREF(callable);
	return result;
}


AlifObject* alifObject_callFunctionObjArgs(AlifObject* _callable, ...) { // 918
	AlifThread* thread = _alifThread_get();
	va_list vargs{};
	AlifObject* result{};

	va_start(vargs, _callable);
	result = object_vacall(thread, nullptr, _callable, vargs);
	va_end(vargs);

	return result;
}



AlifObject* alifStack_asDict(AlifObject* const* _values, AlifObject* _kwNames) {  // 936
	AlifSizeT nkwargs{};

	nkwargs = ALIFTUPLE_GET_SIZE(_kwNames);
	return _alifDict_fromItems(&ALIFTUPLE_GET_ITEM(_kwNames, 0), 1, _values, 1, nkwargs);
}


AlifObject* const* _alifStack_unpackDict(AlifThread* _thread,
	AlifObject* const* _args, AlifSizeT _nargs,
	AlifObject* _kwargs, AlifObject** _pKwnames) { // 959

	AlifSizeT nkwargs = ALIFDICT_GET_SIZE(_kwargs);
	AlifSizeT maxnargs = ALIF_SIZET_MAX / sizeof(_args[0]) - 1;
	if (_nargs > maxnargs - nkwargs) {
		//_alifErr_noMemory(_thread);
		return nullptr;
	}

	AlifObject** stack = (AlifObject**)alifMem_dataAlloc((1 + _nargs + nkwargs) * sizeof(_args[0]));
	if (stack == nullptr) {
		//_alifErr_noMemory(_thread);
		return nullptr;
	}

	AlifObject* kwnames = alifTuple_new(nkwargs);
	if (kwnames == nullptr) {
		alifMem_dataFree(stack);
		return nullptr;
	}

	stack++;  /* For ALIF_VECTORCALL_ARGUMENTS_OFFSET */

	/* Copy positional arguments */
	for (AlifSizeT i = 0; i < _nargs; i++) {
		stack[i] = ALIF_NEWREF(_args[i]);
	}

	AlifObject** kwstack = stack + _nargs;
	AlifSizeT pos = 0, i = 0;
	AlifObject* key{}, * value{};
	unsigned long keys_are_strings = ALIF_TPFLAGS_UNICODE_SUBCLASS;
	while (alifDict_next(_kwargs, &pos, &key, &value)) {
		keys_are_strings &= ALIF_TYPE(key)->flags;
		ALIFTUPLE_SET_ITEM(kwnames, i, ALIF_NEWREF(key));
		kwstack[i] = ALIF_NEWREF(value);
		i++;
	}

	if (!keys_are_strings) {
		//_alifErr_setString(_thread, _alifExcTypeError_,
		//	"keywords must be strings");
		_alifStack_unpackDictFree(stack, _nargs, kwnames);
		return nullptr;
	}

	*_pKwnames = kwnames;
	return stack;
}

void _alifStack_unpackDictFree(AlifObject* const* _stack,
	AlifSizeT _nArgs, AlifObject* _kwNames) { // 1028
	AlifSizeT n = ALIFTUPLE_GET_SIZE(_kwNames) + _nArgs;
	for (AlifSizeT i = 0; i < n; i++) {
		ALIF_DECREF(_stack[i]);
	}
	_alifStack_unpackDictFreeNoDecRef(_stack, _kwNames);
}

void _alifStack_unpackDictFreeNoDecRef(AlifObject* const* _stack,
	AlifObject* _kwnames) { // 1039
	alifMem_dataFree((AlifObject**)_stack - 1);
	ALIF_DECREF(_kwnames);
}
