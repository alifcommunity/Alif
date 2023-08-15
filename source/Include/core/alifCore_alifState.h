#pragma once

#include "alifCore_runtime.h"

// سيتم تجهيز هذا الملف ل تهيئة الدوال و وتهيئة ال API الخاص بدوال ملف alifState.c































































ALIFAPI_FUNC(AlifThreadState*) alifThreadState_getCurrent();









static inline AlifThreadState* alifThreadState_get()
{
#if defined(HAVE_THREAD_LOCAL) && !defined(ALIF_BUILD_CORE_MODULE)
	return alifTssTstate;
#else
	return alifThreadState_getCurrent();
#endif
}


static inline void alif_ensureFuncTstateNotNULL(const char* _func, AlifThreadState* _tstate)
{
	if (_tstate == nullptr) {
		alif_fatalErrorFunc(_func, "يجب استدعاء الدالة مع إمساك قفل جيل");
	}
}


#define ALIF_ENSURETSTATENOTNULL(tstate) alif_ensureFuncTstateNotNULL(__func__, (tstate))











static inline AlifInterpreterState* alifInterpreterState_get()
{
	AlifThreadState* tstate = alifThreadState_get();
#ifdef ALIF_DEBUG
	ALIF_ENSURETSTATENOTNULL(tstate);
#endif
	return tstate->interp;
}
