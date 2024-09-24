#include "alif.h"




AlifTypeObject _alifSetType_ = { // 2449
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مميزة",                           
	.basicSize = sizeof(AlifSetObject),
	.itemSize = 0,
};



AlifTypeObject _alifFrozenSetType_ = { // 2539
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مميزة_مجمدة",
	.basicSize = sizeof(AlifSetObject),
	.itemSize = 0,
};
