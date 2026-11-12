
#include "AlifCore_ModSupport.h"


#define MATH_GCD_METHODDEF    \
    {"قم_اكبر", ALIF_CPPFUNCTION_CAST(math_gcd), METHOD_FASTCALL},

static AlifObject* math_gcdImpl(AlifObject*, AlifObject* const*, AlifSizeT);

static AlifObject* math_gcd(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nArgs) { // 23
	AlifObject* returnValue{};
	AlifObject* const* __clinicArgs{};
	AlifSizeT argsLength{};

	__clinicArgs = _args;
	argsLength = _nArgs;
	returnValue = math_gcdImpl(_module, __clinicArgs, argsLength);
	return returnValue;
}

#define MATH_LCM_METHODDEF    \
    {"قم_اصغر", ALIF_CPPFUNCTION_CAST(math_lcm), METHOD_FASTCALL},

static AlifObject* math_lcmImpl(AlifObject*, AlifObject* const*, AlifSizeT);

static AlifObject* math_lcm(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nArgs) { // 52
	AlifObject* returnValue{};
	AlifObject* const* __clinicArgs{};
	AlifSizeT argsLength{};

	__clinicArgs = _args;
	argsLength = _nArgs;
	returnValue = math_lcmImpl(_module, __clinicArgs, argsLength);

	return returnValue;
}





// 77
#define MATH_CEIL_METHODDEF    \
    {"حد_اعلى", (AlifCPPFunction)math_ceil, METHOD_O},

// 88
#define MATH_FLOOR_METHODDEF    \
    {"حد_ادنى", (AlifCPPFunction)math_floor, METHOD_O},


// 119
#define MATH_FACTORIAL_METHODDEF    \
    {"المضروب", (AlifCPPFunction)math_factorial, METHOD_O},

// 388
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

