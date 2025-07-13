#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Object.h"
#include "AlifCore_Errors.h"
#include "AlifCore_LifeCycle.h"

#include "_IOModule.h"




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




static AlifIntT buffered_traverse(Buffered* self, VisitProc visit, void* arg) { // 450
	ALIF_VISIT(ALIF_TYPE(self));
	ALIF_VISIT(self->raw);
	ALIF_VISIT(self->dict);
	return 0;
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



#include "clinic/BufferedIO.cpp.h" // 2484



static AlifTypeSlot _bufferedIOBaseSlots_[] = { // 2497
	//{ALIF_TP_METHODS, _bufferedIOBaseMethods_},
	{0, nullptr},
};


AlifTypeSpec _bufferedIOBaseSpec_ = { // 2504
	.name = "تبادل.قاعدة_مخزن",
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _bufferedIOBaseSlots_,
};


static AlifMemberDef _bufferedReaderMembers_[] = { // 2538
	{"raw", ALIF_T_OBJECT, offsetof(Buffered, raw), ALIF_READONLY},
	{"_finalizing", ALIF_T_BOOL, offsetof(Buffered, finalizing), 0},
	{"__weaklistoffset__", ALIF_T_ALIFSIZET, offsetof(Buffered, weakRefList), ALIF_READONLY},
	{"__dictoffset__", ALIF_T_ALIFSIZET, offsetof(Buffered, dict), ALIF_READONLY},
	{nullptr}
};


static AlifTypeSlot _bufferedReaderSlots_[] = { // 2554
	{ALIF_TP_TRAVERSE, buffered_traverse},
	{ALIF_TP_MEMBERS, _bufferedReaderMembers_},

	{ALIF_TP_INIT, _ioBufferedReader___init__},

	{0, nullptr},
};




AlifTypeSpec _bufferedReaderSpec_ = { // 2568
	.name = "تبادل.قارئ_مخزن",
	.basicsize = sizeof(Buffered),
	.flags = (ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_GC |
			  ALIF_TPFLAGS_IMMUTABLETYPE),
	.slots = _bufferedReaderSlots_,
};
