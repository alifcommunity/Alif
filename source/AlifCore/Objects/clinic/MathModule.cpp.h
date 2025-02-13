
#include "AlifCore_ModSupport.h"

// 19
#define MATH_CEIL_METHODDEF    \
    {"حد_اعلى", (AlifCPPFunction)math_ceil, METHOD_O},

// 30
#define MATH_FLOOR_METHODDEF    \
    {"حد_ادنى", (AlifCPPFunction)math_floor, METHOD_O},


// 61
#define MATH_FACTORIAL_METHODDEF    \
    {"المضروب", (AlifCPPFunction)math_factorial, METHOD_O},

// 330
#define MATH_DIST_METHODDEF    \
    {"مسافة", ALIF_CPPFUNCTION_CAST(math_dist), METHOD_FASTCALL},

static AlifObject* math_distImpl(AlifObject* , AlifObject* , AlifObject* ); // 334

static AlifObject* math_dist(AlifObject* _module, AlifObject* const* _args, AlifSizeT _nArgs) { // 337
	AlifObject* returnValue = nullptr;
	AlifObject* p{};
	AlifObject* q{};

	if (!_ALIFARG_CHECKPOSITIONAL("مسافة", _nArgs, 2, 2)) {
		goto exit;
	}
	p = _args[0];
	q = _args[1];
	returnValue = math_distImpl(_module, p, q);

exit:
	return returnValue;
}

// 445
#define MATH_DEGREES_METHODDEF    \
    {"درجة", (AlifCPPFunction)math_degrees, METHOD_O},

static AlifObject* math_degreesImpl(AlifObject* , double ); // 449

static AlifObject* math_degrees(AlifObject* _module, AlifObject* _arg) { // 452
	AlifObject* returnValue = nullptr;
	double x{};

	if (ALIFFLOAT_CHECKEXACT(_arg)) {
		x = ALIFFLOAT_AS_DOUBLE(_arg);
	}
	else
	{
		x = alifFloat_asDouble(_arg);
		if (x == -1.0 and alifErr_occurred()) {
			goto exit;
		}
	}
	returnValue = math_degreesImpl(_module, x);

exit:
	return returnValue;
}

// 479
#define MATH_RADIANS_METHODDEF    \
    {"راديان", (AlifCPPFunction)math_radians, METHOD_O},


static AlifObject* math_radiansImpl(AlifObject* , double ); // 483

static AlifObject* math_radians(AlifObject* _module, AlifObject* _arg) { // 486
	AlifObject* returnValue = nullptr;
	double x{};

	if (ALIFFLOAT_CHECKEXACT(_arg)) {
		x = ALIFFLOAT_AS_DOUBLE(_arg);
	}
	else
	{
		x = alifFloat_asDouble(_arg);
		if (x == -1.0 and alifErr_occurred()) {
			goto exit;
		}
	}
	returnValue = math_radiansImpl(_module, x);

exit:
	return returnValue;
}

