#include "alif.h"

#include "AlifCore_FreeList.h"
#include "AlifCore_Object.h"


#ifdef ALIF_GIL_DISABLED
class AlifListArray{ // 28
public:
	AlifSizeT allocated;
	AlifObject* item[];
};
static AlifListArray* list_allocateArray(size_t _capacity) { // 34
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

AlifObject* alifList_New(AlifSizeT _size) { // 212 
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


AlifTypeObject _alfiListType_ = { // 3737
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مصفوفة",
	.basicSize = sizeof(AlifListObject),
};
