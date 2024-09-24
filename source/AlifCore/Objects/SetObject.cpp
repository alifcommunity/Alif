#include "alif.h"




AlifTypeObject _alifSetType_ = { // 24449
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "set",                           
	.basicSize = sizeof(AlifSetObject),                
};



AlifTypeObject _alifFrozenSetType_ = { // 2539
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "frozenset",
	.basicSize = sizeof(AlifSetObject),              
};
