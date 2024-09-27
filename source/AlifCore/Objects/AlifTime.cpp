#include "alif.h"

#include "AlifCore_Time.h"

#include <time.h>                 // gmtime_r()
#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>           // gettimeofday()
#endif
#ifdef MS_WINDOWS
#  include <winsock2.h>           // struct timeval
#endif

#if defined(__APPLE__)
#  include <mach/mach_time.h>     // mach_absolute_time(), mach_timebase_info()

#if defined(__APPLE__) and defined(__has_builtin)
#  if __has_builtin(__builtin_available)
#    define HAVE_CLOCK_GETTIME_RUNTIME __builtin_available(macOS 10.12, iOS 10.0, tvOS 10.0, watchOS 3.0, *)
#  endif
#endif
#endif


/* To millisecond (10^-3) */
#define SEC_TO_MS 1000

/* To microseconds (10^-6) */
#define MS_TO_US 1000
#define SEC_TO_US (SEC_TO_MS * MS_TO_US)

/* To nanoseconds (10^-9) */
#define US_TO_NS 1000
#define MS_TO_NS (MS_TO_US * US_TO_NS)
#define SEC_TO_NS (SEC_TO_MS * MS_TO_NS)

/* Conversion from nanoseconds */
#define NS_TO_MS (1000 * 1000)
#define NS_TO_US (1000)
#define NS_TO_100NS (100)











// 58
#ifdef _WINDOWS
AlifTimeFraction _alifQPCBase_ = { .numer = 0, .denom = 0 };

// Forward declaration
static AlifIntT alifWin_perfCounterFrequency(AlifTimeFraction*, AlifIntT);
#endif



static AlifTimeT alifTime_gcd(AlifTimeT x, AlifTimeT y) { // 66
	while (y != 0) {
		AlifTimeT tmp = y;
		y = x % y;
		x = tmp;
	}

	return x;
}

AlifIntT alifTimeFraction_set(AlifTimeFraction* frac,
	AlifTimeT numer, AlifTimeT denom) { // 82
	if (numer < 1 or denom < 1) {
		return -1;
	}

	AlifTimeT gcd = alifTime_gcd(numer, denom);
	frac->numer = numer / gcd;
	frac->denom = denom / gcd;
	return 0;
}




double alifTimeFraction_resolution(const AlifTimeFraction* _frac) { // 96
	return (double)_frac->numer / (double)_frac->denom / 1e9;
}


static inline AlifIntT alifTime_add(AlifTimeT* _t1, AlifTimeT _t2) { // 120
	if (_t2 > 0 && *_t1 > ALIFTIME_MAX - _t2) {
		*_t1 = ALIFTIME_MAX;
		return -1;
	}
	else if (_t2 < 0 && *_t1 < ALIFTIME_MIN - _t2) {
		*_t1 = ALIFTIME_MIN;
		return -1;
	}
	else {
		*_t1 += _t2;
		return 0;
	}
}


AlifTimeT _alifTime_add(AlifTimeT _t1, AlifTimeT _t2) { // 138
	(void)alifTime_add(&_t1, _t2);
	return _t1;
}


static inline AlifIntT alifTimeMul_checkOverflow(AlifTimeT _a, AlifTimeT _b) { // 146
	if (_b != 0) {
		return ((_a < ALIFTIME_MIN / _b) or (ALIFTIME_MAX / _b < _a));
	}
	else {
		return 0;
	}
}



static inline AlifIntT alifTime_mul(AlifTimeT* _t, AlifTimeT _k) { // 160
	if (alifTimeMul_checkOverflow(*_t, _k)) {
		*_t = (*_t >= 0) ? ALIFTIME_MAX : ALIFTIME_MIN;
		return -1;
	}
	else {
		*_t *= _k;
		return 0;
	}
}


static inline AlifTimeT _alifTime_mul(AlifTimeT _t, AlifTimeT _k) { // 176
	(void)alifTime_mul(&_t, _k);
	return _t;
}




AlifTimeT alifTimeFraction_mul(AlifTimeT _ticks,
	const AlifTimeFraction* _frac) { // 184
	const AlifTimeT mul = _frac->numer;
	const AlifTimeT div = _frac->denom;

	if (div == 1) {
		// Fast-path taken by mach_absolute_time() with 1/1 time base.
		return _alifTime_mul(_ticks, mul);
	}

	/* Compute (ticks * mul / div) in two parts to reduce the risk of integer
	   overflow: compute the integer part, and then the remaining part.

	   (ticks * mul) / div == (ticks / div) * mul + (ticks % div) * mul / div
	*/
	AlifTimeT intpart, remaining;
	intpart = _ticks / div;
	_ticks %= div;
	remaining = _alifTime_mul(_ticks, mul) / div;
	// intpart * mul + remaining
	return _alifTime_add(_alifTime_mul(intpart, mul), remaining);
}






#ifdef HAVE_CLOCK_GETTIME
static AlifIntT alifTime_fromTimeSpec(AlifTimeT* tp, const struct timespec* ts,
	AlifIntT raise_exc) { // 498

	AlifTimeT t{}, tv_nsec{};

	t = (AlifTimeT)ts->tv_sec;

	AlifIntT res1 = alifTime_mul(&t, SEC_TO_NS);

	tv_nsec = ts->tv_nsec;
	AlifIntT res2 = alifTime_add(&t, tv_nsec);

	*tp = t;

	if (raise_exc and (res1 < 0 or res2 < 0)) {
		//alifTime_overflow();
		return -1;
	}
	return 0;
}

AlifIntT _alifTime_fromTimeSpec(AlifTimeT* tp, const struct timespec* ts) { // 522
	return alifTime_fromTimeSpec(tp, ts, 1);
}
#endif











#ifdef _WINDOWS
static AlifIntT alifWin_perfCounterFrequency(AlifTimeFraction* _base,
	AlifIntT _raiseExc) { // 1041
	LARGE_INTEGER freq;
	(void)QueryPerformanceFrequency(&freq);
	LONGLONG frequency = freq.QuadPart;

	AlifTimeT denom = (AlifTimeT)frequency;

	// Known QueryPerformanceFrequency() values:
	//
	// * 10,000,000 (10 MHz): 100 ns resolution
	// * 3,579,545 Hz (3.6 MHz): 279 ns resolution
	if (alifTimeFraction_set(_base, SEC_TO_NS, denom) < 0) {
		if (_raiseExc) {
			//alifErr_setString(alifExcDureRunError,
			//	"invalid QueryPerformanceFrequency");
		}
		return -1;
	}
	return 0;
}


static AlifIntT alifGet_winPerfCounter(AlifTimeT* _tp,
	AlifClockInfoT* _info, AlifIntT _raiseExc) { // 1071

	if (_alifQPCBase_.denom == 0) {
		if (alifWin_perfCounterFrequency(&_alifQPCBase_, _raiseExc) < 0) {
			return -1;
		}
	}

	if (_info) {
		_info->implementation = "QueryPerformanceCounter()";
		_info->resolution = alifTimeFraction_resolution(&_alifQPCBase_);
		_info->monotonic = 1;
		_info->adjustable = 0;
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	LONGLONG ticksll = now.QuadPart;

	AlifTimeT ticks;
	static_assert(sizeof(ticksll) <= sizeof(ticks),
		"LONGLONG is larger than AlifTimeT");
	ticks = (AlifTimeT)ticksll;

	*_tp = alifTimeFraction_mul(ticks, &_alifQPCBase_);
	return 0;
}
#endif  // _WINDOWS



#ifdef __APPLE__
static AlifIntT alifMach_timeBaseInfo(AlifTimeFraction* base, AlifIntT raise_exc) { // 1107
	mach_timebase_info_data_t timebase;
	(void)mach_timebase_info(&timebase);

	AlifTimeT numer = (AlifTimeT)timebase.numer;
	AlifTimeT denom = (AlifTimeT)timebase.denom;

	// Known time bases:
	//
	// * (1, 1) on Intel: 1 ns
	// * (1000000000, 33333335) on PowerPC: ~30 ns
	// * (1000000000, 25000000) on PowerPC: 40 ns
	if (alifTimeFraction_set(base, numer, denom) < 0) {
		if (raise_exc) {
			//alifErr_setString(alifExcDureRunError,
			//	"invalid mach_timebase_info");
		}
		return -1;
	}
	return 0;
}
#endif



static AlifIntT alifGet_monotonicClock(AlifTimeT* _tp,
	AlifClockInfoT* _info, AlifIntT _raiseExc) { // 1142

#if defined(_WINDOWS)
	if (alifGet_winPerfCounter(_tp, _info, _raiseExc) < 0) {
		return -1;
	}
#elif defined(__APPLE__)
	AlifTimeFraction base = { .numer = 0, .denom = 0 };
	if (base.denom == 0) {
		if (alifMach_timeBaseInfo(&base, _raiseExc) < 0) {
			return -1;
		}
	}

	if (_info) {
		_info->implementation = "mach_absolute_time()";
		_info->resolution = alifTimeFraction_resolution(&base);
		_info->monotonic = 1;
		_info->adjustable = 0;
	}

	uint64_t uticks = mach_absolute_time();
	// unsigned => signed
	AlifTimeT ticks = (AlifTimeT)uticks;

	AlifTimeT ns = alifTimeFraction_mul(ticks, &base);
	*_tp = ns;

#elif defined(__hpux)
	hrtime_t time = gethrtime();
	if (time == -1) {
		if (_raiseExc) {
			//alifErr_setFromErrno(alifExcOSError);
		}
		return -1;
	}

	*_tp = time;

	if (_info) {
		_info->implementation = "gethrtime()";
		_info->resolution = 1e-9;
		_info->monotonic = 1;
		_info->adjustable = 0;
	}

#else

#ifdef CLOCK_HIGHRES
	const clockid_t clk_id = CLOCK_HIGHRES;
	const char* implementation = "clock_gettime(CLOCK_HIGHRES)";
#else
	const clockid_t clk_id = CLOCK_MONOTONIC;
	const char* implementation = "clock_gettime(CLOCK_MONOTONIC)";
#endif

	struct timespec ts;
	if (clock_gettime(clk_id, &ts) != 0) {
		if (_raiseExc) {
			//alifErr_setFromErrno(alifExcOSError);
			return -1;
		}
		return -1;
	}

	if (alifTime_fromTimeSpec(_tp, &ts, _raiseExc) < 0) {
		return -1;
	}

	if (_info) {
		_info->monotonic = 1;
		_info->implementation = implementation;
		_info->adjustable = 0;
		struct timespec res;
		if (clock_getres(clk_id, &res) != 0) {
			//alifErr_setFromErrno(alifExcOSError);
			return -1;
		}
		_info->resolution = res.tv_sec + res.tv_nsec * 1e-9;
	}
#endif
	return 0;
}




AlifIntT alifTime_monotonicRaw(AlifTimeT* _result) { // 1246
	if (alifGet_monotonicClock(_result, nullptr, 0) < 0) {
		*_result = 0;
		return -1;
	}
	return 0;
}




AlifTimeT alifDeadline_get(AlifTimeT _deadline) { // 1362
	AlifTimeT now{};
	(void)alifTime_monotonicRaw(&now);
	return _deadline - now;
}
