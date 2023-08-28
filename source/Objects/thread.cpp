






#include "alif.h"
#include "alifCore_alifState.h"      
//#include "alifCore_structSeq.h"     
#include "alifCore_alifThread.h"

#ifndef DONT_HAVESTDIO_H
#include <stdio.h>
#endif

#include <stdlib.h>


//static void alifThread__init_thread(); 

#define INITIALIZED alifRuntime.threads.initialized

void alifThread_initThread()
{
	if (INITIALIZED) {
		return;
	}
	INITIALIZED = 1;
	//alifThread__init_thread();
}


#if defined(HAVE_PTHREAD_STUBS)
#   define ALIFTHREAD_NAME "pthread-stubs"
#   include "thread_pthreadStubs.h"
#elif defined(USE_PTHREADS) 
#   if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
#     define ALIFTHREAD_NAME "pthread-stubs"
#   else
#     define ALIFTHREAD_NAME "pthread"
#   endif
#   include "thread_pthread.h"
#elif defined(NT_THREADS)
#   define ALIFTHREAD_NAME "nt"
#   include "thread_nt.h"


#endif


















































int alifThread_tssIsCreated(AlifTssT* _key)
{

	return _key->isInitialized;
}
