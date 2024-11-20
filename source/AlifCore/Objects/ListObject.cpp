#include "alif.h"

#include "AlifCore_Dict.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_List.h"
#include "AlifCore_Object.h"
#include "AlifCore_SetObject.h"


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

static AlifIntT list_preallocateExact(AlifListObject* _self,
	AlifSizeT _size) { //167
	AlifObject** items{};
	_size = (_size + 1) & ~(size_t)1;
#ifdef ALIF_GIL_DISABLED
	AlifListArray* array = list_allocateArray(_size);
	if (array == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	items = array->item;
	memset(items, 0, _size * sizeof(AlifObject*));
#else
	items = (AlifObject**)alifMem_objAlloc(_size * sizeof(AlifObject*)); // alif
	if (items == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
#endif
	alifAtomic_storePtrRelaxed(&_self->item, items);
	_self->allocated = _size;
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

AlifSizeT alifList_size(AlifObject* _op) { // 279
	if (!ALIFLIST_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	else {
		return ALIFLIST_GET_SIZE(_op);
	}
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


static AlifObject* listSlice_lockHeld(AlifListObject* _a,
	AlifSizeT _iLow, AlifSizeT _iHigh) { // 635
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


static AlifIntT listInplaceRepeat_lockHeld(AlifListObject* _self, AlifSizeT _n) { // 968
	AlifSizeT inputSize = ALIFLIST_GET_SIZE(_self);
	if (inputSize == 0 or _n == 1) {
		return 0;
	}

	if (_n < 1) {
		list_clear(_self);
		return 0;
	}

	if (inputSize > ALIF_SIZET_MAX / _n) {
		//alifErr_noMemory();
		return -1;
	}
	AlifSizeT outputSize = inputSize * _n;

	if (list_resize(_self, outputSize) < 0) {
		return -1;
	}

	AlifObject** items = _self->item;
	for (AlifSizeT j = 0; j < inputSize; j++) {
		ALIF_REFCNTADD(items[j], _n - 1);
	}
	alif_memoryRepeat((char*)items, sizeof(AlifObject*) * outputSize,
		sizeof(AlifObject*) * inputSize);
	return 0;
}

static AlifIntT list_extendFast(AlifListObject* _self, AlifObject* _iterable) { // 1120
	AlifSizeT n_ = ALIFSEQUENCE_FAST_GET_SIZE(_iterable);
	if (n_ == 0) {
		return 0;
	}

	AlifSizeT m_ = ALIF_SIZE(_self);
	if (_self->item == nullptr) {
		if (list_preallocateExact(_self, n_) < 0) {
			return -1;
		}
		ALIF_SET_SIZE(_self, n_);
	}
	else if (list_resize(_self, m_ + n_) < 0) {
		return -1;
	}
	AlifObject** src_ = ALIFSEQUENCE_FAST_ITEMS(_iterable);
	AlifObject** dest = _self->item + m_;
	for (AlifSizeT i_ = 0; i_ < n_; i_++) {
		AlifObject* o_ = src_[i_];
		alifAtomic_storePtrRelaxed(&dest[i_], ALIF_NEWREF(o_));
	}
	return 0;
}



static AlifIntT listExtendIter_lockHeld(AlifListObject* _self, AlifObject* _iterable) { // 1157
	AlifObject* it = alifObject_getIter(_iterable);
	if (it == nullptr) {
		return -1;
	}
	AlifObject* (*iternext)(AlifObject*) = *ALIF_TYPE(it)->iterNext;

	/* Guess a result list size. */
	AlifSizeT n = alifObject_lengthHint(_iterable, 8);
	if (n < 0) {
		ALIF_DECREF(it);
		return -1;
	}

	AlifSizeT m = ALIF_SIZE(_self);
	if (m > ALIF_SIZET_MAX - n) {
		/* m + n overflowed; on the chance that n lied, and there really
		 * is enough room, ignore it.  If n was telling the truth, we'll
		 * eventually run out of memory during the loop.
		 */
	}
	else if (_self->item == nullptr) {
		if (n and list_preallocateExact(_self, n) < 0)
			goto error;
	}
	else {
		/* Make room. */
		if (list_resize(_self, m + n) < 0) {
			goto error;
		}

		/* Make the list sane again. */
		ALIF_SET_SIZE(_self, m);
	}

	/* Run iterator to exhaustion. */
	for (;;) {
		AlifObject* item = iternext(it);
		if (item == nullptr) {
			//if (alifErr_occurred()) {
			//	if (alifErr_exceptionMatches(_alifExcStopIteration_))
			//		alifErr_clear();
			//	else
			//		goto error;
			//}
			break;
		}

		if (ALIF_SIZE(_self) < _self->allocated) {
			AlifSizeT len = ALIF_SIZE(_self);
			alifAtomic_storePtrRelease(&_self->item[len], item); // alif
			ALIF_SET_SIZE(_self, len + 1);
		}
		else {
			if (alifList_appendTakeRef(_self, item) < 0)
				goto error;
		}
	}

	/* Cut back result list if initial guess was too large. */
	if (ALIF_SIZE(_self) < _self->allocated) {
		if (list_resize(_self, ALIF_SIZE(_self)) < 0)
			goto error;
	}

	ALIF_DECREF(it);
	return 0;

error:
	ALIF_DECREF(it);
	return -1;
}


static AlifIntT listExtend_lockHeld(AlifListObject* _self, AlifObject* _iterable) { // 1233
	AlifObject* seq_ = alifSequence_fast(_iterable, "argument must be iterable");
	if (!seq_) {
		return -1;
	}

	AlifIntT res_ = list_extendFast(_self, seq_);
	ALIF_DECREF(seq_);
	return res_;
}

static AlifIntT listExtend_set(AlifListObject* _self, AlifSetObject* _other) { // 1245
	AlifSizeT m_ = ALIF_SIZE(_self);
	AlifSizeT n_ = ALIFSET_GET_SIZE(_other);
	if (list_resize(_self, m_ + n_) < 0) {
		return -1;
	}
	/* populate the end of self with iterable's items */
	AlifSizeT setpos = 0;
	AlifHashT hash{};
	AlifObject* key{};
	AlifObject** dest = _self->item + m_;
	while (_alifSet_nextEntryRef((AlifObject*)_other, &setpos, &key, &hash)) {
		alifAtomic_storePtrRelease(&*dest, key);
		dest++;
	}
	ALIF_SET_SIZE(_self, m_ + n_);
	return 0;
}

static AlifIntT listExtend_dict(AlifListObject* _self, AlifDictObject* _dict, AlifIntT _whichItem) { // 1267
	AlifSizeT m_ = ALIF_SIZE(_self);
	AlifSizeT n_ = ALIFDICT_GET_SIZE(_dict);
	if (list_resize(_self, m_ + n_) < 0) {
		return -1;
	}

	AlifObject** dest = _self->item + m_;
	AlifSizeT pos_ = 0;
	AlifObject* keyValue[2]{};
	while (_alifDict_next((AlifObject*)_dict, &pos_, &keyValue[0], &keyValue[1], nullptr)) {
		AlifObject* obj = keyValue[_whichItem];
		ALIF_INCREF(obj);
		alifAtomic_storePtrRelaxed(&*dest, obj);
		dest++;
	}

	ALIF_SET_SIZE(_self, m_ + n_);
	return 0;
}

static AlifIntT listExtend_dictItems(AlifListObject* _self, AlifDictObject* _dict) { // 1291
	AlifSizeT m_ = ALIF_SIZE(_self);
	AlifSizeT n_ = ALIFDICT_GET_SIZE(_dict);
	if (list_resize(_self, m_ + n_) < 0) {
		return -1;
	}

	AlifObject** dest = _self->item + m_;
	AlifSizeT pos_ = 0;
	AlifSizeT i_ = 0;
	AlifObject* key_{}, * value{};
	while (_alifDict_next((AlifObject*)_dict, &pos_, &key_, &value, nullptr)) {
		AlifObject* item = alifTuple_pack(2, key_, value);
		if (item == nullptr) {
			ALIF_SET_SIZE(_self, m_ + i_);
			return -1;
		}
		alifAtomic_storePtrRelaxed(&*dest, item);
		dest++;
		i_++;
	}

	ALIF_SET_SIZE(_self, m_ + n_);
	return 0;
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



static AlifObject* list_sortImpl(AlifListObject* _self,
	AlifObject* _keyFunc, AlifIntT _reverse) { // 2814
	MergeState ms{};
	AlifSizeT nremaining{};
	AlifSizeT minrun{};
	SortSlice lo{};
	AlifSizeT savedObjSize{}, savedAllocated{};
	AlifObject** savedObjItem{};
	AlifObject** finalObjItem{};
	AlifObject* result = nullptr;            /* guilty until proved innocent */
	AlifSizeT i{};
	AlifObject** keys{};

	if (_keyFunc == ALIF_NONE) _keyFunc = nullptr;

	savedObjSize = ALIF_SIZE(_self);
	savedObjItem = _self->item;
	savedAllocated = _self->allocated;
	ALIF_SET_SIZE(_self, 0);
	alifAtomic_storePtrRelease(&_self->item, nullptr);
	_self->allocated = -1; /* any operation will reset it to >= 0 */

	if (_keyFunc == nullptr) {
		keys = nullptr;
		lo.keys = savedObjItem;
		lo.values = nullptr;
	}
	else {
		if (savedObjSize < MERGESTATE_TEMP_SIZE / 2)
			/* Leverage stack space we allocated but won't otherwise use */
			keys = &ms.temparray[savedObjSize + 1];
		else {
			keys = (AlifObject**)alifMem_dataAlloc(sizeof(AlifObject*) * savedObjSize);
			if (keys == nullptr) {
				//alifErr_noMemory();
				goto keyFuncFail;
			}
		}

		for (i = 0; i < savedObjSize; i++) {
			keys[i] = alifObject_callOneArg(_keyFunc, savedObjItem[i]);
			if (keys[i] == nullptr) {
				for (i = i - 1; i >= 0; i--)
					ALIF_DECREF(keys[i]);
				if (savedObjSize >= MERGESTATE_TEMP_SIZE / 2)
					alifMem_dataFree(keys);
				goto keyFuncFail;
			}
		}

		lo.keys = keys;
		lo.values = savedObjItem;
	}


	if (savedObjSize > 1) {
		/* Assume the first element is representative of the whole list. */
		AlifIntT keysAreInTuples = (ALIF_IS_TYPE(lo.keys[0], &_alifTupleType_) and
			ALIF_SIZE(lo.keys[0]) > 0);

		AlifTypeObject* keyType = (keysAreInTuples ?
			ALIF_TYPE(ALIFTUPLE_GET_ITEM(lo.keys[0], 0)) :
			ALIF_TYPE(lo.keys[0]));

		AlifIntT keysAreAllSameType = 1;
		AlifIntT stringsAreLatin = 1;
		AlifIntT intsAreBounded = 1;

		/* Prove that assumption by checking every key. */
		for (i = 0; i < savedObjSize; i++) {

			if (keysAreInTuples &&
				!(ALIF_IS_TYPE(lo.keys[i], &_alifTupleType_)
					and ALIF_SIZE(lo.keys[i]) != 0)) {
				keysAreInTuples = 0;
				keysAreAllSameType = 0;
				break;
			}

			AlifObject* key = (keysAreInTuples ?
				ALIFTUPLE_GET_ITEM(lo.keys[i], 0) :
				lo.keys[i]);

			if (!ALIF_IS_TYPE(key, keyType)) {
				keysAreAllSameType = 0;
				if (!keysAreInTuples) {
					break;
				}
			}

			if (keysAreAllSameType) {
				if (keyType == &_alifLongType_ and
					intsAreBounded and
					!alifLong_isCompact((AlifLongObject*)key)) {

					intsAreBounded = 0;
				}
				else if (keyType == &_alifUStrType_ and
					stringsAreLatin and
					ALIFUSTR_KIND(key) != AlifUStrKind_::AlifUStr_1Byte_Kind) {

					stringsAreLatin = 0;
				}
			}
		}

		/* Choose the best compare, given what we now know about the keys. */
		if (keysAreAllSameType) {

			if (keyType == &_alifUStrType_ and stringsAreLatin) {
				ms.key_compare = unsafe_latinCompare;
			}
			else if (keyType == &_alifLongType_ and intsAreBounded) {
				ms.key_compare = unsafe_longCompare;
			}
			else if (keyType == &_alifFloatType_) {
				ms.key_compare = unsafe_floatCompare;
			}
			else if ((ms.key_richcompare = keyType->richCompare) != nullptr) {
				ms.key_compare = unsafe_objectCompare;
			}
			else {
				ms.key_compare = safe_objectCompare;
			}
		}
		else {
			ms.key_compare = safe_objectCompare;
		}

		if (keysAreInTuples) {
			/* Make sure we're not dealing with tuples of tuples
			 * (remember: here, key_type refers list [key[0] for key in keys]) */
			if (keyType == &_alifTupleType_) {
				ms.tuple_elem_compare = safe_objectCompare;
			}
			else {
				ms.tuple_elem_compare = ms.key_compare;
			}

			ms.key_compare = unsafe_tupleCompare;
		}
	}

	merge_init(&ms, savedObjSize, keys != nullptr, &lo);

	nremaining = savedObjSize;
	if (nremaining < 2) goto succeed;

	if (_reverse) {
		if (keys != nullptr)
			reverse_slice(&keys[0], &keys[savedObjSize]);
		reverse_slice(&savedObjItem[0], &savedObjItem[savedObjSize]);
	}

	minrun = merge_computeMinRun(nremaining);
	do {
		AlifSizeT n{};

		/* Identify next run. */
		n = count_run(&ms, &lo, nremaining);
		if (n < 0) goto fail;
		/* If short, extend to min(minrun, nremaining). */
		if (n < minrun) {
			const AlifSizeT force = nremaining <= minrun ?
				nremaining : minrun;
			if (binarysort(&ms, &lo, force, n) < 0)
				goto fail;
			n = force;
		}
		/* Maybe merge pending runs. */
		if (found_newRun(&ms, n) < 0)
			goto fail;
		/* Push new run on stack. */
		ms.pending[ms.n].base = lo;
		ms.pending[ms.n].len = n;
		++ms.n;
		/* Advance to find next run. */
		sortSlice_advance(&lo, n);
		nremaining -= n;
	} while (nremaining);

	if (merge_forceCollapse(&ms) < 0)
		goto fail;
	lo = ms.pending[0].base;

succeed:
	result = ALIF_NONE;
fail:
	if (keys != nullptr) {
		for (i = 0; i < savedObjSize; i++)
			ALIF_DECREF(keys[i]);
		if (savedObjSize >= MERGESTATE_TEMP_SIZE / 2)
			alifMem_dataFree(keys);
	}

	if (_self->allocated != -1 and result != nullptr) {
		//alifErr_setString(_alifExcValueError_, "list modified during sort");
		result = nullptr;
	}

	if (_reverse and savedObjSize > 1)
		reverse_slice(savedObjItem, savedObjItem + savedObjSize);

	merge_freeMem(&ms);

keyFuncFail:
	finalObjItem = _self->item;
	i = ALIF_SIZE(_self);
	ALIF_SET_SIZE(_self, savedObjSize);
	alifAtomic_storePtrRelease(&_self->item, savedObjItem);
	alifAtomic_storeSizeRelaxed(&_self->allocated, savedAllocated);
	if (finalObjItem != nullptr) {
		/* we cannot use list_clear() for this because it does not
		   guarantee that the list is really empty when it returns */
		while (--i >= 0) {
			ALIF_XDECREF(finalObjItem[i]);
		}
#ifdef ALIF_GIL_DISABLED
		bool use_qsbr = ALIFOBJECT_GC_IS_SHARED(_self);
#else
		bool use_qsbr = false;
#endif
		free_listItems(finalObjItem, use_qsbr);
	}
	return ALIF_XNEWREF(result);
}
#undef IFLT
#undef ISLT

AlifIntT alifList_sort(AlifObject* _v) { // 3080
	if (_v == nullptr or !ALIFLIST_CHECK(_v)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	ALIF_BEGIN_CRITICAL_SECTION(_v);
	_v = list_sortImpl((AlifListObject*)_v, nullptr, 0);
	ALIF_END_CRITICAL_SECTION();
	if (_v == nullptr) return -1;
	ALIF_DECREF(_v);
	return 0;
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
