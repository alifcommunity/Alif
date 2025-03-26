#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"








static AlifObject* method_vectorCall(AlifObject* method, AlifObject* const* args,
	AlifUSizeT nargsf, AlifObject* kwnames) { // 43

	AlifThread* tstate = _alifThread_get();
	AlifObject* self = ALIFMETHOD_GET_SELF(method);
	AlifObject* func = ALIFMETHOD_GET_FUNCTION(method);
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);

	AlifObject* result{};
	if (nargsf & ALIF_VECTORCALL_ARGUMENTS_OFFSET) {
		AlifObject** newargs = (AlifObject**)args - 1;
		nargs += 1;
		AlifObject* tmp = newargs[0];
		newargs[0] = self;
		result = alifObject_vectorCallThread(tstate, func, newargs,
			nargs, kwnames);
		newargs[0] = tmp;
	}
	else {
		AlifSizeT nkwargs = (kwnames == nullptr) ? 0 : ALIFTUPLE_GET_SIZE(kwnames);
		AlifSizeT totalargs = nargs + nkwargs;
		if (totalargs == 0) {
			return alifObject_vectorCallThread(tstate, func, &self, 1, nullptr);
		}

		AlifObject* newargs_stack[ALIF_FASTCALL_SMALL_STACK];
		AlifObject** newargs;
		if (totalargs <= (AlifSizeT)ALIF_ARRAY_LENGTH(newargs_stack) - 1) {
			newargs = newargs_stack;
		}
		else {
			newargs = (AlifObject**)alifMem_dataAlloc((totalargs + 1) * sizeof(AlifObject*));
			if (newargs == nullptr) {
				//_alifErr_noMemory(tstate);
				return nullptr;
			}
		}
		newargs[0] = self;
		memcpy(newargs + 1, args, totalargs * sizeof(AlifObject*));
		result = alifObject_vectorCallThread(tstate, func,
			newargs, nargs + 1, kwnames);
		if (newargs != newargs_stack) {
			alifMem_dataFree(newargs);
		}
	}
	return result;
}






AlifObject* alifMethod_new(AlifObject* _func, AlifObject* _self) { // 108
	if (_self == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	AlifMethodObject* im = ALIFOBJECT_GC_NEW(AlifMethodObject, &_alifMethodType_);
	if (im == nullptr) {
		return nullptr;
	}
	im->weakRefList = nullptr;
	im->func = ALIF_NEWREF(_func);
	im->self = ALIF_NEWREF(_self);
	im->vectorCall = method_vectorCall;
	ALIFOBJECT_GC_TRACK(im);
	return (AlifObject*)im;
}






AlifTypeObject _alifMethodType_ = { // 340
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "صفة",
	.basicSize = sizeof(AlifMethodObject),
	//.dealloc = (Destructor)method_dealloc,
	.vectorCallOffset = offsetof(AlifMethodObject, vectorCall),
	//.repr = (ReprFunc)method_repr,
	//.hash = (HashFunc)method_hash,
	.call = alifVectorCall_call,
	//.getAttro = method_getAttro,
	.setAttro = alifObject_genericSetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
				ALIF_TPFLAGS_HAVE_VECTORCALL,
	//.traverse = (TraverseProc)method_traverse,
	//.richCompare = method_richCompare,
	.weakListOffset = offsetof(AlifMethodObject, weakRefList),
	//.methods = method_methods,
	//.members = method_memberlist,
	//.getSet = method_getSet,
	//.descrGet = method_descrGet,
	//.new_ = method_new,
};
