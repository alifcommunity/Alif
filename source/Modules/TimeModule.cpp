#include "alif.h"

#include "AlifCore_ModuleObject.h"
#include "AlifCore_DureRun.h"
#include "AlifCore_Time.h"



#include <time.h>                 // clock()
#ifdef HAVE_SYS_TIMES_H
#  include <sys/times.h>          // times()
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif



static AlifIntT alif_sleep(AlifTimeT); // 74


class TimeModuleState { // 77
public:
	AlifTypeObject* structTimeType{};
};


static inline TimeModuleState* get_timeState(AlifObject* module) { // 90
	void* state = _alifModule_getState(module);
	return (TimeModuleState*)state;
}


static AlifObject* time_sleep(AlifObject* _self, AlifObject* _timeoutObj) { // 391
	//if (alifSys_audit("time.sleep", "O", timeout_obj) < 0) {
	//	return nullptr;
	//}

	AlifTimeT timeout{};
	if (_alifTime_fromSecondsObject(&timeout, _timeoutObj,
		AlifTimeRoundT::AlifTime_Round_TIMEOUT))
		return nullptr;
	if (timeout < 0) {
		//alifErr_setString(_alifExcValueError_,
		//	"sleep length must be non-negative");
		return nullptr;
	}
	if (alif_sleep(timeout) != 0) {
		return nullptr;
	}

	return ALIF_NONE;
}



static AlifStructSequenceField _structTimeTypeFields_[] = { // 418
	{0}
};

static AlifStructSequenceDesc _structTimeTypeDesc_ = { // 433
	"time.struct_time",
	"",
	_structTimeTypeFields_,
	0,
};

#if defined(_WINDOWS)
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

static DWORD _timerFlags_ = (DWORD)-1;
#endif









static AlifIntT time_exec(AlifObject* module) { // 1942
	TimeModuleState* state = get_timeState(module);
	// struct_time type
	state->structTimeType = alifStructSequence_newType(&_structTimeTypeDesc_);
	if (state->structTimeType == nullptr) {
		return -1;
	}
	if (alifModule_addType(module, state->structTimeType)) {
		return -1;
	}

#if defined(__linux__) && !defined(__GLIBC__)
	struct tm tm;
	const time_t zero = 0;
	if (gmtime_r(&zero, &tm) != NULL)
		utc_string = tm.tm_zone;
#endif

#if defined(_WINDOWS)
	if (_timerFlags_ == (DWORD)-1) {
		DWORD test_flags = CREATE_WAITABLE_TIMER_HIGH_RESOLUTION;
		HANDLE timer = CreateWaitableTimerExW(NULL, NULL, test_flags,
			TIMER_ALL_ACCESS);
		if (timer == NULL) {
			// CREATE_WAITABLE_TIMER_HIGH_RESOLUTION is not supported.
			_timerFlags_ = 0;
		}
		else {
			// CREATE_WAITABLE_TIMER_HIGH_RESOLUTION is supported.
			_timerFlags_ = CREATE_WAITABLE_TIMER_HIGH_RESOLUTION;
			CloseHandle(timer);
		}
	}
#endif

	return 0;
}

static AlifMethodDef _timeMethods_[] = {
	{"غفوة", time_sleep, METHOD_O},
	{nullptr, nullptr}           /* sentinel */
};

static AlifModuleDefSlot _timeSlots_[] = {
	{ALIF_MOD_EXEC, (void*)time_exec},
	{ALIF_MOD_MULTIPLE_INTERPRETERS, ALIF_MOD_PER_INTERPRETER_GIL_SUPPORTED},
	{ALIF_MOD_GIL, ALIF_MOD_GIL_NOT_USED},
	{0, nullptr}
};

static AlifModuleDef _timeModule_ = {
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "الزمن",
	//.size = sizeof(TimeModuleState),
	.methods = _timeMethods_,
	.slots = _timeSlots_,
	//.traverse = time_module_traverse,
	//.clear = time_module_clear,
	//.free = time_module_free,
};



AlifObject* alifInit_time(void) {
	return alifModuleDef_init(&_timeModule_);
}





static AlifIntT alif_sleep(AlifTimeT timeout) { // 2181
#ifndef _WINDOWS
#ifdef HAVE_CLOCK_NANOSLEEP
	struct timespec timeout_abs {};
#elif defined(HAVE_NANOSLEEP)
	struct timespec timeout_ts {};
#else
	struct timeval timeout_tv {};
#endif
	AlifTimeT deadline{}, monotonic{};
	AlifIntT err = 0;

	if (alifTime_monotonic(&monotonic) < 0) {
		return -1;
	}
	deadline = monotonic + timeout;
#ifdef HAVE_CLOCK_NANOSLEEP
	if (_alifTime_asTimeSpec(deadline, &timeout_abs) < 0) {
		return -1;
	}
#endif

	do {
#ifdef HAVE_CLOCK_NANOSLEEP
		// use timeout_abs
#elif defined(HAVE_NANOSLEEP)
		if (_alifTime_asTimeSpec(timeout, &timeout_ts) < 0) {
			return -1;
		}
#else
		if (_alifTime_asTimEval(timeout, &timeout_tv, AlifTimeRoundT::AlifTime_Round_CEILING) < 0) {
			return -1;
		}
#endif

		AlifIntT ret{};
		ALIF_BEGIN_ALLOW_THREADS
#ifdef HAVE_CLOCK_NANOSLEEP
			ret = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &timeout_abs, nullptr);
		err = ret;
#elif defined(HAVE_NANOSLEEP)
			ret = nanosleep(&timeout_ts, nullptr);
		err = errno;
#else
			ret = select(0, (fd_set*)0, (fd_set*)0, (fd_set*)0, &timeout_tv);
		err = errno;
#endif
		ALIF_END_ALLOW_THREADS

			if (ret == 0) {
				break;
			}

		if (err != EINTR) {
			errno = err;
			//alifErr_setFromErrno(_alifExcOSError_);
			return -1;
		}

		/* sleep was interrupted by SIGINT */
		//if (alifErr_checkSignals()) {
		//	return -1;
		//}

#ifndef HAVE_CLOCK_NANOSLEEP
		if (alifTime_monotonic(&monotonic) < 0) {
			return -1;
		}
		timeout = deadline - monotonic;
		if (timeout < 0) {
			break;
		}
		/* retry with the recomputed delay */
#endif
	} while (1);

	return 0;
#else  // _WINDOWS
	AlifTimeT timeout_100ns = _alifTime_as100Nanoseconds(timeout,
		AlifTimeRoundT::AlifTime_Round_CEILING);

	// Maintain Windows Sleep() semantics for time.sleep(0)
	if (timeout_100ns == 0) {
		ALIF_BEGIN_ALLOW_THREADS
			// A value of zero causes the thread to relinquish the remainder of its
			// time slice to any other thread that is ready to run. If there are no
			// other threads ready to run, the function returns immediately, and
			// the thread continues execution.
			Sleep(0);
		ALIF_END_ALLOW_THREADS
			return 0;
	}

	LARGE_INTEGER relative_timeout{};
	// SetWaitableTimer(): a negative due time indicates relative time
	relative_timeout.QuadPart = -timeout_100ns;

	HANDLE timer = CreateWaitableTimerExW(nullptr, nullptr, _timerFlags_,
		TIMER_ALL_ACCESS);
	if (timer == nullptr) {
		//alifErr_setFromWindowsErr(0);
		return -1;
	}

	if (!SetWaitableTimerEx(timer, &relative_timeout,
		0, // no period; the timer is signaled once
		nullptr, nullptr, // no completion routine
		nullptr,  // no wake context; do not resume from suspend
		0)) // no tolerable delay for timer coalescing
	{
		//alifErr_setFromWindowsErr(0);
		goto error;
	}

	// Only the main thread can be interrupted by SIGINT.
	// Signal handlers are only executed in the main thread.
	if (_alifOS_isMainThread()) {
		HANDLE sigint_event = _alifOS_sigintEvent();

		while (1) {
			// Check for pending SIGINT signal before resetting the event
			//if (alifErr_checkSignals()) {
			//	goto error;
			//}
			ResetEvent(sigint_event);

			HANDLE events[] = { timer, sigint_event };
			DWORD rc;

			ALIF_BEGIN_ALLOW_THREADS
				rc = WaitForMultipleObjects(ALIF_ARRAY_LENGTH(events), events,
					// bWaitAll
					FALSE,
					// No wait timeout
					INFINITE);
			ALIF_END_ALLOW_THREADS

				if (rc == WAIT_FAILED) {
					//alifErr_setFromWindowsErr(0);
					goto error;
				}

			if (rc == WAIT_OBJECT_0) {
				// Timer signaled: we are done
				break;
			}
		}
	}
	else {
		DWORD rc;
	
		ALIF_BEGIN_ALLOW_THREADS
		rc = WaitForSingleObject(timer, INFINITE);
		ALIF_END_ALLOW_THREADS
	
			if (rc == WAIT_FAILED) {
				//alifErr_setFromWindowsErr(0);
				goto error;
			}
	}

	CloseHandle(timer);
	return 0;

error:
	CloseHandle(timer);
	return -1;
#endif
}
