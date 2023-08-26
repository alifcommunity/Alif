#pragma once



#define ALIFTHREAD_INVALID_THREADID ((unsigned long)-1)








#ifdef HAVE_PTHREAD_H

#   include <pthread.h>
#   define NATIVE_TSS_KEYT     PthreadKeyT
#elif defined(NT_THREADS)



#   define NATIVE_TSS_KEYT     unsigned long
#elif defined(HAVE_PTHREAD_STUBS)
#   include "AlifCpp/pthread_stubs.h"
#   define NATIVE_TSS_KEYT     PthreadKeyT


#endif




class AlifTssT {
public:
	int isInitialized;
	NATIVE_TSS_KEYT key;
};

#undef NATIVE_TSS_KEY_T


#define Py_tss_NEEDS_INIT   {0}
