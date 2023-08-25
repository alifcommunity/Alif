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


























static inline alifTimeT alifTime_from_nanoseconds(alifTimeT t)
{

	return t;
}



static inline alifTimeT alifTime_as_nanoseconds(alifTimeT t)
{

	return t;
}




static inline int alif_Time_add(alifTimeT* t1, alifTimeT t2)
{
	if (t2 > 0 && *t1 > ALIFTIME_MAX - t2) {
		*t1 = ALIFTIME_MAX;
		return -1;
	}
	else if (t2 < 0 && *t1 < ALIFTIME_MIN - t2) {
		*t1 = ALIFTIME_MIN;
		return -1;
	}
	else {
		*t1 += t2;
		return 0;
	}
}



alifTimeT alifTime_add(alifTimeT t1, alifTimeT t2)
{
	(void)alif_Time_add(&t1, t2);
	return t1;
}



static inline int alifTime_mul_checkOverflow(alifTimeT a, alifTimeT b)
{
	if (b != 0) {

		return ((a < ALIFTIME_MIN / b) || (ALIFTIME_MAX / b < a));
	}
	else {
		return 0;
	}
}




static inline int alif_time_mul(alifTimeT* t, alifTimeT k)
{

	if (alifTime_mul_checkOverflow(*t, k)) {
		*t = (*t >= 0) ? ALIFTIME_MAX : ALIFTIME_MIN;
		return -1;
	}
	else {
		*t *= k;
		return 0;
	}
}




static inline alifTimeT alifTime_mul(alifTimeT t, alifTimeT k)
{
	(void)alif_time_mul(&t, k);
	return t;
}



alifTimeT alifTime_mulDiv(alifTimeT ticks, alifTimeT mul, alifTimeT div)
{





	alifTimeT intpart, remaining;
	intpart = ticks / div;
	ticks %= div;
	remaining = alifTime_mul(ticks, mul) / div;
	// intpart * mul + remaining
	return alifTime_add(alifTime_mul(intpart, mul), remaining);
}


































































































































































































































































alifTimeT alifTime_fromNanoseconds(alifTimeT ns)
{
	return alifTime_from_nanoseconds(ns);
}
















































































































































































































static alifTimeT alifTime_divide_roundUp(const alifTimeT t, const alifTimeT k)
{

	if (t >= 0) {
		// Don't use (t + k - 1) / k to avoid integer overflow

		alifTimeT q = t / k;
		if (t % k) {
			q += 1;
		}
		return q;
	}
	else {
		// Don't use (t - (k - 1)) / k to avoid integer overflow

		alifTimeT q = t / k;
		if (t % k) {
			q -= 1;
		}
		return q;
	}
}



static alifTimeT alifTime_divide(const alifTimeT t, const alifTimeT k,
	const AlifTimeRoundT round)
{

	if (round == AlifTime_Round_HalfEven) {
		alifTimeT x = t / k;
		alifTimeT r = t % k;
		alifTimeT abs_r = ALIF_ABS(r);
		if (abs_r > k / 2 || (abs_r == k / 2 && (ALIF_ABS(x) & 1))) {
			if (t >= 0) {
				x++;
			}
			else {
				x--;
			}
		}
		return x;
	}
	else if (round == AlifTime_Round_Ceiling) {
		if (t >= 0) {
			return alifTime_divide_roundUp(t, k);
		}
		else {
			return t / k;
		}
	}
	else if (round == AlifTime_Round_Floor) {
		if (t >= 0) {
			return t / k;
		}
		else {
			return alifTime_divide_roundUp(t, k);
		}
	}
	else {

		return alifTime_divide_roundUp(t, k);
	}
}
















































alifTimeT alifTime_asMicroseconds(alifTimeT t, AlifTimeRoundT round)
{
	alifTimeT ns = alifTime_as_nanoseconds(t);
	return alifTime_divide(ns, NS_TO_US, round);
}































































































































































































































































































































































































































































#ifdef MS_WINDOWS
static int alifWin_perf_counterFrequency(LONGLONG* pfrequency, int raise)
{
	LONGLONG frequency;

	LARGE_INTEGER freq;

	(void)QueryPerformanceFrequency(&freq);
	frequency = freq.QuadPart;














	if (frequency > ALIFTIME_MAX / SEC_TO_NS) {




		return -1;
	}

	*pfrequency = frequency;
	return 0;
}



static int alifGet_win_perfCounter(alifTimeT* tp, AlifClockInfoT* info, int raise_exc)
{


	static LONGLONG frequency = 0;
	if (frequency == 0) {
		if (alifWin_perf_counterFrequency(&frequency, raise_exc) < 0) {
			return -1;
		}
	}

	if (info) {
		info->implementation = "QueryPerformanceCounter()";
		info->resolution = 1.0 / (double)frequency;
		info->monotonic = 1;
		info->adjustable = 0;
	}

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	LONGLONG ticksll = now.QuadPart;



	alifTimeT ticks;
	static_assert(sizeof(ticksll) <= sizeof(ticks),
		"LONGLONG is larger than alifTimeT");
	ticks = (alifTimeT)ticksll;

	alifTimeT ns = alifTime_mulDiv(ticks, SEC_TO_NS, (alifTimeT)frequency);
	*tp = alifTime_from_nanoseconds(ns);
	return 0;
}
#endif














alifTimeT alifTime_getPerfCounter(void)
{
	alifTimeT t;
	int res;
#ifdef MS_WINDOWS
	res = alifGet_win_perfCounter(&t, NULL, 0);
#else
	res = alifGet_monotonic_clock(&t, NULL, 0);
#endif
	if (res < 0) {


		t = 0;
	}
	return t;
}
