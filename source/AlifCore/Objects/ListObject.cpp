#include "alif.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_Object.h"
#include "AlifMemory.h"



class AlifListArray{ // 28
public:
	AlifSizeT allocated;
	AlifObject* objectItem[];
} ;

static AlifListArray* list_allocateArray(size_t _capacity) // 34
{
	if (_capacity > LLONG_MAX / sizeof(AlifObject*) - 1) {
		return NULL;
	}
	AlifListArray* array = (AlifListArray*)alifMem_objAlloc(sizeof(AlifListArray) + _capacity * sizeof(AlifObject*));
	if (array == NULL) {
		return NULL;
	}
	array->allocated = _capacity;
	return array;
}

AlifObject* alifList_New(AlifSizeT _size) // 212
{
	if (_size < 0) {
		//alifErr_badInternalCall();
		return NULL;
	}

	AlifListObject* op = ALIF_FREELIST_POP(AlifListObject, lists);
	if (op == NULL) {
		op = ALIFOBJECT_GC_New(AlifListObject, &_alfiListType_);
		if (op == NULL) {
			return NULL;
		}
	}
	if (_size <= 0) {
		op->item = NULL;
	}
	else {
		AlifListArray* array = list_allocateArray(_size);
		if (array == NULL) {
			ALIF_DECREF(op);
			return nullptr;
			//return alifErr_noMemory();
		}
		memset(&array->objectItem, 0, _size * sizeof(AlifObject*));
		op->item = array->objectItem;
		if (op->item == NULL) {
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
