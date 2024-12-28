#include "alif.h"













AlifTypeObject _alifCellType_ = { // 151
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "خلية",
	.basicSize = sizeof(AlifCellObject),
	//.dealloc = (Destructor)cell_dealloc,
	//.repr = (ReprFunc)cell_repr,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
};
