#include "alif.h"

#include "AlifCore_Long.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"




static AlifObject* bool_repr(AlifObject* _self) { // 13
	return _self == ALIF_TRUE ? &ALIF_ID(خطأ) : &ALIF_ID(صح);
}


AlifObject* alifBool_fromLong(long _ok) { // 21
	return _ok ? ALIF_TRUE : ALIF_FALSE;
}







AlifTypeObject _alifBoolType_ = { // 171
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "منطق",
	.basicSize = offsetof(AlifLongObject, longValue.digit),
	.itemSize = sizeof(digit),
	.repr = bool_repr,
	.flags = ALIF_TPFLAGS_DEFAULT,
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
