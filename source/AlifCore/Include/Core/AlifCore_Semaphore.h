#pragma once





#if (defined(_POSIX_SEMAPHORES) and (_POSIX_SEMAPHORES+0) != -1 and \
        defined(HAVE_SEM_TIMEDWAIT))
#   define ALIF_USE_SEMAPHORES
#   include <semaphore.h>
#endif


class AlifSemaphore { // 34
public:
#if defined(_WINDOWS)
	HANDLE platformSem{};
#elif defined(ALIF_USE_SEMAPHORES)
	sem_t platformSem{};
#else
	pthread_mutex_t mutex{};
	pthread_cond_t cond{};
	AlifIntT counter{};
#endif
};

AlifIntT alifSemaphore_wait(AlifSemaphore*, AlifTimeT, AlifIntT); // 50

void alifSemaphore_wakeup(AlifSemaphore*); // 54

void alifSemaphore_init(AlifSemaphore*); // 58
void alifSemaphore_destroy(AlifSemaphore*); // 59
