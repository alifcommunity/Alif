#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_BytesObject.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"
#include "BytesObject.h"






char _alifByteArrayEmptyString_[] = "";





static AlifIntT can_resize(AlifByteArrayObject* self) { // 67
	if (self->exports > 0) {
		//alifErr_setString(_alifExcBufferError_, "Existing exports of data: object cannot be re-sized");
		return 0;
	}
	return 1;
}





AlifObject* alifByteArray_fromStringAndSize(const char* _bytes, AlifSizeT _size) { // 108
	AlifByteArrayObject* new_{};
	AlifSizeT alloc{};

	if (_size < 0) {
		//alifErr_setString(_alifExcSystemError_,
		//	"Negative size passed to alifByteArray_fromStringAndSize");
		return nullptr;
	}

	/* Prevent buffer overflow when setting alloc to size+1. */
	if (_size == ALIF_SIZET_MAX) {
		//return alifErr_noMemory();
		return nullptr; //* alif
	}

	new_ = ALIFOBJECT_NEW(AlifByteArrayObject, &_alifByteArrayType_);
	if (new_ == nullptr)
		return nullptr;

	if (_size == 0) {
		new_->bytes = nullptr;
		alloc = 0;
	}
	else {
		alloc = _size + 1;
		new_->bytes = (char*)alifMem_dataAlloc(alloc);
		if (new_->bytes == nullptr) {
			ALIF_DECREF(new_);
			//return alifErr_noMemory();
			return nullptr; //* alif
		}
		if (_bytes != nullptr and _size > 0)
			memcpy(new_->bytes, _bytes, _size);
		new_->bytes[_size] = '\0';  /* Trailing null byte */
	}
	ALIF_SET_SIZE(new_, _size);
	new_->alloc = alloc;
	new_->start = new_->bytes;
	new_->exports = 0;

	return (AlifObject*)new_;
}



AlifIntT alifByteArray_resize(AlifObject* _self, AlifSizeT _requestedSize) { // 170
	void* sval{};
	AlifByteArrayObject* obj = ((AlifByteArrayObject*)_self);
	size_t alloc = (size_t)obj->alloc;
	size_t logicalOffset = (size_t)(obj->start - obj->bytes);
	size_t size = (size_t)_requestedSize;

	if (_requestedSize == ALIF_SIZE(_self)) {
		return 0;
	}
	if (!can_resize(obj)) {
		return -1;
	}

	if (size + logicalOffset + 1 <= alloc) {
		/* Current buffer is large enough to host the requested size,
		   decide on a strategy. */
		if (size < alloc / 2) {
			/* Major downsize; resize down to exact size */
			alloc = size + 1;
		}
		else {
			/* Minor downsize; quick exit */
			ALIF_SET_SIZE(_self, size);
			ALIFBYTEARRAY_AS_STRING(_self)[size] = '\0'; /* Trailing null */
			return 0;
		}
	}
	else {
		/* Need growing, decide on a strategy */
		if (size <= alloc * 1.125) {
			/* Moderate upsize; overallocate similar to list_resize() */
			alloc = size + (size >> 3) + (size < 9 ? 3 : 6);
		}
		else {
			/* Major upsize; resize up to exact size */
			alloc = size + 1;
		}
	}
	if (alloc > ALIF_SIZET_MAX) {
		//alifErr_noMemory();
		return -1;
	}

	if (logicalOffset > 0) {
		sval = alifMem_dataAlloc(alloc);
		if (sval == nullptr) {
			//alifErr_noMemory();
			return -1;
		}
		memcpy(sval, ALIFBYTEARRAY_AS_STRING(_self),
			ALIF_MIN((AlifUSizeT)_requestedSize, (AlifUSizeT)ALIF_SIZE(_self)));
		alifMem_dataFree(obj->bytes);
	}
	else {
		sval = alifMem_dataRealloc(obj->bytes, alloc);
		if (sval == nullptr) {
			//alifErr_noMemory();
			return -1;
		}
	}

	obj->bytes = obj->start = (char*)sval;
	ALIF_SET_SIZE(_self, size);
	obj->alloc = alloc;
	obj->bytes[size] = '\0'; /* Trailing null byte */

	return 0;
}















AlifTypeObject _alifByteArrayType_ = { // 2389
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مصفوفة_بايت",
	.basicSize = sizeof(AlifByteArrayObject),
	.itemSize = 0,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		_ALIF_TPFLAGS_MATCH_SELF,
};
