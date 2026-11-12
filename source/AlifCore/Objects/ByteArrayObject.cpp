#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_BytesObject.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Object.h"
#include "AlifCore_Long.h"
#include "BytesObject.h"






char _alifByteArrayEmptyString_[] = "";

static AlifIntT _get_bytevalue(AlifObject* _arg, AlifIntT* _value) { // 23
	AlifIntT overflow{};
	long faceValue = alifLong_asLongAndOverflow(_arg, &overflow);

	if (faceValue == -1 and alifErr_occurred()) {
		*_value = -1;
		return 0;
	}
	if (faceValue < 0 or faceValue >= 256) {
		/* this includes an overflow in converting to C long */
		alifErr_setString(_alifExcValueError_, "الثمانية يجب أن تكون في المدى(0, 256)");
		*_value = -1;
		return 0;
	}

	*_value = faceValue;
	return 1;
}



static AlifIntT can_resize(AlifByteArrayObject* self) { // 67
	if (self->exports > 0) {
		//alifErr_setString(_alifExcBufferError_, "Existing exports of data: object cannot be re-sized");
		return 0;
	}
	return 1;
}


static AlifIntT _can_resize(AlifByteArrayObject* _self) { // 70
	if (_self->exports > 0) {
		//alifErr_setString(_alifExcBufferError_,
		//	"تصدير بيانات متاحة: كائن لا يمكن إعادة تحجيمه");
		return 0;
	}
	return 1;
}

#include "clinic/ByteArrayObject.cpp.h"



AlifObject* alifByteArray_fromStringAndSize(const char* _bytes, AlifSizeT _size) { // 108
	AlifByteArrayObject* new_{};
	AlifSizeT alloc{};

	if (_size < 0) {
		alifErr_setString(_alifExcSystemError_,
			"تم تمرير حجم بالقيمة السالبة لـ alifByteArray_fromStringAndSize");
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












static AlifIntT byteArray_setSliceLinear(AlifByteArrayObject* _self, AlifSizeT _lo,
	AlifSizeT _hi, char* _bytes, AlifSizeT _bytesLen) { // 443
	AlifSizeT avail = _hi - _lo;
	char* buf = ALIFBYTEARRAY_AS_STRING(_self);
	AlifSizeT growth = _bytesLen - avail;
	AlifIntT res = 0;

	if (growth < 0) {
		if (!_can_resize(_self))
			return -1;

		if (_lo == 0) {
			/* Shrink the buffer by advancing its logical start */
			_self->start -= growth;
			/*
			0   lo               hi             old_size
			|   |<----avail----->|<-----tail------>|
			|      |<-bytes_len->|<-----tail------>|
			0    new_lo         new_hi          new_size
			*/
		}
		else {
			/*
			0   lo               hi               old_size
			|   |<----avail----->|<-----tomove------>|
			|   |<-bytes_len->|<-----tomove------>|
			0   lo         new_hi              new_size
			*/
			memmove(buf + _lo + _bytesLen, buf + _hi,
				ALIF_SIZE(_self) - _hi);
		}
		if (alifByteArray_resize((AlifObject*)_self,
			ALIF_SIZE(_self) + growth) < 0) {
			if (_lo == 0) {
				_self->start += growth;
				return -1;
			}
			ALIF_SET_SIZE(_self, ALIF_SIZE(_self) + growth);
			res = -1;
		}
		buf = ALIFBYTEARRAY_AS_STRING(_self);
	}
	else if (growth > 0) {
		if (ALIF_SIZE(_self) > (AlifSizeT)ALIF_SIZET_MAX - growth) {
			//alifErr_noMemory();
			return -1;
		}

		if (alifByteArray_resize((AlifObject*)_self,
			ALIF_SIZE(_self) + growth) < 0) {
			return -1;
		}
		buf = ALIFBYTEARRAY_AS_STRING(_self);
		/* Make the place for the additional bytes */
		/*
		0   lo        hi               old_size
		|   |<-avail->|<-----tomove------>|
		|   |<---bytes_len-->|<-----tomove------>|
		0   lo            new_hi              new_size
		*/
		memmove(buf + _lo + _bytesLen, buf + _hi,
			ALIF_SIZE(_self) - _lo - _bytesLen);
	}

	if (_bytesLen > 0)
		memcpy(buf + _lo, _bytes, _bytesLen);
	return res;
}

static AlifIntT byteArray_setSlice(AlifByteArrayObject* _self,
	AlifSizeT _lo, AlifSizeT _hi, AlifObject* _values) { // 527
	AlifSizeT needed{};
	void* bytes{};
	AlifBuffer vbytes{};
	AlifIntT res = 0;

	vbytes.len = -1;
	if (_values == (AlifObject*)_self) {
		AlifIntT err;
		_values = alifByteArray_fromStringAndSize(ALIFBYTEARRAY_AS_STRING(_values),
			ALIFBYTEARRAY_GET_SIZE(_values));
		if (_values == nullptr)
			return -1;
		err = byteArray_setSlice(_self, _lo, _hi, _values);
		ALIF_DECREF(_values);
		return err;
	}
	if (_values == nullptr) {
		bytes = nullptr;
		needed = 0;
	}
	else {
		if (alifObject_getBuffer(_values, &vbytes, ALIFBUF_SIMPLE) != 0) {
			alifErr_format(_alifExcTypeError_,
				"لا يمكن ضبط قطع مصفوفة_ثمانية من %.100s",
				ALIF_TYPE(_values)->name);
			return -1;
		}
		needed = vbytes.len;
		bytes = vbytes.buf;
	}

	if (_lo < 0)
		_lo = 0;
	if (_hi < _lo)
		_hi = _lo;
	if (_hi > ALIF_SIZE(_self))
		_hi = ALIF_SIZE(_self);

	res = byteArray_setSliceLinear(_self, _lo, _hi, (char*)bytes, needed);
	if (vbytes.len != -1)
		alifBuffer_release(&vbytes);
	return res;
}





















static AlifObject* byteArray_appendImpl(AlifByteArrayObject* _self,
	AlifIntT _item) { // 1784
	AlifSizeT n = ALIF_SIZE(_self);

	if (n == ALIF_SIZET_MAX) {
		alifErr_setString(_alifExcOverflowError_,
			"لا يمكن إضافة كائنات أكثر إلى مصفوفة_ثمانية");
		return nullptr;
	}
	if (alifByteArray_resize((AlifObject*)_self, n + 1) < 0)
		return nullptr;

	ALIFBYTEARRAY_AS_STRING(_self)[n] = _item;

	return ALIF_NONE;
}



static AlifObject* byteArray_extend(AlifByteArrayObject* _self,
	AlifObject* _iterableOfInts) { // 1813
	AlifObject* it{}, * item{}, *byteArrayObj{};
	AlifSizeT bufSize = 0, len = 0;
	AlifIntT value{};
	char* buf{};

	if (alifObject_checkBuffer(_iterableOfInts)) {
		if (byteArray_setSlice(_self, ALIF_SIZE(_self), ALIF_SIZE(_self), _iterableOfInts) == -1)
			return nullptr;

		return ALIF_NONE;
	}

	it = alifObject_getIter(_iterableOfInts);
	if (it == nullptr) {
		if (alifErr_exceptionMatches(_alifExcTypeError_)) {
			alifErr_format(_alifExcTypeError_,
				"لا يمكن إطالة مصفوفة_ثمانية مع %.100s",
				ALIF_TYPE(_iterableOfInts)->name);
		}
		return nullptr;
	}

	bufSize = alifObject_lengthHint(_iterableOfInts, 32);
	if (bufSize == -1) {
		ALIF_DECREF(it);
		return nullptr;
	}

	byteArrayObj = alifByteArray_fromStringAndSize(nullptr, bufSize);
	if (byteArrayObj == nullptr) {
		ALIF_DECREF(it);
		return nullptr;
	}
	buf = ALIFBYTEARRAY_AS_STRING(byteArrayObj);

	while ((item = alifIter_next(it)) != nullptr) {
		if (!_get_bytevalue(item, &value)) {
			if (alifErr_exceptionMatches(_alifExcTypeError_) and ALIFUSTR_CHECK(_iterableOfInts)) {
				alifErr_format(_alifExcTypeError_,
					"من المتوقع مجموعة تكرار من الاعداد الصحيحة; ولكن تم تمرير: 'نص'");
			}
			ALIF_DECREF(item);
			ALIF_DECREF(it);
			ALIF_DECREF(byteArrayObj);
			return nullptr;
		}
		buf[len++] = value;
		ALIF_DECREF(item);

		if (len >= bufSize) {
			AlifSizeT addition;
			if (len == ALIF_SIZET_MAX) {
				ALIF_DECREF(it);
				ALIF_DECREF(byteArrayObj);
				//return alifErr_noMemory();
				return nullptr; //* temp
			}
			addition = len >> 1;
			if (addition > ALIF_SIZET_MAX - len - 1)
				bufSize = ALIF_SIZET_MAX;
			else
				bufSize = len + addition + 1;
			if (alifByteArray_resize((AlifObject*)byteArrayObj, bufSize) < 0) {
				ALIF_DECREF(it);
				ALIF_DECREF(byteArrayObj);
				return nullptr;
			}
			buf = ALIFBYTEARRAY_AS_STRING(byteArrayObj);
		}
	}
	ALIF_DECREF(it);

	if (alifErr_occurred()) {
		ALIF_DECREF(byteArrayObj);
		return nullptr;
	}

	/* Resize down to exact size. */
	if (alifByteArray_resize((AlifObject*)byteArrayObj, len) < 0) {
		ALIF_DECREF(byteArrayObj);
		return nullptr;
	}

	if (byteArray_setSlice(_self, ALIF_SIZE(_self), ALIF_SIZE(_self), byteArrayObj) == -1) {
		ALIF_DECREF(byteArrayObj);
		return nullptr;
	}
	ALIF_DECREF(byteArrayObj);

	return ALIF_NONE;
}









static AlifMethodDef _byteArrayMethods_[] = { // 2315
	BYTEARRAY_APPEND_METHODDEF


	BYTEARRAY_EXTEND_METHODDEF


	{nullptr}
};







static AlifObject* byteArray_iter(AlifObject*); // 2413


AlifTypeObject _alifByteArrayType_ = { // 2389
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مصفوفة_ثمانية",
	.basicSize = sizeof(AlifByteArrayObject),
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		_ALIF_TPFLAGS_MATCH_SELF,
	.iter = byteArray_iter,
	.methods = _byteArrayMethods_,
	.alloc = alifType_genericAlloc,
	.new_ = alifType_genericNew,
	.free = alifMem_objFree,
	.versionTag = _ALIF_TYPE_VERSION_BYTEARRAY,
};





/*********************** Bytearray Iterator ****************************/

class BytesIterObject { // 2461
public:
	ALIFOBJECT_HEAD;
	AlifSizeT itIndex{};
	AlifByteArrayObject *itSeq{}; /* Set to nullptr when iterator is exhausted */
};

static void byteArrayIter_dealloc(BytesIterObject* _it) { // 2467
	ALIFOBJECT_GC_UNTRACK(_it);
	ALIF_XDECREF(_it->itSeq);
	alifObject_gcDel(_it);
}

static AlifObject* byteArrayIter_next(BytesIterObject* _it) { // 2482
	AlifByteArrayObject* seq{};

	seq = _it->itSeq;
	if (seq == nullptr)
		return nullptr;

	if (_it->itIndex < ALIFBYTEARRAY_GET_SIZE(seq)) {
		return _alifLong_fromUnsignedChar(
			(unsigned char)ALIFBYTEARRAY_AS_STRING(seq)[_it->itIndex++]);
	}

	_it->itSeq = nullptr;
	ALIF_DECREF(seq);
	return nullptr;
}

AlifTypeObject _alifByteArrayIterType_ = { // 2563
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "تكرار_مصفوفة_ثمانية",
	.basicSize = sizeof(BytesIterObject),
	/* methods */
	.dealloc = (Destructor)byteArrayIter_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = (TraverseProc)byteArrayIter_traverse,
	.iter = alifObject_selfIter,
	.iterNext = (IterNextFunc)byteArrayIter_next,
	//.methods = _byteArrayIterMethods_,
};




static AlifObject* byteArray_iter(AlifObject* _seq) { // 2596
	BytesIterObject* it{};

	if (!ALIFBYTEARRAY_CHECK(_seq)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	it = ALIFOBJECT_GC_NEW(BytesIterObject, &_alifByteArrayIterType_);
	if (it == nullptr)
		return nullptr;
	it->itIndex = 0;
	it->itSeq = (AlifByteArrayObject*)ALIF_NEWREF(_seq);
	ALIFOBJECT_GC_TRACK(it);
	return (AlifObject*)it;
}
