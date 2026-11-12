#pragma once



using AlifTimeT = int64_t;
#define ALIFTIME_MIN INT64_MIN
#define ALIFTIME_MAX INT64_MAX



ALIFAPI_FUNC(double) alifTime_asSecondsDouble(AlifTimeT);
ALIFAPI_FUNC(AlifIntT) alifTime_monotonic(AlifTimeT*);

ALIFAPI_FUNC(AlifIntT) alifTime_time(AlifTimeT*);

ALIFAPI_FUNC(AlifIntT) alifTime_monotonicRaw(AlifTimeT*);


ALIFAPI_FUNC(AlifIntT) alifTime_timeRaw(AlifTimeT*);
