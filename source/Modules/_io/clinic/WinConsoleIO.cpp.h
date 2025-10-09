#include "AlifCore_ModSupport.h"




static AlifIntT _io_windowsConsoleIO__init__Impl(WinConsoleIO*, AlifObject*,
	const char*, AlifIntT, AlifObject*); // 53

static AlifIntT _io_windowsConsoleIO__init__(AlifObject* self, AlifObject* args, AlifObject* kwargs) { // 58
	AlifIntT return_value = -1;
#define NUM_KEYWORDS 4
	static struct {
		AlifGCHead thisNotUsed;
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_ID(File), &ALIF_ID(Mode), &ALIF_ID(CloseFD), &ALIF_ID(Opener), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

	static const char* const _keywords[] = { "file", "mode", "closefd", "opener", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "_WindowsConsoleIO",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[4]{};
	AlifObject* const* fastargs{};
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);
	AlifSizeT noptargs = nargs + (kwargs ? ALIFDICT_GET_SIZE(kwargs) : 0) - 1;
	AlifObject* nameobj{};
	const char* mode = "r";
	AlifIntT closefd = 1;
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
			//_alifArg_badArgument("_WindowsConsoleIO", "argument 'mode'", "str", fastargs[1]);
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
	return_value = _io_windowsConsoleIO__init__Impl((WinConsoleIO*)self, nameobj, mode, closefd, opener);

exit:
	return return_value;
}


#if defined(HAVE_WINDOWS_CONSOLE_IO) // 141

#define _IO__WINDOWSCONSOLEIO_FILENO_METHODDEF    \
    {"Fileno", (AlifCPPFunction)_io_WindowsConsoleIO_fileno, METHOD_NOARGS},

static AlifObject* _io_WindowsConsoleIO_filenoImpl(WinConsoleIO*);

static AlifObject* _io_WindowsConsoleIO_fileno(WinConsoleIO* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 155
	return _io_WindowsConsoleIO_filenoImpl(self);
}

#endif /* defined(HAVE_WINDOWS_CONSOLE_IO) */


#if defined(HAVE_WINDOWS_CONSOLE_IO) // 163

#define _IO__WINDOWSCONSOLEIO_READABLE_METHODDEF    \
    {"Readable", (AlifCPPFunction)_io_windowsConsoleIO_readable, METHOD_NOARGS},

static AlifObject* _io_windowsConsoleIO_readableImpl(WinConsoleIO*);

static AlifObject* _io_windowsConsoleIO_readable(WinConsoleIO* self, AlifObject* ALIF_UNUSED(ignored)) {
	return _io_windowsConsoleIO_readableImpl(self);
}

#endif  // 183

// 193
#define _IO__WINDOWSCONSOLEIO_WRITABLE_METHODDEF    \
    {"Writable", (AlifCPPFunction)_io_WindowsConsoleIO_writable, METHOD_NOARGS},

static AlifObject* _io_WindowsConsoleIO_writableImpl(WinConsoleIO*);

static AlifObject* _io_WindowsConsoleIO_writable(WinConsoleIO* _self,
	AlifObject* ALIF_UNUSED(ignored)) { // 199
	return _io_WindowsConsoleIO_writableImpl(_self);
}




#if defined(HAVE_WINDOWS_CONSOLE_IO) // 345

#define _IO__WINDOWSCONSOLEIO_WRITE_METHODDEF    \
    {"Write", ALIF_CPPFUNCTION_CAST(_io_WindowsConsoleIO_write), METHOD_METHOD|METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* _io_WindowsConsoleIO_writeImpl(WinConsoleIO*, AlifTypeObject*, AlifBuffer*);

static AlifObject* _io_WindowsConsoleIO_write(WinConsoleIO* self,
	AlifTypeObject* cls, AlifObject* const* args, AlifSizeT nargs, AlifObject* kwnames) { // 363
	AlifObject* returnValue = nullptr;
#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#  define KWTUPLE (AlifObject *)&_alifSingleton_(tupleEmpty)
#else
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "write",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[1];
	AlifBuffer b = { nullptr, nullptr };

	args = ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser, 1, 1, 0, argsbuf);
	if (!args) {
		goto exit;
	}
	if (alifObject_getBuffer(args[0], &b, ALIFBUF_SIMPLE) != 0) {
		goto exit;
	}
	returnValue = _io_WindowsConsoleIO_writeImpl(self, cls, &b);

exit:
	/* Cleanup for b */
	if (b.obj) {
		alifBuffer_release(&b);
	}

	return returnValue;
}

#endif /* defined(HAVE_WINDOWS_CONSOLE_IO) */




// 411
#define _IO__WINDOWSCONSOLEIO_ISATTY_METHODDEF    \
    {"isatty", (AlifCPPFunction)_io_windowsConsoleIOIsAtty, METHOD_NOARGS},

static AlifObject* _io_windowsConsoleIOIsAttyImpl(WinConsoleIO*);

static AlifObject* _io_windowsConsoleIOIsAtty(WinConsoleIO* _self, AlifObject* ALIF_UNUSED(ignored)) { // 417
	return _io_windowsConsoleIOIsAttyImpl(_self);
}
