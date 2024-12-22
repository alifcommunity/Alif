#include "alif.h"











AlifTypeObject _alifMethodType_ = { // 332
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "صفة",
	.basicSize = sizeof(AlifMethodObject),
	//.dealloc = (Destructor)method_dealloc,
	.vectorCallOffset = offsetof(AlifMethodObject, vectorCall),
	//.repr = (ReprFunc)method_repr,
	//.hash = (HashFunc)method_hash,
	//.call = alifVectorCall_call,
	//.getAttro = method_getAttro,
	.setAttro = alifObject_genericSetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
				ALIF_TPFLAGS_HAVE_VECTORCALL,
	//.traverse = (TraverseProc)method_traverse,
	//.richCompare = method_richCompare,
	.weakListOffset = offsetof(AlifMethodObject, weakRefList),
	//.methods = method_methods,
	//.members = method_memberlist,
	//.getSet = method_getSet,
	//.descrGet = method_descrGet,
	//.new_ = method_new,
};
