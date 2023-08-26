#pragma once






#ifndef POSIX_THREADS



# ifdef HAVE_PTHREAD_H
#  include <pthread.h> 
# endif
#endif

#ifdef POSIX_THREADS



#define ALIF_HAVE_CONDVAR

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#define ALIFMUTEXT PthreadMutexT
#define ALIFCONDT PthreadCondT

#elif defined(NT_THREADS)






#define ALIF_HAVE_CONDVAR


#define WIN32_LEANAND_MEAN
#include <windows.h>





#ifndef ALIF_EMULATED_WINCV
#define ALIF_EMULATED_WINCV 1  
#endif


#if !defined NTDDI_VISTA || NTDDI_VERSION < NTDDI_VISTA
#undef ALIF_EMULATED_WINCV
#define ALIF_EMULATED_WINCV 1
#endif

#if ALIF_EMULATED_WINCV

typedef CRITICAL_SECTION AlifMutexT;


















class AlifCondT{
public:
	HANDLE sem;
	int waiting;
};

#else /* !ALIF_EMULATED_WINCV */





typedef SRWLOCK AlifMutexT;

typedef CONDITION_VARIABLE  AlifCondT;

#endif 

#endif 

