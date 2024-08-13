#include "alif.h"






// 55
#if defined(USE_PTHREADS)
#     define ALIFTHREAD_NAME "pthread"
#   include "ThreadPThread.h"
#elif defined(NT_THREADS)
#   define ALIFTHREAD_NAME "nt"
#   include "ThreadNT.h"
#endif
