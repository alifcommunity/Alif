#pragma once


static inline size_t alifObject_size(AlifTypeObject* _type) { // 5
	return (size_t )_type->basicSize;
}





AlifIntT alifObject_isGC(AlifObject*); // 78











#define ALIFTYPE_IS_GC(_t) alifType_hasFeature(_t, ALIF_TPFLAGS_HAVE_GC) // 157


AlifObject* alifObject_gcNew(AlifTypeObject* _tp); // 165

#define ALIFOBJECT_GC_New(type, typeobj) ALIF_CAST(type*, alifObject_gcNew(typeobj)) 
