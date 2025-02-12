
#include "AlifCore_ModSupport.h"


// in file MathModule.c.h line 61
#define MATH_FACTORIAL_METHODDEF    \
    {"المضروب", (AlifCPPFunction)math_factorial, METHOD_O},


#define MATH_RADIANS_METHODDEF    \
    {"راديان", (AlifCPPFunction)math_radians, METHOD_O},


static AlifObject* math_radiansImpl(AlifObject* , double );

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

