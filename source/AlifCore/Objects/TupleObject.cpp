#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_GC.h"
#include "AlifCore_Object.h"




static AlifTupleObject* tuple_alloc(AlifSizeT _size) { // 34
	if (_size < 0) {
		//alifErr_badInternalCall();
		return NULL;
	}
	AlifSizeT index = _size - 1;
	if (index < ALIFTUPLE_MAXSAVESIZE) {
		AlifTupleObject* op_ = ALIF_FREELIST_POP(AlifTupleObject, tuples[index]);
		if (op_ != NULL) {
			return op_;
		}
	}
	if ((size_t)_size > ((size_t)LLONG_MAX - (sizeof(AlifTupleObject) -
		sizeof(AlifObject*))) / sizeof(AlifObject*)) {
		return nullptr;
		//return (AlifTupleObject*)alifErr_noMemory();
	}
	return ALIFOBJECT_GC_NEWVAR(AlifTupleObject, &_alifTupleType_, _size);
}

AlifObject* alifTuple_new(AlifSizeT _size) { // 68
	AlifTupleObject* op_;
	if (_size == 0) {
		return tuple_get_empty();
	}
	op_ = tuple_alloc(_size);
	if (op_ == NULL) {
		return NULL;
	}
	for (AlifSizeT i = 0; i < _size; i++) {
		op_->item[i] = NULL;
	}
	ALIFOBJECT_GC_TRACK(op_);
	return (AlifObject*)op_;
}


AlifTypeObject _alifTupleType_ = { // 865
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مترابطة",
	.basicSize = sizeof(AlifTupleObject) - sizeof(AlifObject*),
	.itemSize = sizeof(AlifObject*),
};
