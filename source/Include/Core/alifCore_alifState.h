#pragma once









#include "alifCore_runtime.h"   












static inline AlifInterpreterState* alifInterpreter_state_main()
{
	return alifRuntime.alifInterpreters.main;
}


static inline int alifIsMainInterpreter(AlifInterpreterState* _interp)
{
	return (_interp == alifInterpreter_state_main());
}


static inline int alifisMainInterpreterFinalizing(AlifInterpreterState* _interp)
{
	return (alifRuntimeState_getFinalizing(_interp->runtime) != nullptr &&
		_interp == &_interp->runtime->mainInterpreter);
}

























#if defined(HAVE_THREAD_LOCAL) && !defined(ALIFBUILD_CORE_MODULE)
extern ALIF_THREAD_LOCAL AlifThreadState* alifTssTState;
#endif










static inline AlifThreadState* alifThreadState_get()
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
    //alifThread_acquireLock((runtime)->alifInterpreters.mutex, WAIT_LOCK)
#define HEAD_UNLOCK(runtime) \
    alifThread_releaseLock((runtime)->alifInterpreters.mutex)
