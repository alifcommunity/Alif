#include "alif.h"

#include "AlifCore_BytesObject.h"


















/* AlifBytesWriter API */

#ifdef _WINDOWS
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
