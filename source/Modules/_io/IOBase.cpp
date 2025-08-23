#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"     

#include <stddef.h>               // offsetof()
#include "_IOModule.h"



class IOBase { // 31
public:
	ALIFOBJECT_HEAD;
	AlifObject* dict{};
	AlifObject* weakRefList{};
};


static AlifIntT ioBase_isClosed(AlifObject* self) { // 76
	return alifObject_hasAttrWithError(self, &ALIF_ID(__IOBaseClosed));
}

static AlifIntT iobase_checkClosed(AlifObject* self) { // 191
	AlifObject* res{};
	AlifIntT closed{};
	closed = alifObject_getOptionalAttr(self, &ALIF_ID(Closed), &res);
	if (closed > 0) {
		closed = alifObject_isTrue(res);
		ALIF_DECREF(res);
		if (closed > 0) {
			alifErr_setString(_alifExcValueError_, "عملية تبادل على ملف مغلق");
			return -1;
		}
	}
	return closed;
}

AlifObject* _alifIOBase_checkClosed(AlifObject* self, AlifObject* args) { // 210
	if (iobase_checkClosed(self)) {
		return nullptr;
	}
	if (args == ALIF_TRUE) {
		return ALIF_NONE;
	}
	return ALIF_NONE;
}

static AlifObject* _io_IOBase_closeImpl(AlifObject* self) { // 263
	AlifIntT rc1{}, rc2{}, closed = ioBase_isClosed(self);

	if (closed < 0) {
		return nullptr;
	}
	if (closed) {
		return ALIF_NONE;
	}

	//rc1 = _alifFile_flush(self);
	AlifObject* exc = alifErr_getRaisedException();
	rc2 = alifObject_setAttr(self, &ALIF_ID(__IOBaseClosed), ALIF_TRUE);
	_alifErr_chainExceptions1(exc);
	if (rc1 < 0 or rc2 < 0) {
		return nullptr;
	}

	return ALIF_NONE;
}


static AlifIntT ioBase_traverse(IOBase* self, VisitProc visit, void* arg) { // 344
	ALIF_VISIT(ALIF_TYPE(self));
	ALIF_VISIT(self->dict);
	return 0;
}




AlifObject* _alifIOBase_checkReadable(AlifIOState* state,
	AlifObject* self, AlifObject* args) { // 437
	AlifObject* res = alifObject_callMethodNoArgs(self, &ALIF_ID(Readable));
	if (res == nullptr)
		return nullptr;
	if (res != ALIF_TRUE) {
		ALIF_CLEAR(res);
		//iobase_unsupported(state, "File or stream is not readable.");
		return nullptr;
	}
	if (args == ALIF_TRUE) {
		ALIF_DECREF(res);
	}
	return res;
}


static AlifObject* _io_IOBase_writableImpl(AlifObject* self) { // 462
	ALIF_RETURN_FALSE;
}


static AlifObject* _io_IOBase_readlineImpl(AlifObject* self, AlifSizeT limit) { // 559
	AlifObject* peek{}, * buffer{}, * result{};
	AlifSizeT oldSize = -1;

	if (alifObject_getOptionalAttr(self, &ALIF_ID(Peek), &peek) < 0) {
		return nullptr;
	}

	buffer = alifByteArray_fromStringAndSize(nullptr, 0);
	if (buffer == nullptr) {
		ALIF_XDECREF(peek);
		return nullptr;
	}

	while (limit < 0 or ALIFBYTEARRAY_GET_SIZE(buffer) < limit) {
		AlifSizeT nreadahead = 1;
		AlifObject* b{};

		if (peek != nullptr) {
			AlifObject* readahead = alifObject_callOneArg(peek, _alifLong_getOne());
			if (readahead == nullptr) {
				if (_alifIO_trapEintr()) {
					continue;
				}
				goto fail;
			}
			if (!ALIFBYTES_CHECK(readahead)) {
				//alifErr_format(_alifExcOSError_,
				//	"peek() should have returned a bytes object, "
				//	"not '%.200s'", ALIF_TYPE(readahead)->name);
				ALIF_DECREF(readahead);
				goto fail;
			}
			if (ALIFBYTES_GET_SIZE(readahead) > 0) {
				AlifSizeT n = 0;
				const char* buf = ALIFBYTES_AS_STRING(readahead);
				if (limit >= 0) {
					do {
						if (n >= ALIFBYTES_GET_SIZE(readahead) || n >= limit)
							break;
						if (buf[n++] == '\n')
							break;
					} while (1);
				}
				else {
					do {
						if (n >= ALIFBYTES_GET_SIZE(readahead))
							break;
						if (buf[n++] == '\n')
							break;
					} while (1);
				}
				nreadahead = n;
			}
			ALIF_DECREF(readahead);
		}

		b = _alifObject_callMethod(self, &ALIF_STR(Read), "n", nreadahead);
		if (b == nullptr) {
			if (_alifIO_trapEintr()) {
				continue;
			}
			goto fail;
		}
		if (!ALIFBYTES_CHECK(b)) {
			//alifErr_format(_alifExcOSError_,
			//	"read() should have returned a bytes object, "
			//	"not '%.200s'", ALIF_TYPE(b)->name);
			ALIF_DECREF(b);
			goto fail;
		}
		if (ALIFBYTES_GET_SIZE(b) == 0) {
			ALIF_DECREF(b);
			break;
		}

		oldSize = ALIFBYTEARRAY_GET_SIZE(buffer);
		if (alifByteArray_resize(buffer, oldSize + ALIFBYTES_GET_SIZE(b)) < 0) {
			ALIF_DECREF(b);
			goto fail;
		}
		memcpy(ALIFBYTEARRAY_AS_STRING(buffer) + oldSize,
			ALIFBYTES_AS_STRING(b), ALIFBYTES_GET_SIZE(b));

		ALIF_DECREF(b);

		if (ALIFBYTEARRAY_AS_STRING(buffer)[ALIFBYTEARRAY_GET_SIZE(buffer) - 1] == '\n')
			break;
	}

	result = alifBytes_fromStringAndSize(ALIFBYTEARRAY_AS_STRING(buffer),
		ALIFBYTEARRAY_GET_SIZE(buffer));
	ALIF_XDECREF(peek);
	ALIF_DECREF(buffer);
	return result;
fail:
	ALIF_XDECREF(peek);
	ALIF_DECREF(buffer);
	return nullptr;
}



#include "clinic/IOBase.cpp.h"

static AlifMethodDef _ioBaseMethods_[] = { // 824
	_IO__IOBASE_CLOSE_METHODDEF

	_IO__IOBASE_READLINE_METHODDEF
	//_IO__IOBASE_READLINES_METHODDEF
	_IO__IOBASE_WRITABLE_METHODDEF
	{nullptr, nullptr}
};

static AlifGetSetDef _ioBaseGetSet_[] = { // 853
	{"__dict__", alifObject_genericGetDict, nullptr, nullptr},
	{nullptr},
};

static AlifMemberDef _ioBaseMembers_[] = { // 859
	{"__weakListOffset__", ALIF_T_ALIFSIZET, offsetof(IOBase, weakRefList), ALIF_READONLY},
	{"__dictOffset__", ALIF_T_ALIFSIZET, offsetof(IOBase, dict), ALIF_READONLY},
	{nullptr},
};




static AlifTypeSlot _ioBaseSlots_[] = { // 866
	{ALIF_TP_TRAVERSE, (void*)ioBase_traverse},
	{ALIF_TP_METHODS, _ioBaseMethods_},
	{ALIF_TP_MEMBERS, _ioBaseMembers_},
	{ALIF_TP_GETSET, _ioBaseGetSet_},
	{0, nullptr},
};
AlifTypeSpec _ioBaseSpec_ = { // 880
	.name = "تبادل.قاعدة",
	.basicsize = sizeof(IOBase),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _ioBaseSlots_,
};


static AlifMethodDef _rawIOBaseMethods_[] = { // 1021
	{nullptr, nullptr}
};

static AlifTypeSlot _rawIOBaseSlots_[] = { // 1029
	{ALIF_TP_METHODS, _rawIOBaseMethods_},
	{0, nullptr},
};


AlifTypeSpec _rawIOBaseSpec_ = { // 1036
	.name = "تبادل.خام",
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _rawIOBaseSlots_,
};
