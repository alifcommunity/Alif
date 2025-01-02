#pragma once



#include "AlifCore_GC.h"




extern void alifObject_setDeferredRefcount(AlifObject*);

static inline AlifIntT _alifObject_hasDeferredRefCount(AlifObject* _op) { // 20
	return alifObject_hasGCBits(_op, ALIFGC_BITS_DEFERRED);
}
