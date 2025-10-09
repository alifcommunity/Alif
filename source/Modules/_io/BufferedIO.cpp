#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"
#include "AlifCore_LifeCycle.h"

#include "_IOModule.h"




static AlifObject* bufferedIOBase_unsupported(AlifIOState* _state,
	const char* _message) { // 111
	alifErr_setString(_state->unsupportedOperation, _message);
	return nullptr;
}

static AlifObject* _io_BufferedIOBase_read1Impl(AlifObject* self, AlifTypeObject* cls,
	AlifIntT ALIF_UNUSED(size)) { // 185
	AlifIOState* state = getIOState_byCls(cls);
	return bufferedIOBase_unsupported(state, "read1");
}



typedef class Buffered { // 220
public:
	ALIFOBJECT_HEAD;
	AlifObject* raw{};
	AlifIntT ok{};    /* Initialized? */
	AlifIntT detached{};
	AlifIntT readable{};
	AlifIntT writable{};
	char finalizing{};

	/* True if this is a vanilla Buffered object (rather than a user derived
	   class) *and* the raw stream is a vanilla FileIO object. */
	AlifIntT fastClosedChecks{};

	/* Absolute position inside the raw stream (-1 if unknown). */
	AlifOffT absPos{};

	/* A static buffer of size `buffer_size` */
	char* buffer{};
	/* Current logical position in the buffer. */
	AlifOffT pos{};
	/* Position of the raw stream in the buffer. */
	AlifOffT rawPos{};

	/* Just after the last buffered byte in the buffer, or -1 if the buffer
	   isn't ready for reading. */
	AlifOffT readEnd{};

	/* Just after the last byte actually written */
	AlifOffT writePos{};
	/* Just after the last byte waiting to be written, or -1 if the buffer
	   isn't ready for writing. */
	AlifOffT writeEnd{};

	AlifThreadTypeLock lock{};
	volatile unsigned long owner{};

	AlifSizeT bufferSize{};
	AlifSizeT bufferMask{};

	AlifObject* dict{};
	AlifObject* weakRefList{};
};


// 324
#define ENTER_BUFFERED(_self) \
    ( (alifThread_acquireLock(_self->lock, 0) ? \
       1 : _enterBuffered_busy(_self)) \
     and (_self->owner = alifThread_getThreadIdent(), 1) )

#define LEAVE_BUFFERED(_self) \
    do { \
        _self->owner = 0; \
        alifThread_releaseLock(_self->lock); \
    } while(0);

#define CHECK_INITIALIZED(_self) \
    if (_self->ok <= 0) { \
        if (_self->detached) { \
            alifErr_setString(_alifExcValueError_, \
                 "التدفق الخام للنص قد تم فصله"); \
        } else { \
            alifErr_setString(_alifExcValueError_, \
                "عمليات بتادل على كائن غير مهيئ"); \
        } \
        return nullptr; \
    }
// 347
#define CHECK_INITIALIZED_INT(_self) \
    if (_self->ok <= 0) { \
        if (_self->detached) { \
            alifErr_setString(_alifExcValueError_, \
                 "التدفق الخام للنص قد تم فصله"); \
        } else { \
            alifErr_setString(_alifExcValueError_, \
                "عمليات بتادل على كائن غير مهيئ"); \
        } \
        return -1; \
    }

// 359
#define IS_CLOSED(_self) \
    (!_self->buffer or \
    (_self->fastClosedChecks \
     ? _alifFileIO_closed(_self->raw) \
     : buffered_closed(_self)))
// 365
#define CHECK_CLOSED(_self, _errorMsg) \
    if (IS_CLOSED(_self) and (ALIF_SAFE_DOWNCAST(READAHEAD(_self), AlifOffT, AlifSizeT) == 0)) { \
        alifErr_setString(_alifExcValueError_, _errorMsg); \
        return nullptr; \
    } \

#define VALID_READ_BUFFER(_self) \
    (_self->readable and self->readEnd != -1) // 371

#define VALID_WRITE_BUFFER(self) \
    (self->writable and self->writeEnd != -1) // 374

// 377
#define ADJUST_POSITION(self, _new_pos) \
    do { \
        self->pos = _new_pos; \
        if (VALID_READ_BUFFER(self) && self->readEnd < self->pos) \
            self->readEnd = self->pos; \
    } while(0)

#define READAHEAD(_self) \
    ((self->readable and VALID_READ_BUFFER(_self)) \
        ? (_self->readEnd - _self->pos) : 0) // 384

#define RAW_OFFSET(self) \
    (((VALID_READ_BUFFER(self) or VALID_WRITE_BUFFER(self)) \
        and self->rawPos >= 0) ? self->rawPos - self->pos : 0) // 388



static AlifIntT buffered_traverse(Buffered* self, VisitProc visit, void* arg) { // 450
	ALIF_VISIT(ALIF_TYPE(self));
	ALIF_VISIT(self->raw);
	ALIF_VISIT(self->dict);
	return 0;
}

static AlifObject* _io_Buffered__deallocWarn(Buffered* self, AlifObject* source) { // 470
	if (self->ok and self->raw) {
		AlifObject* r{};
		r = alifObject_callMethodOneArg(self->raw, &ALIF_ID(_deallocWarn), source);
		if (r)
			ALIF_DECREF(r);
		else
			alifErr_clear();
	}
	return ALIF_NONE;
}


static AlifIntT buffered_closed(Buffered* _self) { // 505
	AlifIntT closed{};
	AlifObject* res{};
	CHECK_INITIALIZED_INT(_self);
	res = alifObject_getAttr(_self->raw, &ALIF_ID(Closed));
	if (res == nullptr)
		return -1;
	closed = alifObject_isTrue(res);
	ALIF_DECREF(res);
	return closed;
}


static AlifObject* _io_Buffered_closedGetImpl(Buffered* self) { // 525
	CHECK_INITIALIZED(self)
		return alifObject_getAttr(self->raw, &ALIF_ID(Closed));
}


static AlifObject* _io_Buffered_closeImpl(Buffered* self) { // 538
	AlifObject* res = nullptr;
	AlifIntT r{};

	CHECK_INITIALIZED(self);
	//if (!ENTER_BUFFERED(self)) {
	//	return nullptr;
	//}

	r = buffered_closed(self);
	if (r < 0)
		goto end;
	if (r > 0) {
		res = ALIF_NEWREF(ALIF_NONE);
		goto end;
	}

	if (self->finalizing) {
		AlifObject* r = _io_Buffered__deallocWarn(self, (AlifObject*)self);
		if (r)
			ALIF_DECREF(r);
		else
			alifErr_clear();
	}
	/* flush() will most probably re-take the lock, so drop it first */
	//LEAVE_BUFFERED(self)
	//r = _alifFile_flush((AlifObject*)self);
	//if (!ENTER_BUFFERED(self)) {
	//	return nullptr;
	//}
	AlifObject* exc; exc = nullptr;
	if (r < 0) {
		exc = alifErr_getRaisedException();
	}

	res = alifObject_callMethodNoArgs(self->raw, &ALIF_STR(Close));

	if (self->buffer) {
		alifMem_dataFree(self->buffer);
		self->buffer = nullptr;
	}

	if (exc != nullptr) {
		_alifErr_chainExceptions1(exc);
		ALIF_CLEAR(res);
	}

	self->readEnd = 0;
	self->pos = 0;

end:
	//LEAVE_BUFFERED(self)
	return res;
}


static AlifObject* _io_Buffered_seekableImpl(Buffered* self) { // 624
	CHECK_INITIALIZED(self);
	return alifObject_callMethodNoArgs(self->raw, &ALIF_ID(Seekable));
}


static AlifObject* _io_Buffered_readableImpl(Buffered* self) { // 637
	CHECK_INITIALIZED(self);
	return alifObject_callMethodNoArgs(self->raw, &ALIF_ID(Readable));
}


static AlifObject* _io_Buffered_writableImpl(Buffered* _self) { // 650
	CHECK_INITIALIZED(_self);
	return alifObject_callMethodNoArgs(_self->raw, &ALIF_ID(Writable));
}


static AlifObject* _io_Buffered_filenoImpl(Buffered* _self) { // 694
	CHECK_INITIALIZED(_self);
	return alifObject_callMethodNoArgs(_self->raw, &ALIF_ID(Fileno));
}


static AlifObject* _bufferedWriter_flushUnlocked(Buffered*); // 717
static void _bufferedReader_resetBuf(Buffered*); // 720
static AlifObject* _bufferedReader_readAll(Buffered*); // 727
static AlifObject* _bufferedReader_readFast(Buffered*, AlifSizeT); // 728
static AlifSizeT _bufferedReader_rawRead(Buffered*, char*, AlifSizeT); // 732




static AlifSizeT* _buffered_checkBlockingError(void) { // 754
	AlifObject* exc = alifErr_getRaisedException();
	if (exc == nullptr or !alifErr_givenExceptionMatches(exc, _alifExcBlockingIOError_)) {
		alifErr_setRaisedException(exc);
		return nullptr;
	}
	AlifOSErrorObject* err = (AlifOSErrorObject*)exc;
	alifErr_setRaisedException(exc);
	return &err->written;
}


static AlifIntT _buffered_init(Buffered* self) { // 822
	AlifSizeT n{};
	if (self->bufferSize <= 0) {
		alifErr_setString(_alifExcValueError_,
			"buffer size must be strictly positive");
		return -1;
	}
	if (self->buffer)
		alifMem_dataFree(self->buffer);
	self->buffer = (char*)alifMem_dataAlloc(self->bufferSize);
	if (self->buffer == nullptr) {
		//alifErr_noMemory();
		return -1;
	}
	if (self->lock)
		alifThread_freeLock(self->lock);
	self->lock = alifThread_allocateLock();
	if (self->lock == nullptr) {
		//alifErr_setString(_alifExcRuntimeError_, "can't allocate read lock");
		return -1;
	}
	self->owner = 0;
	/* Find out whether bufferSize is a power of 2 */
	/* XXX is this optimization useful? */
	for (n = self->bufferSize - 1; n & 1; n >>= 1)
		;
	if (n == 0)
		self->bufferMask = self->bufferSize - 1;
	else
		self->bufferMask = 0;
	//if (_buffered_rawTell(self) == -1)
	//	alifErr_clear();
	return 0;
}


AlifIntT _alifIO_trapEintr(void) { // 863
	if (!alifErr_exceptionMatches(_alifExcOSError_)) {
		return 0;
	}
	AlifObject* exc = alifErr_getRaisedException();
	AlifOSErrorObject* env_err = (AlifOSErrorObject*)exc;
	if (env_err->myErrno != nullptr) {
		AlifIntT overflow;
		AlifIntT myerrno = alifLong_asLongAndOverflow(env_err->myErrno, &overflow);
		alifErr_clear();
		if (myerrno == EINTR) {
			ALIF_DECREF(exc);
			return 1;
		}
	}
	alifErr_setRaisedException(exc);
	return 0;
}


static AlifObject* buffered_flushAndRewindUnlocked(Buffered* self) { // 892
	AlifObject* res{};

	res = _bufferedWriter_flushUnlocked(self);
	if (res == nullptr)
		return nullptr;
	ALIF_DECREF(res);

	if (self->readable) {
		AlifOffT n{};
		//n = _buffered_rawSeek(self, -RAW_OFFSET(self), 1);
		_bufferedReader_resetBuf(self);
		if (n == -1)
			return nullptr;
	}
	return ALIF_NONE;
}


static AlifObject* _io_Buffered_flushImpl(Buffered* self) { // 919
	AlifObject* res{};

	CHECK_INITIALIZED(self);
	CHECK_CLOSED(self, "flush of closed file");

	//if (!ENTER_BUFFERED(self))
	//	return nullptr;
	res = buffered_flushAndRewindUnlocked(self);
	//LEAVE_BUFFERED(self);

	return res;
}


static AlifObject* _io_Buffered_readImpl(Buffered* self, AlifSizeT n) { // 976
	AlifObject* res{};

	CHECK_INITIALIZED(self)
		if (n < -1) {
			alifErr_setString(_alifExcValueError_,
				"read length must be non-negative or -1");
			return nullptr;
		}

	CHECK_CLOSED(self, "محاولة قراءة ملف مغلق");

	if (n == -1) {
		/* The number of bytes is unspecified, read until the end of stream */
		//if (!ENTER_BUFFERED(self))
		//	return nullptr;
		res = _bufferedReader_readAll(self);
	}
	else {
		res = _bufferedReader_readFast(self, n);
		if (res != ALIF_NONE)
			return res;
		ALIF_DECREF(res);
		//if (!ENTER_BUFFERED(self))
		//	return nullptr;
		//res = _bufferedReader_readGeneric(self, n);
	}

	//LEAVE_BUFFERED(self)
	return res;
}


static AlifObject* _io_Buffered_read1Impl(Buffered* self, AlifSizeT n) { // 1018
	AlifSizeT have{}, r{};
	AlifObject* res = nullptr;

	CHECK_INITIALIZED(self)
		if (n < 0) {
			n = self->bufferSize;
		}

	CHECK_CLOSED(self, "read of closed file");

	if (n == 0)
		return alifBytes_fromStringAndSize(nullptr, 0);

	have = ALIF_SAFE_DOWNCAST(READAHEAD(self), AlifOffT, AlifSizeT);
	if (have > 0) {
		n = ALIF_MIN(have, n);
		res = _bufferedReader_readFast(self, n);
		return res;
	}
	res = alifBytes_fromStringAndSize(nullptr, n);
	if (res == nullptr)
		return nullptr;
	//if (!ENTER_BUFFERED(self)) {
	//	ALIF_DECREF(res);
	//	return nullptr;
	//}
	/* Flush the write buffer if necessary */
	//if (self->writable) {
	//	AlifObject* r = buffered_flushAndRewindUnlocked(self);
	//	if (r == nullptr) {
	//		LEAVE_BUFFERED(self)
	//		ALIF_DECREF(res);
	//		return nullptr;
	//	}
	//	ALIF_DECREF(r);
	//}
	_bufferedReader_resetBuf(self);
	r = _bufferedReader_rawRead(self, ALIFBYTES_AS_STRING(res), n);
	//LEAVE_BUFFERED(self)
	if (r == -1) {
		ALIF_DECREF(res);
		return nullptr;
	}
	if (r == -2)
		r = 0;
	if (n > r)
		_alifBytes_resize(&res, r);
	return res;
}

static void _bufferedReader_resetBuf(Buffered* self) { // 1556
	self->readEnd = -1;
}



static AlifIntT _ioBufferedReader___init__Impl(Buffered* self, AlifObject* raw,
	AlifSizeT buffer_size) { // 1569
	self->ok = 0;
	self->detached = 0;

	AlifIOState* state = findIOState_byDef(ALIF_TYPE(self));
	if (_alifIOBase_checkReadable(state, raw, ALIF_TRUE) == nullptr) {
		return -1;
	}

	ALIF_XSETREF(self->raw, ALIF_NEWREF(raw));
	self->bufferSize = buffer_size;
	self->readable = 1;
	self->writable = 0;

	if (_buffered_init(self) < 0)
		return -1;
	_bufferedReader_resetBuf(self);

	self->fastClosedChecks = (
		ALIF_IS_TYPE(self, state->alifBufferedReaderType) and
		ALIF_IS_TYPE(raw, state->alifFileIOType)
		);

	self->ok = 1;
	return 0;
}



static AlifSizeT _bufferedReader_rawRead(Buffered* self, char* start, AlifSizeT len) { // 1600
	AlifBuffer buf{};
	AlifObject* memobj{}, * res{};
	AlifSizeT n{};
	if (alifBuffer_fillInfo(&buf, nullptr, start, len, 0, ALIFBUF_CONTIG) == -1)
		return -1;
	memobj = alifMemoryView_fromBuffer(&buf);
	if (memobj == nullptr)
		return -1;
	do {
		res = alifObject_callMethodOneArg(self->raw, &ALIF_ID(ReadInto), memobj);
	}
	while (res == nullptr and _alifIO_trapEintr());
	ALIF_DECREF(memobj);
	if (res == nullptr)
		return -1;
	if (res == ALIF_NONE) {
		/* Non-blocking stream would have blocked. Special return code! */
		ALIF_DECREF(res);
		return -2;
	}
	n = alifNumber_asSizeT(res, _alifExcValueError_);
	ALIF_DECREF(res);

	if (n == -1 and alifErr_occurred()) {
		//_alifErr_formatFromCause(
		//	_alifExcOSError_,
		//	"raw readinto() failed"
		//);
		return -1;
	}

	if (n < 0 or n > len) {
		alifErr_format(_alifExcOSError_,
			"raw readinto() returned invalid length %zd "
			"(should have been between 0 and %zd)", n, len);
		return -1;
	}
	if (n > 0 and self->absPos != -1)
		self->absPos += n;
	return n;
}


static AlifObject* _bufferedReader_readAll(Buffered* self) { // 1667
	AlifSizeT currentSize{};
	AlifObject* res = nullptr, * data = nullptr, * tmp = nullptr, * chunks = nullptr, * readall{};

	/* First copy what we have in the current buffer. */
	currentSize = ALIF_SAFE_DOWNCAST(READAHEAD(self), AlifOffT, AlifSizeT);
	if (currentSize) {
		data = alifBytes_fromStringAndSize(
			self->buffer + self->pos, currentSize);
		if (data == nullptr)
			return nullptr;
		self->pos += currentSize;
	}
	/* We're going past the buffer's bounds, flush it */
	//if (self->writable) {
	//	tmp = buffered_flushAndRewindUnlocked(self);
	//	if (tmp == nullptr)
	//		goto cleanup;
	//	ALIF_CLEAR(tmp);
	//}
	_bufferedReader_resetBuf(self);

	if (alifObject_getOptionalAttr(self->raw, &ALIF_ID(ReadAll), &readall) < 0) {
		goto cleanup;
	}
	if (readall) {
		tmp = _alifObject_callNoArgs(readall);
		ALIF_DECREF(readall);
		if (tmp == nullptr)
			goto cleanup;
		if (tmp != ALIF_NONE and !ALIFBYTES_CHECK(tmp)) {
			alifErr_setString(_alifExcTypeError_, "readall() should return bytes");
			goto cleanup;
		}
		if (currentSize == 0) {
			res = tmp;
		}
		else {
			if (tmp != ALIF_NONE) {
				alifBytes_concat(&data, tmp);
			}
			res = data;
		}
		goto cleanup;
	}

	chunks = alifList_new(0);
	if (chunks == nullptr)
		goto cleanup;

	while (1) {
		if (data) {
			if (alifList_append(chunks, data) < 0)
				goto cleanup;
			ALIF_CLEAR(data);
		}

		/* Read until EOF or until read() would block. */
		data = alifObject_callMethodNoArgs(self->raw, &ALIF_ID(Read));
		if (data == nullptr)
			goto cleanup;
		if (data != ALIF_NONE and !ALIFBYTES_CHECK(data)) {
			alifErr_setString(_alifExcTypeError_, "read() should return bytes");
			goto cleanup;
		}
		if (data == ALIF_NONE or ALIFBYTES_GET_SIZE(data) == 0) {
			if (currentSize == 0) {
				res = data;
				goto cleanup;
			}
			else {
				//tmp = alifBytes_join((AlifObject*)&ALIF_SINGLETON(bytesEmpty), chunks);
				res = tmp;
				goto cleanup;
			}
		}
		currentSize += ALIFBYTES_GET_SIZE(data);
		if (self->absPos != -1)
			self->absPos += ALIFBYTES_GET_SIZE(data);
	}
cleanup:
	/* res is either nullptr or a borrowed ref */
	ALIF_XINCREF(res);
	ALIF_XDECREF(data);
	ALIF_XDECREF(tmp);
	ALIF_XDECREF(chunks);
	return res;
}


static AlifObject* _bufferedReader_readFast(Buffered* self, AlifSizeT n) { // 1759
	AlifSizeT current_size{};

	current_size = ALIF_SAFE_DOWNCAST(READAHEAD(self), AlifOffT, AlifSizeT);
	if (n <= current_size) {
		AlifObject* res = alifBytes_fromStringAndSize(self->buffer + self->pos, n);
		if (res != nullptr)
			self->pos += n;
		return res;
	}
	return ALIF_NONE;
}





static void _bufferedWriter_resetBuf(Buffered* _self) { // 1906
	_self->writePos = 0;
	_self->writeEnd = -1;
}


static AlifIntT _ioBufferedWriter___init__Impl(Buffered* _self, AlifObject* _raw,
	AlifSizeT _bufferSize) { // 1925
	_self->ok = 0;
	_self->detached = 0;

	AlifIOState* state = findIOState_byDef(ALIF_TYPE(_self));
	if (_alifIOBase_checkWritable(state, _raw, ALIF_TRUE) == nullptr) {
		return -1;
	}

	ALIF_INCREF(_raw);
	ALIF_XSETREF(_self->raw, _raw);
	_self->readable = 0;
	_self->writable = 1;

	_self->bufferSize = _bufferSize;
	if (_buffered_init(_self) < 0)
		return -1;
	_bufferedWriter_resetBuf(_self);
	_self->pos = 0;

	_self->fastClosedChecks = (
		ALIF_IS_TYPE(_self, state->alifBufferedWriterType) and
		ALIF_IS_TYPE(_raw, state->alifFileIOType)
		);

	_self->ok = 1;
	return 0;
}


static AlifSizeT _bufferedWriter_rawWrite(Buffered* self, char* start, AlifSizeT len) { // 1958
	AlifBuffer buf{};
	AlifObject* memobj{}, * res{};
	AlifSizeT n{};
	AlifIntT errnum{};
	if (alifBuffer_fillInfo(&buf, nullptr, start, len, 1, ALIFBUF_CONTIG_RO) == -1)
		return -1;
	memobj = alifMemoryView_fromBuffer(&buf);
	if (memobj == nullptr)
		return -1;
	do {
		errno = 0;
		res = alifObject_callMethodOneArg(self->raw, &ALIF_ID(Write), memobj);
		errnum = errno;
	}
	while (res == nullptr and _alifIO_trapEintr());
	ALIF_DECREF(memobj);
	if (res == nullptr)
		return -1;
	if (res == ALIF_NONE) {
		ALIF_DECREF(res);
		errno = errnum;
		return -2;
	}
	n = alifNumber_asSizeT(res, _alifExcValueError_);
	ALIF_DECREF(res);
	if (n < 0 or n > len) {
		alifErr_format(_alifExcOSError_,
			"raw write() returned invalid length %zd "
			"(should have been between 0 and %zd)", n, len);
		return -1;
	}
	if (n > 0 and self->absPos != -1)
		self->absPos += n;
	return n;
}


static AlifObject* _bufferedWriter_flushUnlocked(Buffered* self) { // 2005
	AlifOffT n{}, rewind{};

	if (!VALID_WRITE_BUFFER(self) or self->writePos == self->writeEnd)
		goto end;
	/* First, rewind */
	rewind = RAW_OFFSET(self) + (self->pos - self->writePos);
	if (rewind != 0) {
		//n = _buffered_rawSeek(self, -rewind, 1);
		//if (n < 0) {
		//	goto error;
		//}
		//self->rawPos -= rewind;
	}
	while (self->writePos < self->writeEnd) {
		n = _bufferedWriter_rawWrite(self,
			self->buffer + self->writePos,
			ALIF_SAFE_DOWNCAST(self->writeEnd - self->writePos,
				AlifoffT, AlifSizeT));
		if (n == -1) {
			goto error;
		}
		else if (n == -2) {
			//_set_BlockingIOError("write could not complete without blocking", 0);
			goto error;
		}
		self->writePos += n;
		self->rawPos = self->writePos;
		//if (alifErr_checkSignals() < 0)
		//	goto error;
	}


end:
	_bufferedWriter_resetBuf(self);
	return ALIF_NONE;

error:
	return nullptr;
}



static AlifObject* _ioBufferedWriter_writeImpl(Buffered* self, AlifBuffer* buffer) { // 2068
	AlifObject* res = nullptr;
	AlifSizeT written{}, avail{}, remaining{};
	AlifOffT offset{};

	CHECK_INITIALIZED(self);

	//if (!ENTER_BUFFERED(self))
	//	return nullptr;

	if (IS_CLOSED(self)) {
		alifErr_setString(_alifExcValueError_, "write to closed file");
		goto error;
	}

	/* Fast path: the data to write can be fully buffered. */
	if (!VALID_READ_BUFFER(self) and !VALID_WRITE_BUFFER(self)) {
		self->pos = 0;
		self->rawPos = 0;
	}
	avail = ALIF_SAFE_DOWNCAST(self->bufferSize - self->pos, AlifOffT, AlifSizeT);
	if (buffer->len <= avail and buffer->len < self->bufferSize) {
		memcpy(self->buffer + self->pos, buffer->buf, buffer->len);
		if (!VALID_WRITE_BUFFER(self) or self->writePos > self->pos) {
			self->writePos = self->pos;
		}
		ADJUST_POSITION(self, self->pos + buffer->len);
		if (self->pos > self->writeEnd)
			self->writeEnd = self->pos;
		written = buffer->len;
		goto end;
	}

	/* First write the current buffer */
	res = _bufferedWriter_flushUnlocked(self);
	if (res == nullptr) {
		AlifSizeT* w = _buffered_checkBlockingError();
		if (w == nullptr)
			goto error;
		if (self->readable)
			_bufferedReader_resetBuf(self);
		memmove(self->buffer, self->buffer + self->writePos,
			ALIF_SAFE_DOWNCAST(self->writeEnd - self->writePos,
				AlifOffT, AlifSizeT));
		self->writeEnd -= self->writePos;
		self->rawPos -= self->writePos;
		self->pos -= self->writePos;
		self->writePos = 0;
		avail = ALIF_SAFE_DOWNCAST(self->bufferSize - self->writeEnd,
			AlifOffT, AlifSizeT);
		if (buffer->len <= avail) {
			/* Everything can be buffered */
			alifErr_clear();
			memcpy(self->buffer + self->writeEnd, buffer->buf, buffer->len);
			self->writeEnd += buffer->len;
			self->pos += buffer->len;
			written = buffer->len;
			goto end;
		}
		/* Buffer as much as possible. */
		memcpy(self->buffer + self->writeEnd, buffer->buf, avail);
		self->writeEnd += avail;
		self->pos += avail;
		//_set_blockingIOError("write could not complete without blocking",
		//	avail);
		goto error;
	}
	ALIF_CLEAR(res);

	offset = RAW_OFFSET(self);
	if (offset != 0) {
		//if (_buffered_rawSeek(self, -offset, 1) < 0)
		//	goto error;
		self->rawPos -= offset;
	}

	remaining = buffer->len;
	written = 0;
	while (remaining >= self->bufferSize) {
		AlifSizeT n = _bufferedWriter_rawWrite(
			self, (char*)buffer->buf + written, buffer->len - written);
		if (n == -1) {
			goto error;
		}
		else if (n == -2) {
			if (remaining > self->bufferSize) {
				memcpy(self->buffer,
					(char*)buffer->buf + written, self->bufferSize);
				self->rawPos = 0;
				ADJUST_POSITION(self, self->bufferSize);
				self->writeEnd = self->bufferSize;
				written += self->bufferSize;
				//_set_blockingIOError("write could not complete without "
				//	"blocking", written);
				goto error;
			}
			alifErr_clear();
			break;
		}
		written += n;
		remaining -= n;
		//if (alifErr_checkSignals() < 0)
		//	goto error;
	}
	if (self->readable)
		_bufferedReader_resetBuf(self);
	if (remaining > 0) {
		memcpy(self->buffer, (char*)buffer->buf + written, remaining);
		written += remaining;
	}
	self->writePos = 0;
	self->writeEnd = remaining;
	ADJUST_POSITION(self, remaining);
	self->rawPos = 0;

end:
	res = alifLong_fromSizeT(written);

error:
	//LEAVE_BUFFERED(self);
	return res;
}






#include "clinic/BufferedIO.cpp.h" // 2484


static AlifMethodDef _bufferedIOBaseMethods_[] = { // 2487
	//_IO__BUFFEREDIOBASE_DETACH_METHODDEF
	//_IO__BUFFEREDIOBASE_READ_METHODDEF
	_IO__BUFFEREDIOBASE_READ1_METHODDEF
	//_IO__BUFFEREDIOBASE_READINTO_METHODDEF
	//_IO__BUFFEREDIOBASE_READINTO1_METHODDEF
	//_IO__BUFFEREDIOBASE_WRITE_METHODDEF
	{nullptr, nullptr}
};

static AlifTypeSlot _bufferedIOBaseSlots_[] = { // 2497
	{ALIF_TP_METHODS, _bufferedIOBaseMethods_},
	{0, nullptr},
};


AlifTypeSpec _bufferedIOBaseSpec_ = { // 2504
	.name = "تبادل.قاعدة_مخزن",
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _bufferedIOBaseSlots_,
};


static AlifMethodDef _bufferedReaderMethods_[] = { // 2511
	_IO__BUFFERED_CLOSE_METHODDEF
	_IO__BUFFERED_SEEKABLE_METHODDEF
	_IO__BUFFERED_READABLE_METHODDEF
	_IO__BUFFERED_FILENO_METHODDEF

	_IO__BUFFERED_READ_METHODDEF
	_IO__BUFFERED_READ1_METHODDEF
	{nullptr, nullptr}
};


static AlifMemberDef _bufferedReaderMembers_[] = { // 2538
	{"raw", ALIF_T_OBJECT, offsetof(Buffered, raw), ALIF_READONLY},
	{"_finalizing", ALIF_T_BOOL, offsetof(Buffered, finalizing), 0},
	{"__weakListOffset__", ALIF_T_ALIFSIZET, offsetof(Buffered, weakRefList), ALIF_READONLY},
	{"__dictOffset__", ALIF_T_ALIFSIZET, offsetof(Buffered, dict), ALIF_READONLY},
	{nullptr}
};

static AlifGetSetDef _bufferedReaderGetSet_[] = {
	_IO__BUFFERED_CLOSED_GETSETDEF
	{nullptr}
};

static AlifTypeSlot _bufferedReaderSlots_[] = { // 2554
	{ALIF_TP_TRAVERSE, (void*)buffered_traverse},
	{ALIF_TP_METHODS, _bufferedReaderMethods_},
	{ALIF_TP_MEMBERS, _bufferedReaderMembers_},
	{ALIF_TP_GETSET, _bufferedReaderGetSet_},
	{ALIF_TP_INIT, (void*)_ioBufferedReader___init__},

	{0, nullptr},
};




AlifTypeSpec _bufferedReaderSpec_ = { // 2568
	.name = "تبادل.قارئ_مخزن",
	.basicsize = sizeof(Buffered),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _bufferedReaderSlots_,
};



static AlifMethodDef _bufferedWriterMethods_[] = { // 2576
	/* BufferedIOMixin methods */
	//_IO__BUFFERED_CLOSE_METHODDEF
	//_IO__BUFFERED_DETACH_METHODDEF
	_IO__BUFFERED_SEEKABLE_METHODDEF
	_IO__BUFFERED_WRITABLE_METHODDEF
	_IO__BUFFERED_FILENO_METHODDEF
	//_IO__BUFFERED_ISATTY_METHODDEF
	//_IO__BUFFERED__DEALLOC_WARN_METHODDEF

	_IO_BUFFEREDWRITER_WRITE_METHODDEF
	//_IO__BUFFERED_TRUNCATE_METHODDEF
	_IO__BUFFERED_FLUSH_METHODDEF
	//_IO__BUFFERED_SEEK_METHODDEF
	//_IO__BUFFERED_TELL_METHODDEF
	//_IO__BUFFERED___SIZEOF___METHODDEF

	//{"__reduce__", _alifIOBase_cannotPickle, METHOD_NOARGS},
	//{"__reduce_ex__", _alifIOBase_cannotPickle, METHOD_O},
	{nullptr, nullptr}
};

static AlifMemberDef _bufferedWriterMembers_[] = { // 2598
	{"raw", ALIF_T_OBJECT, offsetof(Buffered, raw), ALIF_READONLY},
	{"_finalizing", ALIF_T_BOOL, offsetof(Buffered, finalizing), 0},
	{"__weakListOffset__", ALIF_T_ALIFSIZET, offsetof(Buffered, weakRefList), ALIF_READONLY},
	{"__dictOffset__", ALIF_T_ALIFSIZET, offsetof(Buffered, dict), ALIF_READONLY},
	{nullptr}
};

static AlifGetSetDef _bufferedWriterGetSet_[] = { // 2606
	_IO__BUFFERED_CLOSED_GETSETDEF
	//_IO__BUFFERED_NAME_GETSETDEF
	//_IO__BUFFERED_MODE_GETSETDEF
	{nullptr}
};

static AlifTypeSlot _bufferedWriterSlots_[] = { // 2614
	//{ALIF_TP_DEALLOC, buffered_dealloc},
	//{ALIF_TP_REPR, buffered_repr},
	//{ALIF_TP_DOC, (void*)_ioBufferedWriter___init____Doc__},
	{ALIF_TP_TRAVERSE, (void*)buffered_traverse},
	//{ALIF_TP_CLEAR, buffered_clear},
	{ALIF_TP_METHODS, _bufferedWriterMethods_},
	{ALIF_TP_MEMBERS, _bufferedWriterMembers_},
	{ALIF_TP_GETSET, _bufferedWriterGetSet_},
	{ALIF_TP_INIT, (void*)_ioBufferedWriter___init__},
	{0, nullptr},
};

AlifTypeSpec _bufferedWriterSpec_ = { // 2627
	.name = "تبادل.كاتب_مخزن",
	.basicsize = sizeof(Buffered),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _bufferedWriterSlots_,
};
