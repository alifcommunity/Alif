#include "alif.h"
#include "alifCore_time.h"

inline int _alifTime_add(int64_t* t1, int64_t t2) {

	if (t2 > 0 && *t1 > 9223372036854775807i64 - t2) {
		*t1 = 9223372036854775807i64;
		return -1;
	}
	else if (t2 < 0 && *t1 < -9223372036854775807i64 - 1 - t2) {
		*t1 = -9223372036854775807i64 - 1;
		return -1;
	}
	else {
		*t1 += t2;
		return 0;
	}

}

int64_t alifTime_add(int64_t t1, int64_t t2) {
	(void)_alifTime_add(&t1, t2);
	return t1;
}

inline int alifTime_mul_check_overflow(int64_t a, int64_t b)
{
	if (b != 0) {
		return ((a < -9223372036854775807i64 - 1 / b) || (9223372036854775807i64 / b < a));
	}
	else {
		return 0;
	}
}
inline int alifTime_mul(int64_t* t, int64_t k) {

	if (alifTime_mul_check_overflow(*t, k)) {
		*t = (*t >= 0) ? 9223372036854775807i64 : -9223372036854775807i64 - 1;
		return -1;
	}
	else {
		*t *= k;
		return 0;
	}

}

inline int64_t _alifTime_mul(int64_t t, int64_t k) {
	(void)alifTime_mul(&t, k);
	return t;
}

int64_t alifTime_mulDiv(int64_t ticks, int64_t mul, int64_t div) {

	int64_t intPart, remaining;
	intPart = ticks / div;
	ticks %= div;
	remaining = _alifTime_mul(ticks, mul) / div;
	return alifTime_add(_alifTime_mul(intPart, mul), remaining);

}

#ifdef MS_WINDOWS

int alifWin_perf_counter_frequency(LONGLONG* pfrequency, int raise)
{
	LONGLONG frequency;

	LARGE_INTEGER freq;
	// Since Windows XP, the function cannot fail.
	(void)QueryPerformanceFrequency(&freq);
	frequency = freq.QuadPart;

	if (frequency > 9223372036854775807i64 / (1000 * (1000 * 1000))) {
		if (raise) {
			// error
		}
		return -1;
	}

	*pfrequency = frequency;
	return 0;
}

int alif_get_win_perf_counter(int64_t* tp, AlifClockInfoT* info, int raiseExc)
{

	static LONGLONG frequency = 0;
	if (frequency == 0) {
		if (alifWin_perf_counter_frequency(&frequency, raiseExc) < 0) {
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

	/* Make sure that casting LONGLONG to _PyTime_t cannot overflow,
	   both types are signed */
	int64_t ticks;

	ticks = (int64_t)ticksll;

	int64_t ns = alifTime_mulDiv(ticks, (1000 * (1000 * 1000)), (int64_t)frequency);
	*tp = ns;
	return 0;
}
#endif  // MS_WINDOWS

int64_t alifTime_divide_round_up(const int64_t t, const int64_t k)
{
	if (t >= 0) {

		int64_t q = t / k;
		if (t % k) {
			q += 1;
		}
		return q;
	}
	else {

		int64_t q = t / k;
		if (t % k) {
			q -= 1;
		}
		return q;
	}
}

#define ALIF_ABS(x) ((x) < 0 ? -(x) : (x))

int64_t alifTime_divide(const int64_t t, const int64_t k,	const AlifTimeRoundT round)
{

	if (round == alifTimeRoundHalfEven) {
		int64_t x = t / k;
		int64_t r = t % k;
		int64_t abs_r = ALIF_ABS(r);
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
	else if (round == alifTimeRoundCeiling) {
		if (t >= 0) {
			return alifTime_divide_round_up(t, k);
		}
		else {
			return t / k;
		}
	}
	else if (round == alifTimeRoundFloor) {
		if (t >= 0) {
			return t / k;
		}
		else {
			return alifTime_divide_round_up(t, k);
		}
	}
	else {
		return alifTime_divide_round_up(t, k);
	}
}

int64_t alifTime_asMicroseconds(int64_t t, AlifTimeRoundT round) {

	return alifTime_divide(t, 1000, round);

}

int64_t alifTime_getPerfCounter() {

	int64_t t;
	int res;

#ifdef MS_WINDOWS
	res = alif_get_win_perf_counter(&t, NULL, 0);
#else
	res = py_get_monotonic_clock(&t, NULL, 0);
#endif
	if (res < 0) {
		// If py_win_perf_counter_frequency() or py_get_monotonic_clock()
		// fails: silently ignore the failure and return 0.
		t = 0;
	}
	return t;

}
