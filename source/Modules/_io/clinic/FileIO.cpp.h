#include "AlifCore_GC.h"         
#include "AlifCore_Runtime.h"    
#include "AlifCore_Abstract.h"   
#include "AlifCore_ModSupport.h" 





// 21
#define _IO_FILEIO_CLOSE_METHODDEF    \
    {"اغلق", ALIF_CPPFUNCTION_CAST(_ioFileIO_close), METHOD_METHOD|METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* _ioFileIO_closeImpl(FileIO*, AlifTypeObject*);

static AlifObject* _ioFileIO_close(FileIO* self, AlifTypeObject* cls,
	AlifObject* const* args, AlifSizeT nargs, AlifObject* kwnames) { // 27
	if (nargs or (kwnames and ALIFTUPLE_GET_SIZE(kwnames))) {
		alifErr_setString(_alifExcTypeError_, "close() takes no arguments");
		return nullptr;
	}
	return _ioFileIO_closeImpl(self, cls);
}







static AlifIntT _ioFileIO___init__Impl(FileIO*, AlifObject*,
	const char*, AlifIntT, AlifObject*); // 55

static AlifIntT _ioFileIO___init__(AlifObject* self,
	AlifObject* args, AlifObject* kwargs) { // 60
	AlifIntT return_value = -1;
#define NUM_KEYWORDS 4
	static struct {
		AlifGCHead thisNotUsed;
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS];
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_ID(File), &ALIF_ID(Mode), &ALIF_ID(CloseFD), &ALIF_ID(Opener), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)


	static const char* const _keywords[] = { "file", "mode", "closefd", "opener", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "FileIO",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[4];
	AlifObject* const* fastargs{};
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);
	AlifSizeT noptargs = nargs + (kwargs ? ALIFDICT_GET_SIZE(kwargs) : 0) - 1;
	AlifObject* nameobj{};
	const char* mode = "r";
	int closefd = 1;
	AlifObject* opener = ALIF_NONE;

	fastargs = ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(args)->item, nargs, kwargs, nullptr, &_parser, 1, 4, 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	nameobj = fastargs[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (fastargs[1]) {
		if (!ALIFUSTR_CHECK(fastargs[1])) {
			//_alifArg_badArgument("FileIO", "argument 'mode'", "str", fastargs[1]);
			goto exit;
		}
		AlifSizeT mode_length{};
		mode = alifUStr_asUTF8AndSize(fastargs[1], &mode_length);
		if (mode == nullptr) {
			goto exit;
		}
		if (strlen(mode) != (size_t)mode_length) {
			alifErr_setString(_alifExcValueError_, "embedded null character");
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (fastargs[2]) {
		closefd = alifObject_isTrue(fastargs[2]);
		if (closefd < 0) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	opener = fastargs[3];
skip_optional_pos:
	return_value = _ioFileIO___init__Impl((FileIO*)self, nameobj, mode, closefd, opener);

exit:
	return return_value;
}






// 164
#define _IO_FILEIO_READABLE_METHODDEF    \
    {"Readable", (AlifCPPFunction)_ioFileIO_readable, METHOD_NOARGS},

static AlifObject* _ioFileIO_readableImpl(FileIO*);

static AlifObject* _ioFileIO_readable(FileIO* self, AlifObject* ALIF_UNUSED(ignored)) { // 170
	return _ioFileIO_readableImpl(self);
}


// 200
#define _IO_FILEIO_SEEKABLE_METHODDEF    \
    {"Seekable", (AlifCPPFunction)_ioFileIO_seekable, METHOD_NOARGS},

static AlifObject* _ioFileIO_seekableImpl(FileIO*);

static AlifObject* _ioFileIO_seekable(FileIO* self, AlifObject* ALIF_UNUSED(ignored)) { // 206
	return _ioFileIO_seekableImpl(self);
}


// 218
#define _IO_FILEIO_READINTO_METHODDEF    \
    {"ReadInto", ALIF_CPPFUNCTION_CAST(_ioFileIO_readinto), METHOD_METHOD|METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* _ioFileIO_readintoImpl(FileIO*, AlifTypeObject*, AlifBuffer*);

static AlifObject* _ioFileIO_readinto(FileIO* self, AlifTypeObject* cls,
	AlifObject* const* args, AlifSizeT nargs, AlifObject* kwnames) { // 224
	AlifObject* returnValue = nullptr;

#define KWTUPLE (AlifObject *)&ALIF_SINGLETON(tupleEmpty)


	static const char* const _keywords[] = { "", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "ReadInto",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[1];
	AlifBuffer buffer = { nullptr, nullptr };

	args = ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser, 1, 1, 0, argsbuf);
	if (!args) {
		goto exit;
	}
	if (alifObject_getBuffer(args[0], &buffer, ALIFBUF_WRITABLE) < 0) {
		//_alifArg_badArgument("readinto", "argument 1", "read-write bytes-like object", args[0]);
		goto exit;
	}
	returnValue = _ioFileIO_readintoImpl(self, cls, &buffer);

exit:
	/* Cleanup for buffer */
	if (buffer.obj) {
		alifBuffer_release(&buffer);
	}

	return returnValue;
}


// 272
#define _IO_FILEIO_READALL_METHODDEF    \
    {"ReadAll", (AlifCPPFunction)_ioFileIO_readall, METHOD_NOARGS},

static AlifObject* _ioFileIO_readallImpl(FileIO*);

static AlifObject* _ioFileIO_readall(FileIO* self, AlifObject* ALIF_UNUSED(ignored)) { // 278
	return _ioFileIO_readallImpl(self);
}







// 516
#define _IO_FILEIO_ISATTY_METHODDEF    \
    {"isatty", (AlifCPPFunction)_ioFileIO_isAtty, METHOD_NOARGS},

static AlifObject* _ioFileIO_isAttyImpl(FileIO*);

static AlifObject* _ioFileIO_isAtty(FileIO* self, AlifObject* ALIF_UNUSED(ignored)) { // 522
	return _ioFileIO_isAttyImpl(self);
}
