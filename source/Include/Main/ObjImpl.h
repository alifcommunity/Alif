#pragma once


AlifObject* alifObject_new(AlifTypeObject*);

#define ALIFOBJECT_NEW(_type, _typeObj) ((_type *)alifObject_new(_typeObj))



static inline size_t alifSubObject_varSize(AlifTypeObject* _type, int64_t _nItems) {
    size_t size_ =  _type->basicSize;
    size_ +=  _nItems * _type->itemsSize;
    return ALIFSIZE_ROUND_UP(size_, SIZEOF_VOID_P);
}








/*
 * Garbage Collection
 * ==================
 */
AlifObject* alifObjectGC_new(AlifTypeObject*);

#define ALIFOBJECT_GC_NEW(_type, _typeObj) (_type*)(alifObjectGC_new(_typeObj))
