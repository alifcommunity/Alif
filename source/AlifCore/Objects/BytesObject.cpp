#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_BytesObject.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_Long.h"
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










AlifObject* _alifBytes_decodeEscape(const char* _str, AlifSizeT _len, const char* _errors,
	const char** _firstInvalidEscape) { // 1058
	AlifIntT c_{};
	char* p_{};
	const char* end{};
	AlifBytesWriter writer{};

	alifBytesWriter_init(&writer);

	p_ = (char*)alifBytesWriter_alloc(&writer, _len);
	if (p_ == nullptr)
		return nullptr;
	writer.overAllocate = 1;

	*_firstInvalidEscape = nullptr;

	end = _str + _len;
	while (_str < end) {
		if (*_str != '\\') {
			*p_++ = *_str++;
			continue;
		}

		_str++;
		if (_str == end) {
			//alifErr_setString(_alifExcValueError_,
			//	"Trailing \\ in string");
			goto failed;
		}

		switch (*_str++) {
			/* XXX This assumes ASCII! */
		case '\n': break;
		case '\\': *p_++ = '\\'; break;
		case '\'': *p_++ = '\''; break;
		case '\"': *p_++ = '\"'; break;
		case 'b': *p_++ = '\b'; break;
		case 'f': *p_++ = '\014'; break; /* FF */
		case 't': *p_++ = '\t'; break;
		case 'n': *p_++ = '\n'; break;
		case 'r': *p_++ = '\r'; break;
		case 'v': *p_++ = '\013'; break; /* VT */
		case 'a': *p_++ = '\007'; break; /* BEL, not classic C */
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			c_ = _str[-1] - '0';
			if (_str < end and '0' <= *_str and *_str <= '7') {
				c_ = (c_ << 3) + *_str++ - '0';
				if (_str < end and '0' <= *_str and *_str <= '7')
					c_ = (c_ << 3) + *_str++ - '0';
			}
			if (c_ > 0377) {
				if (*_firstInvalidEscape == NULL) {
					*_firstInvalidEscape = _str - 3; /* Back up 3 chars, since we've
													already incremented s. */
				}
			}
			*p_++ = c_;
			break;
		case 'x':
			if (_str + 1 < end) {
				int digit1, digit2;
				digit1 = _alifLongDigitValue_[ALIF_CHARMASK(_str[0])];
				digit2 = _alifLongDigitValue_[ALIF_CHARMASK(_str[1])];
				if (digit1 < 16 and digit2 < 16) {
					*p_++ = (unsigned char)((digit1 << 4) + digit2);
					_str += 2;
					break;
				}
			}
			/* invalid hexadecimal digits */

			if (!_errors or strcmp(_errors, "strict") == 0) {
				//alifErr_format(_alifExcValueError_,
				//	"invalid \\x escape at position %zd",
				//	_str - 2 - (end - _len));
				goto failed;
			}
			if (strcmp(_errors, "replace") == 0) {
				*p_++ = '?';
			}
			else if (strcmp(_errors, "ignore") == 0)
				/* do nothing */;
			else {
				//alifErr_format(_alifExcValueError_,
				//	"decoding error; unknown "
				//	"error handling code: %.400s",
				//	_errors);
				goto failed;
			}
			/* skip \x */
			if (_str < end and ALIF_ISXDIGIT(_str[0]))
				_str++; /* and a hexdigit */
			break;

		default:
			if (*_firstInvalidEscape == nullptr) {
				*_firstInvalidEscape = _str - 1; /* Back up one char, since we've
												already incremented s. */
			}
			*p_++ = '\\';
			_str--;
		}
	}

	return alifBytesWriter_finish(&writer, p_);

failed:
	alifBytesWriter_dealloc(&writer);
	return nullptr;
}


AlifSizeT alifBytes_size(AlifObject* op) { // 1211
	if (!ALIFBYTES_CHECK(op)) {
		//alifErr_format(_alifExcTypeError_,
		//	"expected bytes, %.200s found", ALIF_TYPE(op)->name);
		return -1;
	}
	return ALIF_SIZE(op);
}


char* alifBytes_asString(AlifObject* _op) { // 1221
	if (!ALIFBYTES_CHECK(_op)) {
		//alifErr_format(_alifExcTypeError_,
		//	"expected bytes, %.200s found", ALIF_TYPE(_op)->name);
		return nullptr;
	}
	return ((AlifBytesObject*)_op)->val;
}


AlifIntT alifBytes_asStringAndSize(AlifObject* _obj,
	char** _str, AlifSizeT* _len) { // 1232
	if (_str == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	if (!ALIFBYTES_CHECK(_obj)) {
		//alifErr_format(_alifExcTypeError_,
		//	"expected bytes, %.200s found", ALIF_TYPE(_obj)->name);
		return -1;
	}

	*_str = ALIFBYTES_AS_STRING(_obj);
	if (_len != nullptr)
		*_len = ALIFBYTES_GET_SIZE(_obj);
	else if (strlen(*_str) != (AlifUSizeT)ALIFBYTES_GET_SIZE(_obj)) {
		//alifErr_setString(_alifExcValueError_,
		//	"embedded null byte");
		return -1;
	}
	return 0;
}



static AlifObject* bytes_concat(AlifObject* _a, AlifObject* _b) { // 1414
	AlifBuffer va{}, vb{};
	AlifObject* result = nullptr;

	va.len = -1;
	vb.len = -1;
	if (alifObject_getBuffer(_a, &va, ALIFBUF_SIMPLE) != 0 or
		alifObject_getBuffer(_b, &vb, ALIFBUF_SIMPLE) != 0) {
		//alifErr_format(_alifExcTypeError_, "can't concat %.100s to %.100s",
		//	ALIF_TYPE(_b)->name, ALIF_TYPE(_a)->name);
		goto done;
	}

	/* Optimize end cases */
	if (va.len == 0 and ALIFBYTES_CHECKEXACT(_b)) {
		result = ALIF_NEWREF(_b);
		goto done;
	}
	if (vb.len == 0 and ALIFBYTES_CHECKEXACT(_a)) {
		result = ALIF_NEWREF(_a);
		goto done;
	}

	if (va.len > ALIF_SIZET_MAX - vb.len) {
		//alifErr_noMemory();
		goto done;
	}

	result = alifBytes_fromStringAndSize(nullptr, va.len + vb.len);
	if (result != nullptr) {
		memcpy(ALIFBYTES_AS_STRING(result), va.buf, va.len);
		memcpy(ALIFBYTES_AS_STRING(result) + va.len, vb.buf, vb.len);
	}

done:
	if (va.len != -1)
		alifBuffer_release(&va);
	if (vb.len != -1)
		alifBuffer_release(&vb);
	return result;
}








AlifTypeObject _alifBytesType_ = { // 3028
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "بايت",
	.basicSize = ALIFBYTESOBJECT_SIZE,
	.itemSize = sizeof(char),
	.dealloc = 0,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_BYTES_SUBCLASS | _ALIF_TPFLAGS_MATCH_SELF,
	.free = alifMem_objFree,
};

void alifBytes_concat(AlifObject** _pv, AlifObject* _w) { // 3072
	if (*_pv == nullptr) return;
	if (_w == nullptr) {
		ALIF_CLEAR(*_pv);
		return;
	}

	if (ALIF_REFCNT(*_pv) == 1 and ALIFBYTES_CHECKEXACT(*_pv)) {
		/* Only one reference, so we can resize in place */
		AlifSizeT oldsize{};
		AlifBuffer wb{};

		if (alifObject_getBuffer(_w, &wb, ALIFBUF_SIMPLE) != 0) {
			//alifErr_format(_alifExcTypeError_, "can't concat %.100s to %.100s",
			//	ALIF_TYPE(_w)->name, ALIF_TYPE(*_pv)->name);
			ALIF_CLEAR(*_pv);
			return;
		}

		oldsize = ALIFBYTES_GET_SIZE(*_pv);
		if (oldsize > ALIF_SIZET_MAX - wb.len) {
			//alifErr_noMemory();
			goto error;
		}
		if (alifBytes_resize(_pv, oldsize + wb.len) < 0)
			goto error;

		memcpy(ALIFBYTES_AS_STRING(*_pv) + oldsize, wb.buf, wb.len);
		alifBuffer_release(&wb);
		return;

	error:
		alifBuffer_release(&wb);
		ALIF_CLEAR(*_pv);
		return;
	}

	else {
		/* Multiple references, need to create new object */
		AlifObject* v{};
		v = bytes_concat(*_pv, _w);
		ALIF_SETREF(*_pv, v);
	}
}


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



AlifObject* alifBytesWriter_finish(AlifBytesWriter* _writer, void* _str) { // 3582
	AlifSizeT size{};
	AlifObject* result{};

	size = alifBytesWriter_getSize(_writer, (char*)_str);
	if (size == 0 and !_writer->useByteArray) {
		ALIF_CLEAR(_writer->buffer);
		/* Get the empty byte string singleton */
		result = alifBytes_fromStringAndSize(nullptr, 0);
	}
	else if (_writer->useSmallBuffer) {
		if (_writer->useByteArray) {
			result = alifByteArray_fromStringAndSize(_writer->smallBuffer, size);
		}
		else {
			result = alifBytes_fromStringAndSize(_writer->smallBuffer, size);
		}
	}
	else {
		result = _writer->buffer;
		_writer->buffer = nullptr;

		if (size != _writer->allocated) {
			if (_writer->useByteArray) {
				if (alifByteArray_resize(result, size)) {
					ALIF_DECREF(result);
					return nullptr;
				}
			}
			else {
				if (alifBytes_resize(&result, size)) {
					return nullptr;
				}
			}
		}
	}
	return result;
}





void* alifBytesWriter_writeBytes(AlifBytesWriter* _writer, void* _ptr,
	const void* _bytes, AlifSizeT _size) { // 3626
	char* str = (char*)_ptr;

	str = (char*)alifBytesWriter_prepare(_writer, str, _size);
	if (str == nullptr)
		return nullptr;

	memcpy(str, _bytes, _size);
	str += _size;

	return str;
}
