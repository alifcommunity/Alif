#pragma once

//#include "AlifCore_FreeList.h"
#include "AlifCore_DureRun.h"
#include "AlifCore_ThreadState.h"

// 113
#if defined(HAVE_LOCAL_THREAD)
extern ALIF_LOCAL_THREAD AlifThread* _alifTSSThread_;
#endif

static inline AlifThread* alifThread_get() { // 134
#ifdef HAVE_LOCAL_THREAD
	return _alifTSSThread_;
#else
	return alifThread_getCurrent();
#endif // HAVE_LOCAL_THREAD
}


AlifThread* alifThreadState_new(AlifInterpreter*); // 219
extern void alifThread_bind(AlifThread*); // 222
extern void alifThread_attach(AlifThread*); // temp

//static inline AlifInterpreter* alifInterpreter_get() {
//	AlifThread* thread_ = alifThread_get();
//
//	return thread_->interpreter;
//}


extern AlifIntT alifInterpreter_enable(AlifDureRun*); // 245


//const AlifConfig* alifConfig_get();


//static inline AlifObjectFreeLists* alifObject_freeListsGet() {
//	AlifThread* thread = alifThread_get();
//
//#ifdef ALIF_GIL_DISABLED
//	return &((AlifThreadImpl*)tstate)->freelists;
//#else
//	return &thread->interpreter->objectState.freelists;
//#endif
//}
