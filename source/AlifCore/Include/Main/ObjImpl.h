#pragma once


static inline AlifUSizeT alifObject_size(AlifTypeObject* _type) { // 5
	return ALIF_STATIC_CAST(AlifUSizeT, _type->basicSize);
}

AlifIntT alifObject_isGC(AlifObject*); // 78

AlifObject* alifObject_init(AlifObject*, AlifTypeObject*); // 117
AlifVarObject* alifObject_initVar(AlifVarObject*, AlifTypeObject*, AlifSizeT); // 118

AlifObject* alifObject_new(AlifTypeObject*); // 127

#define ALIFOBJECT_NEW(_type, _typeObj) ((_type *)alifObject_new(_typeObj)) // 130


#define ALIFTYPE_IS_GC(_t) alifType_hasFeature(_t, ALIF_TPFLAGS_HAVE_GC) // 157

AlifObject* alifObject_gcNew(AlifTypeObject* _tp); // 165
AlifVarObject* alifObject_gcNewVar(AlifTypeObject*, AlifSizeT); // 166

void alifObject_gcTrack(void*);

#define ALIFOBJECT_GC_NEW(_type, _typeObj) ALIF_CAST(_type*, alifObject_gcNew(_typeObj)) // 180
#define ALIFOBJECT_GC_NEWVAR(_type, _typeObj, _n) ALIF_CAST(_type*, alifObject_gcNewVar((_typeObj), (_n))) // 181












/* -------------------------------------------------------------------------------------------------------------------------- */






static inline AlifUSizeT alifObject_varSize(AlifTypeObject* _type, AlifSizeT _nItems) { // 23
	AlifUSizeT size = ALIF_STATIC_CAST(AlifUSizeT, _type->basicSize);
	size += ALIF_STATIC_CAST(AlifUSizeT, _nItems) * ALIF_STATIC_CAST(AlifUSizeT, _type->itemSize);
	return ALIF_SIZE_ROUND_UP(size, SIZEOF_VOID_P);
}




