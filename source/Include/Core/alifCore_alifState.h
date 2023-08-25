#pragma once









#include "alifCore_runtime.h"   












static inline AlifInterpreterState* _alifInterpreterState_main(void)
{
	return alifRuntime.alifInterpreters.main;
}


static inline int alifIsMainInterpreter(AlifInterpreterState* interp)
{
	return (interp == _alifInterpreterState_main());
}


static inline int alifisMainInterpreterFinalizing(AlifInterpreterState* interp)
{
	return (alifRuntimeState_getFinalizing(interp->runtime) != NULL &&
		interp == &interp->runtime->mainInterpreter);
}
























#ifdef HAVE_THREAD_LOCAL && !defined(ALIFBUILD_CORE_MODULE)
extern ALIF_THREAD_LOCAL AlifThreadState* alifTssTState;
#endif










static inline AlifThreadState* alifThreadState_get(void)
{
#if defined(HAVE_THREAD_LOCAL) && !defined(ALIFBUILD_CORE_MODULE)
	return alifTssTState;
#else
	return alifThreadState_getCurrent();
#endif
}



static inline void alifEnsureFuncTstateNotNULL(const char* func, AlifThreadState* tstate)
{
	if (tstate == nullptr) {


			// error

	}
}



#define ALIFENSURETSTATENOtNULL(tstate) \
    alifEnsureFuncTstateNotNULL(__func__, (tstate))










static inline AlifInterpreterState* alifInterpreterState_get() {
	AlifThreadState* tState = alifThreadState_get();
#ifdef ALIF_DEBUG
	ALIFENSURETSTATENOtNULL(tState);
#endif
	return tState->interp;
}












































#define HEAD_LOCK(runtime) \
    //alifThread_acquire_lock((runtime)->alifInterpreters.mutex, WAIT_LOCK)
#define HEAD_UNLOCK(runtime) \
    alifThread_release_lock((runtime)->alifInterpreters.mutex)
