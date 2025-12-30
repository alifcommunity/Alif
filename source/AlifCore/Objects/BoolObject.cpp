#include "alif.h"

#include "AlifCore_Long.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"




static AlifObject* bool_repr(AlifObject* _self) { // 13
	return _self == ALIF_TRUE ? &ALIF_STR(True) : &ALIF_STR(False);
}


AlifObject* alifBool_fromLong(long _ok) { // 21
	return _ok ? ALIF_TRUE : ALIF_FALSE;
}



static AlifObject* bool_vectorCall(AlifObject* _type, AlifObject* const* _args,
	AlifUSizeT _nargsf, AlifObject* _kwnames) { // 44
	long ok = 0;
	if (!_ALIFARG_NOKWNAMES("منطق", _kwnames)) {
		return nullptr;
	}

	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	if (!_ALIFARG_CHECKPOSITIONAL("منطق", nargs, 0, 1)) {
		return nullptr;
	}

	if (nargs) {
		ok = alifObject_isTrue(_args[0]);
		if (ok < 0) {
			return nullptr;
		}
	}
	return alifBool_fromLong(ok);
}

static AlifObject* bool_and(AlifObject* _a, AlifObject* _b) { // 86
	if (!ALIFBOOL_CHECK(_a) or !ALIFBOOL_CHECK(_b))
		return _alifLongType_.asNumber->and_(_a, _b);
	return alifBool_fromLong((_a == ALIF_TRUE) & (_b == ALIF_TRUE));
}

static AlifObject* bool_or(AlifObject* _a, AlifObject* _b) {
	if (!ALIFBOOL_CHECK(_a) or !ALIFBOOL_CHECK(_b))
		return _alifLongType_.asNumber->or_(_a, _b);
	return alifBool_fromLong((_a == ALIF_TRUE) | (_b == ALIF_TRUE));
}

static AlifObject* bool_xor(AlifObject* _a, AlifObject* _b) { // 102
	if (!ALIFBOOL_CHECK(_a) or !ALIFBOOL_CHECK(_b))
		return _alifLongType_.asNumber->xor_(_a, _b);
	return alifBool_fromLong((_a == ALIF_TRUE) ^ (_b == ALIF_TRUE));
}

static AlifNumberMethods _boolAsNumber_ = { // 122
	//.invert = (UnaryFunc)bool_invert, // depricated
	.and_ = bool_and,
	.xor_ = bool_xor,
	.or_ = bool_or,
};

AlifTypeObject _alifBoolType_ = { // 171
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "منطق",
	.basicSize = offsetof(AlifLongObject, longValue.digit),
	.itemSize = sizeof(digit),
	.repr = bool_repr,
	.asNumber = &_boolAsNumber_,
	.flags = ALIF_TPFLAGS_DEFAULT,
	.base = &_alifLongType_,
	.vectorCall = bool_vectorCall,
};

AlifLongObject _alifFalseClass_ = { // 215
	ALIFOBJECT_HEAD_INIT(&_alifBoolType_),
	{
		.tag = ALIFLONG_FALSE_TAG,
		.digit = {0},
	}
};

AlifLongObject _alifTrueClass_ = { // 222
	ALIFOBJECT_HEAD_INIT(&_alifBoolType_),
	{
		.tag = ALIFLONG_TRUE_TAG,
		.digit = {1},
	}
};
