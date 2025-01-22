#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Dict.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_List.h"
#include "AlifCore_Object.h"
#include "AlifCore_SetObject.h"


#include "clinic/ListObject.cpp.h"

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

static AlifSizeT list_capacity(AlifObject** _items) { // 48
	AlifListArray* array = ALIF_CONTAINER_OF(_items, AlifListArray, item);
	return array->allocated;
}



static void free_listItems(AlifObject** _items, bool _useQSBR) { // 55
	AlifListArray* array = ALIF_CONTAINER_OF(_items, AlifListArray, item);
	if (_useQSBR) {
		alifMem_freeDelayed(array);
	}
	else {
		alifMem_objFree(array);
	}
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
	return 0;
}

static AlifIntT list_preallocateExact(AlifListObject* _self,
	AlifSizeT _size) { //167
	AlifObject** items{};
	_size = (_size + 1) & ~(size_t)1;
	AlifListArray* array = list_allocateArray(_size);
	if (array == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	items = array->item;
	memset(items, 0, _size * sizeof(AlifObject*));
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
		AlifListArray* array = list_allocateArray(_size);
		if (array == nullptr) {
			ALIF_DECREF(op_);
			return nullptr;
			//return alifErr_noMemory();
		}
		memset(&array->item, 0, _size * sizeof(AlifObject*));
		op_->item = array->item;
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
	AlifListArray* array = list_allocateArray(_size);
	if (array == nullptr) {
		ALIF_DECREF(op_);
		//return alifErr_noMemory();
	}
	op_->item = array->item;
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

static inline AlifIntT valid_index(AlifSizeT i, AlifSizeT limit) { // 291
	/* The cast to AlifUSizeT lets us use just a single comparison
	   to check whether i is in the range: 0 <= i < limit.

	   See:  Section 14.2 "Bounds Checking" in the Agner Fog
	   optimization manual found at:
	   https://www.agner.org/optimize/optimizing_cpp.pdf
	*/
	return (AlifUSizeT)i < (AlifUSizeT)limit;
}


static AlifObject* list_itemImpl(AlifListObject* _self, AlifSizeT _idx) { // 307
	AlifObject* item = nullptr;
	ALIF_BEGIN_CRITICAL_SECTION(_self);
	if (!ALIFOBJECT_GC_IS_SHARED(_self)) {
		ALIFOBJECT_GC_SET_SHARED(_self);
	}
	AlifSizeT size = ALIF_SIZE(_self);
	if (!valid_index(_idx, size)) {
		goto exit;
	}
	item = _alif_newRefWithLock(_self->item[_idx]);
exit:
	ALIF_END_CRITICAL_SECTION();
	return item;
}

static inline AlifObject* listGet_itemRef(AlifListObject* _op, AlifSizeT _i) { // 329
	if (!alif_isOwnedByCurrentThread((AlifObject*)_op) and !ALIFOBJECT_GC_IS_SHARED(_op)) {
		return list_itemImpl(_op, _i);
	}
	AlifSizeT size = ALIFLIST_GET_SIZE(_op);
	if (!valid_index(_i, size)) {
		return nullptr;
	}
	AlifObject** obItem = (AlifObject**)alifAtomic_loadPtr(&_op->item);
	if (obItem == nullptr) {
		return nullptr;
	}
	AlifSizeT cap_ = list_capacity(obItem);
	if (!valid_index(_i, cap_)) {
		return nullptr;
	}
	AlifObject* item = alif_tryXGetRef(&obItem[_i]);
	if (item == nullptr) {
		return list_itemImpl(_op, _i);
	}
	return item;
}


AlifObject* alifList_getItem(AlifObject* _op, AlifSizeT _i) { // 365
	if (!ALIFLIST_CHECK(_op)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	if (!valid_index(_i, ALIF_SIZE(_op))) {
		//alifErr_setObject(_alifExcIndexError_, &ALIF_STR(ListErr));
		return nullptr;
	}
	return ((AlifListObject*)_op)->item[_i];
}


AlifObject* alifList_getItemRef(AlifObject* op, AlifSizeT i) { // 380
	if (!ALIFLIST_CHECK(op)) {
		//alifErr_setString(_alifExcTypeError_, "expected a list");
		return nullptr;
	}
	AlifObject* item = listGet_itemRef((AlifListObject*)op, i);
	if (item == nullptr) {
		//alifErr_setObject(_alifExcIndexError_, &ALIF_STR(ListErr));
		return nullptr;
	}
	return item;
}



static AlifIntT ins1(AlifListObject* _self,
	AlifSizeT _where, AlifObject* _v) { // 424
	AlifSizeT i{}, n = ALIF_SIZE(_self);
	AlifObject** items{};
	if (_v == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	if (list_resize(_self, n + 1) < 0)
		return -1;

	if (_where < 0) {
		_where += n;
		if (_where < 0)
			_where = 0;
	}
	if (_where > n)
		_where = n;
	items = _self->item;
	for (i = n; --i >= _where; )
		items[i + 1] = items[i];
	items[_where] = ALIF_NEWREF(_v);
	return 0;
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
	ALIF_TRASHCAN_BEGIN(op_, list_dealloc)
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
	ALIF_TRASHCAN_END
}


static AlifObject* list_reprImpl(AlifListObject* v) { // 524
	AlifObject* s{};
	AlifUStrWriter writer{};
	AlifSizeT i = alif_reprEnter((AlifObject*)v);
	if (i != 0) {
		return i > 0 ? alifUStr_fromString("[...]") : nullptr;
	}

	alifUStrWriter_init(&writer);
	writer.overAllocate = 1;
	/* "[" + "1" + ", 2" * (len - 1) + "]" */
	writer.minLength = 1 + 1 + (2 + 1) * (ALIF_SIZE(v) - 1) + 1;

	if (alifUStrWriter_writeChar(&writer, '[') < 0)
		goto error;

	/* Do repr() on each element.  Note that this may mutate the list,
	   so must refetch the list size on each iteration. */
	for (i = 0; i < ALIF_SIZE(v); ++i) {
		if (i > 0) {
			if (alifUStrWriter_writeASCIIString(&writer, ", ", 2) < 0)
				goto error;
		}

		s = alifObject_repr(v->item[i]);
		if (s == nullptr)
			goto error;

		if (alifUStrWriter_writeStr(&writer, s) < 0) {
			ALIF_DECREF(s);
			goto error;
		}
		ALIF_DECREF(s);
	}

	writer.overAllocate = 0;
	if (alifUStrWriter_writeChar(&writer, ']') < 0)
		goto error;

	alif_reprLeave((AlifObject*)v);
	return alifUStrWriter_finish(&writer);

error:
	alifUStrWriter_dealloc(&writer);
	alif_reprLeave((AlifObject*)v);
	return nullptr;
}

static AlifObject* list_repr(AlifObject* self) { // 574
	if (ALIFLIST_GET_SIZE(self) == 0) {
		return alifUStr_fromString("[]");
	}
	AlifListObject* v = (AlifListObject*)self;
	AlifObject* ret = nullptr;
	ALIF_BEGIN_CRITICAL_SECTION(v);
	ret = list_reprImpl(v);
	ALIF_END_CRITICAL_SECTION();
	return ret;
}


static AlifSizeT list_length(AlifObject* _a) { // 588
	return ALIFLIST_GET_SIZE(_a);
}

static AlifIntT list_contains(AlifObject* _aa, AlifObject* _el) { // 594

	for (AlifSizeT i = 0; ; i++) {
		AlifObject* item = listGet_itemRef((AlifListObject*)_aa, i);
		if (item == nullptr) {
			// out-of-bounds
			return 0;
		}
		AlifIntT cmp = alifObject_richCompareBool(item, _el, ALIF_EQ);
		ALIF_DECREF(item);
		if (cmp != 0) {
			return cmp;
		}
	}
	return 0;
}

static AlifObject* list_item(AlifObject* _aa, AlifSizeT _i) { // 613
	AlifListObject* a = (AlifListObject*)_aa;
	if (!valid_index(_i, ALIFLIST_GET_SIZE(a))) {
		//alifErr_setObject(_alifExcIndexError_, &ALIF_STR(ListErr));
		return nullptr;
	}
	AlifObject* item{};
	item = listGet_itemRef(a, _i);
	if (item == nullptr) {
		//alifErr_setObject(_alifExcIndexError_, &ALIF_STR(ListErr));
		return nullptr;
	}
	return item;
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

static AlifObject* listConcat_lockHeld(AlifListObject* _a, AlifListObject* _b) { // 685
	AlifSizeT size{};
	AlifSizeT i_{};
	AlifObject** src_{}, ** dest{};
	AlifListObject* np_{};
	size = ALIF_SIZE(_a) + ALIF_SIZE(_b);
	if (size == 0) {
		return alifList_new(0);
	}
	np_ = (AlifListObject*)list_newPrealloc(size);
	if (np_ == nullptr) {
		return nullptr;
	}
	src_ = _a->item;
	dest = np_->item;
	for (i_ = 0; i_ < ALIF_SIZE(_a); i_++) {
		AlifObject* v_ = src_[i_];
		dest[i_] = ALIF_NEWREF(v_);
	}
	src_ = _b->item;
	dest = np_->item + ALIF_SIZE(_a);
	for (i_ = 0; i_ < ALIF_SIZE(_b); i_++) {
		AlifObject* v_ = src_[i_];
		dest[i_] = ALIF_NEWREF(v_);
	}
	ALIF_SET_SIZE(np_, size);
	return (AlifObject*)np_;
}

static AlifObject* list_concat(AlifObject* aa, AlifObject* bb) { // 716
	if (!ALIFLIST_CHECK(bb)) {
		//alifErr_format(_alifExcTypeError_,
		//	"can only concatenate list (not \"%.200s\") to list",
		//	ALIF_TYPE(bb)->name);
		return nullptr;
	}
	AlifListObject* a = (AlifListObject*)aa;
	AlifListObject* b = (AlifListObject*)bb;
	AlifObject* ret{};
	ALIF_BEGIN_CRITICAL_SECTION2(a, b);
	ret = listConcat_lockHeld(a, b);
	ALIF_END_CRITICAL_SECTION2();
	return ret;
}

static AlifObject* listRepeat_lockHeld(AlifListObject* _a, AlifSizeT _n) { // 735
	const AlifSizeT inputSize = ALIF_SIZE(_a);
	if (inputSize == 0 or _n <= 0)
		return alifList_new(0);

	//if (inputSize > ALIF_SIZET_MAX / _n)
		//return alifErr_noMemory();
	AlifSizeT outputSize = inputSize * _n;

	AlifListObject* np_ = (AlifListObject*)list_newPrealloc(outputSize);
	if (np_ == nullptr)
		return nullptr;

	AlifObject** dest = np_->item;
	if (inputSize == 1) {
		AlifObject* elem = _a->item[0];
		ALIF_REFCNTADD(elem, _n);
		AlifObject** destEnd = dest + outputSize;
		while (dest < destEnd) {
			*dest++ = elem;
		}
	}
	else {
		AlifObject** src_ = _a->item;
		AlifObject** srcEnd = src_ + inputSize;
		while (src_ < srcEnd) {
			ALIF_REFCNTADD(*src_, _n);
			*dest++ = *src_++;
		}

		alif_memoryRepeat((char*)np_->item, sizeof(AlifObject*) * outputSize,
			sizeof(AlifObject*) * inputSize);
	}

	ALIF_SET_SIZE(np_, outputSize);
	return (AlifObject*)np_;
}

static AlifObject* list_repeat(AlifObject* _aa, AlifSizeT _n) { // 775
	AlifObject* ret{};
	AlifListObject* a = (AlifListObject*)_aa;
	ALIF_BEGIN_CRITICAL_SECTION(a);
	ret = listRepeat_lockHeld(a, _n);
	ALIF_END_CRITICAL_SECTION();
	return ret;
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
	bool useQsbr = _isResize and ALIFOBJECT_GC_IS_SHARED(_a);
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
			recycle = (AlifObject**)alifMem_dataAlloc(s_);
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
		alifMem_dataFree(recycle);
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

static AlifObject* list_inplaceRepeat(AlifObject* _self, AlifSizeT n) { // 999
	AlifObject* ret{};
	AlifListObject* self = (AlifListObject*)_self;
	ALIF_BEGIN_CRITICAL_SECTION(self);
	if (listInplaceRepeat_lockHeld(self, n) < 0) {
		ret = nullptr;
	}
	else {
		ret = ALIF_NEWREF(self);
	}
	ALIF_END_CRITICAL_SECTION();
	return ret;
}

static AlifIntT listAssItem_lockHeld(AlifListObject* a, AlifSizeT i, AlifObject* v) { // 1015
	if (!valid_index(i, ALIF_SIZE(a))) {
		//alifErr_setString(_alifExcIndexError_,
		//	"list assignment index out of range");
		return -1;
	}
	AlifObject* tmp = a->item[i];
	if (v == nullptr) {
		AlifSizeT size = ALIF_SIZE(a);
		for (AlifSizeT idx = i; idx < size - 1; idx++) {
			alifAtomic_storePtrRelaxed(&a->item[idx], a->item[idx + 1]);
		}
		ALIF_SET_SIZE(a, size - 1);
	}
	else {
		alifAtomic_storePtrRelease(&a->item[i], ALIF_NEWREF(v));
	}
	ALIF_DECREF(tmp);
	return 0;
}

static AlifIntT list_assItem(AlifObject* _aa, AlifSizeT _i, AlifObject* _v) { // 1038
	AlifIntT ret{};
	AlifListObject* a = (AlifListObject*)_aa;
	ALIF_BEGIN_CRITICAL_SECTION(a);
	ret = listAssItem_lockHeld(a, _i, _v);
	ALIF_END_CRITICAL_SECTION();
	return ret;
}

static AlifObject* list_insertImpl(AlifListObject* _self,
	AlifSizeT _index, AlifObject* _object) { // 1060
	if (ins1(_self, _index, _object) == 0) {
		return ALIF_NONE;
	}
	return nullptr;
}

static AlifObject* list_appendImpl(AlifListObject* _self, AlifObject* _object) { // 1109
	if (alifList_appendTakeRef(_self, ALIF_NEWREF(_object)) < 0) {
		return nullptr;
	}
	return ALIF_NONE;
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
			alifAtomic_storePtrRelease(&_self->item[len], item); //* alif
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
	AlifIntT res = -1;
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
	return ALIF_NONE; //* alif
}

AlifObject* _alifList_extend(AlifListObject* _self, AlifObject* _iterable) { // 1394
	return list_extend(_self, _iterable);
}

static AlifObject* list_inplaceConcat(AlifObject* _self, AlifObject* other) { // 1423
	AlifListObject* self = (AlifListObject*)_self;
	if (_list_extend(self, other) < 0) {
		return nullptr;
	}
	return ALIF_NEWREF(self);
}

static void reverse_slice(AlifObject** _lo, AlifObject** _hi) { // 1491
	--_hi;
	while (_lo < _hi) {
		AlifObject* t_ = *_lo;
		*_lo = *_hi;
		*_hi = t_;
		++_lo;
		--_hi;
	}
}

class SortSlice { // 1518
public:
	AlifObject** keys{};
	AlifObject** values{};
};


ALIF_LOCAL_INLINE(void) sortSlice_copy(SortSlice* _s1, AlifSizeT _i, SortSlice* _s2, AlifSizeT _j) { // 1524
	_s1->keys[_i] = _s2->keys[_j];
	if (_s1->values != nullptr)
		_s1->values[_i] = _s2->values[_j];
}

ALIF_LOCAL_INLINE(void) sortSlice_copyIncr(SortSlice* _dst, SortSlice* _src) { // 1532
	*_dst->keys++ = *_src->keys++;
	if (_dst->values != nullptr)
		*_dst->values++ = *_src->values++;
}

ALIF_LOCAL_INLINE(void) sortSlice_copyDecr(SortSlice* _dst, SortSlice* _src) { // 1540
	*_dst->keys-- = *_src->keys--;
	if (_dst->values != nullptr)
		*_dst->values-- = *_src->values--;
}

ALIF_LOCAL_INLINE(void) sortSlice_memcpy(SortSlice* _s1, AlifSizeT _i, SortSlice* _s2, AlifSizeT _j,
	AlifSizeT _n) { // 1549
	memcpy(&_s1->keys[_i], &_s2->keys[_j], sizeof(AlifObject*) * _n);
	if (_s1->values != nullptr)
		memcpy(&_s1->values[_i], &_s2->values[_j], sizeof(AlifObject*) * _n);
}

ALIF_LOCAL_INLINE(void) sortSlice_memMove(SortSlice* _s1, AlifSizeT _i, SortSlice* _s2, AlifSizeT _j,
	AlifSizeT _n) { //  1558
	memmove(&_s1->keys[_i], &_s2->keys[_j], sizeof(AlifObject*) * _n);
	if (_s1->values != nullptr)
		memmove(&_s1->values[_i], &_s2->values[_j], sizeof(AlifObject*) * _n);
}

ALIF_LOCAL_INLINE(void) sortSlice_advance(SortSlice* _slice, AlifSizeT _n) { // 1566
	_slice->keys += _n;
	if (_slice->values != nullptr)
		_slice->values += _n;
}


#define ISLT(_x, _y) (*(_ms->KeyCompare))(_x, _y, _ms) // 1579


#define IFLT(_x, _y) if ((k_ = ISLT(_x, _y)) < 0) goto fail;  \
           if (k_) // 1585

#define MAX_MERGE_PENDING (SIZEOF_SIZE_T * 8) // 1593
#define MIN_GALLOP 7 // 1598
#define MERGESTATE_TEMP_SIZE 256 // 1601

#define MAX_MINRUN 64 // 1607

class SSlice { // 1615
public:
	SortSlice base{};
	AlifSizeT len_{};   /* length of run */
	AlifIntT power{}; /* node "level" for powersort merge strategy */
};

typedef class SMergeState MergeState; // 1621
class SMergeState { // 1622
public:
	AlifSizeT minGallop{};

	AlifSizeT listLen{};
	AlifObject** baseKeys{};
	SortSlice a_{};
	AlifSizeT alloced{};

	AlifIntT n_{};
	SSlice pending[MAX_MERGE_PENDING]{};

	AlifObject* temparray[MERGESTATE_TEMP_SIZE]{};

	AlifIntT(*KeyCompare)(AlifObject*, AlifObject*, MergeState*);

	AlifObject* (*KeyRichcompare)(AlifObject*, AlifObject*, AlifIntT);

	AlifIntT(*TupleElemCompare)(AlifObject*, AlifObject*, MergeState*);
};

static AlifIntT binarySort(MergeState* _ms, const SortSlice* _ss, AlifSizeT _n, AlifSizeT _ok) { // 1681
	AlifSizeT k_{};
	AlifObject** const a_ = _ss->keys;
	AlifObject** const v_ = _ss->values;
	const bool hasValues = v_ != nullptr;
	AlifObject* pivot{};
	AlifSizeT m_{};

	if (!_ok)
		++_ok;

#if 0 
	AlifObject* vPivot = nullptr;
	for (; ok < n; ++ok) {
		pivot = a[ok];
		if (hasValues)
			vPivot = v[ok];
		for (M = ok - 1; M >= 0; --M) {
			k = ISLT(pivot, a[M]);
			if (k < 0) {
				a[M + 1] = pivot;
				if (hasValues)
					v[M + 1] = vPivot;
				goto fail;
			}
			else if (k) {
				a[M + 1] = a[M];
				if (hasValues)
					v[M + 1] = v[M];
			}
			else
				break;
		}
		a[M + 1] = pivot;
		if (hasValues)
			v[M + 1] = vPivot;
	}
#else // binary insertion sort
	AlifSizeT l_{}, r_{};
	for (; _ok < _n; ++_ok) {
		l_ = 0;
		r_ = _ok;
		pivot = a_[_ok];
		do {

			m_ = (l_ + r_) >> 1;
#if 1 
			IFLT(pivot, a_[m_])
				r_ = m_;
	else
		l_ = m_ + 1;
#else
			k = ISLT(pivot, a[M]);
			if (k < 0)
				goto fail;
			AlifSizeT Mp1 = M + 1;
			R = k ? M : R;
			L = k ? L : Mp1;
#endif
		} while (l_ < r_);
		for (m_ = _ok; m_ > l_; --m_)
			a_[m_] = a_[m_ - 1];
		a_[l_] = pivot;
		if (hasValues) {
			pivot = v_[_ok];
			for (m_ = _ok; m_ > l_; --m_)
				v_[m_] = v_[m_ - 1];
			v_[l_] = pivot;
		}
	}
#endif /
	return 0;

fail:
	return -1;
}

static void sortSlice_reverse(SortSlice* _s, AlifSizeT _n) { // 1803
	reverse_slice(_s->keys, &_s->keys[_n]);
	if (_s->values != nullptr)
		reverse_slice(_s->values, &_s->values[_n]);
}

static AlifSizeT count_run(MergeState* _ms, SortSlice* _slo, AlifSizeT _nremaining) { // 1819
	AlifSizeT k_{};
	AlifSizeT n_{};
	AlifObject** const lo_ = _slo->keys;
	AlifSizeT neq_{};

#define REVERSE_LAST_NEQ                        \
    if (neq_) {                                  \
        SortSlice slice = *_slo;                 \
        ++neq_;                                  \
        sortSlice_advance(&slice, n_ - neq_);     \
        sortSlice_reverse(&slice, neq_);         \
        neq_ = 0;                                \
    }


#define IF_NEXT_LARGER  IFLT(lo_[n_-1], lo_[n_])
#define IF_NEXT_SMALLER IFLT(lo_[n_], lo_[n_-1])

	for (n_ = 1; n_ < _nremaining; ++n_) {
		IF_NEXT_SMALLER
			break;
	}
	if (n_ == _nremaining)
		return n_;

	if (n_ > 1) {
		IFLT(lo_[0], lo_[n_ - 1])
			return n_;
		sortSlice_reverse(_slo, n_);
	}
	++n_;

	neq_ = 0;
	for (; n_ < _nremaining; ++n_) {
		IF_NEXT_SMALLER{

			REVERSE_LAST_NEQ
		}
	else {
		IF_NEXT_LARGER /* descending run is over */
			break;
	else /* not x < y and not y < x implies x == y */
		++neq_;
	}
	}
	REVERSE_LAST_NEQ
		sortSlice_reverse(_slo, n_);
	for (; n_ < _nremaining; ++n_) {
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

static AlifSizeT gallop_left(MergeState* _ms, AlifObject* _key, AlifObject** _a, AlifSizeT _n, AlifSizeT _hint) { // 1935
	AlifSizeT ofs_{};
	AlifSizeT lastOfS{};
	AlifSizeT k_{};

	_a += _hint;
	lastOfS = 0;
	ofs_ = 1;
	IFLT(*_a, _key) {
		const AlifSizeT maxofs = _n - _hint;             
		while (ofs_ < maxofs) {
			IFLT(_a[ofs_], _key) {
				lastOfS = ofs_;
				ofs_ = (ofs_ << 1) + 1;
			}
		   else              
			   break;
		}
		if (ofs_ > maxofs)
			ofs_ = maxofs;
		lastOfS += _hint;
		ofs_ += _hint;
	}
	else {
		const AlifSizeT maxofs = _hint + 1;           
		while (ofs_ < maxofs) {
			IFLT(*(_a - ofs_), _key)
				break;
			lastOfS = ofs_;
			ofs_ = (ofs_ << 1) + 1;
		}
		if (ofs_ > maxofs)
			ofs_ = maxofs;
		k_ = lastOfS;
		lastOfS = _hint - ofs_;
		ofs_ = _hint - k_;
	}
	_a -= _hint;

	++lastOfS;
	while (lastOfS < ofs_) {
		AlifSizeT m = lastOfS + ((ofs_ - lastOfS) >> 1);

		IFLT(_a[m], _key)
			lastOfS = m + 1;              
		else
			ofs_ = m;                    
	}
	return ofs_;

fail:
	return -1;
}

static AlifSizeT gallop_right(MergeState * _ms, AlifObject* _key,
	AlifObject* *_a, AlifSizeT _n, AlifSizeT _hint) { // 2024
	AlifSizeT ofs_{};
	AlifSizeT lastOfS{};
	AlifSizeT k_{};


	_a += _hint;
	lastOfS = 0;
	ofs_ = 1;
	IFLT(_key, *_a) {
		const AlifSizeT maxofs = _hint + 1;             
		while (ofs_ < maxofs) {
			IFLT(_key, *(_a - ofs_)) {
				lastOfS = ofs_;
				ofs_ = (ofs_ << 1) + 1;
			}
			else               
				break;
		}
		if (ofs_ > maxofs)
			ofs_ = maxofs;
		k_ = lastOfS;
		lastOfS = _hint - ofs_;
		ofs_ = _hint - k_;
	}
	else {
		const AlifSizeT maxOfS = _n - _hint;             /* &a[n-1] is highest */
		while (ofs_ < maxOfS) {
			IFLT(_key, _a[ofs_])
				break;
			lastOfS = ofs_;
			ofs_ = (ofs_ << 1) + 1;
		}
		if (ofs_ > maxOfS)
			ofs_ = maxOfS;
		lastOfS += _hint;
		ofs_ += _hint;
	}
	_a -= _hint;

	++lastOfS;
	while (lastOfS < ofs_) {
		AlifSizeT m_ = lastOfS + ((ofs_ - lastOfS) >> 1);

		IFLT(_key, _a[m_])
			ofs_ = m_;                    /* key < a[m] */
		else
			lastOfS = m_ + 1;             
	}
	return ofs_;

fail:
	return -1;
}

static void merge_init(MergeState* _ms, AlifSizeT _listSize, AlifIntT _hasKeyFunc,
	SortSlice* _lo) { // 2099
	if (_hasKeyFunc) {

		_ms->alloced = (_listSize + 1) / 2;

		if (MERGESTATE_TEMP_SIZE / 2 < _ms->alloced)
			_ms->alloced = MERGESTATE_TEMP_SIZE / 2;
		_ms->a_.values = &_ms->temparray[_ms->alloced];
	}
	else {
		_ms->alloced = MERGESTATE_TEMP_SIZE;
		_ms->a_.values = nullptr;
	}
	_ms->a_.keys = _ms->temparray;
	_ms->n_ = 0;
	_ms->minGallop = MIN_GALLOP;
	_ms->listLen = _listSize;
	_ms->baseKeys = _lo->keys;
}

static void merge_freeMem(MergeState* _ms) { // 2135
	if (_ms->a_.keys != _ms->temparray) {
		alifMem_dataAlloc((AlifUSizeT)_ms->a_.keys);
		_ms->a_.keys = nullptr;
	}
}

static AlifIntT merge_getMem(MergeState* _ms, AlifSizeT _need) { // 2147
	AlifIntT multiplier{};

	if (_need <= _ms->alloced)
		return 0;

	multiplier = _ms->a_.values != nullptr ? 2 : 1;

	merge_freeMem(_ms);
	if ((AlifUSizeT)_need > ALIF_SIZET_MAX / sizeof(AlifObject*) / multiplier) {
		//alifErr_noMemory();
		return -1;
	}
	_ms->a_.keys = (AlifObject**)alifMem_dataAlloc(multiplier * _need
		* sizeof(AlifObject*));
	if (_ms->a_.keys != nullptr) {
		_ms->alloced = _need;
		if (_ms->a_.values != nullptr)
			_ms->a_.values = &_ms->a_.keys[_need];
		return 0;
	}
	//alifErr_noMemory();
	return -1;
}
#define MERGE_GETMEM(_ms, _need) ((_need) <= (_ms)->alloced ? 0 :   \
                                merge_getMem(_ms, _need))

static AlifSizeT merge_lo(MergeState* _ms, SortSlice _ssa, AlifSizeT _na,
	SortSlice _ssb, AlifSizeT _nb) { // 2186
	AlifSizeT k_{};
	SortSlice dest_{};
	AlifIntT result = -1;            
	AlifSizeT minGallop{};

	if (MERGE_GETMEM(_ms, _na) < 0)
		return -1;
	sortSlice_memcpy(&_ms->a_, 0, &_ssa, 0, _na);
	dest_ = _ssa;
	_ssa = _ms->a_;

	sortSlice_copyIncr(&dest_, &_ssb);
	--_nb;
	if (_nb == 0)
		goto Succeed;
	if (_na == 1)
		goto CopyB;

	minGallop = _ms->minGallop;
	for (;;) {
		AlifSizeT aCount = 0;          /* # of times A won in a row */
		AlifSizeT bCount = 0;          /* # of times B won in a row */
		for (;;) {
			k_ = ISLT(_ssb.keys[0], _ssa.keys[0]);
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_copyIncr(&dest_, &_ssb);
				++bCount;
				aCount = 0;
				--_nb;
				if (_nb == 0)
					goto Succeed;
				if (bCount >= minGallop)
					break;
			}
			else {
				sortSlice_copyIncr(&dest_, &_ssa);
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
			k_ = gallop_right(_ms, _ssb.keys[0], _ssa.keys, _na, 0);
			aCount = k_;
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_memcpy(&dest_, 0, &_ssa, 0, k_);
				sortSlice_advance(&dest_, k_);
				sortSlice_advance(&_ssa, k_);
				_na -= k_;
				if (_na == 1)
					goto CopyB;

				if (_na == 0)
					goto Succeed;
			}
			sortSlice_copyIncr(&dest_, &_ssb);
			--_nb;
			if (_nb == 0)
				goto Succeed;

			k_ = gallop_left(_ms, _ssa.keys[0], _ssb.keys, _nb, 0);
			bCount = k_;
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_memMove(&dest_, 0, &_ssb, 0, k_);
				sortSlice_advance(&dest_, k_);
				sortSlice_advance(&_ssb, k_);
				_nb -= k_;
				if (_nb == 0)
					goto Succeed;
			}
			sortSlice_copyIncr(&dest_, &_ssa);
			--_na;
			if (_na == 1)
				goto CopyB;
		} while (aCount >= MIN_GALLOP or bCount >= MIN_GALLOP);
		++minGallop;          
		_ms->minGallop = minGallop;
	}
Succeed:
	result = 0;
Fail:
	if (_na)
		sortSlice_memcpy(&dest_, 0, &_ssa, 0, _na);
	return result;
CopyB:
	sortSlice_memMove(&dest_, 0, &_ssb, 0, _nb);
	sortSlice_copy(&dest_, _nb, &_ssa, 0);
	return 0;
}

static AlifSizeT merge_hi(MergeState* _ms, SortSlice _ssa, AlifSizeT _na,
	SortSlice _ssb, AlifSizeT _nb) { // 2319
	AlifSizeT k_{};
	SortSlice dest{}, basea{}, baseb{};
	AlifIntT result = -1;            /* guilty until proved innocent */
	AlifSizeT minGallop{};

	
	if (MERGE_GETMEM(_ms, _nb) < 0)
		return -1;
	dest = _ssb;
	sortSlice_advance(&dest, _nb - 1);
	sortSlice_memcpy(&_ms->a_, 0, &_ssb, 0, _nb);
	basea = _ssa;
	baseb = _ms->a_;
	_ssb.keys = _ms->a_.keys + _nb - 1;
	if (_ssb.values != nullptr)
		_ssb.values = _ms->a_.values + _nb - 1;
	sortSlice_advance(&_ssa, _na - 1);

	sortSlice_copyDecr(&dest, &_ssa);
	--_na;
	if (_na == 0)
		goto Succeed;
	if (_nb == 1)
		goto CopyA;

	minGallop = _ms->minGallop;
	for (;;) {
		AlifSizeT aCount = 0;          /* # of times A won in a row */
		AlifSizeT bCount = 0;          /* # of times B won in a row */

		for (;;) {
			k_ = ISLT(_ssb.keys[0], _ssa.keys[0]);
			if (k_) {
				if (k_ < 0)
					goto Fail;
				sortSlice_copyDecr(&dest, &_ssa);
				++aCount;
				bCount = 0;
				--_na;
				if (_na == 0)
					goto Succeed;
				if (aCount >= minGallop)
					break;
			}
			else {
				sortSlice_copyDecr(&dest, &_ssb);
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
			k_ = gallop_right(_ms, _ssb.keys[0], basea.keys, _na, _na - 1);
			if (k_ < 0)
				goto Fail;
			k_ = _na - k_;
			aCount = k_;
			if (k_) {
				sortSlice_advance(&dest, -k_);
				sortSlice_advance(&_ssa, -k_);
				sortSlice_memMove(&dest, 1, &_ssa, 1, k_);
				_na -= k_;
				if (_na == 0)
					goto Succeed;
			}
			sortSlice_copyDecr(&dest, &_ssb);
			--_nb;
			if (_nb == 1)
				goto CopyA;

			k_ = gallop_left(_ms, _ssa.keys[0], baseb.keys, _nb, _nb - 1);
			if (k_ < 0)
				goto Fail;
			k_ = _nb - k_;
			bCount = k_;
			if (k_) {
				sortSlice_advance(&dest, -k_);
				sortSlice_advance(&_ssb, -k_);
				sortSlice_memcpy(&dest, 1, &_ssb, 1, k_);
				_nb -= k_;
				if (_nb == 1)
					goto CopyA;
				if (_nb == 0)
					goto Succeed;
			}
			sortSlice_copyDecr(&dest, &_ssa);
			--_na;
			if (_na == 0)
				goto Succeed;
		} while (aCount >= MIN_GALLOP or bCount >= MIN_GALLOP);
		++minGallop;           /* penalize it for leaving galloping mode */
		_ms->minGallop = minGallop;
	}
Succeed:
	result = 0;
Fail:
	if (_nb)
		sortSlice_memcpy(&dest, -(_nb - 1), &baseb, 0, _nb);
	return result;
CopyA:
	sortSlice_memMove(&dest, 1 - _na, &_ssa, 1 - _na, _na);
	sortSlice_advance(&dest, -_na);
	sortSlice_advance(&_ssa, -_na);
	sortSlice_copy(&dest, 0, &_ssb, 0);
	return 0;
}


static AlifSizeT merge_at(MergeState* _ms, AlifSizeT _i) { // 2458
	SortSlice ssa_{}, ssb_{};
	AlifSizeT na_{}, nb_{};
	AlifSizeT k_{};

	ssa_ = _ms->pending[_i].base;
	na_ = _ms->pending[_i].len_;
	ssb_ = _ms->pending[_i + 1].base;
	nb_ = _ms->pending[_i + 1].len_;

	_ms->pending[_i].len_ = na_ + nb_;
	if (_i == _ms->n_ - 3)
		_ms->pending[_i + 1] = _ms->pending[_i + 2];
	--_ms->n_;

	k_ = gallop_right(_ms, *ssb_.keys, ssa_.keys, na_, 0);
	if (k_ < 0)
		return -1;
	sortSlice_advance(&ssa_, k_);
	na_ -= k_;
	if (na_ == 0)
		return 0;


	nb_ = gallop_left(_ms, ssa_.keys[na_ - 1], ssb_.keys, nb_, nb_ - 1);
	if (nb_ <= 0)
		return nb_;

	if (na_ <= nb_)
		return merge_lo(_ms, ssa_, na_, ssb_, nb_);
	else
		return merge_hi(_ms, ssa_, na_, ssb_, nb_);
}

static AlifIntT powerLoop(AlifSizeT _s1, AlifSizeT _n1, AlifSizeT _n2, AlifSizeT _n) { // 2518
	AlifIntT result = 0;
	AlifSizeT a_ = 2 * _s1 + _n1;  /* 2*a */
	AlifSizeT b_ = a_ + _n1 + _n2;  /* 2*b */
	for (;;) {
		++result;
		if (a_ >= _n) {  /* both quotient bits are 1 */
			a_ -= _n;
			b_ -= _n;
		}
		else if (b_ >= _n) {  /* a/n bit is 0, b/n bit is 1 */
			break;
		} /* else both quotient bits are 0 */
		a_ <<= 1;
		b_ <<= 1;
	}
	return result;
}

static AlifIntT found_newRun(MergeState* _ms, AlifSizeT _n2) { // 2565
	if (_ms->n_) {
		SSlice* p_ = _ms->pending;
		AlifSizeT s1_ = p_[_ms->n_ - 1].base.keys - _ms->baseKeys; 
		AlifSizeT n1_ = p_[_ms->n_ - 1].len_;
		AlifIntT power = powerLoop(s1_, n1_, _n2, _ms->listLen);
		while (_ms->n_ > 1 and p_[_ms->n_ - 2].power > power) {
			if (merge_at(_ms, _ms->n_ - 2) < 0)
				return -1;
		}
		p_[_ms->n_ - 1].power = power;
	}
	return 0;
}


static AlifIntT merge_forceCollapse(MergeState* _ms) { // 2590
	SSlice* p_ = _ms->pending;
	while (_ms->n_ > 1) {
		AlifSizeT n_ = _ms->n_ - 2;
		if (n_ > 0 and p_[n_ - 1].len_ < p_[n_ + 1].len_)
			--n_;
		if (merge_at(_ms, n_) < 0)
			return -1;
	}
	return 0;
}

static AlifSizeT merge_computeMinRun(AlifSizeT _n) { // 2616
	AlifSizeT r_ = 0;         

	while (_n >= MAX_MINRUN) {
		r_ |= _n & 1;
		_n >>= 1;
	}
	return _n + r_;
}


static AlifIntT safe_objectCompare(AlifObject* _v, AlifObject* _w, MergeState* _ms) { // 2639
	return alifObject_richCompareBool(_v, _w, ALIF_LT);
}
static AlifIntT unsafe_objectCompare(AlifObject* _v, AlifObject* _w, MergeState* _ms) { // 2650
	AlifObject* resObj{}; AlifIntT res_{};

	if (ALIF_TYPE(_v)->richCompare != _ms->KeyRichcompare)
		return alifObject_richCompareBool(_v, _w, ALIF_LT);

	resObj = (*(_ms->KeyRichcompare))(_v, _w, ALIF_LT);

	if (resObj == ALIF_NOTIMPLEMENTED) {
		ALIF_DECREF(resObj);
		return alifObject_richCompareBool(_v, _w, ALIF_LT);
	}
	if (resObj == nullptr)
		return -1;

	if (ALIFBOOL_CHECK(resObj)) {
		res_ = (resObj == ALIF_TRUE);
	}
	else {
		res_ = alifObject_isTrue(resObj);
	}
	ALIF_DECREF(resObj);


	return res_;
}


static AlifIntT unsafe_latinCompare(AlifObject* _v, AlifObject* _w, MergeState* _ms) { // 2686
	AlifSizeT len_{};
	AlifIntT res_{};

	len_ = ALIF_MIN(ALIFUSTR_GET_LENGTH(_v), ALIFUSTR_GET_LENGTH(_w));
	res_ = memcmp(ALIFUSTR_DATA(_v), ALIFUSTR_DATA(_w), len_);

	res_ = (res_ != 0 ?
		res_ < 0 :
		ALIFUSTR_GET_LENGTH(_v) < ALIFUSTR_GET_LENGTH(_w));

	return res_;
}

static AlifIntT unsafe_longCompare(AlifObject* _v, AlifObject* _w, MergeState* _ms) { // 2710
	AlifLongObject* vl_{}, * wl_{};
	intptr_t v0_{}, w0_{};
	AlifIntT res_{};

	vl_ = (AlifLongObject*)_v;
	wl_ = (AlifLongObject*)_w;

	v0_ = alifLong_compactValue(vl_);
	w0_ = alifLong_compactValue(wl_);

	res_ = v0_ < w0_;
	return res_;
}

static AlifIntT unsafe_floatCompare(AlifObject* _v, AlifObject* _w, MergeState* _ms) { // 2735
	AlifIntT res_{};
	res_ = ALIFFLOAT_AS_DOUBLE(_v) < ALIFFLOAT_AS_DOUBLE(_w);
	return res_;
}


static AlifIntT unsafe_tupleCompare(AlifObject* _v, AlifObject* _w, MergeState* _ms) { // 2755
	AlifTupleObject* vt_{}, * wt_{};
	AlifSizeT i_{}, vLen{}, wLen{};
	AlifIntT k_{};

	vt_ = (AlifTupleObject*)_v;
	wt_ = (AlifTupleObject*)_w;

	vLen = ALIF_SIZE(vt_);
	wLen = ALIF_SIZE(wt_);

	for (i_ = 0; i_ < vLen and i_ < wLen; i_++) {
		k_ = alifObject_richCompareBool(vt_->item[i_], wt_->item[i_], ALIF_EQ);
		if (k_ < 0)
			return -1;
		if (!k_)
			break;
	}

	if (i_ >= vLen or i_ >= wLen)
		return vLen < wLen;

	if (i_ == 0)
		return _ms->TupleElemCompare(vt_->item[i_], wt_->item[i_], _ms);
	else
		return alifObject_richCompareBool(vt_->item[i_], wt_->item[i_], ALIF_LT);
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

			if (keysAreInTuples and
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
				ms.KeyCompare = unsafe_latinCompare;
			}
			else if (keyType == &_alifLongType_ and intsAreBounded) {
				ms.KeyCompare = unsafe_longCompare;
			}
			else if (keyType == &_alifFloatType_) {
				ms.KeyCompare = unsafe_floatCompare;
			}
			else if ((ms.KeyRichcompare = keyType->richCompare) != nullptr) {
				ms.KeyCompare = unsafe_objectCompare;
			}
			else {
				ms.KeyCompare = safe_objectCompare;
			}
		}
		else {
			ms.KeyCompare = safe_objectCompare;
		}

		if (keysAreInTuples) {
			/* Make sure we're not dealing with tuples of tuples
			 * (remember: here, key_type refers list [key[0] for key in keys]) */
			if (keyType == &_alifTupleType_) {
				ms.TupleElemCompare = safe_objectCompare;
			}
			else {
				ms.TupleElemCompare = ms.KeyCompare;
			}

			ms.KeyCompare = unsafe_tupleCompare;
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
			if (binarySort(&ms, &lo, force, n) < 0)
				goto fail;
			n = force;
		}
		/* Maybe merge pending runs. */
		if (found_newRun(&ms, n) < 0)
			goto fail;
		/* Push new run on stack. */
		ms.pending[ms.n_].base = lo;
		ms.pending[ms.n_].len_ = n;
		++ms.n_;
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
		bool use_qsbr = ALIFOBJECT_GC_IS_SHARED(_self);
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


AlifObject* _alifList_fromStackRefSteal(const AlifStackRef* _src, AlifSizeT _n) { // 3144
	if (_n == 0) {
		return alifList_new(0);
	}

	AlifListObject* list = (AlifListObject*)alifList_new(_n);
	if (list == nullptr) {
		for (AlifSizeT i = 0; i < _n; i++) {
			alifStackRef_close(_src[i]);
		}
		return nullptr;
	}

	AlifObject** dst = list->item;
	for (AlifSizeT i = 0; i < _n; i++) {
		dst[i] = alifStackRef_asAlifObjectSteal(_src[i]);
	}

	return (AlifObject*)list;
}


static AlifObject* list_removeImpl(AlifListObject* _self, AlifObject* _value) { // 3259
	AlifSizeT i{};

	for (i = 0; i < ALIF_SIZE(_self); i++) {
		AlifObject* obj = _self->item[i];
		ALIF_INCREF(obj);
		AlifIntT cmp = alifObject_richCompareBool(obj, _value, ALIF_EQ);
		ALIF_DECREF(obj);
		if (cmp > 0) {
			if (listAssSlice_lockHeld(_self, i, i + 1, nullptr) == 0)
				return ALIF_NONE;
			return nullptr;
		}
		else if (cmp < 0)
			return nullptr;
	}
	//alifErr_setString(_alifExcValueError_, "list.remove(x): x not in list");
	return nullptr;
}



static AlifMethodDef _listMethods_[] = { // 3445
	LIST_APPEND_METHODDEF
	LIST_INSERT_METHODDEF
	//LIST_EXTEND_METHODDEF
	//LIST_POP_METHODDEF
	LIST_REMOVE_METHODDEF
	//LIST_INDEX_METHODDEF
	//LIST_COUNT_METHODDEF
	//LIST_REVERSE_METHODDEF
	//LIST_SORT_METHODDEF
	{nullptr, nullptr}
};



static AlifSequenceMethods _listAsSequence_ = { // 3465
	.length = list_length,
	.concat = list_concat,
	.repeat = list_repeat,
	.item = list_item,
	.assItem = list_assItem,
	.contains = list_contains,
	.inplaceConcat = list_inplaceConcat,
	.inplaceRepeat = list_inplaceRepeat,
};



AlifTypeObject _alifListType_ = { // 3737
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مصفوفة",
	.basicSize = sizeof(AlifListObject),
	.dealloc = list_dealloc,
	.repr = list_repr,
	.asSequence = &_listAsSequence_,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_LIST_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF | ALIF_TPFLAGS_SEQUENCE,
	.methods = _listMethods_,
	.alloc = alifType_genericAlloc,
	.free = alifObject_gcDel,
	//.vectorCall = list_vectorCall,
};
