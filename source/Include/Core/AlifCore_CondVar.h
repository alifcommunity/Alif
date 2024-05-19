#pragma once

#include "AlifCore_AlifThread.h" 
#define NT_THREADS

#ifdef _POSIX_THREADS

#define ALIF_HAVE_CONDVAR

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>            
#endif

#define AlifMutexT PthreadMutexT
#define AlifCondT PthreadCondT

#elif defined(NT_THREADS)

#define ALIF_HAVE_CONDVAR

#define WIN32_LEAN_AND_MEAN
#include <windows.h>             

#ifndef ALIFSUB_EMULATED_WIN_CV
#define ALIFSUB_EMULATED_WIN_CV 0 
#endif

#if !defined NTDDI_VISTA || NTDDI_VERSION < NTDDI_VISTA
#undef ALIFSUB_EMULATED_WIN_CV
#define ALIFSUB_EMULATED_WIN_CV 1
#endif

#if ALIFSUB_EMULATED_WIN_CV

typedef CRITICAL_SECTION AlifMutexT;

class AlifSubCondT
{
public:
    HANDLE sem_;
    int waiting_; 
} ;

#else

typedef SRWLOCK AlifMutexT;

typedef CONDITION_VARIABLE  AlifCondT;

#endif 
#endif 