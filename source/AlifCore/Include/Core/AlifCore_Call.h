#pragma once




//AlifObject* alif_checkFunctionResult(AlifThread*, AlifObject*, AlifObject*, const wchar_t*); 
//
//
//AlifObject* alifObject_makeTpCall(AlifThread*, AlifObject*, AlifObject* const*, AlifSizeT, AlifObject*);


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



static inline AlifObject* alifObject_vectorCallThread(AlifThread* _tstate, AlifObject* _callable,
	AlifObject* const* _args, AlifUSizeT _nArgsF,
	AlifObject* _kWNames) { 
	VectorCallFunc func{};
	AlifObject* res_{};

	func = _alifVectorCall_functionInline(_callable);
	if (func == nullptr) {
		AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nArgsF);
		return alifObject_makeTpCall(_tstate, _callable, _args, nargs, _kWNames);
	}
	res_ = func(_callable, _args, _nArgsF, _kWNames);
	return alif_checkFunctionResult(_tstate, _callable, res_, nullptr);
}
//
//
//AlifObject* const* alifStack_unpackDict(AlifObject* const* args, int64_t nArgs, AlifObject* kwArgs, AlifObject** p_kwnames);
