#include "alif.h"



AlifTypeObject _alifEllipsisType_ = { // 66
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "ellipsis",                   
};

AlifObject _alifEllipsisObject_ = ALIFOBJECT_HEAD_INIT(&_alifEllipsisType_); // 107
