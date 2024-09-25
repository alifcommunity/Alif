#include "alif.h"

#include "AlifCore_Eval.h"
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
	for (AlifSizeT i_ = 0; i_ < _size; i_++) {
		op_->item[i_] = nullptr;
	}
	ALIFOBJECT_GC_TRACK(op_);
	return (AlifObject*)op_;
}

AlifObject* alifTuple_pack(AlifSizeT _n, ...) { // 153
	AlifSizeT i_{};
	AlifObject* o_{};
	AlifObject** items{};
	va_list vargs{};

	if (_n == 0) {
		return tuple_getEmpty();
	}

	va_start(vargs, _n);
	AlifTupleObject* result = tuple_alloc(_n);
	if (result == nullptr) {
		va_end(vargs);
		return nullptr;
	}
	items = result->item;
	for (i_ = 0; i_ < _n; i_++) {
		o_ = va_arg(vargs, AlifObject*);
		items[i_] = ALIF_NEWREF(o_);
	}
	va_end(vargs);
	ALIFOBJECT_GC_TRACK(result);
	return (AlifObject*)result;
}





AlifObject* alifTuple_fromArray(AlifObject* const* _src, AlifSizeT _n) { // 371
	if (_n == 0) {
		return tuple_getEmpty();
	}

	AlifTupleObject* tuple = tuple_alloc(_n);
	if (tuple == nullptr) {
		return nullptr;
	}
	AlifObject** dst = tuple->item;
	for (AlifSizeT i_ = 0; i_ < _n; i_++) {
		AlifObject* item = _src[i_];
		dst[i_] = ALIF_NEWREF(item);
	}
	ALIFOBJECT_GC_TRACK(tuple);
	return (AlifObject*)tuple;
}






AlifTypeObject _alifTupleType_ = { // 865
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مترابطة",
	.basicSize = sizeof(AlifTupleObject) - sizeof(AlifObject*),
	.itemSize = sizeof(AlifObject*),

	.getAttro = alifObject_genericGetAttr,

	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_TUPLE_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF | ALIF_TPFLAGS_SEQUENCE,
};


AlifIntT alifTuple_resize(AlifObject** _pv, AlifSizeT _newSize) { // 918
	AlifTupleObject* v_{};
	AlifTupleObject* sv_{};
	AlifSizeT i_{};
	AlifSizeT oldSize{};

	v_ = (AlifTupleObject*)*_pv;
	if (v_ == nullptr or !ALIF_IS_TYPE(v_, &_alifTupleType_) ||
		(ALIF_SIZE(v_) != 0 and ALIF_REFCNT(v_) != 1)) {
		*_pv = 0;
		ALIF_XDECREF(v_);
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	oldSize = ALIF_SIZE(v_);
	if (oldSize == _newSize) {
		return 0;
	}
	if (_newSize == 0) {
		ALIF_DECREF(v_);
		*_pv = tuple_getEmpty();
		return 0;
	}
	if (oldSize == 0) {
		ALIF_DECREF(v_);
		*_pv = alifTuple_new(_newSize);
		return *_pv == nullptr ? -1 : 0;
	}

	if (ALIFOBJECT_GC_IS_TRACKED(v_)) {
		ALIFOBJECT_GC_UNTRACK(v_);
	}
	for (i_ = _newSize; i_ < oldSize; i_++) {
		ALIF_CLEAR(v_->item[i_]);
	}
	sv_ = ALIFOBJECT_GC_RESIZE(AlifTupleObject, v_, _newSize);
	if (sv_ == nullptr) {
		*_pv = nullptr;
		alifObject_gcDel(v_);
		return -1;
	}
	alif_newReferenceNoTotal((AlifObject*)sv_);
	if (_newSize > oldSize)
		memset(&sv_->item[oldSize], 0,
			sizeof(*sv_->item) * (_newSize - oldSize));
	*_pv = (AlifObject*)sv_;
	ALIFOBJECT_GC_TRACK(sv_);
	return 0;
}
