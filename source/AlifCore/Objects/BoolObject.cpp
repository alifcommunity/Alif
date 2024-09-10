#include "alif.h"

#include "AlifCore_Object.h"





AlifTypeObject _alifBoolType_ = { // 171
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "منطق",
	.basicSize = offsetof(AlifLongObject, longValue.digit),
	.itemSize = sizeof(digit),
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
