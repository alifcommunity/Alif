#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_List.h"
#include "AlifCore_Object.h"


#ifdef ALIF_GIL_DISABLED
class AlifListArray{ // 28
public:
	AlifSizeT allocated{};
	AlifObject* item[];
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

	AlifListObject* op = ALIF_FREELIST_POP(AlifListObject, lists);
	if (op == nullptr) {
		op = ALIFOBJECT_GC_NEW(AlifListObject, &_alifListType_);
		if (op == nullptr) {
			return nullptr;
		}
	}
	if (_size <= 0) {
		op->item = nullptr;
	}
	else {
#ifdef ALIF_GIL_DISABLED
		AlifListArray* array = list_allocateArray(_size);
		if (array == nullptr) {
			ALIF_DECREF(op);
			return nullptr;
			//return alifErr_noMemory();
		}
		memset(&array->item, 0, _size * sizeof(AlifObject*));
		op->item = array->item;
#else
		op->item = (AlifObject**)alifMem_objAlloc(_size* sizeof(AlifObject*));
#endif
		if (op->item == nullptr) {
			ALIF_DECREF(op);
			return nullptr;
			//return alifErr_noMemory();
		}
	}
	ALIF_SET_SIZE(op, _size);
	op->allocated = _size;
	ALIFOBJECT_GC_TRACK(op);
	return (AlifObject*)op;
}



AlifIntT alifList_appendTakeRefListResize(AlifListObject* _self,
	AlifObject* _newItem) { // 468
	AlifSizeT len = ALIF_SIZE(_self);
	if (list_resize(_self, len + 1) < 0) {
		ALIF_DECREF(_newItem);
		return -1;
	}
	alifAtomic_storePtrRelease(&_self->item[len], _newItem);
	return 0;
}

AlifIntT alifList_append(AlifObject* _op, AlifObject* _newItem) { // 481
	if (ALIFLIST_CHECK(_op) and (_newItem != nullptr)) {
		AlifIntT ret{};
		ALIF_BEGIN_CRITICAL_SECTION(_op);
		ret = alifList_appendTakeRef((AlifListObject*)_op, ALIF_NEWREF(_newItem));
		ALIF_END_CRITICAL_SECTION();
		return ret;
	}
	//ALIFERR_BADINTERNALCALL();
	return -1;
}














AlifObject* alifList_asTuple(AlifObject* _v) { // 3129
	if (_v == nullptr or !ALIFLIST_CHECK(_v)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	AlifObject* ret{};
	AlifListObject* self = (AlifListObject*)_v;
	ALIF_BEGIN_CRITICAL_SECTION(self);
	ret = alifTuple_fromArray(self->item, ALIF_SIZE(_v));
	ALIF_END_CRITICAL_SECTION();
	return ret;
}







AlifTypeObject _alifListType_ = { // 3737
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مصفوفة",
	.basicSize = sizeof(AlifListObject),
};
