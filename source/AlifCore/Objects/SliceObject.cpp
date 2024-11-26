#include "alif.h"



AlifTypeObject _alifEllipsisType_ = { // 66
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "إضمار",
	.basicSize = 0,
	.itemSize = 0,

	.getAttro = alifObject_genericGetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT,
};

AlifObject _alifEllipsisObject_ = ALIFOBJECT_HEAD_INIT(&_alifEllipsisType_); // 107
