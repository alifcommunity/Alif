#include "alif.h"

#include "AlifCore_DureRun.h" // should replace "AlifCore_State.h" instade this
#include "AlifCore_Thread.h"






#define INITIALIZED _alifDureRun_.threads.initialized // 43



// 55
#if defined(HAVE_PTHREAD_STUBS)
#   define PYTHREAD_NAME "pthread-stubs"
#   include "ThreadPThreadStubs.h"
#elif defined(USE_PTHREADS)
#     define ALIFTHREAD_NAME "pthread"
#   include "ThreadPThread.h"
#elif defined(NT_THREADS)
#   define ALIFTHREAD_NAME "nt"
#   include "ThreadNT.h"
#endif













AlifIntT alifThreadTSS_isCreated(AlifTssT* _key) { // 204
	return _key->isInitialized;
}
