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
























































AlifTypeObject _alifSliceType_ = { // 658
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "قطع",
	.basicSize = sizeof(AlifSliceObject),
	//(Destructor)slice_dealloc,
	//(ReprFunc)slice_repr,
	//(HashFunc)slicehash,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//(TraverseProc)slice_traverse,
	//slice_richcompare,
	//slice_methods,
	//slice_members,
	//slice_new,
};
