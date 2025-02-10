#include "alif.h"


static AlifObject* math_1(AlifObject* _arg, double (*_func) (double), AlifIntT _canOverflow,
	const char* _errMsg) { // 934
	double x{}, r{};
	x = alifFloat_asDouble(_arg);
	if (x == -1.0 and alifErr_occurred())
		return nullptr;
	errno = 0;
	r = (*_func)(x);
	if (isnan(r) and !isnan(x))
		goto domain_err; /* domain error */
	if (isinf(r) and isfinite(x)) {
		//if (_canOverflow)
			//alifErr_setString(_alifExcOverflowError_,
				//"math range error"); /* overflow */
		//else
			//goto domain_err; /* singularity */
		return nullptr;
	}
	if (isfinite(r) and errno
		// and is_error(r, 1)
		)
		return nullptr;

	return alifFloat_fromDouble(r);

domain_err:
	if (_errMsg) {
		char* buf = alifOS_doubleToString(x, 'r', 0, ALIF_DTSF_ADD_DOT_0, nullptr);
		if (buf) {
			//alifErr_format(_alifExcValueError_, _errMsg, buf);
			alifMem_dataFree(buf);
		}
	}
	else {
		//alifErr_setString(_alifExcValueError_, "math domain error");
	}
	return nullptr;
}

// 1062
#define FUNC1(_funcName, _func, _canOverflow, _docString)                  \
    static AlifObject * math_##_funcName(AlifObject *self, AlifObject *args) { \
        return math_1(args, _func, _canOverflow, nullptr);                  \
    }\
    ALIFDOC_STRVAR(math_##_funcName##_doc, _docString);

// 1068
#define FUNC1D(_funcName, _func, _canOverflow, _docString, _errMsg)        \
    static AlifObject * math_##_funcName(AlifObject *self, AlifObject *args) { \
        return math_1(args, _func, _canOverflow, _errMsg);               \
    }\
    ALIFDOC_STRVAR(math_##_funcName##_doc, _docString);


FUNC1(cos, cos, 0,
	"cos($module, x, /)\n--\n\n"
	"Return the cosine of x (measured in radians).") // 1164

FUNC1(sin, sin, 0,
		"sin($module, x, /)\n--\n\n"
		"Return the sine of x (measured in radians).") // 1245

FUNC1D(sqrt, sqrt, 0,
	"sqrt($module, x, /)\n--\n\n"
	"Return the square root of x.",
	"expected a nonnegative input, got %s") // 1251

FUNC1(tan, tan, 0,
		"tan($module, x, /)\n--\n\n"
		"Return the tangent of x (measured in radians).") // 1255

static AlifMethodDef _alifMathMethods_[] = { // 4135
	{"الجذر_التربيعي",            math_sqrt,      METHOD_O},
	{"التجيب",             math_cos,       METHOD_O},
	{"الجيب",             math_sin,       METHOD_O},
	{"الظل",             math_tan,       METHOD_O},
	{nullptr,              nullptr}           /* sentinel */
};

static class AlifModuleDef _alifMathModule_ = { // 4207
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = "الرياضيات",
	.methods = _alifMathMethods_
};

AlifObject* alifInit_math(void) { // 4219
	return alifModuleDef_init(&_alifMathModule_);
}
