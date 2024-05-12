#pragma once

static inline size_t alifSubObject_varSize(AlifTypeObject* _type, int64_t _nItems) {
    size_t size_ =  _type->basicSize;
    size_ +=  _nItems * _type->itemsSize;
    return ALIFSIZE_ROUND_UP(size_, SIZEOF_VOID_P);
}
