#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_GC.h"
#include "AlifCore_Object.h"




static AlifTupleObject* tuple_alloc(AlifSizeT _size) { // 34
	if (_size < 0) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	AlifSizeT index = _size - 1;
	if (index < ALIFTUPLE_MAXSAVESIZE) {
		AlifTupleObject* op_ = ALIF_FREELIST_POP(AlifTupleObject, tuples[index]);
		if (op_ != nullptr) {
			return op_;
		}
	}
	if ((AlifUSizeT)_size > ((AlifUSizeT)ALIF_SIZET_MAX - (sizeof(AlifTupleObject) -
		sizeof(AlifObject*))) / sizeof(AlifObject*)) {
		return nullptr;
		//return (AlifTupleObject*)alifErr_noMemory();
	}
	return ALIFOBJECT_GC_NEWVAR(AlifTupleObject, &_alifTupleType_, _size);
}

static inline AlifObject* tuple_getEmpty(void) { // 62
	return (AlifObject*)&ALIF_SINGLETON(tupleEmpty);
}

AlifObject* alifTuple_new(AlifSizeT _size) { // 68
	AlifTupleObject* op_{};
	if (_size == 0) {
		return tuple_getEmpty();
	}
	op_ = tuple_alloc(_size);
	if (op_ == nullptr) {
		return nullptr;
	}
	for (AlifSizeT i = 0; i < _size; i++) {
		op_->item[i] = nullptr;
	}
	ALIFOBJECT_GC_TRACK(op_);
	return (AlifObject*)op_;
}

AlifObject* alifTuple_pack(AlifSizeT _n, ...) { // 153
	AlifSizeT i;
	AlifObject* o;
	AlifObject** items;
	va_list vargs;

	if (_n == 0) {
		return tuple_getEmpty();
	}

	va_start(vargs, _n);
	AlifTupleObject* result = tuple_alloc(_n);
	if (result == NULL) {
		va_end(vargs);
		return NULL;
	}
	items = result->item;
	for (i = 0; i < _n; i++) {
		o = va_arg(vargs, AlifObject*);
		items[i] = ALIF_NEWREF(o);
	}
	va_end(vargs);
	ALIFOBJECT_GC_TRACK(result);
	return (AlifObject*)result;
}



AlifTypeObject _alifTupleType_ = { // 865
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مترابطة",
	.basicSize = sizeof(AlifTupleObject) - sizeof(AlifObject*),
	.itemSize = sizeof(AlifObject*),
};
