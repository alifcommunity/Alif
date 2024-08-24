#include "alif.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_Object.h"
#include "AlifMemory.h"


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
#ifdef Py_GIL_DISABLED
		_PyListArray* array = list_allocate_array(_size);
		if (array == NULL) {
			Py_DECREF(op);
			return PyErr_NoMemory();
		}
		memset(&array->item, 0, _size * sizeof(PyObject*));
		op->item = array->item;
#else
		op->item = (AlifObject**)alifMem_objAlloc(_size * sizeof(AlifObject*));
#endif
		if (op->item == NULL) {
			ALIF_DECREF(op);
			//return alifErr_noMemory();
		}
	}
	ALIF_SETSIZE(op, _size);
	op->allocated = _size;
	ALIFOBJECT_GC_TRACK(op);
	return (AlifObject*)op;
}


AlifTypeObject _alfiListType_ = {
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "list",
	.basicSize = sizeof(AlifListObject),

};
