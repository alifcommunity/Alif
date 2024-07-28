#include "alif.h"
#include "AlifCore_Time.h"          // AlifTimeT

#include <time.h>                 // gmtime_r()
#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>           // gettimeofday()
#endif
#ifdef MS_WINDOWS
#  include <winsock2.h>           // struct timeval
#endif

#if defined(__APPLE__)
#  include <mach/mach_time.h>     // mach_absolute_time(), mach_timebase_info()

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

static inline int alifTime_mul_check_overflow(AlifTimeT _a, AlifTimeT _b)
{
    if (_b != 0) {
        return ((_a < ALIFTime_MIN / _b) || (ALIFTime_MAX / _b < _a));
    }
    else {
        return 0;
    }
}


static inline int alifTime_mul(AlifTimeT* _t, AlifTimeT _k)
{
    if (alifTime_mul_check_overflow(*_t, _k)) {
        *_t = (*_t >= 0) ? ALIFTime_MAX : ALIFTime_MIN;
        return -1;
    }
    else {
        *_t *= _k;
        return 0;
    }
}



// in file AlifMath.h
#define ALIF_IS_NAN(X) isnan(X)

static double aliftime_round_half_even(double _x)
{
    double rounded_ = round(_x);
    if (fabs(_x - rounded_) == 0.5) {
        /* halfway case: round to even */
        rounded_ = 2.0 * round(_x / 2.0);
    }
    return rounded_;
}

static double aliftime_round(double _x, AlifSubTimeRoundT _round)
{
    volatile double d_;

    d_ = _x;
    if (_round == AlifSubTime_Round_Half_Even) {
        d_ = aliftime_round_half_even(d_);
    }
    else if (_round == AlifSubTime_Round_Ceiling) {
        d_ = ceil(d_);
    }
    else if (_round == AlifSubTime_Round_Floor) {
        d_ = floor(d_);
    }
    else {
        d_ = (d_ >= 0.0) ? ceil(d_) : floor(d_);
    }
    return d_;
}

static int alifTime_from_double(AlifTimeT* _tp, double _value, AlifSubTimeRoundT _round,
    long _unitToNs)
{
    volatile double d_;

    d_ = _value;
    d_ *= (double)_unitToNs;
    d_ = aliftime_round(d_, _round);

    if (!((double)ALIFTime_MIN <= d_ && d_ < -(double)ALIFTime_MIN)) {
        *_tp = 0;
        return -1;
    }
    AlifTimeT ns_ = (AlifTimeT)d_;

    *_tp = ns_;
    return 0;
}

static int alifTime_from_object(AlifTimeT* _tp, AlifObject* _obj, AlifSubTimeRoundT _round,
    long _unitToNs)
{
    if ((_obj->type_ == &_alifFloatType)) {
        double d_;
        d_ = alifFloat_asLongDouble(_obj);
        if (ALIF_IS_NAN(d_)) {
            return -1;
        }
        return alifTime_from_double(_tp, d_, _round, _unitToNs);
    }
    else {
        long long sec = alifInteger_asLongLong(_obj);

        AlifTimeT ns = (AlifTimeT)sec;
        if (alifTime_mul(&ns, _unitToNs) < 0) {
            return -1;
        }

        *_tp = ns;
        return 0;
    }
}

int alifSubTime_fromSecondsObject(AlifTimeT* _tp, AlifObject* _obj, AlifSubTimeRoundT _round)
{
    return alifTime_from_object(_tp, _obj, _round, SEC_TO_NS);
}

static AlifTimeT alifTime_divide_round_up(const AlifTimeT _t, const AlifTimeT _k)
{
    if (_t >= 0) {
        AlifTimeT q_ = _t / _k;
        if (_t % _k) {
            q_ += 1;
        }
        return q_;
    }
    else {

        AlifTimeT q_ = _t / _k;
        if (_t % _k) {
            q_ -= 1;
        }
        return q_;
    }
}


static AlifTimeT alifTime_divide(const AlifTimeT _t, const AlifTimeT _k,
    const AlifSubTimeRoundT _round)
{
    if (_round == AlifSubTime_Round_Half_Even) {
        AlifTimeT x_ = _t / _k;
        AlifTimeT r_ = _t % _k;
        AlifTimeT absR = ALIF_ABS(r_);
        if (absR > _k / 2 || (absR == _k / 2 && (ALIF_ABS(x_) & 1))) {
            if (_t >= 0) {
                x_++;
            }
            else {
                x_--;
            }
        }
        return x_;
    }
    else if (_round == AlifSubTime_Round_Ceiling) {
        if (_t >= 0) {
            return alifTime_divide_round_up(_t, _k);
        }
        else {
            return _t / _k;
        }
    }
    else if (_round == AlifSubTime_Round_Floor) {
        if (_t >= 0) {
            return _t / _k;
        }
        else {
            return alifTime_divide_round_up(_t, _k);
        }
    }
    else {
        return alifTime_divide_round_up(_t, _k);
    }
}

AlifTimeT alifSubTime_asMicroseconds(AlifTimeT _ns, AlifSubTimeRoundT _round)
{
    return alifTime_divide(_ns, NS_TO_US, _round);
}