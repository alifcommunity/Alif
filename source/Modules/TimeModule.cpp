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


static AlifObject* _alifFloat_fromAlifTime(AlifTimeT _t) { // 99
	double d = alifTime_asSecondsDouble(_t);
	return alifFloat_fromDouble(d);
}

static AlifObject* time_time(AlifObject* _self, AlifObject* _unused) { // 107
	AlifTimeT t{};
	if (alifTime_time(&t) < 0) {
		return nullptr;
	}
	return _alifFloat_fromAlifTime(t);
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
	{"السنة", "year, for example, 1993"},
	{"الشهر", "month of year, range [1, 12]"},
	{"اليوم", "day of month, range [1, 31]"},
	{"الساعة", "hours, range [0, 23]"},
	{"الدقيقة", "minutes, range [0, 59]"},
	{"الثانية", "seconds, range [0, 61])"},
	{"اليوم_من_الاسبوع", "day of week, range [0, 6], Monday is 0"},
	{"اليو_من_السنة", "day of year, range [1, 366]"},
	{"النظام_الصيفي", "1 if summer time is in effect, 0 if not, and -1 if unknown"},
	{"tm_zone", "abbreviation of timezone name"},
	{"tm_gmtoff", "offset from UTC in seconds"},
	{0}
};

static AlifStructSequenceDesc _structTimeTypeDesc_ = { // 433
	"time.struct_time",
	"",
	_structTimeTypeFields_,
	9,
};

 // 446
#if defined(_WINDOWS)
#ifndef CREATE_WAITABLE_TIMER_HIGH_RESOLUTION
#define CREATE_WAITABLE_TIMER_HIGH_RESOLUTION 0x00000002
#endif

static DWORD _timerFlags_ = (DWORD)-1;
#endif


static AlifObject* tmtotuple(TimeModuleState* _state, struct tm* _p
#ifndef HAVE_STRUCT_TM_TM_ZONE
	, const char* _zone, time_t _gmtoff
#endif
) { // 454
	AlifObject* v = alifStructSequence_new(_state->structTimeType);
	if (v == nullptr) return nullptr;

#define SET_ITEM(_index, _call)							\
    do {												\
        AlifObject *obj = (_call);						\
        if (obj == nullptr) {							\
            ALIF_DECREF(v);								\
            return nullptr;								\
        }												\
        ALIFSTRUCTSEQUENCE_SET_ITEM(v, (_index), obj);	\
    } while (0)

#define SET(_index, _val) \
    SET_ITEM((_index), alifLong_fromLong((long)(_val)))

	SET(0, _p->tm_year + 1900);
	SET(1, _p->tm_mon + 1);         /* Want January == 1 */
	SET(2, _p->tm_mday);
	SET(3, _p->tm_hour);
	SET(4, _p->tm_min);
	SET(5, _p->tm_sec);
	SET(6, (_p->tm_wday + 6) % 7); /* Want Monday == 0 */
	SET(7, _p->tm_yday + 1);        /* Want January, 1 == 1 */
	SET(8, _p->tm_isdst);
#ifdef HAVE_STRUCT_TM_TM_ZONE
	SET_ITEM(9, alifUStr_decodeLocale(_p->tm_zone, "surrogateescape"));
	SET(10, _p->tm_gmtoff);
#else
	SET_ITEM(9, alifUStr_decodeLocale(_zone, "surrogateescape"));
	SET_ITEM(10, _alifLong_fromTimeT(_gmtoff));
#endif /* HAVE_STRUCT_TM_TM_ZONE */

#undef SET
#undef SET_ITEM

	return v;
}

static AlifObject* tmtodict(TimeModuleState* _state, struct tm* _p
#ifndef HAVE_STRUCT_TM_TM_ZONE
	, const char* _zone, time_t _gmtoff
#endif
) { // alif
	AlifObject* v = alifDict_new();
	if (v == nullptr) return nullptr;

#define SET_ITEM(_index, _call)							\
    do {												\
        AlifObject *obj = (_call);						\
        if (obj == nullptr) {							\
            ALIF_DECREF(v);								\
            return nullptr;								\
        }												\
        alifDict_setItemString(v, (_index), obj);	\
    } while (0)

#define SET(_index, _val) \
    SET_ITEM((_index), alifLong_fromLong((long)(_val)))

	SET("السنة", _p->tm_year + 1900);
	SET("الشهر", _p->tm_mon + 1);         /* Want January == 1 */
	SET("اليوم", _p->tm_mday);
	SET("الساعة", _p->tm_hour);
	SET("الدقيقة", _p->tm_min);
	SET("الثانية", _p->tm_sec);
	SET("يوم_اسبوع", (_p->tm_wday + 6) % 7); /* Want Monday == 0 */
	SET("يوم_سنة", _p->tm_yday + 1);        /* Want January, 1 == 1 */
	SET("هل_صيفي", _p->tm_isdst);
#ifdef HAVE_STRUCT_TM_TM_ZONE
	SET_ITEM(9, alifUStr_decodeLocale(_p->tm_zone, "surrogateescape"));
	SET(10, _p->tm_gmtoff);
#else
	SET_ITEM("النطاق", alifUStr_decodeLocale(_zone, "surrogateescape"));
	SET_ITEM("الإزاحة", _alifLong_fromTimeT(_gmtoff));
#endif /* HAVE_STRUCT_TM_TM_ZONE */

#undef SET
#undef SET_ITEM

	return v;
}

static AlifIntT parse_timeTArgs(AlifObject* _args, const char* _format, time_t* _pwhen) { // 505
	AlifObject* ot = nullptr;
	time_t whent{};

	if (!alifArg_parseTuple(_args, _format, &ot))
		return 0;
	if (ot == nullptr or ot == ALIF_NONE) {
		whent = time(nullptr);
	}
	else {
		if (_alifTime_objectToTimeT(ot, &whent, AlifTimeRoundT::AlifTime_Round_FLOOR) == -1)
			return 0;
	}
	*_pwhen = whent;
	return 1;
}

#ifndef HAVE_TIMEGM
static time_t timegm(struct tm* p) { // 545
	return p->tm_sec + p->tm_min * 60 + p->tm_hour * 3600 + p->tm_yday * 86400 +
		(p->tm_year - 70) * 31536000 + ((p->tm_year - 69) / 4) * 86400 -
		((p->tm_year - 1) / 100) * 86400 + ((p->tm_year + 299) / 400) * 86400;
}
#endif


static AlifObject* time_localtime(AlifObject* _module, AlifObject* _args) { // 568 //* review
	time_t when{};
	struct tm buf{};

	if (!parse_timeTArgs(_args, "|O:localtime", &when))
		return nullptr;
	if (_alifTime_localtime(when, &buf) != 0)
		return nullptr;

	TimeModuleState* state = get_timeState(_module);
#ifdef HAVE_STRUCT_TM_TM_ZONE
	return tmtotuple(state, &buf);
#else
	{
		struct tm local = buf;
		char zone[100]{};
		time_t gmtoff{};
		strftime(zone, sizeof(zone), "%Z", &buf);
		gmtoff = timegm(&buf) - when;
		return tmtotuple(state, &local, zone, gmtoff);
	}
#endif
}

static AlifObject* time_localtimeDict(AlifObject* _module, AlifObject* _args) { // alif
	time_t when{};
	struct tm buf {};

	if (!parse_timeTArgs(_args, "|O:localtime", &when))
		return nullptr;
	if (_alifTime_localtime(when, &buf) != 0)
		return nullptr;

	TimeModuleState* state = get_timeState(_module);
#ifdef HAVE_STRUCT_TM_TM_ZONE
	return tmtotuple(state, &buf);
#else
	{
		struct tm local = buf;
		char zone[100]{};
		time_t gmtoff{};
		strftime(zone, sizeof(zone), "%Z", &buf);
		gmtoff = timegm(&buf) - when;
		return tmtodict(state, &local, zone, gmtoff);
	}
#endif
}




 // 594
#if defined(__linux__) && !defined(__GLIBC__)
static const char* _utcString_ = nullptr;
#endif



static AlifIntT get_tmArg(TimeModuleState* _state, AlifObject* _args,
	struct tm* _p, const char* _format) { // 608
	AlifIntT y{};

	memset((void*)_p, '\0', sizeof(struct tm));

	if (!ALIFTUPLE_CHECK(_args)) {
		//alifErr_setString(_alifExcTypeError_,
		//	"Tuple or struct_time argument required");
		return 0;
	}

	if (!alifArg_parseTuple(_args, _format,
		&y, &_p->tm_mon, &_p->tm_mday,
		&_p->tm_hour, &_p->tm_min, &_p->tm_sec,
		&_p->tm_wday, &_p->tm_yday, &_p->tm_isdst))
		return 0;

	if (y < INT_MIN + 1900) {
		//alifErr_setString(_alifExcOverflowError_, "السنة المدخلة خارج النطاق");
		return 0;
	}

	_p->tm_year = y - 1900;
	_p->tm_mon--;
	_p->tm_wday = (_p->tm_wday + 1) % 7;
	_p->tm_yday--;
#ifdef HAVE_STRUCT_TM_TM_ZONE
	if (ALIF_IS_TYPE(_args, state->structTimeType)) {
		AlifObject* item{};
		item = ALIFSTRUCTSEQUENCE_GET_ITEM(_args, 9);
		if (item != ALIF_NONE) {
			_p->tm_zone = (char*)alifUStr_asUTF8(item);
			if (_p->tm_zone == nullptr) {
				return 0;
			}
#if defined(__linux__) and !defined(__GLIBC__)
			if (_utcString_ and strcmp(_p->tm_zone, _utcString_) == 0) {
				_p->tm_zone = _utcString_;
			}
			else if (_tzname[0] and strcmp(_p->tm_zone, _tzname[0]) == 0) {
				_p->tm_zone = _tzname[0];
			}
			else if (_tzname[1] and strcmp(_p->tm_zone, _tzname[1]) == 0) {
				_p->tm_zone = _tzname[1];
			}
#endif
		}
		item = ALIFSTRUCTSEQUENCE_GET_ITEM(_args, 10);
		if (item != ALIF_NONE) {
			_p->tm_gmtoff = alifLong_asLong(item);
			if (alifErr_occurred())
				return 0;
		}
	}
#endif /* HAVE_STRUCT_TM_TM_ZONE */
	return 1;
}


static AlifIntT checktm(struct tm* _buf) { // 676
	if (_buf->tm_mon == -1)
		_buf->tm_mon = 0;
	else if (_buf->tm_mon < 0 or _buf->tm_mon > 11) {
		alifErr_setString(_alifExcValueError_, "الشهر المدخل خارج النطاق السنوي");
		return 0;
	}
	if (_buf->tm_mday == 0)
		_buf->tm_mday = 1;
	else if (_buf->tm_mday < 0 or _buf->tm_mday > 31) {
		alifErr_setString(_alifExcValueError_, "اليوم المدخل خارج النطاق الشهري");
		return 0;
	}
	if (_buf->tm_hour < 0 or _buf->tm_hour > 23) {
		alifErr_setString(_alifExcValueError_, "الساعة المدخلة خارج النطاق اليومي");
		return 0;
	}
	if (_buf->tm_min < 0 or _buf->tm_min > 59) {
		alifErr_setString(_alifExcValueError_, "الدقيقة المدخلة خارج نطاق الساعة");
		return 0;
	}
	if (_buf->tm_sec < 0 or _buf->tm_sec > 61) {
		alifErr_setString(_alifExcValueError_, "الثانية المدخلة خارج نطاق الدقيقة");
		return 0;
	}
	if (_buf->tm_wday < 0) {
		alifErr_setString(_alifExcValueError_, "اليوم المدخل خارج نطاق الاسبوع");
		return 0;
	}
	if (_buf->tm_yday == -1)
		_buf->tm_yday = 0;
	else if (_buf->tm_yday < 0 or _buf->tm_yday > 365) {
		alifErr_setString(_alifExcValueError_, "اليوم المدخل خارج نطاق السنة");
		return 0;
	}
	return 1;
}


static AlifObject* _asctime(struct tm* _timeptr) { // 969
	static const char wday_name[7][17] = {
		"الأحد", "الإثنين", "الثلاثاء", "الإربعاء", "الخميس", "الجمعة", "السبت"
	};
	static const char mon_name[12][13] = {
		"يناير", "فبراير", "مارس", "أبريل", "مايو", "يونيو",
		"يوليو", "أغسطس", "سبتمبر", "أكتوبر", "نوفمبر", "ديسمبر"
	};
	return alifUStr_fromFormat(
		"%.18s %.14s%3d %.2d:%.2d:%.2d %d",
		wday_name[_timeptr->tm_wday],
		mon_name[_timeptr->tm_mon],
		_timeptr->tm_mday, _timeptr->tm_hour,
		_timeptr->tm_min, _timeptr->tm_sec,
		1900 + _timeptr->tm_year);
}

static AlifObject* time_asctime(AlifObject* _module, AlifObject* _args) { // 990
	AlifObject* tup = nullptr;
	struct tm buf {};

	if (!alifArg_unpackTuple(_args, "منسق", 0, 1, &tup))
		return nullptr;

	TimeModuleState* state = get_timeState(_module);
	if (tup == nullptr) {
		time_t tt = time(nullptr);
		if (_alifTime_localtime(tt, &buf) != 0)
			return nullptr;
	}
	else if (!get_tmArg(state, tup, &buf,
		"iiiiiiiii;asctime(): معاملات مترابطة الوقت غير صحيحة") ||
		!checktm(&buf))
	{
		return nullptr;
	}
	return _asctime(&buf);
}






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
	if (gmtime_r(&zero, &tm) != nullptr)
		_utcString_ = tm.tm_zone;
#endif

#if defined(_WINDOWS)
	if (_timerFlags_ == (DWORD)-1) {
		DWORD test_flags = CREATE_WAITABLE_TIMER_HIGH_RESOLUTION;
		HANDLE timer = CreateWaitableTimerExW(nullptr, nullptr, test_flags,
			TIMER_ALL_ACCESS);
		if (timer == nullptr) {
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

static AlifMethodDef _timeMethods_[] = { // 1870
	{"الان", time_time, METHOD_NOARGS},
	{"غفوة", time_sleep, METHOD_O},
	{"وقت", time_localtimeDict, METHOD_VARARGS},
	{"منسق", time_asctime, METHOD_VARARGS},
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
	.name = "الوقت",
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

	LARGE_INTEGER relativeTimeout{};
	// SetWaitableTimer(): a negative due time indicates relative time
	relativeTimeout.QuadPart = -timeout_100ns;

	HANDLE timer = CreateWaitableTimerExW(nullptr, nullptr, _timerFlags_,
		TIMER_ALL_ACCESS);
	if (timer == nullptr) {
		//alifErr_setFromWindowsErr(0);
		return -1;
	}

	if (!SetWaitableTimerEx(timer, &relativeTimeout,
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
