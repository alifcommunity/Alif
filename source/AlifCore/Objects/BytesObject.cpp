#include "alif.h"

#include "AlifCore_BytesObject.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_Object.h"








#define ALIFBYTESOBJECT_SIZE (offsetof(AlifBytesObject, val) + 1) // 32

// 39
#define CHARACTERS ALIF_SINGLETON(bytesCharacters)
#define CHARACTER(ch) \
     ((AlifBytesObject *)&(CHARACTERS[ch]));
#define EMPTY (&ALIF_SINGLETON(bytesEmpty)) // 42


static inline AlifObject* bytes_getEmpty(void) { // 46
	AlifObject* empty = &EMPTY->objBase.objBase;
	return empty;
}



static AlifObject* alifBytes_fromSize(AlifSizeT _size, AlifIntT _useCalloc) { // 76
	AlifBytesObject* op{};

	if (_size == 0) {
		return bytes_getEmpty();
	}

	if ((AlifUSizeT)_size > (AlifUSizeT)ALIF_SIZET_MAX - ALIFBYTESOBJECT_SIZE) {
		//alifErr_SetString(_alifExcOverflowError_,
		//	"byte string is too large");
		return nullptr;
	}

	if (_useCalloc) {
		op = (AlifBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + _size); // this calloc and need review
	} else {
		op = (AlifBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + _size);
	}
	if (op == nullptr) {
		//return alifErr_noMemory();
		return nullptr; // temp
	}
	alifObject_initVar((AlifVarObject*)op, &_alifBytesType_, _size);
	op->hash = -1;
	if (!_useCalloc) {
		op->val[_size] = '\0';
	}
	return (AlifObject*)op;
}


AlifObject* alifBytes_fromStringAndSize(const char* _str, AlifSizeT _size) { // 111
	AlifBytesObject* op{};
	if (_size < 0) {
		//alifErr_setString(_alifExcSystemError_,
		//	"Negative size passed to alifBytes_fromStringAndSize");
		return nullptr;
	}
	if (_size == 1 and _str != nullptr) {
		op = CHARACTER(*_str & 255);
		return (AlifObject*)op;
	}
	if (_size == 0) {
		return bytes_getEmpty();
	}

	op = (AlifBytesObject*)alifBytes_fromSize(_size, 0);
	if (op == nullptr)
		return nullptr;
	if (_str == nullptr)
		return (AlifObject*)op;

	memcpy(op->val, _str, _size);
	return (AlifObject*)op;
}


AlifObject* alifBytes_fromString(const char* _str) { // 139
	AlifUSizeT size{};
	AlifBytesObject* op{};

	size = strlen(_str);
	if (size > ALIF_SIZET_MAX - ALIFBYTESOBJECT_SIZE) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"byte string is too long");
		return nullptr;
	}

	if (size == 0) {
		return bytes_getEmpty();
	}
	else if (size == 1) {
		op = CHARACTER(*_str & 255);
		return (AlifObject*)op;
	}

	/* Inline AlifObject_NewVar */
	op = (AlifBytesObject*)alifMem_objAlloc(ALIFBYTESOBJECT_SIZE + size);
	if (op == nullptr) {
		//return alifErr_noMemory();
		return nullptr; // temp
	}
	_alifObject_initVar((AlifVarObject*)op, &_alifBytesType_, size);
	op->hash = -1;
	memcpy(op->val, _str, size + 1);
	return (AlifObject*)op;
}






AlifTypeObject _alifBytesType_ = { // 3028
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "بايت",
	.basicSize = ALIFBYTESOBJECT_SIZE,
	.itemSize = sizeof(char),
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_BYTES_SUBCLASS | _ALIF_TPFLAGS_MATCH_SELF,
};



AlifIntT alifBytes_resize(AlifObject** _pv, AlifSizeT _newSize) { // 3141
	AlifObject* v{};
	AlifBytesObject* sv{};
	v = *_pv;
	if (!ALIFBYTES_CHECK(v) or _newSize < 0) {
		*_pv = 0;
		ALIF_DECREF(v);
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	AlifSizeT oldsize = ALIFBYTES_GET_SIZE(v);
	if (oldsize == _newSize) {
		/* return early if newsize equals to v->size */
		return 0;
	}
	if (oldsize == 0) {
		*_pv = alifBytes_fromSize(_newSize, 0);
		ALIF_DECREF(v);
		return (*_pv == nullptr) ? -1 : 0;
	}
	if (_newSize == 0) {
		*_pv = bytes_getEmpty();
		ALIF_DECREF(v);
		return 0;
	}
	if (ALIF_REFCNT(v) != 1) {
		if (oldsize < _newSize) {
			*_pv = alifBytes_fromSize(_newSize, 0);
			if (*_pv) {
				memcpy(ALIFBYTES_AS_STRING(*_pv), ALIFBYTES_AS_STRING(v), oldsize);
			}
		}
		else {
			*_pv = alifBytes_fromStringAndSize(ALIFBYTES_AS_STRING(v), _newSize);
		}
		ALIF_DECREF(v);
		return (*_pv == nullptr) ? -1 : 0;
	}

	* _pv = (AlifObject*)
		alifMem_objRealloc(v, ALIFBYTESOBJECT_SIZE + _newSize);
	if (*_pv == NULL) {
		alifMem_objFree(v);
		//alifErr_noMemory();
		return -1;
	}
	alif_newReferenceNoTotal(*_pv);
	sv = (AlifBytesObject*)*_pv;
	ALIF_SET_SIZE(sv, _newSize);
	sv->val[_newSize] = '\0';
	sv->hash = -1;          /* invalidate cached hash value */
	return 0;
}





/* AlifBytesWriter API */

#ifdef _WINDOWS // 3363
   /* On Windows, overallocate by 50% is the best factor */
#  define OVERALLOCATE_FACTOR 2
#else
   /* On Linux, overallocate by 25% is the best factor */
#  define OVERALLOCATE_FACTOR 4
#endif


void alifBytesWriter_init(AlifBytesWriter* _writer) { // 3371
	memset(_writer, 0, offsetof(AlifBytesWriter, smallBuffer));
}



void alifBytesWriter_dealloc(AlifBytesWriter* _writer) { // 3382
	ALIF_CLEAR(_writer->buffer);
}

ALIF_LOCAL_INLINE(char*)
alifBytesWriter_asString(AlifBytesWriter* _writer) { // 3388
	if (_writer->useSmallBuffer) {
		return _writer->smallBuffer;
	}
	else if (_writer->useByteArray) {
		return ALIFBYTEARRAY_AS_STRING(_writer->buffer);
	}
	else {
		return ALIFBYTES_AS_STRING(_writer->buffer);
	}
}

ALIF_LOCAL_INLINE(AlifSizeT)
alifBytesWriter_getSize(AlifBytesWriter* _writer, char* _str) { // 3405
	const char* start = alifBytesWriter_asString(_writer);
	return _str - start;
}

void* alifBytesWriter_resize(AlifBytesWriter* _writer, void* _str, AlifSizeT _size) { // 3452
	AlifSizeT allocated{}, pos{};

	allocated = _size;
	if (_writer->overAllocate
		and allocated <= (ALIF_SIZET_MAX - allocated / OVERALLOCATE_FACTOR)) {
		/* overallocate to limit the number of realloc() */
		allocated += allocated / OVERALLOCATE_FACTOR;
	}

	pos = alifBytesWriter_getSize(_writer, (char*)_str);
	if (!_writer->useSmallBuffer) {
		if (_writer->useByteArray) {
			if (alifByteArray_resize(_writer->buffer, allocated))
				goto error;
		}
		else {
			if (alifBytes_resize(&_writer->buffer, allocated))
				goto error;
		}
	}
	else {
		/* convert from stack buffer to bytes object buffer */

		if (_writer->useByteArray)
			_writer->buffer = alifByteArray_fromStringAndSize(nullptr, allocated);
		else
			_writer->buffer = alifBytes_fromStringAndSize(nullptr, allocated);
		if (_writer->buffer == nullptr)
			goto error;

		if (pos != 0) {
			char* dest;
			if (_writer->useByteArray)
				dest = ALIFBYTEARRAY_AS_STRING(_writer->buffer);
			else
				dest = ALIFBYTES_AS_STRING(_writer->buffer);
			memcpy(dest,
				_writer->smallBuffer,
				pos);
		}

		_writer->useSmallBuffer = 0;
	}
	_writer->allocated = allocated;

	_str = alifBytesWriter_asString(_writer) + pos;
	return _str;

error:
	alifBytesWriter_dealloc(_writer);
	return NULL;
}




void* alifBytesWriter_prepare(AlifBytesWriter* _writer, void* _str, AlifSizeT _size) { // 3522
	AlifSizeT newMinSize{};

	if (_size == 0) {
		/* nothing to do */
		return _str;
	}

	if (_writer->minSize > ALIF_SIZET_MAX - _size) {
		//alifErr_noMemory();
		alifBytesWriter_dealloc(_writer);
		return nullptr;
	}
	newMinSize = _writer->minSize + _size;

	if (newMinSize > _writer->allocated)
		_str = alifBytesWriter_resize(_writer, _str, newMinSize);

	_writer->minSize = newMinSize;
	return _str;
}


void* alifBytesWriter_alloc(AlifBytesWriter* writer, AlifSizeT size) { // 3552
	writer->useSmallBuffer = 1;
	writer->allocated = sizeof(writer->smallBuffer);
	return alifBytesWriter_prepare(writer, writer->smallBuffer, size);
}
