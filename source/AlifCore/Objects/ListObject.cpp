#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_List.h"
#include "AlifCore_Object.h"


#ifdef ALIF_GIL_DISABLED
class AlifListArray{ // 28
public:
	AlifSizeT allocated{};
	AlifObject* item[0];
};
static AlifListArray* list_allocateArray(AlifUSizeT _capacity) { // 34
	if (_capacity > ALIF_SIZET_MAX / sizeof(AlifObject*) - 1) {
		return nullptr;
	}
	AlifListArray* array = (AlifListArray*)alifMem_objAlloc(sizeof(AlifListArray) + _capacity * sizeof(AlifObject*));
	if (array == nullptr) {
		return nullptr;
	}
	array->allocated = _capacity;
	return array;
}
#endif



static void free_listItems(AlifObject** _items, bool _useQSBR) { // 55
#ifdef ALIF_GIL_DISABLED
	AlifListArray* array = ALIF_CONTAINER_OF(_items, AlifListArray, item);
	if (_useQSBR) {
		alifMem_freeDelayed(array);
	}
	else {
		alifMem_objFree(array);
	}
#else
	alifMem_objFree(_items);
#endif
}






static AlifIntT list_resize(AlifListObject* _self, AlifSizeT _newSize) { // 84
	AlifUSizeT newAllocated{}, targetBytes{};
	AlifSizeT allocated = _self->allocated;

	if (allocated >= _newSize and _newSize >= (allocated >> 1)) {
		ALIF_SET_SIZE(_self, _newSize);
		return 0;
	}

	newAllocated = ((AlifUSizeT)_newSize + (_newSize >> 3) + 6) & ~(AlifUSizeT)3;
	if (_newSize - ALIF_SIZE(_self) > (AlifSizeT)(newAllocated - _newSize))
		newAllocated = ((AlifUSizeT)_newSize + 3) & ~(AlifUSizeT)3;

	if (_newSize == 0)
		newAllocated = 0;

#ifdef ALIF_GIL_DISABLED
	AlifListArray* array = list_allocateArray(newAllocated);
	if (array == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	AlifObject** oldItems = _self->item;
	if (_self->item) {
		if (newAllocated < (AlifUSizeT)allocated) {
			targetBytes = newAllocated * sizeof(AlifObject*);
		}
		else {
			targetBytes = allocated * sizeof(AlifObject*);
		}
		memcpy(array->item, _self->item, targetBytes);
	}
	if (newAllocated > (AlifUSizeT)allocated) {
		memset(array->item + allocated, 0, sizeof(AlifObject*) * (newAllocated - allocated));
	}
	alifAtomic_storePtrRelease(&_self->item, &array->item);
	_self->allocated = newAllocated;
	ALIF_SET_SIZE(_self, _newSize);
	if (oldItems != nullptr) {
		free_listItems(oldItems, ALIFOBJECT_GC_IS_SHARED(_self));
	}
#else
	AlifObject** items{};
	if (newAllocated <= (AlifUSizeT)ALIF_SIZET_MAX / sizeof(AlifObject*)) {
		targetBytes = newAllocated * sizeof(AlifObject*);
		items = (AlifObject**)alifMem_objRealloc(_self->item, targetBytes);
	}
	else {
		// integer overflow
		items = nullptr;
	}
	if (items == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	_self->item = items;
	ALIF_SET_SIZE(_self, _newSize);
	_self->allocated = newAllocated;
#endif
	return 0;
}



AlifObject* alifList_new(AlifSizeT _size) { // 212 
	if (_size < 0) {
		//alifErr_badInternalCall();
		return nullptr;
	}

	AlifListObject* op_ = ALIF_FREELIST_POP(AlifListObject, lists);
	if (op_ == nullptr) {
		op_ = ALIFOBJECT_GC_NEW(AlifListObject, &_alifListType_);
		if (op_ == nullptr) {
			return nullptr;
		}
	}
	if (_size <= 0) {
		op_->item = nullptr;
	}
	else {
#ifdef ALIF_GIL_DISABLED
		AlifListArray* array = list_allocateArray(_size);
		if (array == nullptr) {
			ALIF_DECREF(op_);
			return nullptr;
			//return alifErr_noMemory();
		}
		memset(&array->item, 0, _size * sizeof(AlifObject*));
		op_->item = array->item;
#else
		op_->item = (AlifObject**)alifMem_objAlloc(_size* sizeof(AlifObject*));
#endif
		if (op_->item == nullptr) {
			ALIF_DECREF(op_);
			return nullptr;
			//return alifErr_noMemory();
		}
	}
	ALIF_SET_SIZE(op_, _size);
	op_->allocated = _size;
	ALIFOBJECT_GC_TRACK(op_);
	return (AlifObject*)op_;
}

static AlifObject* list_newPrealloc(AlifSizeT _size) { // 252
	AlifListObject* op_ = (AlifListObject*)alifList_new(0);
	if (op_ == nullptr) {
		return nullptr;
	}
#ifdef ALIF_GIL_DISABLED
	AlifListArray* array = list_allocateArray(_size);
	if (array == nullptr) {
		ALIF_DECREF(op_);
		//return alifErr_noMemory();
	}
	op_->item = array->item;
#else
	op_->item = alifMem_objAlloc(AlifObject*, _size);
	if (op_->item == nullptr) {
		ALIF_DECREF(op_);
		//return alifErr_noMemory();
	}
#endif
	op_->allocated = _size;
	return (AlifObject*)op_;
}

AlifIntT alifList_appendTakeRefListResize(AlifListObject* _self,
	AlifObject* _newItem) { // 468
	AlifSizeT len_ = ALIF_SIZE(_self);
	if (list_resize(_self, len_ + 1) < 0) {
		ALIF_DECREF(_newItem);
		return -1;
	}
	alifAtomic_storePtrRelease(&_self->item[len_], _newItem);
	return 0;
}

AlifIntT alifList_append(AlifObject* _op, AlifObject* _newItem) { // 481
	if (ALIFLIST_CHECK(_op) and (_newItem != nullptr)) {
		AlifIntT ret_{};
		ALIF_BEGIN_CRITICAL_SECTION(_op);
		ret_ = alifList_appendTakeRef((AlifListObject*)_op, ALIF_NEWREF(_newItem));
		ALIF_END_CRITICAL_SECTION();
		return ret_;
	}
	//ALIFERR_BADINTERNALCALL();
	return -1;
}

static void list_dealloc(AlifObject* _self) { // 497
	AlifListObject* op_ = (AlifListObject*)_self;
	AlifSizeT i_{};
	ALIFOBJECT_GC_UNTRACK(op_);
	//ALIF_TRASHCAN_BEGIN(op_, list_dealloc)
		if (op_->item != nullptr) {
			i_ = ALIF_SIZE(op_);
			while (--i_ >= 0) {
				ALIF_XDECREF(op_->item[i_]);
			}
			free_listItems(op_->item, false);
		}
	if (ALIFLIST_CHECKEXACT(op_)) {
		ALIF_FREELIST_FREE(lists, LISTS, op_, alifObject_gcDel);
	}
	else {
		alifObject_gcDel(op_);
	}
	//ALIF_TRASHCAN_END
}


static AlifObject* listSlice_lockHeld(AlifListObject* _a, AlifSizeT _iLow, AlifSizeT _iHigh) { // 635
	AlifListObject* np_{};
	AlifObject** src_{}, ** dest{};
	AlifSizeT i_{}, len_{};
	len_ = _iHigh - _iLow;
	if (len_ <= 0) {
		return alifList_new(0);
	}
	np_ = (AlifListObject*)list_newPrealloc(len_);
	if (np_ == nullptr)
		return nullptr;

	src_ = _a->item + _iLow;
	dest = np_->item;
	for (i_ = 0; i_ < len_; i_++) {
		AlifObject* _v = src_[i_];
		dest[i_] = ALIF_NEWREF(_v);
	}
	ALIF_SET_SIZE(np_, len_);
	return (AlifObject*)np_;
}

static void list_clearImpl(AlifListObject* _a, bool _isResize) { // 787
	AlifObject** items = _a->item;
	if (items == nullptr) {
		return;
	}

	AlifSizeT i_ = ALIF_SIZE(_a);
	ALIF_SET_SIZE(_a, 0);
	alifAtomic_storePtrRelease(&_a->item, nullptr);
	_a->allocated = 0;
	while (--i_ >= 0) {
		ALIF_XDECREF(items[i_]);
	}
#ifdef ALIF_GIL_DISABLED
	bool useQsbr = _isResize and ALIFOBJECT_GC_IS_SHARED(_a);
#else
	bool useQsbr = false;
#endif
	free_listItems(items, useQsbr);
}

static void list_clear(AlifListObject* _a) { // 814
	list_clearImpl(_a, true);
}

static AlifIntT listAssSlice_lockHeld(AlifListObject* _a, AlifSizeT _iLow,
	AlifSizeT _iHigh, AlifObject* _v) { // 833 
	AlifObject* recycleOnStack[8]{};
	AlifObject** recycle = recycleOnStack; /* will allocate more if needed */
	AlifObject** item{};
	AlifObject** vitem = nullptr;
	AlifObject* vAsSF = nullptr;	/* alifSequence_fast(_v) */
	AlifSizeT n_{};					/* # of elements in replacement list */
	AlifSizeT norig{};				/* # of elements in list getting replaced */
	AlifSizeT d_{};					/* Change in size */
	AlifSizeT k_{};
	AlifUSizeT s_{};
	AlifIntT result = -1;            /* guilty until proved innocent */
#define b ((AlifListObject *)_v)
	if (_v == nullptr)
		n_ = 0;
	else {
		vAsSF = alifSequence_fast(_v, "can only assign an iterable");
		if (vAsSF == nullptr)
			goto Error;
		n_ = ALIFSEQUENCE_FAST_GET_SIZE(vAsSF);
		vitem = ALIFSEQUENCE_FAST_ITEMS(vAsSF);
	}
	if (_iLow < 0)
		_iLow = 0;
	else if (_iLow > ALIF_SIZE(_a))
		_iLow = ALIF_SIZE(_a);

	if (_iHigh < _iLow)
		_iHigh = _iLow;
	else if (_iHigh > ALIF_SIZE(_a))
		_iHigh = ALIF_SIZE(_a);

	norig = _iHigh - _iLow;
	d_ = n_ - norig;
	if (ALIF_SIZE(_a) + d_ == 0) {
		ALIF_XDECREF(vAsSF);
		list_clear(_a);
		return 0;
	}
	item = _a->item;
	s_ = norig * sizeof(AlifObject*);
	if (s_) {
		if (s_ > sizeof(recycleOnStack)) {
			recycle = (AlifObject**)alifMem_objAlloc(s_);
			if (recycle == nullptr) {
				//alifErr_noMemory();
				goto Error;
			}
		}
		memcpy(recycle, &item[_iLow], s_);
	}

	if (d_ < 0) { /* Delete -d_ items */
		AlifSizeT tail{};
		tail = (ALIF_SIZE(_a) - _iHigh) * sizeof(AlifObject*);
		memmove(&item[_iHigh + d_], &item[_iHigh], tail);
		if (list_resize(_a, ALIF_SIZE(_a) + d_) < 0) {
			memmove(&item[_iHigh], &item[_iHigh + d_], tail);
			memcpy(&item[_iLow], recycle, s_);
			goto Error;
		}
		item = _a->item;
	}
	else if (d_ > 0) { /* Insert d_ items */
		k_ = ALIF_SIZE(_a);
		if (list_resize(_a, k_ + d_) < 0)
			goto Error;
		item = _a->item;
		memmove(&item[_iHigh + d_], &item[_iHigh],
			(k_ - _iHigh) * sizeof(AlifObject*));
	}
	for (k_ = 0; k_ < n_; k_++, _iLow++) {
		AlifObject* w_ = vitem[k_];
		item[_iLow] = ALIF_XNEWREF(w_);
	}
	for (k_ = norig - 1; k_ >= 0; --k_)
		ALIF_XDECREF(recycle[k_]);
	result = 0;
Error:
	if (recycle != recycleOnStack)
		alifMem_objFree(recycle);
	ALIF_XDECREF(vAsSF);
	return result;
#undef b
}


static AlifIntT list_assSlice(AlifListObject* _a, AlifSizeT _iLow,
	AlifSizeT _iHigh, AlifObject* _v) { // 930
	AlifIntT ret_{};
	if (_a == (AlifListObject*)_v) {
		ALIF_BEGIN_CRITICAL_SECTION(_a);
		AlifSizeT n_ = ALIFLIST_GET_SIZE(_a);
		AlifObject* copy = listSlice_lockHeld(_a, 0, n_);
		if (copy == nullptr) {
			return -1;
		}
		ret_ = listAssSlice_lockHeld(_a, _iLow, _iHigh, copy);
		ALIF_DECREF(copy);
		ALIF_END_CRITICAL_SECTION();
	}
	else if (_v != nullptr and ALIFLIST_CHECKEXACT(_v)) {
		ALIF_BEGIN_CRITICAL_SECTION2(_a, _v);
		ret_ = listAssSlice_lockHeld(_a, _iLow, _iHigh, _v);
		ALIF_END_CRITICAL_SECTION2();
	}
	else {
		ALIF_BEGIN_CRITICAL_SECTION(_a);
		ret_ = listAssSlice_lockHeld(_a, _iLow, _iHigh, _v);
		ALIF_END_CRITICAL_SECTION();
	}
	return ret_;
}

AlifIntT alifList_setSlice(AlifObject* _a, AlifSizeT _iLow,
	AlifSizeT _iHigh, AlifObject* _v) { // 958
	if (!ALIFLIST_CHECK(_a)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	return list_assSlice((AlifListObject*)_a, _iLow, _iHigh, _v);
}




static AlifIntT _list_extend(AlifListObject* _self, AlifObject* _iterable) { // 1318
	// Special case:
	// lists and tuples which can use alifSequence_fast ops
	int res = -1;
	if ((AlifObject*)_self == _iterable) {
		ALIF_BEGIN_CRITICAL_SECTION(_self);
		res = listInplaceRepeat_lockHeld(_self, 2);
		ALIF_END_CRITICAL_SECTION();
	}
	else if (ALIFLIST_CHECKEXACT(_iterable)) {
		ALIF_BEGIN_CRITICAL_SECTION2(_self, _iterable);
		res = listExtend_lockHeld(_self, _iterable);
		ALIF_END_CRITICAL_SECTION2();
	}
	else if (ALIFTUPLE_CHECKEXACT(_iterable)) {
		ALIF_BEGIN_CRITICAL_SECTION(_self);
		res = listExtend_lockHeld(_self, _iterable);
		ALIF_END_CRITICAL_SECTION();
	}
	else if (ALIFANYSET_CHECKEXACT(_iterable)) {
		ALIF_BEGIN_CRITICAL_SECTION2(_self, _iterable);
		res = listExtend_set(_self, (AlifSetObject*)_iterable);
		ALIF_END_CRITICAL_SECTION2();
	}
	else if (ALIFDICT_CHECKEXACT(_iterable)) {
		ALIF_BEGIN_CRITICAL_SECTION2(_self, _iterable);
		res = listExtend_dict(_self, (AlifDictObject*)_iterable, 0 /*keys*/);
		ALIF_END_CRITICAL_SECTION2();
	}
	else if (ALIF_IS_TYPE(_iterable, &_alifDictKeysType_)) {
		AlifDictObject* dict = ((AlifDictViewObject*)_iterable)->dict;
		ALIF_BEGIN_CRITICAL_SECTION2(_self, dict);
		res = listExtend_dict(_self, dict, 0 /*keys*/);
		ALIF_END_CRITICAL_SECTION2();
	}
	else if (ALIF_IS_TYPE(_iterable, &_alifDictValuesType_)) {
		AlifDictObject* dict = ((AlifDictViewObject*)_iterable)->dict;
		ALIF_BEGIN_CRITICAL_SECTION2(_self, dict);
		res = listExtend_dict(_self, dict, 1 /*values*/);
		ALIF_END_CRITICAL_SECTION2();
	}
	else if (ALIF_IS_TYPE(_iterable, &_alifDictItemsType_)) {
		AlifDictObject* dict = ((AlifDictViewObject*)_iterable)->dict;
		ALIF_BEGIN_CRITICAL_SECTION2(_self, dict);
		res = listExtend_dictItems(_self, dict);
		ALIF_END_CRITICAL_SECTION2();
	}
	else {
		ALIF_BEGIN_CRITICAL_SECTION(_self);
		res = listExtendIter_lockHeld(_self, _iterable);
		ALIF_END_CRITICAL_SECTION();
	}
	return res;
}

static AlifObject* list_extend(AlifListObject* _self, AlifObject* _iterable) { // 1384
	if (_list_extend(_self, _iterable) < 0) {
		return nullptr;
	}
	return ALIF_NONE; // alif
}

AlifObject* _alifList_extend(AlifListObject* _self, AlifObject* _iterable) { // 1394
	return list_extend(_self, _iterable);
}





AlifObject* alifList_asTuple(AlifObject* _v) { // 3129
	if (_v == nullptr or !ALIFLIST_CHECK(_v)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	AlifObject* ret_{};
	AlifListObject* self = (AlifListObject*)_v;
	ALIF_BEGIN_CRITICAL_SECTION(self);
	ret_ = alifTuple_fromArray(self->item, ALIF_SIZE(_v));
	ALIF_END_CRITICAL_SECTION();
	return ret_;
}







AlifTypeObject _alifListType_ = { // 3737
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مصفوفة",
	.basicSize = sizeof(AlifListObject),
	.itemSize = 0,
	.dealloc = list_dealloc,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_LIST_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF | ALIF_TPFLAGS_SEQUENCE,
};
