#pragma once

//#include "AlifCore_FreeList.h"
#include "AlifCore_DureRun.h"
#include "AlifCore_ThreadState.h"


// 40
#define ALIF_THREAD_DETACHED     0
#define ALIF_THREAD_ATTACHED     1
#define ALIF_THREAD_SUSPENDED    2


static inline AlifIntT alif_isMainThread(void) { // 47
	unsigned long thread = alifThread_getThreadID();
	return (thread == _alifDureRun_.mainThreadID);
}


static inline AlifInterpreter* alifInterpreter_main(void) { // 55
	return _alifDureRun_.interpreters.main;
}

static inline AlifIntT alif_isMainInterpreter(AlifInterpreter* _interpreter) { // 61
	return (_interpreter == alifInterpreter_main());
}


static inline AlifIntT alif_threadCanHandleSignals(AlifInterpreter* _interp) { // 103
	return (alif_isMainThread() and alif_isMainInterpreter(_interp));
}


// 113
#if defined(HAVE_LOCAL_THREAD)
extern ALIF_LOCAL_THREAD AlifThread* _alifTSSThread_;
#endif

AlifIntT alifThreadState_mustExit(AlifThread*); // 121

AlifThread* alifThread_getCurrent(); // 125

static inline AlifThread* _alifThread_get() { // 134
#ifdef HAVE_LOCAL_THREAD
	return _alifTSSThread_;
#else
	return alifThread_getCurrent();
#endif // HAVE_LOCAL_THREAD
}

extern void alifThread_attach(AlifThread*); // 151




extern void alifEval_stopTheWorld(AlifInterpreter*); // 179
extern void alifEval_startTheWorld(AlifInterpreter*); // 180



static inline void alif_ensureFuncTstateNotNULL(const char* _func, AlifThread* _tstate) { // 183
	if (_tstate == nullptr) {
		//alif_fatalErrorFunc(func,
		//	"the function must be called with the GIL held, "
		//	"after Alif initialization and before Alif finalization, "
		//	"but the GIL is released (the current Alif thread state is NULL)");
		return;
	}
}


#define ALIF_ENSURETHREADNOTNULL(_thread) \
    alif_ensureFuncTstateNotNULL(__func__, (_thread)) // 195


static inline AlifInterpreter* _alifInterpreter_get() { // 207
	AlifThread* tstate = _alifThread_get();
	return tstate->interpreter;
}


AlifThread* alifThreadState_new(AlifInterpreter*); // 219
extern void alifThread_bind(AlifThread*); // 222



extern AlifIntT alifInterpreter_enable(AlifDureRun*); // 245


// 261
#define HEAD_LOCK(_dureRun) \
    alifMutex_lockFlags(&(_dureRun)->interpreters.mutex, AlifLockFlags_::Alif_Lock_Dont_Detach)
#define HEAD_UNLOCK(_dureRun) \
    ALIFMUTEX_UNLOCK(&(_dureRun)->interpreters.mutex)

const AlifConfig* alif_getConfig(); // 269
