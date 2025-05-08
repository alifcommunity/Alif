#include "alif.h"

#include "AlifCore_BitUtils.h"
#include "AlifCore_Call.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"


#include "float.h"

#include "../clinic/MathModule.cpp.h"



class MathModuleState { // 81
public:
	AlifObject* str___ceil__;
	AlifObject* str___floor__;
	AlifObject* str___trunc__;
};

static inline MathModuleState* getMath_moduleState(AlifObject* _module) { // 88
	void* state = _alifModule_getState(_module);
	return (MathModuleState*)state;
}

class DoubleLength { public: double hi{}; double lo{}; }; // 105

static DoubleLength dl_fastSum(double _a, double _b) { // 108
    double x = _a + _b;
    double y = (_a - x) + _b;
    return {x, y};
}

static DoubleLength dl_mul(double _x, double _y) { // 130
	double z = _x * _y;
	double zz = fma(_x, _y, -z);
	return { z, zz };
}

// 217
#define ASSIGN_DOUBLE(_targetVar, _obj, _errorLabel)        \
    if (ALIFFLOAT_CHECKEXACT(_obj)) {                         \
        _targetVar = ALIFFLOAT_AS_DOUBLE(_obj);               \
    }                                                      \
    else if (ALIFLONG_CHECKEXACT(_obj)) {                     \
        _targetVar = alifLong_asDouble(_obj);                 \
        if (_targetVar == -1.0 and alifErr_occurred()) {      \
            goto _errorLabel;                              \
        }                                                  \
    }                                                      \
    else {                                                 \
        _targetVar = alifFloat_asDouble(_obj);                \
        if (_targetVar == -1.0 and alifErr_occurred()) {      \
            goto _errorLabel;                              \
        }                                                  \
    }

static double m_log(double _x) { // 641
	if (isfinite(_x)) {
		if (_x > 0.0)
			return log(_x);
		errno = EDOM;
		if (_x == 0.0)
			return -ALIF_HUGE_VAL; /* log(0) = -inf */
		else
			return ALIF_NAN; /* log(-ve) = nan */
	}
	else if (isnan(_x))
		return _x; /* log(nan) = nan */
	else if (_x > 0.0)
		return _x; /* log(inf) = inf */
	else {
		errno = EDOM;
		return ALIF_NAN; /* log(-inf) = nan */
	}
}

static AlifObject* math_gcd(AlifObject* _module, AlifObject* const* _args, AlifSizeT _nArgs) { // 723
	if (_nArgs == 2 and ALIFLONG_CHECKEXACT(_args[0]) and ALIFLONG_CHECKEXACT(_args[1]))
	{
		return alifLong_gcd(_args[0], _args[1]);
	}

	if (_nArgs == 0) {
		return alifLong_fromLong(0);
	}

	AlifObject* res = alifNumber_index(_args[0]);
	if (res ==  nullptr) {
		return nullptr;
	}
	if (_nArgs == 1) {
		ALIF_SETREF(res, alifNumber_absolute(res));
		return res;
	}

	AlifObject* one = _alifLong_getOne();  // borrowed ref
	for (AlifSizeT i = 1; i < _nArgs; i++) {
		AlifObject* x = alifNumber_index(_args[i]);
		if (x == nullptr) {
			ALIF_DECREF(res);
			return nullptr;
		}
		if (res == one) {
			ALIF_DECREF(x);
			continue;
		}
		ALIF_SETREF(res, alifLong_gcd(res, x));
		ALIF_DECREF(x);
		if (res == nullptr) {
			return nullptr;
		}
	}
	return res;
}


static AlifObject* long_lcm(AlifObject* _a, AlifObject* _b) { // 774
	AlifObject* g{}, * m{}, * f{}, * ab{};

	if (_alifLong_isZero((AlifLongObject*)_a) or _alifLong_isZero((AlifLongObject*)_b)) {
		return alifLong_fromLong(0);
	}
	g = alifLong_gcd(_a, _b);
	if (g == nullptr) {
		return nullptr;
	}
	f = alifNumber_floorDivide(_a, g);
	ALIF_DECREF(g);
	if (f == nullptr) {
		return nullptr;
	}
	m = alifNumber_multiply(f, _b);
	ALIF_DECREF(f);
	if (m == nullptr) {
		return nullptr;
	}
	ab = alifNumber_absolute(m);
	ALIF_DECREF(m);
	return ab;
}

static AlifObject* math_lcm(AlifObject* _module, AlifObject* const* _args, AlifSizeT _nArgs) { // 802
	AlifObject* res{}, * x{};
	AlifSizeT i{};

	if (_nArgs == 0) {
		return alifLong_fromLong(1);
	}
	res = alifNumber_index(_args[0]);
	if (res == nullptr) {
		return nullptr;
	}
	if (_nArgs == 1) {
		ALIF_SETREF(res, alifNumber_absolute(res));
		return res;
	}

	AlifObject* zero = _alifLong_getZero();  // borrowed ref
	for (i = 1; i < _nArgs; i++) {
		x = alifNumber_index(_args[i]);
		if (x == nullptr) {
			ALIF_DECREF(res);
			return nullptr;
		}
		if (res == zero) {
			/* Fast path: just check arguments.
			   It is okay to use identity comparison here. */
			ALIF_DECREF(x);
			continue;
		}
		ALIF_SETREF(res, long_lcm(res, x));
		ALIF_DECREF(x);
		if (res == nullptr) {
			return nullptr;
		}
	}
	return res;
}

static AlifObject* math_1(AlifObject* _arg,
	double (*_func) (double), AlifIntT _canOverFlow) { // 924

	double x{}, r{};
	x = alifFloat_asDouble(_arg);
	if (x == -1.0 and alifErr_occurred())
		return nullptr;
	errno = 0;
	r = (*_func)(x);
	if (isnan(r) and !isnan(x)) {
		//alifErr_setString(_alifExcValueError_,
			//"math domain error"); /* invalid arg */
		return nullptr;
	}
	if (isinf(r) and isfinite(x)) {
		//if (_canOverFlow)
			//alifErr_setString(_alifExcOverflowError_,
				//"math range error"); /* overflow */
		//else
			//alifErr_setString(_alifExcValueError_,
				//"math domain error"); /* singularity */
		//return nullptr;
	}
	//if (isfinite(r) and errno and is_error(r))
		/* this branch unnecessary on most platforms */
		//return nullptr;

	return alifFloat_fromDouble(r);
}

// 1033
#define FUNC1(_funcName, _func, _canOverflow, _docString)                  \
    static AlifObject * math_##_funcName(AlifObject *_self, AlifObject *_args) { \
        return math_1(_args, _func, _canOverflow);                            \
    }\
    ALIFDOC_STRVAR(math_##_funcName##_doc, _docString);



static AlifObject* math_ceil(AlifObject* _module, AlifObject* _number) { // 1091
	double x{};
	if (ALIFFLOAT_CHECKEXACT(_number)) {
		x = ALIFFLOAT_AS_DOUBLE(_number);
	}
	else {
		MathModuleState* state = getMath_moduleState(_module);
		AlifObject* method = alifObject_lookupSpecial(_number, state->str___ceil__);
		if (method != nullptr) {
			AlifObject* result = _alifObject_callNoArgs(method);
			ALIF_DECREF(method);
			return result;
		}
		if (alifErr_occurred())
			return nullptr;
		x = alifFloat_asDouble(_number);
		if (x == -1.0 and alifErr_occurred())
			return nullptr;
	}
	return alifLong_fromDouble(ceil(x));
}

static AlifObject* math_floor(AlifObject* _module, AlifObject* _number) { // 1161
	double x{};
	if (ALIFFLOAT_CHECKEXACT(_number)) {
		x = ALIFFLOAT_AS_DOUBLE(_number);
	}
	else {
		MathModuleState* state = getMath_moduleState(_module);
		AlifObject* method = alifObject_lookupSpecial(_number, state->str___floor__);
		if (method != nullptr) {
			AlifObject* result = _alifObject_callNoArgs(method);
			ALIF_DECREF(method);
			return result;
		}
		if (alifErr_occurred())
			return nullptr;
		x = alifFloat_asDouble(_number);
		if (x == -1.0 and alifErr_occurred())
			return nullptr;
	}
	return alifLong_fromDouble(floor(x));
}


FUNC1(cos, cos, 0 ,
	"cos($module, x, /)\n--\n\n"
	"Return the cosine of x (measured in radians).") // 1122

FUNC1(fabs, fabs, 0,
		"fabs($module, x, /)\n--\n\n"
		"Return the absolute value of the float x.")

FUNC1(sin, sin, 0,
		"sin($module, x, /)\n--\n\n"
		"Return the sine of x (measured in radians).") // 1202

FUNC1(tan, tan, 0,
		"tan($module, x, /)\n--\n\n"
		"Return the tangent of x (measured in radians).") // 1211


static unsigned long countSet_bits(unsigned long _n) { // 1447
	unsigned long count = 0;
	while (_n != 0) {
		++count;
		_n &= _n - 1; /* clear least significant bit */
	}
	return count;
}

static AlifObject* factorial_partialProduct(unsigned long _start, unsigned long _stop,
	unsigned long _maxBits) { // 1852
	unsigned long midpoint{}, numOperands{};
	AlifObject* left = nullptr, * right = nullptr, * result = nullptr;

	numOperands = (_stop - _start) / 2;
	if (numOperands <= 8 * SIZEOF_LONG and
		numOperands * _maxBits <= 8 * SIZEOF_LONG) {
		unsigned long j{}, total{};
		for (total = _start, j = _start + 2; j < _stop; j += 2)
			total *= j;
		return alifLong_fromUnsignedLong(total);
	}

	midpoint = (_start + numOperands) | 1;
	left = factorial_partialProduct(_start, midpoint,
		alifBit_length(midpoint - 2));
	if (left == nullptr)
		goto error;
	right = factorial_partialProduct(midpoint, _stop, _maxBits);
	if (right == nullptr)
		goto error;
	result = alifNumber_multiply(left, right);

error:
	ALIF_XDECREF(left);
	ALIF_XDECREF(right);
	return result;
}

static AlifObject* factorial_oddPart(unsigned long _n) { // 1905
	long i{};
	unsigned long v{}, lower{}, upper{};
	AlifObject* partial{}, * tmp{}, * inner{}, * outer{};

	inner = alifLong_fromLong(1);
	if (inner == nullptr)
		return nullptr;
	outer = ALIF_NEWREF(inner);

	upper = 3;
	for (i = alifBit_length(_n) - 2; i >= 0; i--) {
		v = _n >> i;
		if (v <= 2)
			continue;
		lower = upper;
		upper = (v + 1) | 1;
		partial = factorial_partialProduct(lower, upper, alifBit_length(upper - 2));
		/* inner *= partial */
		if (partial == nullptr)
			goto error;
		tmp = alifNumber_multiply(inner, partial);
		ALIF_DECREF(partial);
		if (tmp == nullptr)
			goto error;
		ALIF_SETREF(inner, tmp);

		tmp = alifNumber_multiply(outer, inner);
		if (tmp == nullptr)
			goto error;
		ALIF_SETREF(outer, tmp);
	}
	ALIF_DECREF(inner);
	return outer;

error:
	ALIF_DECREF(outer);
	ALIF_DECREF(inner);
	return nullptr;
}


static const unsigned long _alifSmallFactorials_[] = { // 1957
	1, 1, 2, 6, 24, 120, 720, 5040, 40320,
	362880, 3628800, 39916800, 479001600,
#if SIZEOF_LONG >= 8
	6227020800, 87178291200, 1307674368000,
	20922789888000, 355687428096000, 6402373705728000,
	121645100408832000, 2432902008176640000
#endif
};

static AlifObject* math_factorial(AlifObject* _module, AlifObject* _arg) { // 1979
	long x{}, twoValuation{};
	AlifIntT overflow{};
	AlifObject* result{}, * oddPart{};

	x = alifLong_asLongAndOverflow(_arg, &overflow);
	if (x == -1 and alifErr_occurred()) {
		return nullptr;
	}
	else if (overflow == 1) {
		//alifErr_format(_alifExcOverflowError_,
			//"factorial() argument should not exceed %ld",
			//LONG_MAX);
		return nullptr;
	}
	else if (overflow == -1 or x < 0) {
		//alifErr_setString(_alifExcValueError_,
			//"factorial() not defined for negative values");
		return nullptr;
	}

	if (x < (long)ALIF_ARRAY_LENGTH(_alifSmallFactorials_))
		return alifLong_fromUnsignedLong(_alifSmallFactorials_[x]);

	oddPart = factorial_oddPart(x);
	if (oddPart == nullptr)
		return nullptr;
	twoValuation = x - countSet_bits(x);
	result = _alifLong_lShift(oddPart, twoValuation);
	ALIF_DECREF(oddPart);
	return result;
}



static AlifObject* logHelper(AlifObject* _arg, double (*_func)(double)) { // 2182
	/* If it is int, do it ourselves. */
	if (ALIFLONG_CHECK(_arg)) {
		double x{}, result{};
		int64_t e{};

		/* Negative or zero inputs give a ValueError. */
		if (!_alifLong_isPositive((AlifLongObject*)_arg)) {
			//alifErr_setString(_alifExcValueError_,
				//"math domain error");
			return nullptr;
		}

		x = alifLong_asDouble(_arg);
		if (x == -1.0 and alifErr_occurred()) {
			//if (!alifErr_exceptionMatches(_alifExcOverflowError_))
				//return nullptr;
			/* Here the conversion to double overflowed, but it's possible
			   to compute the log anyway.  Clear the exception and continue. */
			//alifErr_clear();
			x = _alifLong_frexp((AlifLongObject*)_arg, &e);
			/* Value is ~= x * 2^e, so the log ~= log(x) + log(2) * e. */
			result = _func(x) + _func(2.0) * e;
		}
		else
			/* Successfully converted x to a double. */
			result = _func(x);
		return alifFloat_fromDouble(result);
	}

	/* Else let libm handle it by itself. */
	return math_1(_arg, _func, 0);
}
static AlifObject* math_log(AlifObject* _module, AlifObject* const* _args, AlifSizeT _nArgs) { // 2223
	AlifObject* num{}, * den{};
	AlifObject* ans{};

	if (!_ALIFARG_CHECKPOSITIONAL("لوغ", _nArgs, 1, 2))
		return nullptr;

	num = logHelper(_args[0], m_log);
	if (num == nullptr or _nArgs == 1)
		return num;

	den = logHelper(_args[1], m_log);
	if (den == nullptr) {
		ALIF_DECREF(num);
		return nullptr;
	}

	ans = alifNumber_trueDivide(num, den);
	ALIF_DECREF(num);
	ALIF_DECREF(den);
	return ans;
}


static inline double vector_norm(AlifSizeT _n, double* _vec, double _max, int _foundNan) { // 2474
	double x{}, h{}, scale{}, csum = 1.0, frac1 = 0.0, frac2 = 0.0;
	DoubleLength pr{}, sm{};
	AlifIntT maxE{};
	AlifSizeT i{};

	if (isinf(_max)) {
		return _max;
	}
	if (_foundNan) {
		return ALIF_NAN;
	}
	if (_max == 0.0 or _n <= 1) {
		return _max;
	}
	frexp(_max, (int*)&maxE);
	if (maxE < -1023) {
		/* When max_e < -1023, ldexp(1.0, -max_e) would overflow. */
		for (i = 0; i < _n; i++) {
			_vec[i] /= DBL_MIN;          // convert subnormals to normals
		}
		return DBL_MIN * vector_norm(_n, _vec, _max / DBL_MIN, _foundNan);
	}
	scale = ldexp(1.0, -maxE);
	for (i = 0; i < _n; i++) {
		x = _vec[i];
		x *= scale;                     // lossless scaling
		pr = dl_mul(x, x);              // lossless squaring
		sm = dl_fastSum(csum, pr.hi);  // lossless addition
		csum = sm.hi;
		frac1 += pr.lo;                 // lossy addition
		frac2 += sm.lo;                 // lossy addition
	}
	h = sqrt(csum - 1.0 + (frac1 + frac2));
	pr = dl_mul(-h, h);
	sm = dl_fastSum(csum, pr.hi);
	csum = sm.hi;
	frac1 += pr.lo;
	frac2 += sm.lo;
	x = csum - 1.0 + (frac1 + frac2);
	h += x / (2.0 * h);                 // differential correction
	return h / scale;
}

#define NUM_STACK_ELEMS 16 // 2525


static AlifObject* math_distImpl(AlifObject* _module, AlifObject* _p, AlifObject* _q) { // 2544
	AlifObject* item{};
	double max = 0.0;
	double x{}, px{}, qx{}, result{};
	AlifSizeT i{}, m{}, n{};
	AlifIntT foundNan = 0, pAllocated = 0, qAllocated = 0;
	double diffsOnStack[NUM_STACK_ELEMS];
	double* diffs = diffsOnStack;

	if (!ALIFTUPLE_CHECK(_p)) {
		_p = alifSequence_tuple(_p);
		if (_p == nullptr) {
			return nullptr;
		}
		pAllocated = 1;
	}
	if (!ALIFTUPLE_CHECK(_q)) {
		_q = alifSequence_tuple(_q);
		if (_q == nullptr) {
			if (pAllocated) {
				ALIF_DECREF(_p);
			}
			return nullptr;
		}
		qAllocated = 1;
	}

	m = ALIFTUPLE_GET_SIZE(_p);
	n = ALIFTUPLE_GET_SIZE(_q);
	if (m != n) {
		//alifErr_setString(_alifExcValueError_,
			//"both points must have the same number of dimensions");
		goto errorExit;
	}
	if (n > NUM_STACK_ELEMS) {
		diffs = (double*)alifMem_dataAlloc(n * sizeof(double));
		if (diffs == nullptr) {
			//alifErr_noMemory();
			goto errorExit;
		}
	}
	for (i = 0; i < n; i++) {
		item = ALIFTUPLE_GET_ITEM(_p, i);
		ASSIGN_DOUBLE(px, item, errorExit);
		item = ALIFTUPLE_GET_ITEM(_q, i);
		ASSIGN_DOUBLE(qx, item, errorExit);
		x = fabs(px - qx);
		diffs[i] = x;
		foundNan |= isnan(x);
		if (x > max) {
			max = x;
		}
	}
	result = vector_norm(n, diffs, max, foundNan);
	if (diffs != diffsOnStack) {
		alifMem_dataFree(diffs);
	}
	if (pAllocated) {
		ALIF_DECREF(_p);
	}
	if (qAllocated) {
		ALIF_DECREF(_q);
	}
	return alifFloat_fromDouble(result);

errorExit:
	if (diffs != diffsOnStack) {
		alifMem_dataFree(diffs);
	}
	if (pAllocated) {
		ALIF_DECREF(_p);
	}
	if (qAllocated) {
		ALIF_DECREF(_q);
	}
	return nullptr;
}

static const double _alifDegToRad_ = ALIF_MATH_PI / 180.0; // 3009
static const double _alifRadToDeg_ = 180.0 / ALIF_MATH_PI; // 3010

static AlifObject* math_degreesImpl(AlifObject* _module, double _x) { // 3022
	return alifFloat_fromDouble(_x * _alifRadToDeg_);
}

static AlifObject* math_radiansImpl(AlifObject* _module, double _x) { // 3039
	return alifFloat_fromDouble(_x * _alifDegToRad_);
}


static AlifMethodDef _alifMathMethods_[] = { // 4087
	MATH_CEIL_METHODDEF
	MATH_FLOOR_METHODDEF
	{"جيب",             math_sin,       METHOD_O},
	{"تجيب",            math_cos,       METHOD_O},
	{"ظل",              math_tan,       METHOD_O},
    MATH_DEGREES_METHODDEF
	MATH_DIST_METHODDEF
	{"قيمة_مطلقة",      math_fabs,      METHOD_O},
	MATH_FACTORIAL_METHODDEF
	{"قم_اكبر", ALIF_CPPFUNCTION_CAST(math_gcd),       METHOD_FASTCALL},
	{"قم_اصغر", ALIF_CPPFUNCTION_CAST(math_lcm),       METHOD_FASTCALL},
	{"لوغ",        ALIF_CPPFUNCTION_CAST(math_log),       METHOD_FASTCALL},
	MATH_RADIANS_METHODDEF
	{nullptr,              nullptr}           /* sentinel */
};



static class AlifModuleDef _alifMathModule_ = { // 4159
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "الرياضيات",
	.methods = _alifMathMethods_
};

AlifObject* alifInit_math(void) { // 4171
	return alifModuleDef_init(&_alifMathModule_);
}
