#include "alif.h"

//#include "AlifCore_Thread.h"



#if defined(USE_PTHREADS)
#   if defined(__EMSCRIPTEN__) and !defined(__EMSCRIPTEN_PTHREADS__)
#     define ALIFTHREAD_NAME "pthread-stubs"
#   else
#     define ALIFTHREAD_NAME "pthread"
#   endif
#   include "ThreadPThread.h"
#elif defined(NT_THREADS)
#   define ALIFTHREAD_NAME "nt"
#   include "ThreadNT.h"
#endif
