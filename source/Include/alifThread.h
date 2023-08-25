#pragma once


typedef void* AlifThreadTypeLock;







enum AlifLockStatus {
	Alif_Lock_Failure = 0,
	Alif_Lock_Acquired = 1,
	Alif_Lock_Intr
};






#if (defined(__APPLE__) || defined(__linux__) || defined(_WIN32) \
     || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
     || defined(__DragonFly__) || defined(_AIX))
#define ALIFHAVE_THREAD_NATIVEID

#endif


ALIFAPI_FUNC(void) alifThread_free_lock(AlifThreadTypeLock);
ALIFAPI_FUNC(int) alifThread_acquire_lock(AlifThreadTypeLock, int);
#define WAIT_LOCK       1
#define NOWAIT_LOCK     0










#define ALIF_TIMEOUT_T long long

#if defined(POSIX_THREADS)


#  define ALIF_TIMEOUT_MAX (LLONG_MAX / 1000)
#elif defined (NT_THREADS)



#  if 0xFFFFFFFELL * 1000 < LLONG_MAX
#    define ALIF_TIMEOUT_MAX (0xFFFFFFFELL * 1000)
#  else
#    define ALIF_TIMEOUT_MAX LLONG_MAX
#  endif
#else
#  define ALIF_TIMEOUT_MAX LLONG_MAX
#endif


















void alifThread_release_lock(AlifThreadTypeLock);
