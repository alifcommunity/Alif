#include "alif.h"

#include "AlifCore_Long.h"





 // 446
#define ALIF_ABS_LONG_MIN         (0-(unsigned long)LONG_MIN)
#define ALIF_ABS_SIZET_MIN      (0-(AlifUSizeT)ALIF_SIZET_MIN)








AlifSizeT alifLong_asSizeT(AlifObject* _vv) { // 575
	AlifLongObject* v_{};
	AlifUSizeT x_{}, prev{};
	AlifSizeT i_;
	AlifIntT sign{};

	if (_vv == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	if (!ALIFLONG_CHECK(_vv)) {
		//alifErr_setString(_alifExcTypeError_, "an integer is required");
		return -1;
	}

	v_ = (AlifLongObject*)_vv;
	if (alifLong_isCompact(v_)) {
		return alifLong_compactValue(v_);
	}
	i_ = alifLong_digitCount(v_);
	sign = alifLong_nonCompactSign(v_);
	x_ = 0;
	while (--i_ >= 0) {
		prev = x_;
		x_ = (x_ << ALIFLONG_SHIFT) | v_->longValue.digit[i_];
		if ((x_ >> ALIFLONG_SHIFT) != prev)
			goto overflow;
	}
	if (x_ <= (AlifUSizeT)ALIF_SIZET_MAX) {
		return (AlifSizeT)x_ * sign;
	}
	else if (sign < 0 and x_ == ALIF_ABS_SIZET_MIN) {
		return ALIF_SIZET_MIN;
	}
	/* else overflow */

overflow:
	//alifErr_setString(_alifExcOverflowError_,
	//	"ALIF int too large to convert to CPP ssize_t");
	return -1;
}
