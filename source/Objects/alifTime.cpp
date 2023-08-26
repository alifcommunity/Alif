#include "alif.h"
#include "alifCore_time.h"          // alifTimeT
#ifdef MS_WINDOWS
#  include <winsock2.h>           // struct timeval
#endif

#if defined(__APPLE__)
#  include <mach/mach_time.h>   

#if defined(__APPLE__) && defined(__has_builtin)
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


#endif


























static inline alifTimeT alifTime_from_nanoseconds(alifTimeT _t)
{

	return _t;
}



static inline alifTimeT alifTime_as_nanoseconds(alifTimeT _t)
{

	return _t;
}




static inline int alif_Time_add(alifTimeT* _t1, alifTimeT _t2)
{
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



alifTimeT alifTime_add(alifTimeT _t1, alifTimeT _t2)
{
	(void)alif_Time_add(&_t1, _t2);
	return _t1;
}



static inline int alifTime_mul_checkOverflow(alifTimeT _a, alifTimeT _b)
{
	if (_b != 0) {

		return ((_a < ALIFTIME_MIN / _b) || (ALIFTIME_MAX / _b < _a));
	}
	else {
		return 0;
	}
}




static inline int alif_time_mul(alifTimeT* _t, alifTimeT _k)
{

	if (alifTime_mul_checkOverflow(*_t, _k)) {
		*_t = (*_t >= 0) ? ALIFTIME_MAX : ALIFTIME_MIN;
		return -1;
	}
	else {
		*_t *= _k;
		return 0;
	}
}




static inline alifTimeT alifTime_mul(alifTimeT _t, alifTimeT _k)
{
	(void)alif_time_mul(&_t, _k);
	return _t;
}



alifTimeT alifTime_mulDiv(alifTimeT _ticks, alifTimeT _mul, alifTimeT _div)
{





	alifTimeT intpart, remaining;
	intpart = _ticks / _div;
	_ticks %= _div;
	remaining = alifTime_mul(_ticks, _mul) / _div;
	// intpart * _mul + remaining
	return alifTime_add(alifTime_mul(intpart, _mul), remaining);
}


































































































































































































































































alifTimeT alifTime_fromNanoseconds(alifTimeT _ns)
{
	return alifTime_from_nanoseconds(_ns);
}
















































































































































































































static alifTimeT alifTime_divide_roundUp(const alifTimeT _t, const alifTimeT _k)
{

	if (_t >= 0) {


		alifTimeT q = _t / _k;
		if (_t % _k) {
			q += 1;
		}
		return q;
	}
	else {


		alifTimeT q = _t / _k;
		if (_t % _k) {
			q -= 1;
		}
		return q;
	}
}



static alifTimeT alifTime_divide(const alifTimeT _t, const alifTimeT _k,
	const AlifTimeRoundT _round)
{

	if (_round == AlifTime_Round_HalfEven) {
		alifTimeT x = _t / _k;
		alifTimeT r = _t % _k;
		alifTimeT abs_r = ALIF_ABS(r);
		if (abs_r > _k / 2 || (abs_r == _k / 2 && (ALIF_ABS(x) & 1))) {
			if (_t >= 0) {
				x++;
			}
			else {
				x--;
			}
		}
		return x;
	}
	else if (_round == AlifTime_Round_Ceiling) {
		if (_t >= 0) {
			return alifTime_divide_roundUp(_t, _k);
		}
		else {
			return _t / _k;
		}
	}
	else if (_round == AlifTime_Round_Floor) {
		if (_t >= 0) {
			return _t / _k;
		}
		else {
			return alifTime_divide_roundUp(_t, _k);
		}
	}
	else {

		return alifTime_divide_roundUp(_t, _k);
	}
}
















































alifTimeT alifTime_asMicroseconds(alifTimeT _t, AlifTimeRoundT _round)
{
	alifTimeT ns = alifTime_as_nanoseconds(_t);
	return alifTime_divide(ns, NS_TO_US, _round);
}































































































































































































































































































































































































































































#ifdef MS_WINDOWS
static int alifWin_perfCounterFrequency(LONGLONG* _pfrequency, int _raise)
{
	LONGLONG frequency;

	LARGE_INTEGER freq;

	(void)QueryPerformanceFrequency(&freq);
	frequency = freq.QuadPart;














	if (frequency > ALIFTIME_MAX / SEC_TO_NS) {




		return -1;
	}

	*_pfrequency = frequency;
	return 0;
}



static int alifGet_winPerfCounter(alifTimeT* _tp, AlifClockInfoT* _info, int _raiseExc)
{


	static LONGLONG frequency = 0;
	if (frequency == 0) {
		if (alifWin_perfCounterFrequency(&frequency, _raiseExc) < 0) {
			return -1;
		}
	}

	if (_info) {
		_info->implementation = "QueryPerformanceCounter()";
		_info->resolution = 1.0 / (double)frequency;
		_info->monotonic = 1;
		_info->adjustable = 0;
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	LONGLONG ticksll = now.QuadPart;



	alifTimeT ticks;
	static_assert(sizeof(ticksll) <= sizeof(ticks),
		"LONGLONG is larger than alifTimeT");
	ticks = (alifTimeT)ticksll;

	alifTimeT ns = alifTime_mulDiv(ticks, SEC_TO_NS, (alifTimeT)frequency);
	*_tp = alifTime_from_nanoseconds(ns);
	return 0;
}
#endif














alifTimeT alifTime_getPerfCounter()
{
	alifTimeT t;
	int res;
#ifdef MS_WINDOWS
	res = alifGet_winPerfCounter(&t, nullptr, 0);
#else
	res = alifGet_monotonic_clock(&_t, nullptr, 0);
#endif
	if (res < 0) {


		t = 0;
	}
	return t;
}
