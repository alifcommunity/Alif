#include "alif.h"

#include "AlifCore_UnionObject.h"






class UnionObject { // 11
public:
	ALIFOBJECT_HEAD;
	AlifObject* args{};
	AlifObject* parameters{};
};














AlifObject* _alifUnion_args(AlifObject* _self) { // 302
	return ((UnionObject*)_self)->args;
}

AlifTypeObject _alifUnionType_ = { // 309
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "نوع.اتحاد",
	.basicSize = sizeof(UnionObject),
	//.dealloc = unionObject_dealloc,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	.alloc = alifType_genericAlloc,
	.free = alifObject_gcDel,
};
