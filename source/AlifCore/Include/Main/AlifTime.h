#pragma once



using AlifTimeT = int64_t;
#define ALIFTIME_MIN INT64_MIN
#define ALIFTIME_MAX INT64_MAX




AlifIntT alifTime_monotonic(AlifTimeT*); // 15

AlifIntT alifTime_monotonicRaw(AlifTimeT*); // 19


AlifIntT alifTime_timeRaw(AlifTimeT*); // 21
