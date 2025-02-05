#include "alif.h"





class AlifNamespaceObject { // 10
public:
	ALIFOBJECT_HEAD{};
	AlifObject* dict{};
};



static AlifObject* namespace_new(AlifTypeObject* type,
	AlifObject* args, AlifObject* kwds) { // 24
	AlifObject* self{};

	self = type->alloc(type, 0);
	if (self != nullptr) {
		AlifNamespaceObject* ns = (AlifNamespaceObject*)self;
		ns->dict = alifDict_new();
		if (ns->dict == nullptr) {
			ALIF_DECREF(ns);
			return nullptr;
		}
	}
	return self;
}















AlifTypeObject _alifNamespaceType_ = { // 252
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "أنواع.نطاق_اسماء",
	.basicSize = sizeof(AlifNamespaceObject),
	//.dealloc = (Destructor)namespace_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.setAttro = alifObject_genericSetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE,
	//.methods = namespace_methods,
	.dictOffset = offsetof(AlifNamespaceObject, dict),
	//.init = (InitProc)namespace_init,
	.alloc = alifType_genericAlloc,
	.new_ = (NewFunc)namespace_new,
	.free = alifObject_gcDel,
};










AlifObject* alifNamespace_new(AlifObject* kwds) { // 296
	AlifObject* ns = namespace_new(&_alifNamespaceType_, nullptr, nullptr);
	if (ns == nullptr)
		return nullptr;

	if (kwds == nullptr)
		return ns;
	if (alifDict_update(((AlifNamespaceObject*)ns)->dict, kwds) != 0) {
		ALIF_DECREF(ns);
		return nullptr;
	}

	return (AlifObject*)ns;
}

