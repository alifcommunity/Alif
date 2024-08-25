#pragma once


static inline AlifUSizeT alifObject_size(AlifTypeObject* _type) { // 5
	return ALIF_STATIC_CAST(AlifUSizeT, _type->basicSize);
}





AlifIntT alifObject_isGC(AlifObject*); // 78









/* -------------------------------------------------------------------------------------------------------------------------- */

#define ALIFTYPE_IS_GC(_t) alifType_hasFeature(_t, ALIF_TPFLAGS_HAVE_GC) // 157


AlifObject* alifObject_gcNew(AlifTypeObject* _tp); // 165

#define ALIFOBJECT_GC_NEW(_type, _typeObj) ALIF_CAST(_type*, alifObject_gcNew(_typeObj)) // 180
