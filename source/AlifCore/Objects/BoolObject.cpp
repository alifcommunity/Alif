#include "alif.h"

#include "AlifCore_Object.h"





AlifTypeObject _alifBoolType_ = { // 171
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "bool",
	.basicSize = offsetof(struct AlifLongObject, longValue.digit),  /* tp_basicsize */
	.itemSize = sizeof(uint32_t),                              /* tp_itemsize */
};

class AlifLongObject _alifFalseClass_ = {
	ALIFOBJECT_HEAD_INIT(&_alifBoolType_),
	{
		.tag = _PyLong_TRUE_TAG,
		.digit = { 0 }
	}
}
};

class AlifLongObject _alifTrueClass_ = {
	ALIFOBJECT_HEAD_INIT(&_alifBoolType_),
	{
		.tag = _PyLong_TRUE_TAG,
		.digit = { 1 }
	}
};
