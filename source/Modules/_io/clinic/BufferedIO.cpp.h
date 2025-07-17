#include "AlifCore_GC.h"          
#include "AlifCore_DureRun.h"     
#include "AlifCore_Abstract.h"    
#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"    







#define _IO__BUFFERED_SEEKABLE_METHODDEF    \
    {"Seekable", (AlifCPPFunction)_io_Buffered_seekable, METHOD_NOARGS}, // 408

static AlifObject* _io_Buffered_seekableImpl(Buffered*);

static AlifObject* _io_Buffered_seekable(Buffered* self, AlifObject* ALIF_UNUSED(ignored)) { // 414
	AlifObject* return_value = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	return_value = _io_Buffered_seekableImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return return_value;
}


#define _IO__BUFFERED_READABLE_METHODDEF    \
    {"Readable", (AlifCPPFunction)_io_Buffered_readable, METHOD_NOARGS}, // 431

static AlifObject* _io_Buffered_readableImpl(Buffered*);

static AlifObject* _io_Buffered_readable(Buffered* self, AlifObject* ALIF_UNUSED(ignored)) { // 437
	AlifObject* return_value = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	return_value = _io_Buffered_readableImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return return_value;
}

// 679
#define _IO__BUFFERED_READ1_METHODDEF    \
    {"read1", ALIF_CPPFUNCTION_CAST(_io_Buffered_read1), METHOD_FASTCALL},

static AlifObject* _io_Buffered_read1Impl(Buffered*, AlifSizeT);

static AlifObject* _io_Buffered_read1(Buffered* self,
	AlifObject* const* args, AlifSizeT nargs) { // 685
	AlifObject* returnValue = nullptr;
	AlifSizeT n = -1;

	if (!_alifArg_checkPositional("read1", nargs, 0, 1)) {
		goto exit;
	}
	if (nargs < 1) {
		goto skip_optional;
	}
	{
		AlifSizeT ival = -1;
		AlifObject* iobj = alifNumber_index(args[0]);
		if (iobj != nullptr) {
			ival = alifLong_asSizeT(iobj);
			ALIF_DECREF(iobj);
		}
		if (ival == -1 and alifErr_occurred()) {
			goto exit;
		}
		n = ival;
	}
skip_optional:
	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _io_Buffered_read1Impl(self, n);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}



static AlifIntT _ioBufferedReader___init__Impl(Buffered*, AlifObject*, AlifSizeT); // 936

static AlifIntT _ioBufferedReader___init__(AlifObject* self, AlifObject* args, AlifObject* kwargs) { // 940
	AlifIntT returnValue = -1;
#define NUM_KEYWORDS 2
	static struct {
		AlifGCHead _thisNotUsed{};
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_STR(Raw), &ALIF_ID(buffersize), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

	static const char* const _keywords[] = { "raw", "bufferSize", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "BufferedReader",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2];
	AlifObject* const* fastargs;
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);
	AlifSizeT noptargs = nargs + (kwargs ? ALIFDICT_GET_SIZE(kwargs) : 0) - 1;
	AlifObject* raw{};
	AlifSizeT buffer_size = DEFAULT_BUFFER_SIZE;

	fastargs = ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(args)->item, nargs, kwargs, nullptr, &_parser, 1, 2, 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	raw = fastargs[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	{
		AlifSizeT ival = -1;
		AlifObject* iobj = alifNumber_index(fastargs[1]);
		if (iobj != nullptr) {
			ival = alifLong_asSizeT(iobj);
			ALIF_DECREF(iobj);
		}
		if (ival == -1 and alifErr_occurred()) {
			goto exit;
		}
		buffer_size = ival;
	}
skip_optional_pos:
	returnValue = _ioBufferedReader___init__Impl((Buffered*)self, raw, buffer_size);

exit:
	return returnValue;
}
