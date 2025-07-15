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






static AlifIntT ioBase_traverse(IOBase* self, VisitProc visit, void* arg) { // 344
	ALIF_VISIT(ALIF_TYPE(self));
	ALIF_VISIT(self->dict);
	return 0;
}




AlifObject* _alifIOBase_checkReadable(AlifIOState* state,
	AlifObject* self, AlifObject* args) { // 437
	AlifObject* res = alifObject_callMethodNoArgs(self, &ALIF_ID(readable));
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
	_IO__IOBASE_READLINE_METHODDEF
	//_IO__IOBASE_READLINES_METHODDEF
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
	{ALIF_TP_TRAVERSE, ioBase_traverse},
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
