#pragma once


#include "AlifCore_State.h"


#define ALIF_FASTCALL_SMALL_STACK 5 // 22


AlifObject* _alif_checkFunctionResult(AlifThread*, AlifObject*, AlifObject*, const wchar_t*); // 27 


extern AlifObject* _alifObject_callPrepend(AlifThread*, AlifObject*,
	AlifObject*, AlifObject*, AlifObject*); // 33


extern AlifObject* _alifObject_call(AlifThread*, AlifObject*, AlifObject*, AlifObject*); // 47


AlifObject* _alifObject_callMethod(AlifObject*, AlifObject*, const char*, ...); // 60


AlifObject* alifObject_makeTpCall(AlifThread*, AlifObject*, AlifObject* const*, AlifSizeT, AlifObject*); // 106


static inline VectorCallFunc _alifVectorCall_functionInline(AlifObject* callable) { // 113 

	AlifTypeObject* tp = ALIF_TYPE(callable);
	if (!alifType_hasFeature(tp, ALIF_TPFLAGS_HAVE_VECTORCALL)) {
		return nullptr;
	}

	AlifSizeT offset = tp->vectorCallOffset;

	VectorCallFunc ptr{};
	memcpy(&ptr, (char*)callable + offset, sizeof(ptr));
	return ptr;
}



static inline AlifObject* alifObject_vectorCallThread(AlifThread* _thread,
	AlifObject* _callable, AlifObject* const* _args,
	AlifUSizeT _nArgsF, AlifObject* _kWNames) {  // 151
	VectorCallFunc func{};
	AlifObject* res_{};

	func = _alifVectorCall_functionInline(_callable);
	if (func == nullptr) {
		AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nArgsF);
		return alifObject_makeTpCall(_thread, _callable, _args, nargs, _kWNames);
	}
	res_ = func(_callable, _args, _nArgsF, _kWNames);
	return _alif_checkFunctionResult(_thread, _callable, res_, nullptr);
}

static inline AlifObject* _alifObject_callNoArgsThread(AlifThread* _thread, AlifObject* _func) { // 172
	return alifObject_vectorCallThread(_thread, _func, nullptr, 0, nullptr);
}

static inline AlifObject* _alifObject_callNoArgs(AlifObject* _func) { // 179
	AlifThread* thread = _alifThread_get();
	return alifObject_vectorCallThread(thread, _func, nullptr, 0, nullptr);
}


extern AlifObject* const* _alifStack_unpackDict(AlifThread*,
	AlifObject* const*, AlifSizeT, AlifObject*, AlifObject**); // 187

extern void _alifStack_unpackDictFree(AlifObject* const*, AlifSizeT, AlifObject*); // 192

extern void _alifStack_unpackDictFreeNoDecRef(AlifObject* const*, AlifObject*); // 197



