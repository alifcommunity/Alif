#pragma once

//#include "AlifCore_FreeList.h"
#include "AlifCore_Runtime.h"
#include "AlifCore_ThreadState.h"


// 40
#define ALIF_THREAD_DETACHED     0
#define ALIF_THREAD_ATTACHED     1
#define ALIF_THREAD_SUSPENDED    2


static inline AlifIntT alif_isMainThread(void) { // 47
	unsigned long thread = alifThread_getThreadID();
	return (thread == _alifRuntime_.mainThreadID);
}


static inline AlifInterpreter* _alifInterpreterState_main(void) { // 55
	return _alifRuntime_.interpreters.main;
}

static inline AlifIntT alif_isMainInterpreter(AlifInterpreter* _interpreter) { // 61
	return (_interpreter == _alifInterpreterState_main());
}


ALIFAPI_FUNC(AlifIntT) _alifInterpreter_setRunningMain(AlifInterpreter*); // 82
ALIFAPI_FUNC(void) _alifInterpreter_setNotRunningMain(AlifInterpreter*); // 83
ALIFAPI_FUNC(void) _alifErr_setInterpreterAlreadyRunning(void); // 85


extern AlifIntT _alifThread_isRunningMain(AlifThread*); // 87

static inline AlifIntT alif_threadCanHandleSignals(AlifInterpreter* _interp) { // 103
	return (alif_isMainThread() and alif_isMainInterpreter(_interp));
}


// 113
#if defined(HAVE_LOCAL_THREAD)
extern ALIF_LOCAL_THREAD AlifThread* _alifTSSThread_;
#endif

AlifIntT alifThreadState_mustExit(AlifThread*); // 121

ALIFAPI_FUNC(AlifThread*) alifThread_getCurrent(); // 125

static inline AlifThread* _alifThread_get() { // 134
#ifdef HAVE_LOCAL_THREAD
	return _alifTSSThread_;
#else
	return alifThread_getCurrent();
#endif // HAVE_LOCAL_THREAD
}



static inline AlifIntT _alifThreadState_isAttached(AlifThread* _thread) { // 144
	return (alifAtomic_loadIntRelaxed(&_thread->state) == ALIF_THREAD_ATTACHED);
}


extern void _alifThread_attach(AlifThread*); // 157

extern void _alifThread_detach(AlifThread*); // 163


extern void _alifEval_stopTheWorld(AlifInterpreter*); // 179
extern void _alifEval_startTheWorld(AlifInterpreter*); // 180



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


ALIFAPI_FUNC(AlifThread*) _alifThreadState_new(AlifInterpreter*); // 219
extern void alifThread_bind(AlifThread*); // 222

ALIFAPI_FUNC(AlifThread*) _alifThreadState_newBound(AlifInterpreter*); // 229

ALIFAPI_FUNC(AlifObject*) _alifThreadState_getDict(AlifThread*); // 231

extern AlifStatus alifInterpreter_enable(AlifRuntime*); // 245


// 261
#define HEAD_LOCK(_runtime) \
    alifMutex_lockFlags(&(_runtime)->interpreters.mutex, AlifLockFlags_::Alif_Lock_Dont_Detach)
#define HEAD_UNLOCK(_runtime) \
    ALIFMUTEX_UNLOCK(&(_runtime)->interpreters.mutex)

ALIFAPI_FUNC(const AlifConfig*) _alif_getConfig(); // 269
