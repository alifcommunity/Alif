#pragma once




AlifObject* alif_checkFunctionResult(AlifThread*, AlifObject*, AlifObject*, const wchar_t*); // 28


AlifObject* alifObject_makeTpCall(AlifThread*, AlifObject*, AlifObject* const*, AlifSizeT, AlifObject*);


static inline VectorCallFunc alifVectorCall_functionInline(AlifObject* callable) { // 114

	AlifTypeObject* tp = ALIF_TYPE(callable);
	if (!alifType_hasFeature(tp, ALIFTPFLAGS_HAVE_VECTORCALL)) {
		return nullptr;
	}

	AlifSizeT offset = tp->vectorCallOffset;

	VectorCallFunc ptr{};
	memcpy(&ptr, (char*)callable + offset, sizeof(ptr));
	return ptr;
}



static inline AlifObject* alifObject_vectorCallThread(AlifThread* tstate, AlifObject* callable,
	AlifObject* const* args, AlifUSizeT nargsf,
	AlifObject* kwnames) { // 152
	VectorCallFunc func;
	AlifObject* res;

	func = alifVectorCall_functionInline(callable);
	if (func == nullptr) {
		AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);
		return alifObject_makeTpCall(tstate, callable, args, nargs, kwnames);
	}
	res = func(callable, args, nargsf, kwnames);
	return alif_checkFunctionResult(tstate, callable, res, nullptr);
}


AlifObject* const* alifStack_unpackDict(AlifObject* const* args, int64_t nArgs, AlifObject* kwArgs, AlifObject** p_kwnames);
