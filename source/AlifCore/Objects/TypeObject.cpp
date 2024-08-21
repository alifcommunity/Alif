#include "alif.h"


























AlifTypeObject _alifTypeType_ = { // 6195
	.base = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نوع",
	.basicSize = sizeof(AlifHeapTypeObject),
	.itemSize = sizeof(AlifMemberDef),
};
