#include "AlifCore_GC.h"          
#include "AlifCore_Runtime.h"     
#include "AlifCore_Abstract.h"    
#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"    






// 181
#define _IO__BUFFEREDIOBASE_READ1_METHODDEF    \
    {"Read1", ALIF_CPPFUNCTION_CAST(_io_BufferedIOBase_read1), METHOD_METHOD|METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* _io_BufferedIOBase_read1Impl(AlifObject*, AlifTypeObject*,
	AlifIntT ALIF_UNUSED(size));

static AlifObject* _io_BufferedIOBase_read1(AlifObject* _self,
	AlifTypeObject* _cls, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 188
	AlifObject* returnValue = nullptr;
#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#  define KWTUPLE (AlifObject *)&ALIF_SINGLETON(tupleEmpty)
#else
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "read1",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[1];
	AlifIntT size = -1;

	_args = ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &_parser, 0, 1, 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	if (_nargs < 1) {
		goto skip_optional_posonly;
	}
	size = alifLong_asInt(_args[0]);
	if (size == -1 and alifErr_occurred()) {
		goto exit;
	}
skip_optional_posonly:
	returnValue = _io_BufferedIOBase_read1Impl(_self, _cls, size);

exit:
	return returnValue;
}




// 339
#define _IO__BUFFERED_CLOSED_GETSETDEF \
	{"Closed", (Getter)_io_Buffered_closedGet, nullptr},

static AlifObject* _io_Buffered_closedGetImpl(Buffered*);

static AlifObject* _io_Buffered_closedGet(Buffered* self, void* ALIF_UNUSED(context)) { // 345
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _io_Buffered_closedGetImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}


// 362
#define _IO__BUFFERED_CLOSE_METHODDEF    \
    {"اغلق", (AlifCPPFunction)_io_Buffered_close, METHOD_NOARGS},

static AlifObject* _io_Buffered_closeImpl(Buffered*);

static AlifObject* _io_Buffered_close(Buffered* self, AlifObject* ALIF_UNUSED(ignored)) { // 368
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _io_Buffered_closeImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}


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


#define _IO__BUFFERED_WRITABLE_METHODDEF    \
    {"Writable", (AlifCPPFunction)_io_Buffered_writable, METHOD_NOARGS}, // 454

static AlifObject* _io_Buffered_writableImpl(Buffered*);

static AlifObject* _io_Buffered_writable(Buffered* _self, AlifObject* ALIF_UNUSED(ignored)) { // 460
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(_self);
	returnValue = _io_Buffered_writableImpl(_self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}

// 515
#define _IO__BUFFERED_FILENO_METHODDEF    \
    {"Fileno", (AlifCPPFunction)_io_Buffered_fileno, METHOD_NOARGS},

static AlifObject* _io_Buffered_filenoImpl(Buffered*);

static AlifObject* _io_Buffered_fileno(Buffered* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 521
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _io_Buffered_filenoImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}


// 577
#define _IO__BUFFERED_FLUSH_METHODDEF    \
    {"Flush", (AlifCPPFunction)_io_Buffered_flush, METHOD_NOARGS},

static AlifObject* _io_Buffered_flushImpl(Buffered*);

static AlifObject* _io_Buffered_flush(Buffered* self, AlifObject* ALIF_UNUSED(ignored)) { // 583
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _io_Buffered_flushImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}



// 679
#define _IO__BUFFERED_READ1_METHODDEF    \
    {"Read1", ALIF_CPPFUNCTION_CAST(_io_Buffered_read1), METHOD_FASTCALL},

static AlifObject* _io_Buffered_read1Impl(Buffered*, AlifSizeT);

static AlifObject* _io_Buffered_read1(Buffered* self,
	AlifObject* const* args, AlifSizeT nargs) { // 685
	AlifObject* returnValue = nullptr;
	AlifSizeT n = -1;

	if (!_alifArg_checkPositional("Read1", nargs, 0, 1)) {
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


// 644
#define _IO__BUFFERED_READ_METHODDEF    \
    {"اقرا", ALIF_CPPFUNCTION_CAST(_io_Buffered_read), METHOD_FASTCALL},

static AlifObject* _io_Buffered_readImpl(Buffered*, AlifSizeT);

static AlifObject* _io_Buffered_read(Buffered* self, AlifObject* const* args, AlifSizeT nargs) { // 650
	AlifObject* returnValue = nullptr;
	AlifSizeT n = -1;

	if (!_ALIFARG_CHECKPOSITIONAL("اقرا", nargs, 0, 1)) {
		goto exit;
	}
	if (nargs < 1) {
		goto skip_optional;
	}
	if (!_alifConvertOptional_toSizeT(args[0], &n)) {
		goto exit;
	}
skip_optional:
	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _io_Buffered_readImpl(self, n);
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




static AlifIntT _ioBufferedWriter___init__Impl(Buffered*, AlifObject*, AlifSizeT); // 1013

static AlifIntT _ioBufferedWriter___init__(AlifObject* _self,
	AlifObject* _args, AlifObject* _kwargs) {
	AlifIntT return_value = -1;
#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)

#define NUM_KEYWORDS 2
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_STR(Raw), &ALIF_ID(bufferSize), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "raw", "buffer_size", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "BufferedWriter",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2];
	AlifObject* const* fastargs;
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(_args);
	AlifSizeT noptargs = nargs + (_kwargs ? ALIFDICT_GET_SIZE(_kwargs) : 0) - 1;
	AlifObject* raw{};
	AlifSizeT buffer_size = DEFAULT_BUFFER_SIZE;

	fastargs = ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(_args)->item, nargs, _kwargs, nullptr, &_parser, 1, 2, 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	raw = fastargs[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	{
		AlifSizeT ival = -1;
		AlifObject* iobj = _alifNumber_index(fastargs[1]);
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
	return_value = _ioBufferedWriter___init__Impl((Buffered*)_self, raw, buffer_size);

exit:
	return return_value;
}


// 1085
#define _IO_BUFFEREDWRITER_WRITE_METHODDEF    \
    {"Write", (AlifCPPFunction)_ioBufferedWriter_write, METHOD_O},

static AlifObject* _ioBufferedWriter_writeImpl(Buffered*, AlifBuffer*);

static AlifObject* _ioBufferedWriter_write(Buffered* self, AlifObject* arg) { // 1091
	AlifObject* returnValue = nullptr;
	AlifBuffer buffer = { nullptr, nullptr };

	if (alifObject_getBuffer(arg, &buffer, ALIFBUF_SIMPLE) != 0) {
		goto exit;
	}
	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _ioBufferedWriter_writeImpl(self, &buffer);
	ALIF_END_CRITICAL_SECTION();

exit:
	/* Cleanup for buffer */
	if (buffer.obj) {
		alifBuffer_release(&buffer);
	}

	return returnValue;
}
