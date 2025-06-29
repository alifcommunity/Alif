#pragma once



#include <signal.h>               // NSIG


 // 24
#ifdef _SIG_MAXSIG
#  define ALIF_NSIG _SIG_MAXSIG
#elif defined(NSIG)
#  define ALIF_NSIG NSIG
#elif defined(_NSIG)
#  define ALIF_NSIG _NSIG            // BSD/SysV
#elif defined(_SIGMAX)
#  define ALIF_NSIG (_SIGMAX + 1)    // QNX
#elif defined(SIGMAX)
#  define ALIF_NSIG (SIGMAX + 1)     // djgpp
#else
#  define ALIF_NSIG 64               // Use a reasonable default value
#endif

#define INVALID_FD (-1)


class SignalsDureRunState { // 39
public:
	class {
	public:
		AlifIntT tripped;
		AlifObject* func;
	} handlers[ALIF_NSIG]{};

	volatile struct {
#ifdef _WINDOWS
		volatile int fd{};
#elif defined(__VXWORKS__)
		AlifIntT fd{};
#else
		sig_atomic_t fd{};
#endif

		AlifIntT warnOnFullBuffer{};
#ifdef _WINDOWS
		AlifIntT useSend{};
#endif
	} wakeup;

	AlifIntT isTripped{};

	/* These objects necessarily belong to the main interpreter. */
	AlifObject* defaultHandler{};
	AlifObject* ignoreHandler{};

#ifdef _WINDOWS
	void* sigintEvent{};
#endif

	AlifIntT unhandledKeyboardInterrupt{};
};

#ifdef _WINDOWS
# define SIGNALS_WAKEUP_INIT \
    {.fd = INVALID_FD, .warnOnFullBuffer = 1, .useSend = 0}
#else
# define SIGNALS_WAKEUP_INIT \
    {.fd = INVALID_FD, .warnOnFullBuffer = 1}
#endif

#define SIGNALS_RUNTIME_INIT \
    { \
        .wakeup = SIGNALS_WAKEUP_INIT, \
    }



AlifIntT _alifOS_isMainThread(void); // 97


void* _alifOS_sigintEvent(); // 102

