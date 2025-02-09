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

#if SIZEOF_TIME_T == SIZEOF_LONG_LONG
#  define ALIF_TIMET_MAX LLONG_MAX
#  define ALIF_TIMET_MIN LLONG_MIN
#elif SIZEOF_TIME_T == SIZEOF_LONG
#  define ALIF_TIMET_MAX LONG_MAX
#  define ALIF_TIMET_MIN LONG_MIN
#else
#  error "unsupported time_t size"
#endif









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


static void alifTime_timeTOverflow() { // 103
	//alifErr_setString(_alifExcOverflowError_,
	//	"timestamp out of range for platform time_t");
}



static inline AlifIntT alifTime_add(AlifTimeT* _t1, AlifTimeT _t2) { // 120
	if (_t2 > 0 and *_t1 > ALIFTIME_MAX - _t2) {
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


static AlifIntT _alifTime_asTimeT(AlifTimeT t, time_t* t2) { // 244
#if SIZEOF_TIME_T < _SIZEOF_ALIFTIME_T
	if ((AlifTimeT)ALIF_TIMET_MAX < t) {
		*t2 = ALIF_TIMET_MAX;
		return -1;
	}
	if (t < (AlifTimeT)ALIF_TIMET_MIN) {
		*t2 = ALIF_TIMET_MIN;
		return -1;
	}
#endif
	* t2 = (time_t)t;
	return 0;
}


#ifdef _WINDOWS
static AlifIntT _alifTime_asCLong(AlifTimeT t, long* t2) { // 265
#if SIZEOF_LONG < _SIZEOF_ALIFTIME_T
	if ((AlifTimeT)LONG_MAX < t) {
		*t2 = LONG_MAX;
		return -1;
	}
	if (t < (AlifTimeT)LONG_MIN) {
		*t2 = LONG_MIN;
		return -1;
	}
#endif
	* t2 = (long)t;
	return 0;
}
#endif


static double alifTime_roundHalfEven(double x) { // 286
	double rounded = round(x);
	if (fabs(x - rounded) == 0.5) {
		/* halfway case: round to even */
		rounded = 2.0 * round(x / 2.0);
	}
	return rounded;
}

static double alifTime_round(double x, AlifTimeRoundT round) { // 298
	volatile double d{};

	d = x;
	if (round == AlifTimeRoundT::AlifTime_Round_HALF_EVEN) {
		d = alifTime_roundHalfEven(d);
	}
	else if (round == AlifTimeRoundT::AlifTime_Round_CEILING) {
		d = ceil(d);
	}
	else if (round == AlifTimeRoundT::AlifTime_Round_FLOOR) {
		d = floor(d);
	}
	else {
		d = (d >= 0.0) ? ceil(d) : floor(d);
	}
	return d;
}



AlifTimeT _alifTime_fromMicrosecondsClamp(AlifTimeT _us) { // 465
	AlifTimeT ns_ = _alifTime_mul(_us, US_TO_NS);
	return ns_;
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


static AlifIntT alifTime_fromDouble(AlifTimeT* tp,
	double value, AlifTimeRoundT round, long unit_to_ns) { // 565
	volatile double d{};

	d = value;
	d *= (double)unit_to_ns;
	d = alifTime_round(d, round);

	if (!((double)ALIFTIME_MIN <= d and d < -(double)ALIFTIME_MIN)) {
		alifTime_timeTOverflow();
		*tp = 0;
		return -1;
	}
	AlifTimeT ns = (AlifTimeT)d;

	*tp = ns;
	return 0;
}


static AlifIntT alifTime_fromObject(AlifTimeT* tp,
	AlifObject* obj, AlifTimeRoundT round, long unit_to_ns) { // 590
	if (ALIFFLOAT_CHECK(obj)) {
		double d{};
		d = alifFloat_asDouble(obj);
		if (isnan(d)) {
			//alifErr_setString(_alifExcValueError_, "Invalid value NaN (not a number)");
			return -1;
		}
		return alifTime_fromDouble(tp, d, round, unit_to_ns);
	}
	else {
		long long sec = alifLong_asLongLong(obj);
		if (sec == -1 and alifErr_occurred()) {
			//if (alifErr_exceptionMatches(_alifExcOverflowError_)) {
			//	alifTime_overflow();
			//}
			return -1;
		}

		static_assert(sizeof(long long) <= sizeof(AlifTimeT),
			"AlifTimeT is smaller than long long");
		AlifTimeT ns = (AlifTimeT)sec;
		if (alifTime_mul(&ns, unit_to_ns) < 0) {
			//alifTime_overflow();
			return -1;
		}

		*tp = ns;
		return 0;
	}
}

AlifIntT _alifTime_fromSecondsObject(AlifTimeT* _tp,
	AlifObject* _obj, AlifTimeRoundT _round) { // 626
	return alifTime_fromObject(_tp, _obj, _round, SEC_TO_NS);
}



static AlifTimeT alifTime_divideRoundUp(const AlifTimeT t, const AlifTimeT k) { // 675
	if (t >= 0) {
		AlifTimeT q = t / k;
		if (t % k) {
			q += 1;
		}
		return q;
	}
	else {
		AlifTimeT q = t / k;
		if (t % k) {
			q -= 1;
		}
		return q;
	}
}




static AlifTimeT alifTime_divide(const AlifTimeT t, const AlifTimeT k,
	const AlifTimeRoundT round) { // 700
	if (round == AlifTimeRoundT::AlifTime_Round_HALF_EVEN) {
		AlifTimeT x = t / k;
		AlifTimeT r = t % k;
		AlifTimeT abs_r = ALIF_ABS(r);
		if (abs_r > k / 2 or (abs_r == k / 2 and (ALIF_ABS(x) & 1))) {
			if (t >= 0) {
				x++;
			}
			else {
				x--;
			}
		}
		return x;
	}
	else if (round == AlifTimeRoundT::AlifTime_Round_CEILING) {
		if (t >= 0) {
			return alifTime_divideRoundUp(t, k);
		}
		else {
			return t / k;
		}
	}
	else if (round == AlifTimeRoundT::AlifTime_Round_FLOOR) {
		if (t >= 0) {
			return t / k;
		}
		else {
			return alifTime_divideRoundUp(t, k);
		}
	}
	else {
		return alifTime_divideRoundUp(t, k);
	}
}


static AlifIntT alifTime_divMod(const AlifTimeT _t, const AlifTimeT _k,
	AlifTimeT* _pq, AlifTimeT* _pr) { // 742
	AlifTimeT q = _t / _k;
	AlifTimeT r = _t % _k;
	if (r < 0) {
		if (q == ALIFTIME_MIN) {
			*_pq = ALIFTIME_MIN;
			*_pr = 0;
			return -1;
		}
		r += _k;
		q -= 1;
	}

	*_pq = q;
	*_pr = r;
	return 0;
}


#ifdef _WINDOWS
AlifTimeT _alifTime_as100Nanoseconds(AlifTimeT _ns,
	AlifTimeRoundT _round) { // 771
	return alifTime_divide(_ns, NS_TO_100NS, _round);
}
#endif


AlifTimeT _alifTime_asMilliseconds(AlifTimeT _ns, AlifTimeRoundT _round) { // 786
	return alifTime_divide(_ns, NS_TO_MS, _round);
}




static AlifIntT alifTime_asTimEval(AlifTimeT _ns, AlifTimeT* _ptvSec,
	AlifIntT* _ptvUsec, AlifTimeRoundT _round) { // 793
	AlifTimeT us = alifTime_divide(_ns, US_TO_NS, _round);

	AlifTimeT tvSec{}, tvUsec{};
	AlifIntT res = alifTime_divMod(us, SEC_TO_US, &tvSec, &tvUsec);
	*_ptvSec = tvSec;
	*_ptvUsec = (int)tvUsec;
	return res;
}

static AlifIntT alifTime_asTimEvalStruct(AlifTimeT t, struct timeval* tv,
	AlifTimeRoundT round, AlifIntT raise_exc) { // 807
	AlifTimeT tv_sec{};
	AlifIntT tv_usec{};
	AlifIntT res = alifTime_asTimEval(t, &tv_sec, &tv_usec, round);
	AlifIntT res2{};
#ifdef _WINDOWS
	// On Windows, timeval.tv_sec type is long
	res2 = _alifTime_asCLong(tv_sec, &tv->tv_sec);
#else
	res2 = _alifTime_asTimeT(tv_sec, &tv->tv_sec);
#endif
	if (res2 < 0) {
		tv_usec = 0;
	}
	tv->tv_usec = tv_usec;

	if (raise_exc and (res < 0 or res2 < 0)) {
		alifTime_timeTOverflow();
		return -1;
	}
	return 0;
}


AlifIntT _alifTime_asTimEval(AlifTimeT t,
	struct timeval* tv, AlifTimeRoundT round) { // 834
	return alifTime_asTimEvalStruct(t, tv, round, 1);
}



#if defined(HAVE_CLOCK_GETTIME) or defined(HAVE_KQUEUE) // 862

static AlifIntT alifTime_asTimeSpec(AlifTimeT ns, timespec* ts, AlifIntT raise_exc) { // 863
	AlifTimeT tv_sec{}, tv_nsec{};
	AlifIntT res = alifTime_divMod(ns, SEC_TO_NS, &tv_sec, &tv_nsec);

	AlifIntT res2 = _alifTime_asTimeT(tv_sec, &ts->tv_sec);
	if (res2 < 0) {
		tv_nsec = 0;
	}
	ts->tv_nsec = tv_nsec;

	if (raise_exc and (res < 0 or res2 < 0)) {
		alifTime_timeTOverflow();
		return -1;
	}
	return 0;
}

void _alifTime_asTimeSpecClamp(AlifTimeT t, timespec* ts) { // 882
	alifTime_asTimeSpec(t, ts, 0);
}

AlifIntT _alifTime_asTimeSpec(AlifTimeT t, struct timespec* ts) { // 892
	return alifTime_asTimeSpec(t, ts, 1);
}

#endif // 893

static AlifIntT alifGet_systemClock(AlifTimeT* tp, AlifClockInfoT* info, AlifIntT raise_exc) { // 897
	if (raise_exc) {
	}

#ifdef _WINDOWS
	FILETIME system_time{};
	ULARGE_INTEGER large{};

	GetSystemTimePreciseAsFileTime(&system_time);
	large.u.LowPart = system_time.dwLowDateTime;
	large.u.HighPart = system_time.dwHighDateTime;
	/* 11,644,473,600,000,000,000: number of nanoseconds between
	   the 1st january 1601 and the 1st january 1970 (369 years + 89 leap
	   days). */
	AlifTimeT ns = large.QuadPart * 100 - 11644473600000000000;
	*tp = ns;
	if (info) {
		// GetSystemTimePreciseAsFileTime() is implemented using
		// QueryPerformanceCounter() internally.
		if (_alifQPCBase_.denom == 0) {
			if (alifWin_perfCounterFrequency(&_alifQPCBase_, raise_exc) < 0) {
				return -1;
			}
		}

		info->implementation = "GetSystemTimePreciseAsFileTime()";
		info->monotonic = 0;
		info->resolution = alifTimeFraction_resolution(&_alifQPCBase_);
		info->adjustable = 1;
	}

#else   /* _WINDOWS */
	AlifIntT err{};
#if defined(HAVE_CLOCK_GETTIME)
	struct timespec ts;
#endif

#if !defined(HAVE_CLOCK_GETTIME) or defined(__APPLE__)
	struct timeval tv;
#endif

#ifdef HAVE_CLOCK_GETTIME

#ifdef HAVE_CLOCK_GETTIME_RUNTIME
	if (HAVE_CLOCK_GETTIME_RUNTIME) {
#endif

		err = clock_gettime(CLOCK_REALTIME, &ts);
		if (err) {
			if (raise_exc) {
				//alifErr_setFromErrno(_alifExcOSError_);
			}
			return -1;
		}
		if (alifTime_fromTimeSpec(tp, &ts, raise_exc) < 0) {
			return -1;
		}

		if (info) {
			struct timespec res;
			info->implementation = "clock_gettime(CLOCK_REALTIME)";
			info->monotonic = 0;
			info->adjustable = 1;
			if (clock_getres(CLOCK_REALTIME, &res) == 0) {
				info->resolution = (double)res.tv_sec + (double)res.tv_nsec * 1e-9;
			}
			else {
				info->resolution = 1e-9;
			}
		}

#ifdef HAVE_CLOCK_GETTIME_RUNTIME
	}
	else {
#endif

#endif

#if !defined(HAVE_CLOCK_GETTIME) || defined(HAVE_CLOCK_GETTIME_RUNTIME)

		/* test gettimeofday() */
		err = gettimeofday(&tv, (struct timezone*)NULL);
		if (err) {
			if (raise_exc) {
				//alifErr_setFromErrno(_alifExcOSError_);
			}
			return -1;
		}
		//if (alifTime_fromTimeVal(tp, &tv, raise_exc) < 0) {
		//	return -1;
		//}

		if (info) {
			info->implementation = "gettimeofday()";
			info->resolution = 1e-6;
			info->monotonic = 0;
			info->adjustable = 1;
		}

#if defined(HAVE_CLOCK_GETTIME_RUNTIME) && defined(HAVE_CLOCK_GETTIME)
	} /* end of availability block */
#endif

#endif   /* !HAVE_CLOCK_GETTIME */
#endif   /* !MS_WINDOWS */
	return 0;
}



AlifIntT alifTime_timeRaw(AlifTimeT* _result) { // 1022
	if (alifGet_systemClock(_result, nullptr, 0) < 0) {
		*_result = 0;
		return -1;
	}
	return 0;
}




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



AlifIntT alifTime_monotonic(AlifTimeT* result) { // 1239
	if (alifGet_monotonicClock(result, nullptr, 1) < 0) {
		*result = 0;
		return -1;
	}
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
