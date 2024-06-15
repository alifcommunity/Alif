#include "alif.h"

#include "AlifCore_Dict.h"
#include "alifCore_List.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_Tuple.h"
#include "AlifCore_Memory.h"


static int list_resize(AlifListObject* _list, size_t _newSize) {

	size_t newAllocated{}, targetBytes{};
	uint64_t allocated_ = _list->allocate_;

	if (allocated_ >= _newSize && _newSize >= (allocated_ >> 1)) {
		ALIFSET_SIZE(_list, _newSize);
		return 0;
	}

	//The growth pattern is : 0, 4, 8, 16, 24, 32, 40, 52, 64, ...
	newAllocated = ((size_t)_newSize + (_newSize >> 3) + 6) & ~(size_t)3;

	if (_newSize - ALIF_SIZE(_list) > (int64_t)(newAllocated - _newSize))
		newAllocated = ((size_t)_newSize + 3) & ~(size_t)3;

	if (_newSize == 0) {
		newAllocated = 0;
	}

	AlifObject** items_{};
	if (_list->items_ == nullptr) {
		items_ = (AlifObject**)alifMem_dataAlloc(sizeof(AlifObject*));
	}
	else if (newAllocated <= (size_t)LLONG_MAX / sizeof(AlifObject*)) {
		targetBytes = newAllocated * sizeof(AlifObject*);
		items_ = (AlifObject**)alifMem_objRealloc(_list->items_, targetBytes);
	}
	else {
		items_ = nullptr;
	}
	if (items_ == nullptr) {
		return -1;
	}
	_list->items_ = items_;
	ALIFSET_SIZE(_list, _newSize);
	_list->allocate_ = newAllocated;
	return true;
}

static int list_preallocate_exact(AlifListObject* _self, int64_t _size)
{
	_size = (_size + 1) & ~(size_t)1;

	AlifObject** items_ = (AlifObject**)alifMem_dataAlloc(_size);
	if (items_ == nullptr) {
		return -1;
	}
	_self->items_ = items_;
	_self->allocate_ = _size;
	return 0;
}

AlifObject* alifNew_list(size_t _size) {

	if (_size < 0) {
		return nullptr;
	}

	AlifListObject* object = ALIFOBJECT_GC_NEW(AlifListObject, &_alifListType_);
	if (object == nullptr) {
		return nullptr;
	}

	if (_size <= 0) {
		object->items_ = nullptr;
	}
	else {
		object->items_ = (AlifObject**)alifMem_objAlloc(_size * sizeof(AlifObject*));
		if (object == nullptr) {
			ALIF_DECREF(object);
			return nullptr;
		}

	}
	ALIFSET_SIZE(object, _size);
	object->allocate_ = _size;
	ALIFSUBObject_GC_TRACK(object);
	return (AlifObject*)object;
}

static AlifObject* list_new_prealloc(int64_t _size)
{
	AlifListObject* op_ = (AlifListObject*)alifNew_list(0);
	if (op_ == nullptr) {
		return nullptr;
	}
	op_->items_ = (AlifObject**)alifMem_dataAlloc(_size);
	if (op_->items_ == nullptr) {
		ALIF_DECREF(op_);
		return nullptr;
	}
	op_->allocate_ = _size;
	return (AlifObject*)op_;
}

size_t alifList_size(AlifObject* _list) {

	return ALIF_SIZE(_list);

}

static inline int valid_index(size_t index, size_t limit)
{
	return (size_t)index < (size_t)limit;
}

static inline AlifObject* listGet_itemRef(AlifListObject* _op, int64_t _i)
{
	if (!valid_index(_i, ALIF_SIZE(_op))) {
		return NULL;
	}
	return ALIF_NEWREF(ALIFLIST_GET_ITEM(_op, _i));
}

AlifObject* alifList_getItem(AlifObject* _op, size_t _i) {

	if (!(_op->type_ == &_alifListType_)) {
		return nullptr;
	}
	if (!valid_index(_i, ALIF_SIZE(_op))) {
		//ALIFSUB_DECLARE_STR(list_err, L"list index out of range");
		//ALIFErr_SetObject(alifExc_IndexError, &ALIFSUB_STR(list_err));
		return nullptr;
	}
	return ((AlifListObject*)_op)->items_[_i];
}

#define ALIFLIST_GETITEM(_list, _index) (((AlifListObject*)_list)->items_[(_index)])

static int ins1(AlifListObject* _self, int64_t _where, AlifObject* _v)
{
	int64_t i_, n_ = ALIF_SIZE(_self);
	AlifObject** items_;
	if (_v == nullptr) {
		return -1;
	}

	if (list_resize(_self, n_ + 1) < 0)
		return -1;

	if (_where < 0) {
		_where += n_;
		if (_where < 0)
			_where = 0;
	}
	if (_where > n_)
		_where = n_;
	items_ = _self->items_;
	for (i_ = n_; --i_ >= _where; )
		items_[i_ + 1] = items_[i_];
	items_[_where] = (_v);
	return 0;
}

int alifList_setItem(AlifObject* _list, size_t _index, AlifObject* _newItem) {

	AlifObject** p_;
	if (!(_list->type_ == &_alifListType_)) {
		ALIF_XDECREF(_newItem);
		return -1;
	}

	int ret_;
	AlifListObject* self_ = ((AlifListObject*)_list);
	//ALIF_BEGIN_CRITICAL_SECTION(self_);
	if (!valid_index(_index, ALIF_SIZE(self_))) {
		ALIF_XDECREF(_newItem);

		ret_ = -1;
		goto end;
	}
	p_ = self_->items_ + _index;
	ALIF_XSETREF(*p_, _newItem);
	ret_ = 0;
end:
	//ALIF_END_CRITICAL_SECTION();
	return ret_;
}

int alifList_insert(AlifObject* _list, size_t _where, AlifObject* _newItem)
{
	if (!(_list->type_ == &_alifListType_)) {
		return -1;
	}
	AlifListObject* self_ = (AlifListObject*)_list;
	//ALIF_BEGIN_CRITICAL_SECTION(self);
	int err_ = ins1(self_, _where, _newItem);
	//ALIF_END_CRITICAL_SECTION();
	return err_;
}

int alifSubList_appendTakeRefListResize(AlifListObject* _self, AlifObject* _newItem)
{
	int64_t len_ = ALIF_SIZE(_self);
	if (list_resize(_self, len_ + 1) < 0) {
		ALIF_DECREF(_newItem);
		return -1;
	}
	alifList_setItem((AlifObject*)_self, len_, _newItem);
	return 0;
}

int alifList_append(AlifObject* _list, AlifObject* _newItem)
{
	if ((_list->type_ == &_alifListType_) && (_newItem != nullptr)) {
		int ret_;
		//ALIF_BEGIN_CRITICAL_SECTION(_op);
		ret_ = alifSubList_appendTakeRef((AlifListObject*)_list, ALIF_NEWREF(_newItem));
		//ALIF_END_CRITICAL_SECTION();
		return ret_;
	}
	return -1;
}

static void list_dealloc(AlifObject* _self)
{
	AlifListObject* op_ = (AlifListObject*)_self;
	int64_t i_;
    alifObject_gc_unTrack(op_);
	//ALIF_TRASHCAN_BEGIN(op_, list_dealloc);
	if (op_->items_ != nullptr) {

		i_ = ALIF_SIZE(op_);
		while (--i_ >= 0) {
			ALIF_XDECREF(op_->items_[i_]);
		}
	}

	ALIF_TYPE(op_)->free_((AlifObject*)op_);

	//ALIF_TRASHCAN_END;
}

static size_t list_length(AlifListObject* _list) {
	return ALIF_SIZE(_list);
}

static int list_contains(AlifListObject* _aa, AlifObject* _el) {

	for (int64_t i = 0; ; i++) {
		AlifObject* item_ = listGet_itemRef((AlifListObject*)_aa, i);
		if (item_ == NULL) {
			return 0;
		}
		int cmp = alifObject_richCompareBool(item_, _el, ALIF_EQ);
		ALIF_DECREF(item_);
		if (cmp != 0) {
			return cmp;
		}
	}
	return 0;
}

static AlifObject* list_extend(AlifListObject* _self, AlifObject* _iterable)
{
	AlifObject* it_;
	int64_t m_;
	int64_t n_{};
	int64_t i_;
	AlifObject* (*IterNext)(AlifObject*);

	if ((_iterable->type_ == &_alifListType_) || (_iterable->type_ == &_alifTupleType_) ||
		(AlifObject*)_self == _iterable) {
		AlifObject** src_, ** dest_;
		_iterable = alifSequence_fast(_iterable, L"argument must be _iterable");
		if (!_iterable)
			return nullptr;
		n_ = ALIFSEQUENCE_FAST_GETSIZE(_iterable);
		if (n_ == 0) {
			return ALIF_NONE;
		}
		m_ = ALIF_SIZE(_self);
		if (_self->items_ == nullptr) {
			if (list_preallocate_exact(_self, n_) < 0) {
				return nullptr;
			}
			ALIFSET_SIZE(_self, n_);
		}
		else if (list_resize(_self, m_ + n_) < 0) {
			return nullptr;
		}
		src_ = ALIFSEQUENCE_FAST_ITEMS(_iterable);
		dest_ = _self->items_ + m_;
		for (i_ = 0; i_ < n_; i_++) {
			AlifObject* o = src_[i_];
			dest_[i_] = o;
		}
		return ALIF_NONE;
	}

	it_ = alifObject_getIter(_iterable);
	if (it_ == nullptr)
		return nullptr;
	IterNext = *(it_)->type_->iterNext;

	//n_ = AlifObj_LengthHint(_iterable, 8);
	if (n_ < 0) {
		return nullptr;
	}
	m_ = ALIF_SIZE(_self);
	if (m_ > LLONG_MAX - n_) {

	}
	else if (_self->items_ == nullptr) {
		if (n_ && list_preallocate_exact(_self, n_) < 0)
			goto error;
	}
	else {

		if (list_resize(_self, m_ + n_) < 0)
			goto error;
		(_self, m_);
	}

	for (;;) {
		AlifObject* item_ = IterNext(it_);
		if (item_ == nullptr)
		{
			break;
		}
		if (ALIF_SIZE(_self) < _self->allocate_) {
			int64_t len_ = ALIF_SIZE(_self);
			ALIFSET_SIZE(_self, len_ + 1);
			alifList_setItem((AlifObject*)_self, len_, (AlifObject*)item_);
		}
		else {
			if (alifSubList_appendTakeRef(_self, item_) < 0)
				goto error;
		}
	}

	if (ALIF_SIZE(_self) < _self->allocate_) {
		if (list_resize(_self, ALIF_SIZE(_self)) < 0)
			goto error;
	}


	return ALIF_NONE;

error:
	return nullptr;
}

AlifObject* alifList_extend(AlifListObject* _self, AlifObject* _iterable)
{
	return list_extend(_self, _iterable);
}

static AlifObject* list_item(AlifObject* _aa, int64_t _i)
{
	AlifListObject* a_ = (AlifListObject*)_aa;
	if (!valid_index(_i, ALIFLIST_GET_SIZE(a_))) {
		return nullptr;
	}
	AlifObject* item_;
	//ALIF_BEGIN_CRITICAL_SECTION(_a);
#ifdef ALIF_GIL_DISABLED
	if (!alifSub_IsOwnedByCurrentThread((AlifObject*)_a) && !alifSubObject_GC_IS_SHARED(_a)) {
		alifSubObject_GC_SET_SHARED(_a);
	}
#endif
	item_ = ALIF_NEWREF(a_->items_[_i]);
	//ALIF_END_CRITICAL_SECTION();
	return item_;
}

static AlifObject* listSlice_lockHeld(AlifListObject* _a, int64_t _iLow, int64_t _iHigh)
{
	AlifListObject* np_;
	AlifObject** src_, ** dest_;
	int64_t i_, len_;
	len_ = _iHigh - _iLow;
	if (len_ <= 0) {
		return alifNew_list(0);
	}
	np_ = (AlifListObject*)list_new_prealloc(len_);
	if (np_ == NULL)
		return NULL;

	src_ = _a->items_ + _iLow;
	dest_ = np_->items_;
	for (i_ = 0; i_ < len_; i_++) {
		AlifObject* _v = src_[i_];
		dest_[i_] = ALIF_NEWREF(_v);
	}
	ALIFSET_SIZE(np_, len_);
	return (AlifObject*)np_;
}

AlifObject* alifList_getSlice(AlifObject* _a, int64_t _iLow, int64_t _iHigh)
{
	if (!(_a->type_ == &_alifListType_)) {
		return NULL;
	}
	AlifObject* ret_;
	//ALIF_BEGIN_CRITICAL_SECTION(_a);
	if (_iLow < 0) {
		_iLow = 0;
	}
	else if (_iLow > ALIF_SIZE(_a)) {
		_iLow = ALIF_SIZE(_a);
	}
	if (_iHigh < _iLow) {
		_iHigh = _iLow;
	}
	else if (_iHigh > ALIF_SIZE(_a)) {
		_iHigh = ALIF_SIZE(_a);
	}
	ret_ = listSlice_lockHeld((AlifListObject*)_a, _iLow, _iHigh);
	//ALIF_END_CRITICAL_SECTION();
	return ret_;
}

static AlifObject* listConcat_lockHeld(AlifListObject* _a, AlifListObject* _b)
{
	int64_t size_;
	int64_t i_;
	AlifObject** src_, ** dest_;
	AlifListObject* np_;
	size_ = ALIF_SIZE(_a) + ALIF_SIZE(_b);
	if (size_ == 0) {
		return alifNew_list(0);
	}
	np_ = (AlifListObject*)list_new_prealloc(size_);
	if (np_ == NULL) {
		return NULL;
	}
	src_ = _a->items_;
	dest_ = np_->items_;
	for (i_ = 0; i_ < ALIF_SIZE(_a); i_++) {
		AlifObject* v_ = src_[i_];
		dest_[i_] = ALIF_NEWREF(v_);
	}
	src_ = _b->items_;
	dest_ = np_->items_ + ALIF_SIZE(_a);
	for (i_ = 0; i_ < ALIF_SIZE(_b); i_++) {
		AlifObject* v_ = src_[i_];
		dest_[i_] = ALIF_NEWREF(v_);
	}
	ALIFSET_SIZE(np_, size_);
	return (AlifObject*)np_;
}

static AlifObject* list_concat(AlifObject* _aa, AlifObject* _bb)
{
	if (!(_bb->type_ == &_alifListType_)) {
		return NULL;
	}
	AlifListObject* a_ = (AlifListObject*)_aa;
	AlifListObject* b_ = (AlifListObject*)_bb;
	AlifObject* ret_;
	//ALIF_BEGIN_CRITICAL_SECTION2(_a, _b);
	ret_ = listConcat_lockHeld(a_, b_);
	//ALIF_END_CRITICAL_SECTION2();
	return ret_;
}

static AlifObject* listRepeat_lockHeld(AlifListObject* _a, int64_t _n)
{
	const int64_t inputSize = ALIF_SIZE(_a);
	if (inputSize == 0 || _n <= 0)
		return alifNew_list(0);

	if (inputSize > LLONG_MAX / _n)
		return nullptr;
	int64_t outputSize = inputSize * _n;

	AlifListObject* np_ = (AlifListObject*)list_new_prealloc(outputSize);
	if (np_ == NULL)
		return NULL;

	AlifObject** dest_ = np_->items_;
	if (inputSize == 1) {
		AlifObject* elem_ = _a->items_[0];
		ALIFSUB_REFCNTADD(elem_, _n);
		AlifObject** destEnd = dest_ + outputSize;
		while (dest_ < destEnd) {
			*dest_++ = elem_;
		}
	}
	else {
		AlifObject** src_ = _a->items_;
		AlifObject** srcEnd = src_ + inputSize;
		while (src_ < srcEnd) {
			ALIFSUB_REFCNTADD(*src_, _n);
			*dest_++ = *src_++;
		}

		alifSub_memory_repeat((char*)np_->items_, sizeof(AlifObject*) * outputSize,
			sizeof(AlifObject*) * inputSize);
	}

	ALIFSET_SIZE(np_, outputSize);
	return (AlifObject*)np_;
}

static AlifObject* list_repeat(AlifObject* _aa, int64_t _n)
{
	AlifObject* ret_;
	AlifListObject* a_ = (AlifListObject*)_aa;
	//ALIF_BEGIN_CRITICAL_SECTION(_a);
	ret_ = listRepeat_lockHeld(a_, _n);
	//ALIF_END_CRITICAL_SECTION();
	return ret_;
}

static void list_clear_impl(AlifListObject* _a, bool _isResize)
{
	AlifObject** items_ = _a->items_;
	if (items_ == NULL) {
		return;
	}

	int64_t i_ = ALIF_SIZE(_a);
	ALIFSET_SIZE(_a, 0);
	_a->allocate_ = 0;
	while (--i_ >= 0) {
		ALIF_XDECREF(items_[i_]);
	}
#ifdef ALIF_GIL_DISABLED
	bool useQsbr = _isResize && alifSubObject_GC_IS_SHARED(_a);
#else
	bool useQsbr = false;
#endif

}

static void list_clear(AlifListObject* _a)
{
	list_clear_impl(_a, true);
}

static int list_clear_slot(AlifObject* _self)
{
	list_clear_impl((AlifListObject*)_self, false);
	return 0;
}

static int listAss_slice_lockHeld(AlifListObject* _a, int64_t _iLow, int64_t _iHigh, AlifObject* _v)
{
	AlifObject* recycleOnStack[8];
	AlifObject** recycle_ = recycleOnStack; /* will allocate_ more if needed */
	AlifObject** item_;
	AlifObject** vItem = NULL;
	AlifObject* vAsSF = NULL; /* ALIFSequence_Fast(_v) */
	int64_t n_; /* # of elements in replacement list */
	int64_t norig_; /* # of elements in list getting replaced */
	int64_t d_; /* Change in size */
	int64_t k_;
	size_t s_;
	int result_ = -1;            /* guilty until proved innocent */
#define b ((AlifListObject *)_v)
	if (_v == NULL)
		n_ = 0;
	else {
		vAsSF = alifSequence_fast(_v, L"can only assign an iterable");
		if (vAsSF == NULL)
			goto Error;
		n_ = ALIFSEQUENCE_FAST_GETSIZE(vAsSF);
		vItem = ALIFSEQUENCE_FAST_ITEMS(vAsSF);
	}
	if (_iLow < 0)
		_iLow = 0;
	else if (_iLow > ALIF_SIZE(_a))
		_iLow = ALIF_SIZE(_a);

	if (_iHigh < _iLow)
		_iHigh = _iLow;
	else if (_iHigh > ALIF_SIZE(_a))
		_iHigh = ALIF_SIZE(_a);

	norig_ = _iHigh - _iLow;
	d_ = n_ - norig_;
	if (ALIF_SIZE(_a) + d_ == 0) {
		ALIF_XDECREF(vAsSF);
		list_clear(_a);
		return 0;
	}
	item_ = _a->items_;
	s_ = norig_ * sizeof(AlifObject*);
	if (s_) {
		if (s_ > sizeof(recycleOnStack)) {
			recycle_ = (AlifObject**)alifMem_objAlloc(s_);
			if (recycle_ == NULL) {
				goto Error;
			}
		}
		memcpy(recycle_, &item_[_iLow], s_);
	}

	if (d_ < 0) { /* Delete -d_ items_ */
		int64_t tail_;
		tail_ = (ALIF_SIZE(_a) - _iHigh) * sizeof(AlifObject*);
		memmove(&item_[_iHigh + d_], &item_[_iHigh], tail_);
		if (list_resize(_a, ALIF_SIZE(_a) + d_) < 0) {
			memmove(&item_[_iHigh], &item_[_iHigh + d_], tail_);
			memcpy(&item_[_iLow], recycle_, s_);
			goto Error;
		}
		item_ = _a->items_;
	}
	else if (d_ > 0) { /* Insert d_ items_ */
		k_ = ALIF_SIZE(_a);
		if (list_resize(_a, k_ + d_) < 0)
			goto Error;
		item_ = _a->items_;
		memmove(&item_[_iHigh + d_], &item_[_iHigh],
			(k_ - _iHigh) * sizeof(AlifObject*));
	}
	for (k_ = 0; k_ < n_; k_++, _iLow++) {
		AlifObject* w_ = vItem[k_];
		item_[_iLow] = ALIF_XNEWREF(w_);
	}
	for (k_ = norig_ - 1; k_ >= 0; --k_)
		ALIF_XDECREF(recycle_[k_]);
	result_ = 0;
Error:
	if (recycle_ != recycleOnStack)
		alifMem_objFree(recycle_);
	ALIF_XDECREF(vAsSF);
	return result_;
#undef b
}

static int list_ass_slice(AlifListObject* _a, int64_t _iLow, int64_t _iHigh, AlifObject* _v)
{
	int ret_;
	if (_a == (AlifListObject*)_v) {
		//ALIF_BEGIN_CRITICAL_SECTION(_a);
		int64_t n_ = ALIFLIST_GET_SIZE(_a);
		AlifObject* copy = listSlice_lockHeld(_a, 0, n_);
		if (copy == NULL) {
			return -1;
		}
		ret_ = listAss_slice_lockHeld(_a, _iLow, _iHigh, copy);
		ALIF_DECREF(copy);
		//ALIF_END_CRITICAL_SECTION();
	}
	else if (_v != NULL && (_v->type_ == &_alifListType_)) {
		//ALIF_BEGIN_CRITICAL_SECTION2(_a, _v);
		ret_ = listAss_slice_lockHeld(_a, _iLow, _iHigh, _v);
		//ALIF_END_CRITICAL_SECTION2();
	}
	else {
		//ALIF_BEGIN_CRITICAL_SECTION(_a);
		ret_ = listAss_slice_lockHeld(_a, _iLow, _iHigh, _v);
		//ALIF_END_CRITICAL_SECTION();
	}
	return ret_;
}

int alifList_setSlice(AlifObject* _a, int64_t _iLow, int64_t _iHigh, AlifObject* _v)
{
	if (!(_a->type_ == &_alifListType_)) {
		return -1;
	}
	return list_ass_slice((AlifListObject*)_a, _iLow, _iHigh, _v);
}

static int listAss_item_lockHeld(AlifListObject* a, int64_t i, AlifObject* v)
{
	if (!valid_index(i, ALIF_SIZE(a))) {
		return -1;
	}
	AlifObject* tmp = a->items_[i];
	if (v == NULL) {
		int64_t size = ALIF_SIZE(a);
		for (int64_t idx = i; idx < size - 1; idx++) {
			a->items_[idx] = a->items_[idx + 1];
		}
		ALIFSET_SIZE(a, size - 1);
	}
	else {
		a->items_[i] = ALIF_NEWREF(v);
	}
	ALIF_DECREF(tmp);
	return 0;
}

static int list_ass_item(AlifObject* _aa, int64_t _i, AlifObject* _v)
{
	int ret_;
	AlifListObject* a_ = (AlifListObject*)_aa;
	//ALIF_BEGIN_CRITICAL_SECTION(a);
	ret_ = listAss_item_lockHeld(a_, _i, _v);
	//ALIF_END_CRITICAL_SECTION();
	return ret_;
}

static AlifObject* list_insert_impl(AlifListObject* _self, int64_t index, AlifObject* object)
{
	if (ins1(_self, index, object) == 0)
		return ALIF_NONE;
	return nullptr;
}

static AlifObject* list_append(AlifListObject* _self, AlifObject* _object)
{
	if (alifSubList_appendTakeRef(_self, (_object)) < 0) {
		return nullptr;
	}
	return ALIF_NONE;
}

//static AlifObject* list_pop_impl(AlifListObject* _self, int64_t index)
//{
//    AlifObject* _v;
//    int status;
//
//    if (ALIF_SIZE(_self) == 0) {
//        return nullptr;
//    }
//    if (index < 0)
//        index += ALIF_SIZE(_self);
//    if (!valid_index(index, ALIF_SIZE(_self))) {
//        return nullptr;
//    }
//
//    AlifObject** items_ = _self->items_;
//    _v = items_[index];
//    const int64_t size_after_pop = ALIF_SIZE(_self) - 1;
//    if (size_after_pop == 0) {
//        status = list_clear_slot(_self);
//    }
//    else {
//        if ((size_after_pop - index) > 0) {
//            memmove(&items_[index], &items_[index + 1], (size_after_pop - index) * sizeof(AlifObject*));
//        }
//        status = list_resize(_self, size_after_pop);
//    }
//    if (status >= 0) {
//        return _v; 
//    }
//    else {
//        memmove(&items_[index + 1], &items_[index], (size_after_pop - index) * sizeof(AlifObject*));
//        items_[index] = _v;
//        return nullptr;
//    }
//}
//
//static AlifObject* list_pop(AlifListObject* _self, AlifObject* const* args, int64_t nargs)
//{
//    AlifObject* return_value = nullptr;
//    int64_t index = -1;
//
//    if (!_alifArg_checkPositional(L"pop", nargs, 0, 1)) {
//        goto exit;
//    }
//    if (nargs < 1) {
//        goto skip_optional;
//    }
//    {
//        int64_t ival = -1;
//        AlifObject* iobj = args[0];
//        if (iobj != nullptr) {
//            ival = alifInteger_asLongLong(iobj);
//        }
//
//        index = ival;
//    }
//skip_optional:
//    return_value = list_pop_impl(_self, index);
//
//exit:
//    return return_value;
//}

static AlifObject* list_insert(AlifListObject* _self, AlifObject* const* _args, int64_t _nArgs)
{
	AlifObject* returnValue = nullptr;
	int64_t index_;
	AlifObject* object_;

	if (!alifSubArg_checkPositional(L"insert", _nArgs, 2, 2)) {
		goto exit;
	}
	{
		int64_t iVal = -1;
		AlifObject* iObj = _args[0];
		if (iObj != nullptr) {
			iVal = alifInteger_asLongLong(iObj);
		}
		index_ = iVal;
	}
	object_ = _args[1];
	returnValue = list_insert_impl(_self, index_, object_);

exit:
	return returnValue;
}

int list_setSlice(AlifObject* _a, int64_t _iLow, int64_t _iHigh, AlifObject* _v)
{
	return list_ass_slice((AlifListObject*)_a, _iLow, _iHigh, _v);
}

static void reverse_slice(AlifObject** lo, AlifObject** hi)
{

	--hi;
	while (lo < hi) {
		AlifObject* t_ = *lo;
		*lo = *hi;
		*hi = t_;
		++lo;
		--hi;
	}
}

class SortSlice {
public:
	AlifObject** keys_;
	AlifObject** values_;
};

static __inline void __fastcall sortSlice_copy(SortSlice* _s1, int64_t _i, SortSlice* _s2, int64_t _j)
{
	_s1->keys_[_i] = _s2->keys_[_j];
	if (_s1->values_ != nullptr)
		_s1->values_[_i] = _s2->values_[_j];
}

static __inline void __fastcall sortSlice_copy_incr(SortSlice* _dst, SortSlice* _src)
{
	*_dst->keys_++ = *_src->keys_++;
	if (_dst->values_ != nullptr)
		*_dst->values_++ = *_src->values_++;
}

static __inline void __fastcall sortSlice_copy_decr(SortSlice* _dst, SortSlice* _src)
{
	*_dst->keys_-- = *_src->keys_--;
	if (_dst->values_ != nullptr)
		*_dst->values_-- = *_src->values_--;
}

static __inline void __fastcall sortSlice_memcpy(SortSlice* _s1, int64_t _i, SortSlice* _s2, int64_t _j,
	int64_t _n)
{
	memcpy(&_s1->keys_[_i], &_s2->keys_[_j], sizeof(AlifObject*) * _n);
	if (_s1->values_ != nullptr)
		memcpy(&_s1->values_[_i], &_s2->values_[_j], sizeof(AlifObject*) * _n);
}

static __inline void __fastcall sortSlice_memmove(SortSlice* _s1, int64_t _i, SortSlice* _s2, int64_t _j,
	int64_t _n)
{
	memmove(&_s1->keys_[_i], &_s2->keys_[_j], sizeof(AlifObject*) * _n);
	if (_s1->values_ != nullptr)
		memmove(&_s1->values_[_i], &_s2->values_[_j], sizeof(AlifObject*) * _n);
}

static __inline void __fastcall sortSlice_advance(SortSlice* _slice, int64_t _n)
{
	_slice->keys_ += _n;
	if (_slice->values_ != nullptr)
		_slice->values_ += _n;
}

#define ISLT(_X, _Y) (*(_ms->KeyCompare))(_X, _Y, _ms)

#define IFLT(_X, _Y) if ((k_ = ISLT(_X, _Y)) < 0) goto fail;  \
           if (k_)

#define MAX_MERGE_PENDING (sizeof(size_t) * 8)

#define MERGESTATE_TEMP_SIZE 256

#define MAX_MINRUN 64

#define MIN_GALLOP 7

class SSlice {
public:
	SortSlice base_;
	int64_t len_;   /* length of run */
	int power_; /* node "level" for powersort merge strategy */
};

typedef class SMergeState MergeState;
struct SMergeState {
public:
	int64_t minGallop;

	int64_t listLen;
	AlifObject** baseKeys;

	SortSlice a_;
	int64_t alloced_;

	int n_;
	class SSlice pending_[MAX_MERGE_PENDING];

	AlifObject* tempArray[MERGESTATE_TEMP_SIZE];

	int (*KeyCompare)(AlifObject*, AlifObject*, MergeState*);

	AlifObject* (*KeyRichcompare)(AlifObject*, AlifObject*, int);

	int (*TupleElemCompare)(AlifObject*, AlifObject*, MergeState*);
};

static int binarysort(MergeState* _ms, const SortSlice* _ss, int64_t _n, int64_t _ok)
{
	int64_t k_;
	AlifObject** const a_ = _ss->keys_;
	AlifObject** const v_ = _ss->values_;
	const bool hasValues = v_ != nullptr;
	AlifObject* pivot_;
	int64_t m_;

	if (!_ok)
		++_ok;

#if 0 
	AlifObject* vPivot = nullptr;
	for (; _ok < _n; ++_ok) {
		pivot_ = a_[_ok];
		if (hasValues)
			vPivot = v_[_ok];
		for (m_ = _ok - 1; m_ >= 0; --m_) {
			k_ = ISLT(pivot_, a_[m_]);
			if (k_ < 0) {
				a_[m_ + 1] = pivot_;
				if (hasValues)
					v_[m_ + 1] = vPivot;
				goto fail;
			}
			else if (k_) {
				a_[m_ + 1] = a_[m_];
				if (hasValues)
					v_[m_ + 1] = v_[m_];
			}
			else
				break;
		}
		a_[m_ + 1] = pivot_;
		if (hasValues)
			v_[m_ + 1] = vPivot;
	}
#else 
	int64_t L_, R_;
	for (; _ok < _n; ++_ok) {
		L_ = 0;
		R_ = _ok;
		pivot_ = a_[_ok];

		do {

			m_ = (L_ + R_) >> 1;
#if 1
			IFLT(pivot_, a_[m_])
				R_ = m_;
	else
		L_ = m_ + 1;
#else

			k_ = ISLT(pivot_, a_[m_]);
			if (k_ < 0)
				goto fail;
			int64_t mP1 = m_ + 1;
			R_ = k_ ? m_ : R_;
			L_ = k_ ? L : mP1;
#endif
		} while (L_ < R_);
		for (m_ = _ok; m_ > L_; --m_)
			a_[m_] = a_[m_ - 1];
		a_[L_] = pivot_;
		if (hasValues) {
			pivot_ = v_[_ok];
			for (m_ = _ok; m_ > L_; --m_)
				v_[m_] = v_[m_ - 1];
			v_[L_] = pivot_;
		}
	}
#endif 
	return 0;

fail:
	return -1;
}

static void sortSlice_reverse(SortSlice* _s, int64_t _n)
{
	reverse_slice(_s->keys_, &_s->keys_[_n]);
	if (_s->values_ != nullptr)
		reverse_slice(_s->values_, &_s->values_[_n]);
}

static int64_t count_run(MergeState* _ms, SortSlice* _sLo, int64_t _nRemaining)
{
	int64_t k_;
	int64_t n_;
	AlifObject** const lo_ = _sLo->keys_;

	int64_t neq_ = 0;
#define REVERSE_LAST_NEQ                        \
    if (neq_) {                                  \
        SortSlice slice_ = *_sLo;                 \
        ++neq_;                                  \
        sortSlice_advance(&slice_, n_ - neq_);     \
        sortSlice_reverse(&slice_, neq_);         \
        neq_ = 0;                                \
    }

#define IF_NEXT_LARGER  IFLT(lo_[n_-1], lo_[n_])
#define IF_NEXT_SMALLER IFLT(lo_[n_], lo_[n_-1])

	for (n_ = 1; n_ < _nRemaining; ++n_) {
		IF_NEXT_SMALLER
			break;
	}
	if (n_ == _nRemaining)
		return n_;

	if (n_ > 1) {
		IFLT(lo_[0], lo_[n_ - 1])
			return n_;
		sortSlice_reverse(_sLo, n_);
	}
	++n_;
	for (; n_ < _nRemaining; ++n_) {
		IF_NEXT_SMALLER{

			REVERSE_LAST_NEQ
		}
	else {
		IF_NEXT_LARGER
			break;
	else
		++neq_;
	}
	}
	REVERSE_LAST_NEQ
		sortSlice_reverse(_sLo, n_);
	for (; n_ < _nRemaining; ++n_) {
		IF_NEXT_SMALLER
			break;
	}

	return n_;
fail:
	return -1;

#undef REVERSE_LAST_NEQ
#undef IF_NEXT_SMALLER
#undef IF_NEXT_LARGER
}

static int64_t gallop_left(MergeState* _ms, AlifObject* _key, AlifObject** _a, int64_t _n, int64_t _hInt)
{
	int64_t ofs_;
	int64_t lastOfs;
	int64_t k_;

	_a += _hInt;
	lastOfs = 0;
	ofs_ = 1;
	IFLT(*_a, _key) {

		const int64_t maxOfs = _n - _hInt;
		while (ofs_ < maxOfs) {
			IFLT(_a[ofs_], _key) {
				lastOfs = ofs_;
				ofs_ = (ofs_ << 1) + 1;
			}
		   else
	break;
		}
		if (ofs_ > maxOfs)
			ofs_ = maxOfs;
		lastOfs += _hInt;
		ofs_ += _hInt;
	}
		   else {

			   const int64_t maxOfs = _hInt + 1;
			   while (ofs_ < maxOfs) {
				   IFLT(*(_a - ofs_), _key)
					   break;
				   lastOfs = ofs_;
				   ofs_ = (ofs_ << 1) + 1;
			   }
			   if (ofs_ > maxOfs)
				   ofs_ = maxOfs;
			   k_ = lastOfs;
			   lastOfs = _hInt - ofs_;
			   ofs_ = _hInt - k_;
		   }
_a -= _hInt;


++lastOfs;
while (lastOfs < ofs_) {
	int64_t m_ = lastOfs + ((ofs_ - lastOfs) >> 1);

	IFLT(_a[m_], _key)
		lastOfs = m_ + 1;
else
ofs_ = m_;
}
return ofs_;

fail:
return -1;
}

static int64_t gallop_right(MergeState* _ms, AlifObject* _key, AlifObject** _a, int64_t _n, int64_t _hInt)
{
	int64_t ofs_;
	int64_t lastOfs;
	int64_t k_;

	_a += _hInt;
	lastOfs = 0;
	ofs_ = 1;
	IFLT(_key, *_a) {

		const int64_t maxOfs = _hInt + 1;
		while (ofs_ < maxOfs) {
			IFLT(_key, *(_a - ofs_)) {
				lastOfs = ofs_;
				ofs_ = (ofs_ << 1) + 1;
			}
		   else
	break;
		}
		if (ofs_ > maxOfs)
			ofs_ = maxOfs;
		k_ = lastOfs;
		lastOfs = _hInt - ofs_;
		ofs_ = _hInt - k_;
	}
		   else {

			   const int64_t maxOfs = _n - _hInt;
			   while (ofs_ < maxOfs) {
				   IFLT(_key, _a[ofs_])
					   break;
				   lastOfs = ofs_;
				   ofs_ = (ofs_ << 1) + 1;
			   }
			   if (ofs_ > maxOfs)
				   ofs_ = maxOfs;
			   lastOfs += _hInt;
			   ofs_ += _hInt;
		   }
_a -= _hInt;

++lastOfs;
while (lastOfs < ofs_) {
	int64_t m_ = lastOfs + ((ofs_ - lastOfs) >> 1);

	IFLT(_key, _a[m_])
		ofs_ = m_;
else
lastOfs = m_ + 1;
}
return ofs_;

fail:
return -1;
}

static void merge_init(MergeState* _ms, int64_t _listSize, int _hasKeyFunc,
	SortSlice* lo)
{
	if (_hasKeyFunc) {
		_ms->alloced_ = (_listSize + 1) / 2;

		if (MERGESTATE_TEMP_SIZE / 2 < _ms->alloced_)
			_ms->alloced_ = MERGESTATE_TEMP_SIZE / 2;
		_ms->a_.values_ = &_ms->tempArray[_ms->alloced_];
	}
	else {
		_ms->alloced_ = MERGESTATE_TEMP_SIZE;
		_ms->a_.values_ = nullptr;
	}
	_ms->a_.keys_ = _ms->tempArray;
	_ms->n_ = 0;
	_ms->minGallop = 7;
	_ms->listLen = _listSize;
	_ms->baseKeys = lo->keys_;
}

static void merge_freemem(MergeState* _ms)
{
	if (_ms->a_.keys_ != _ms->tempArray) {
		alifMem_objFree(_ms->a_.keys_);
		_ms->a_.keys_ = nullptr;
	}
}

static int merge_getmem(MergeState* _ms, int64_t _need)
{
	int multiplier_;

	if (_need <= _ms->alloced_)
		return 0;

	multiplier_ = _ms->a_.values_ != nullptr ? 2 : 1;
	merge_freemem(_ms);
	if ((size_t)_need > LLONG_MAX / sizeof(AlifObject*) / multiplier_) {
		return -1;
	}
	_ms->a_.keys_ = (AlifObject**)alifMem_objAlloc(multiplier_ * _need
		* sizeof(AlifObject*));
	if (_ms->a_.keys_ != nullptr) {
		_ms->alloced_ = _need;
		if (_ms->a_.values_ != nullptr)
			_ms->a_.values_ = &_ms->a_.keys_[_need];
		return 0;
	}
	return -1;
}
#define MERGE_GETMEM(_MS, _NEED) ((_NEED) <= (_MS)->alloced_ ? 0 :   \
                                merge_getmem(_MS, _NEED))


static int64_t merge_lo(MergeState* _ms, SortSlice _sSa, int64_t _na,
	SortSlice _sSb, int64_t _nb)
{
	int64_t k_;
	SortSlice dest_;
	int result_ = -1;
	int64_t minGallop;

	if (MERGE_GETMEM(_ms, _na) < 0)
		return -1;
	sortSlice_memcpy(&_ms->a_, 0, &_sSa, 0, _na);
	dest_ = _sSa;
	_sSa = _ms->a_;

	sortSlice_copy_incr(&dest_, &_sSb);
	--_nb;
	if (_nb == 0)
		goto Succeed;
	if (_na == 1)
		goto CopyB;

	minGallop = _ms->minGallop;
	for (;;) {
		int64_t aCount = 0;
		int64_t bCount = 0;
		for (;;) {
			k_ = ISLT(_sSb.keys_[0], _sSa.keys_[0]);
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_copy_incr(&dest_, &_sSb);
				++bCount;
				aCount = 0;
				--_nb;
				if (_nb == 0)
					goto Succeed;
				if (bCount >= minGallop)
					break;
			}
			else {
				sortSlice_copy_incr(&dest_, &_sSa);
				++aCount;
				bCount = 0;
				--_na;
				if (_na == 1)
					goto CopyB;
				if (aCount >= minGallop)
					break;
			}
		}

		++minGallop;
		do {
			minGallop -= minGallop > 1;
			_ms->minGallop = minGallop;
			k_ = gallop_right(_ms, _sSb.keys_[0], _sSa.keys_, _na, 0);
			aCount = k_;
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_memcpy(&dest_, 0, &_sSa, 0, k_);
				sortSlice_advance(&dest_, k_);
				sortSlice_advance(&_sSa, k_);
				_na -= k_;
				if (_na == 1)
					goto CopyB;

				if (_na == 0)
					goto Succeed;
			}
			sortSlice_copy_incr(&dest_, &_sSb);
			--_nb;
			if (_nb == 0)
				goto Succeed;

			k_ = gallop_left(_ms, _sSa.keys_[0], _sSb.keys_, _nb, 0);
			bCount = k_;
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_memmove(&dest_, 0, &_sSb, 0, k_);
				sortSlice_advance(&dest_, k_);
				sortSlice_advance(&_sSb, k_);
				_nb -= k_;
				if (_nb == 0)
					goto Succeed;
			}
			sortSlice_copy_incr(&dest_, &_sSa);
			--_na;
			if (_na == 1)
				goto CopyB;
		} while (aCount >= MIN_GALLOP || bCount >= MIN_GALLOP);
		++minGallop;
		_ms->minGallop = minGallop;
	}
Succeed:
	result_ = 0;
Fail:
	if (_na)
		sortSlice_memcpy(&dest_, 0, &_sSa, 0, _na);
	return result_;
CopyB:

	sortSlice_memmove(&dest_, 0, &_sSb, 0, _nb);
	sortSlice_copy(&dest_, _nb, &_sSa, 0);
	return 0;
}

static int64_t merge_hi(MergeState* _ms, SortSlice _sSa, int64_t _na,
	SortSlice _sSb, int64_t _nb)
{
	int64_t k_;
	SortSlice dest_, basea_, baseb_;
	int result_ = -1;
	int64_t minGallop;


	if (MERGE_GETMEM(_ms, _nb) < 0)
		return -1;
	dest_ = _sSb;
	sortSlice_advance(&dest_, _nb - 1);
	sortSlice_memcpy(&_ms->a_, 0, &_sSb, 0, _nb);
	basea_ = _sSa;
	baseb_ = _ms->a_;
	_sSb.keys_ = _ms->a_.keys_ + _nb - 1;
	if (_sSb.values_ != nullptr)
		_sSb.values_ = _ms->a_.values_ + _nb - 1;
	sortSlice_advance(&_sSa, _na - 1);

	sortSlice_copy_decr(&dest_, &_sSa);
	--_na;
	if (_na == 0)
		goto Succeed;
	if (_nb == 1)
		goto CopyA;

	minGallop = _ms->minGallop;
	for (;;) {
		int64_t aCount = 0;
		int64_t bCount = 0;

		for (;;) {
			k_ = ISLT(_sSb.keys_[0], _sSa.keys_[0]);
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_copy_decr(&dest_, &_sSa);
				++aCount;
				bCount = 0;
				--_na;
				if (_na == 0)
					goto Succeed;
				if (aCount >= minGallop)
					break;
			}
			else {
				sortSlice_copy_decr(&dest_, &_sSb);
				++bCount;
				aCount = 0;
				--_nb;
				if (_nb == 1)
					goto CopyA;
				if (bCount >= minGallop)
					break;
			}
		}

		++minGallop;
		do {
			minGallop -= minGallop > 1;
			_ms->minGallop = minGallop;
			k_ = gallop_right(_ms, _sSb.keys_[0], basea_.keys_, _na, _na - 1);
			if (k_ < 0)
				goto Fail;
			k_ = _na - k_;
			aCount = k_;
			if (k_) {
				sortSlice_advance(&dest_, -k_);
				sortSlice_advance(&_sSa, -k_);
				sortSlice_memmove(&dest_, 1, &_sSa, 1, k_);
				_na -= k_;
				if (_na == 0)
					goto Succeed;
			}
			sortSlice_copy_decr(&dest_, &_sSb);
			--_nb;
			if (_nb == 1)
				goto CopyA;

			k_ = gallop_left(_ms, _sSa.keys_[0], baseb_.keys_, _nb, _nb - 1);
			if (k_ < 0)
				goto Fail;
			k_ = _nb - k_;
			bCount = k_;
			if (k_) {
				sortSlice_advance(&dest_, -k_);
				sortSlice_advance(&_sSb, -k_);
				sortSlice_memcpy(&dest_, 1, &_sSb, 1, k_);
				_nb -= k_;
				if (_nb == 1)
					goto CopyA;
				if (_nb == 0)
					goto Succeed;
			}
			sortSlice_copy_decr(&dest_, &_sSa);
			--_na;
			if (_na == 0)
				goto Succeed;
		} while (aCount >= MIN_GALLOP || bCount >= MIN_GALLOP);
		++minGallop;
		_ms->minGallop = minGallop;
	}
Succeed:
	result_ = 0;
Fail:
	if (_nb)
		sortSlice_memcpy(&dest_, -(_nb - 1), &baseb_, 0, _nb);
	return result_;
CopyA:

	sortSlice_memmove(&dest_, 1 - _na, &_sSa, 1 - _na, _na);
	sortSlice_advance(&dest_, -_na);
	sortSlice_advance(&_sSa, -_na);
	sortSlice_copy(&dest_, 0, &_sSb, 0);
	return 0;
}

static int64_t merge_at(MergeState* _ms, int64_t _i)
{
	SortSlice sSa, sSb;
	int64_t na_, nb_;
	int64_t k_;

	sSa = _ms->pending_[_i].base_;
	na_ = _ms->pending_[_i].len_;
	sSb = _ms->pending_[_i + 1].base_;
	nb_ = _ms->pending_[_i + 1].len_;

	_ms->pending_[_i].len_ = na_ + nb_;
	if (_i == _ms->n_ - 3)
		_ms->pending_[_i + 1] = _ms->pending_[_i + 2];
	--_ms->n_;

	k_ = gallop_right(_ms, *sSb.keys_, sSa.keys_, na_, 0);
	if (k_ < 0)
		return -1;
	sortSlice_advance(&sSa, k_);
	na_ -= k_;
	if (na_ == 0)
		return 0;

	nb_ = gallop_left(_ms, sSa.keys_[na_ - 1], sSb.keys_, nb_, nb_ - 1);
	if (nb_ <= 0)
		return nb_;
	if (na_ <= nb_)
		return merge_lo(_ms, sSa, na_, sSb, nb_);
	else
		return merge_hi(_ms, sSa, na_, sSb, nb_);
}

static int powerLoop(int64_t _s1, int64_t _n1, int64_t _n2, int64_t _n)
{
	int result_ = 0;
	int64_t a_ = 2 * _s1 + _n1;
	int64_t b_ = a_ + _n1 + _n2;
	for (;;) {
		++result_;
		if (a_ >= _n) {
			a_ -= _n;
			b_ -= _n;
		}
		else if (b_ >= _n) {
			break;
		}
		a_ <<= 1;
		b_ <<= 1;
	}
	return result_;
}

static int found_new_run(MergeState* _ms, int64_t _n2)
{
	if (_ms->n_) {
		class SSlice* p_ = _ms->pending_;
		int64_t s1_ = p_[_ms->n_ - 1].base_.keys_ - _ms->baseKeys; /* start index */
		int64_t n1_ = p_[_ms->n_ - 1].len_;
		int power_ = powerLoop(s1_, n1_, _n2, _ms->listLen);
		while (_ms->n_ > 1 && p_[_ms->n_ - 2].power_ > power_) {
			if (merge_at(_ms, _ms->n_ - 2) < 0)
				return -1;
		}
		p_[_ms->n_ - 1].power_ = power_;
	}
	return 0;
}

static int merge_force_collapse(MergeState* _ms)
{
	class SSlice* p = _ms->pending_;

	while (_ms->n_ > 1) {
		int64_t n_ = _ms->n_ - 2;
		if (n_ > 0 && p[n_ - 1].len_ < p[n_ + 1].len_)
			--n_;
		if (merge_at(_ms, n_) < 0)
			return -1;
	}
	return 0;
}

static int64_t merge_compute_minrun(int64_t _n)
{
	int64_t r = 0;

	while (_n >= MAX_MINRUN) {
		r |= _n & 1;
		_n >>= 1;
	}
	return _n + r;
}


static int safe_object_compare(AlifObject* _v, AlifObject* w_, MergeState* _ms)
{
	return alifObject_richCompareBool(_v, w_, ALIF_LT);
}

static int unsafe_object_compare(AlifObject* _v, AlifObject* _w, MergeState* _ms)
{
	AlifObject* resObj; int res_;

	if (ALIF_TYPE(_v)->richCompare != _ms->KeyRichcompare)
		return alifObject_richCompareBool(_v, _w, ALIF_LT);

	resObj = (*(_ms->KeyRichcompare))(_v, _w, ALIF_LT);

	if (resObj == ALIF_NOTIMPLEMENTED) {
		ALIF_DECREF(resObj);
		return alifObject_richCompareBool(_v, _w, ALIF_LT);
	}
	if (resObj == nullptr)
		return -1;

	if ((resObj->type_ == &typeBool)) {
		res_ = (resObj == ALIF_TRUE);
	}
	else {
		res_ = alifObject_isTrue(resObj);
	}
	ALIF_DECREF(resObj);

	return res_;
}

static int unsafe_long_compare(AlifObject* _v, AlifObject* _w, MergeState* _ms)
{
	AlifIntegerObject* vl_, * wl_;
	intptr_t v0_, w0_;
	int res_;

	vl_ = (AlifIntegerObject*)_v;
	wl_ = (AlifIntegerObject*)_w;

	res_ = alifObject_richCompareBool(_v, _w, ALIF_LT);

	return res_;
}

static int unsafe_float_compare(AlifObject* _v, AlifObject* _w, MergeState* _ms)
{
	int res_;
	res_ = alifFloat_asLongDouble(_v) < alifFloat_asLongDouble(_w);
	return res_;
}

static int unsafe_tuple_compare(AlifObject* _v, AlifObject* _w, MergeState* _ms)
{
	AlifTupleObject* vt_, * wt_;
	int64_t i_, vLen, wLen;
	int k_;
	vt_ = (AlifTupleObject*)_v;
	wt_ = (AlifTupleObject*)_w;

	vLen = ((AlifVarObject*)vt_)->size_;
	wLen = ((AlifVarObject*)wt_)->size_;

	for (i_ = 0; i_ < vLen && i_ < wLen; i_++) {
		k_ = alifObject_richCompareBool(vt_->items_[i_], wt_->items_[i_], ALIF_EQ);
		if (k_ < 0)
			return -1;
		if (!k_)
			break;
	}

	if (i_ >= vLen || i_ >= wLen)
		return vLen < wLen;

	if (i_ == 0)
		return _ms->TupleElemCompare(vt_->items_[i_], wt_->items_[i_], _ms);
	else
		return alifObject_richCompareBool(vt_->items_[i_], wt_->items_[i_], ALIF_LT);
}

static AlifObject* list_sort_impl(AlifListObject* _self, AlifObject* _keyFunc, int _reverse)
{
	MergeState ms_;
	int64_t nRemaining;
	int64_t minRun;
	SortSlice lo_;
	int64_t savedObSize, savedAllocated;
	AlifObject** savedObItem;
	AlifObject** finalObItem;
	AlifObject* result_ = nullptr;
	int64_t i_;
	AlifObject** keys_;

	if (_keyFunc == ALIF_NONE)
		_keyFunc = nullptr;

	savedObSize = ALIF_SIZE(_self);
	savedObItem = _self->items_;
	savedAllocated = _self->allocate_;
	ALIF_SIZE(_self, 0);
	_self->items_ = nullptr;
	_self->allocate_ = -1;

	if (_keyFunc == nullptr) {
		keys_ = nullptr;
		lo_.keys_ = savedObItem;
		lo_.values_ = nullptr;
	}
	else {
		if (savedObSize < MERGESTATE_TEMP_SIZE / 2)
			keys_ = &ms_.tempArray[savedObSize + 1];
		else {
			keys_ = (AlifObject**)alifMem_objAlloc(sizeof(AlifObject*) * savedObSize);
			if (keys_ == nullptr) {
				goto keyfunc_fail;
			}
		}

		for (i_ = 0; i_ < savedObSize; i_++) {
			//keys_[i_] = AlifObject_CallOneArg(_keyFunc, savedObItem[i_]);
			if (keys_[i_] == nullptr) {
				for (i_ = i_ - 1; i_ >= 0; i_--)
					ALIF_DECREF(keys_[i_]);
				if (savedObSize >= MERGESTATE_TEMP_SIZE / 2)
					alifMem_objFree(keys_);
				goto keyfunc_fail;
			}
		}

		lo_.keys_ = keys_;
		lo_.values_ = savedObItem;
	}
	if (savedObSize > 1) {
		int keysAreInTuples = (ALIF_IS_TYPE(lo_.keys_[0], &_alifTupleType_) &&
			((AlifVarObject*)lo_.keys_[0])->size_ > 0);

		AlifTypeObject* keyType = (keysAreInTuples ?
			ALIF_TYPE(((AlifTupleObject*)lo_.keys_[0])->items_[0]) :
			ALIF_TYPE(lo_.keys_[0]));

		int keysAreAllSameType = 1;
		int intsAreBounded = 1;

		for (i_ = 0; i_ < savedObSize; i_++) {

			if (keysAreInTuples &&
				!(ALIF_IS_TYPE(lo_.keys_[i_], &_alifTupleType_) && ((AlifVarObject*)lo_.keys_[i_])->size_ != 0)) {
				keysAreInTuples = 0;
				keysAreAllSameType = 0;
				break;
			}

			AlifObject* key = (keysAreInTuples ?
				((AlifTupleObject*)lo_.keys_[i_])->items_[0] :
				lo_.keys_[i_]);

			if (!ALIF_IS_TYPE(key, keyType)) {
				keysAreAllSameType = 0;
				if (!keysAreInTuples) {
					break;
				}
			}

			if (keysAreAllSameType) {
				if (keyType == &_alifIntegerType_ &&
					intsAreBounded) {
					intsAreBounded = 0;
				}
			}
		}

		if (keysAreAllSameType) {

			if (keyType == &_alifIntegerType_ && intsAreBounded) {
				ms_.KeyCompare = unsafe_long_compare;
			}
			else if (keyType == &_typeFloat_) {
				ms_.KeyCompare = unsafe_float_compare;
			}
			else if ((ms_.KeyRichcompare = keyType->richCompare) != nullptr) {
				ms_.KeyCompare = unsafe_object_compare;
			}
			else {
				ms_.KeyCompare = safe_object_compare;
			}
		}
		else {
			ms_.KeyCompare = safe_object_compare;
		}

		if (keysAreInTuples) {
			if (keyType == &_alifTupleType_) {
				ms_.TupleElemCompare = safe_object_compare;
			}
			else {
				ms_.TupleElemCompare = ms_.KeyCompare;
			}

			ms_.KeyCompare = unsafe_tuple_compare;
		}
	}
	merge_init(&ms_, savedObSize, keys_ != nullptr, &lo_);

	nRemaining = savedObSize;
	if (nRemaining < 2)
		goto succeed;

	if (_reverse) {
		if (keys_ != nullptr)
			reverse_slice(&keys_[0], &keys_[savedObSize]);
		reverse_slice(&savedObItem[0], &savedObItem[savedObSize]);
	}

	minRun = merge_compute_minrun(nRemaining);
	do {
		int64_t n_;

		n_ = count_run(&ms_, &lo_, nRemaining);
		if (n_ < 0)
			goto fail;
		if (n_ < minRun) {
			const int64_t force = nRemaining <= minRun ?
				nRemaining : minRun;
			if (binarysort(&ms_, &lo_, force, n_) < 0)
				goto fail;
			n_ = force;
		}

		if (found_new_run(&ms_, n_) < 0)
			goto fail;
		ms_.pending_[ms_.n_].base_ = lo_;
		ms_.pending_[ms_.n_].len_ = n_;
		++ms_.n_;
		sortSlice_advance(&lo_, n_);
		nRemaining -= n_;
	} while (nRemaining);

	if (merge_force_collapse(&ms_) < 0)
		goto fail;
	lo_ = ms_.pending_[0].base_;

succeed:
	result_ = ALIF_NONE;
fail:
	if (keys_ != nullptr) {
		for (i_ = 0; i_ < savedObSize; i_++)
			ALIF_DECREF(keys_[i_]);
		if (savedObSize >= MERGESTATE_TEMP_SIZE / 2)
			alifMem_objFree(keys_);
	}

	if (_self->allocate_ != -1 && result_ != nullptr) {
		result_ = nullptr;
	}

	if (_reverse && savedObSize > 1)
		reverse_slice(savedObItem, savedObItem + savedObSize);

	merge_freemem(&ms_);

keyfunc_fail:
	finalObItem = _self->items_;
	i_ = ALIF_SIZE(_self);
	ALIF_SIZE(_self, savedObSize);
	_self->items_ = savedObItem;
	_self->allocate_ = savedAllocated;
	if (finalObItem != nullptr) {
		while (--i_ >= 0) {
			ALIF_XDECREF(finalObItem[i_]);
		}
#ifdef ALIF_GIL_DISABLED
		bool useQSbr = alifSubObject_GC_is_shared(_self);
#else
		bool useQSbr = false;
#endif
		//free_list_items(finalObItem, useQSbr);
	}
	return ALIF_XNEWREF(result_);
}


int alifList_sort(AlifObject* _v)
{
	if (_v == nullptr || !(_v->type_ == &_alifListType_)) {
		return -1;
	}
	//ALIF_BEGIN_CRITICAL_SECTION(_v);
	_v = list_sort_impl((AlifListObject*)_v, nullptr, 0);
	//ALIF_END_CRITICAL_SECTION();
	if (_v == nullptr)
		return -1;
	ALIF_DECREF(_v);
	return 0;
}

AlifObject* alifList_asTuple(AlifObject* _v) {

	if (_v == nullptr || !(_v->type_ == &_alifListType_)) {
		//Err_BadInternalCall();
		return nullptr;
	}
	AlifObject* ret_;
	AlifListObject* self_ = (AlifListObject*)_v;
	//ALIF_BEGIN_CRITICAL_SECTION(_self);
	ret_ = alifSubTuple_fromArray(self_->items_, ALIF_SIZE(_v));
	//ALIF_END_CRITICAL_SECTION();
	return ret_;
}

//static AlifObject* list_count(AlifListObject* _self, AlifObject* _value)
//{
//    int64_t count = 0;
//    int64_t i_;
//
//    for (i_ = 0; i_ < ALIF_SIZE(_self); i_++) {
//        AlifObject* obj = _self->items_[i_];
//        if (obj == _value) {
//            count++;
//            continue;
//        }
//        int cmp = alifObject_richCompareBool(obj, _value, ALIF_EQ);
//        if (cmp > 0)
//            count++;
//        else if (cmp < 0)
//            return nullptr;
//    }
//    return alifInteger_fromLongLong(count);
//}

//static AlifObject* list_remove(AlifListObject* _self, AlifObject* _value)
//{
//    int64_t i_;
//
//    for (i_ = 0; i_ < ALIF_SIZE(_self); i_++) {
//        AlifObject* obj = _self->items_[i_];
//        int cmp = alifObject_richCompareBool(obj, _value, ALIF_EQ);
//        if (cmp > 0) {
//            if (list_ass_slice(_self, i_, i_ + 1,
//                (AlifObject*)nullptr) == 0)
//                return ALIF_NONE;
//            return nullptr;
//        }
//        else if (cmp < 0)
//            return nullptr;
//    }
//    return nullptr;
//}

static AlifObject* list_compare(AlifObject* _v, AlifObject* w_, int _op)
{
	AlifListObject* vl_, * wl_;
	int64_t i_;

	if (!(_v->type_ == &_alifListType_) || !(w_->type_ == &_alifListType_))
		return ALIF_NOTIMPLEMENTED;

	vl_ = (AlifListObject*)_v;
	wl_ = (AlifListObject*)w_;

	if (ALIF_SIZE(vl_) != ALIF_SIZE(wl_) && (_op == ALIF_EQ || _op == ALIF_NE)) {
		if (_op == ALIF_EQ)
			return ALIF_FALSE;
		else
			return ALIF_TRUE;
	}

	for (i_ = 0; i_ < ALIF_SIZE(vl_) && i_ < ALIF_SIZE(wl_); i_++) {
		AlifObject* vItem = vl_->items_[i_];
		AlifObject* wItem = wl_->items_[i_];
		if (vItem == wItem) {
			continue;
		}

		int k_ = alifObject_richCompareBool(vItem, wItem, ALIF_EQ);
		if (k_ < 0)
			return nullptr;
		if (!k_)
			break;
	}

	if (i_ >= ALIF_SIZE(vl_) || i_ >= ALIF_SIZE(wl_)) {
		ALIF_RETURN_RICHCOMPARE(ALIF_SIZE(vl_), ALIF_SIZE(wl_), _op);
	}

	if (_op == ALIF_EQ) {
		return ALIF_FALSE;
	}
	if (_op == ALIF_NE) {
		return ALIF_TRUE;
	}

	return alifObject_richCompare(vl_->items_[i_], wl_->items_[i_], _op);
}

static AlifObject* list___sizeof___impl(AlifListObject* _self)
{
	size_t res_ = _self->_base_._base_.type_->basicSize;
	res_ += (size_t)_self->allocate_ * sizeof(void*);
	return alifInteger_fromSizeT(res_, true);
}

static AlifObject* list___sizeof__(AlifListObject* _self)
{
	return list___sizeof___impl(_self);
}

static AlifObject* list_subscript(AlifObject*, AlifObject*);

static AlifMethodDef _listMethods_[] = {
	{L"__getItem__", (AlifCFunction)list_subscript, METHOD_O | METHOD_COEXIST,},
	{L"__sizeof__", (AlifCFunction)list___sizeof__, METHOD_NOARGS},
	{L"clear", (AlifCFunction)list_clear, METHOD_NOARGS},
	{L"append", (AlifCFunction)list_append, METHOD_O},
	{L"insert", ALIFCFunction_CAST(list_insert), METHOD_FASTCALL,},
	//{L"extend", (AlifCFunction)list_extend, METHOD_O},
	//{L"pop", ALIFCFunction_CAST(list_pop), METHOD_FASTCALL},
	//{L"remove", (AlifCFunction)list_remove, METHOD_O},
	//{L"count", (AlifCFunction)list_count, METHOD_O},
	{nullptr,              nullptr}
};

AlifSequenceMethods _listAsSeq_ = {
	(LenFunc)list_length,
	(BinaryFunc)list_concat,
	(SSizeArgFunc)list_repeat,
	(SSizeArgFunc)list_item,
	0,
	(SSizeObjArgProc)list_ass_item,
	0,
	(ObjObjProc)list_contains,
	0,
	0,
};

static inline AlifObject* list_slice_step_lock_held(AlifListObject* a, int64_t start, int64_t step, int64_t len)
{
	AlifListObject* np_ = (AlifListObject*)list_new_prealloc(len);
	if (np_ == NULL) {
		return NULL;
	}
	size_t cur_;
	int64_t i_;
	AlifObject** src_ = a->items_;
	AlifObject** dest_ = np_->items_;
	for (cur_ = start, i_ = 0; i_ < len;
		cur_ += (size_t)step, i_++) {
		AlifObject* v = src_[cur_];
		dest_[i_] = ALIF_NEWREF(v);
	}
	ALIFSET_SIZE(np_, len);
	return (AlifObject*)np_;
}

static AlifObject*
list_slice_wrap(AlifListObject* aa, int64_t start, int64_t stop, int64_t step)
{
	AlifObject* res_ = NULL;
	//ALIF_BEGIN_CRITICAL_SECTION(aa);
	int64_t len = alifSlice_adjustIndices(ALIF_SIZE(aa), &start, &stop, step);
	if (len <= 0) {
		res_ = alifNew_list(0);
	}
	else if (step == 1) {
		res_ = listSlice_lockHeld(aa, start, stop);
	}
	else {
		res_ = list_slice_step_lock_held(aa, start, step, len);
	}
	//ALIF_END_CRITICAL_SECTION();
	return res_;
}

static AlifObject*
list_subscript(AlifObject* _self, AlifObject* _item)
{
	AlifListObject* self = (AlifListObject*)_self;
	if ((_item->type_->asNumber != nullptr && _item->type_->asNumber->index_ != nullptr)) {
		int64_t i_;
		i_ = alifInteger_asSizeT(_item);
		if (i_ == -1)
			return NULL;
		if (i_ < 0)
			i_ += ALIFLIST_GET_SIZE(self);
		return list_item((AlifObject*)self, i_);
	}
	else if (ALIFSLICE_CHECK(_item)) {
		int64_t start, stop, step;
		if (slice_unpack((AlifSliceObject*)_item, &start, &stop, &step) < 0) {
			return NULL;
		}
		return list_slice_wrap(self, start, stop, step);
	}
	else {
		return NULL;
	}
}

static int list_ass_subscript(AlifObject* _self, AlifObject* _item, AlifObject* _value)
{
	AlifListObject* self_ = (AlifListObject*)_self;

	if ((_item)->type_->asNumber != nullptr) {
		int64_t i_ = alifInteger_asSizeT(_item);

		if (i_ < 0)
			i_ += ALIF_SIZE(self_);
		return list_ass_item((AlifObject*)self_, i_, _value);
	}
	else if (ALIFSLICE_CHECK(_item)) {
		int64_t start_, stop_, step_, slicelength;

		if (slice_unpack((AlifSliceObject*)_item, &start_, &stop_, &step_) < 0) {
			return -1;
		}
		slicelength = alifSlice_adjustIndices(ALIF_SIZE(self_), &start_, &stop_,
			step_);

		if (step_ == 1)
			return list_ass_slice(self_, start_, stop_, _value);

		/* Make sure s_[5:2] = [..] inserts at the right place:
		   before 5, not before 2. */
		if ((step_ < 0 && start_ < stop_) ||
			(step_ > 0 && start_ > stop_))
			stop_ = start_;

		if (_value == nullptr) {
			/* delete slice */
			AlifObject** garbage_;
			size_t cur_;
			int64_t i_;
			int res_;

			if (slicelength <= 0)
				return 0;

			if (step_ < 0) {
				stop_ = start_ + 1;
				start_ = stop_ + step_ * (slicelength - 1) - 1;
				step_ = -step_;
			}

			garbage_ = (AlifObject**)
				alifMem_dataAlloc(slicelength * sizeof(AlifObject*));
			if (!garbage_) {

				return -1;
			}
			for (cur_ = start_, i_ = 0;
				cur_ < (size_t)stop_;
				cur_ += step_, i_++) {
				int64_t lim_ = step_ - 1;

				garbage_[i_] = ((AlifListObject*)self_)->items_[cur_];

				if (cur_ + step_ >= (size_t)ALIF_SIZE(self_)) {
					lim_ = ALIF_SIZE(self_) - cur_ - 1;
				}

				memmove(self_->items_ + cur_ - i_,
					self_->items_ + cur_ + 1,
					lim_ * sizeof(AlifObject*));
			}
			cur_ = start_ + (size_t)slicelength * step_;
			if (cur_ < (size_t)ALIF_SIZE(self_)) {
				memmove(self_->items_ + cur_ - slicelength,
					self_->items_ + cur_,
					(ALIF_SIZE(self_) - cur_) *
					sizeof(AlifObject*));
			}

			ALIF_SIZE(self_, ALIF_SIZE(self_) - slicelength);
			res_ = list_resize(self_, ALIF_SIZE(self_));

			alifMem_dataFree(garbage_);

			return res_;
		}
		else {
			AlifObject* ins_, * seq_;
			AlifObject** garbage_, ** seqItems, ** selfItems;
			int64_t i_;
			size_t cur_;

			if (self_ == (AlifListObject*)_value) {
				seq_ = listSlice_lockHeld((AlifListObject*)_value, 0,
					ALIF_SIZE(_value));
			}
			else {
				seq_ = alifSequence_fast(_value,
					L"must assign iterable "
					"to extended slice");
			}
			if (!seq_)
				return -1;

			if (ALIFSEQUENCE_FAST_GETSIZE(seq_) != slicelength) {

				return -1;
			}

			if (!slicelength) {
				return 0;
			}

			garbage_ = (AlifObject**)
				alifMem_dataAlloc(slicelength * sizeof(AlifObject*));
			if (!garbage_) {

				return -1;
			}

			selfItems = self_->items_;
			seqItems = ALIFSEQUENCE_FAST_ITEMS(seq_);
			for (cur_ = start_, i_ = 0; i_ < slicelength;
				cur_ += (size_t)step_, i_++) {
				garbage_[i_] = selfItems[cur_];
				ins_ = seqItems[i_];
				selfItems[cur_] = ins_;
			}

			alifMem_dataFree(garbage_);

			return 0;
		}
	}
	else {
		return -1;
	}
}

AlifMappingMethods _listAsMap_ = {
	(LenFunc)list_length,
	(BinaryFunc)list_subscript,
	(ObjObjArgProc)list_ass_subscript
};

AlifTypeObject _alifListType_ = {
	0,
	0,
	0,
	L"list",
	sizeof(AlifListObject),
	0,
	list_dealloc,
	0,
	0,
	0,
	0,
	0,
	&_listAsSeq_,
	&_listAsMap_,
	(HashFunc)alifObject_hashNotImplemented,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	list_clear_slot,
	list_compare,
	0,
	0,
	0,
	_listMethods_,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	alifObject_gc_del,
	0,
};
