#pragma once


static inline AlifUSizeT alifObject_size(AlifTypeObject* _type) { // 5
	return ALIF_STATIC_CAST(AlifUSizeT, _type->basicSize);
}





AlifIntT alifObject_isGC(AlifObject*); // 78









/* -------------------------------------------------------------------------------------------------------------------------- */

static inline size_t alifObject_varSize(AlifTypeObject* _type, AlifSizeT _nItems) { // 23
	size_t size = ALIF_STATIC_CAST(size_t, _type->basicSize);
	size += ALIF_STATIC_CAST(size_t, _nItems) * ALIF_STATIC_CAST(size_t, _type->itemSize);
	return ALIF_SIZE_ROUND_UP(size, SIZEOF_VOID_P);
}


#define ALIFTYPE_IS_GC(_t) alifType_hasFeature(_t, ALIF_TPFLAGS_HAVE_GC) // 157


AlifObject* alifObject_gcNew(AlifTypeObject* _tp); // 165
AlifVarObject* alifObject_gcNewVar(AlifTypeObject*, AlifSizeT); // 166


#define ALIFOBJECT_GC_NEW(_type, _typeObj) ALIF_CAST(_type*, alifObject_gcNew(_typeObj)) // 180
#define ALIFOBJECT_GC_NEWVAR(_type, _typeObj, _n) ALIF_CAST(_type*, alifObject_gcNewVar((_typeObj), (_n))) // 181
