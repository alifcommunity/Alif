#include "AlifCore_Abstract.h"
#include "AlifCore_ModSupport.h"











// 178
#define _IO__IOBASE_CLOSE_METHODDEF    \
    {"اغلق", (AlifCPPFunction)_io_IOBase_close, METHOD_NOARGS},

static AlifObject* _io_IOBase_closeImpl(AlifObject*);

static AlifObject* _io_IOBase_close(AlifObject* self, AlifObject* ALIF_UNUSED(ignored)) { // 184
	return _io_IOBase_closeImpl(self);
}

// 199
#define _IO__IOBASE_SEEKABLE_METHODDEF    \
    {"Seekable", (AlifCPPFunction)_io_IOBase_seekable, METHOD_NOARGS},

static AlifObject* _io_IOBase_seekableImpl(AlifObject*);

static AlifObject* _io_IOBase_seekable(AlifObject* _self, AlifObject* ALIF_UNUSED(ignored)) { // 205
	return _io_IOBase_seekableImpl(_self);
}


// 219
#define _IO__IOBASE_READABLE_METHODDEF    \
    {"Readable", (AlifCPPFunction)_io_IOBase_readable, METHOD_NOARGS},

static AlifObject* _io_IOBase_readableImpl(AlifObject*);

static AlifObject* _io_IOBase_readable(AlifObject* _self, AlifObject* ALIF_UNUSED(ignored)) { // 225
	return _io_IOBase_readableImpl(_self);
}

// 239
#define _IO__IOBASE_WRITABLE_METHODDEF    \
    {"Writable", (AlifCPPFunction)_io_IOBase_writable, METHOD_NOARGS},

static AlifObject* _io_IOBase_writableImpl(AlifObject*);

static AlifObject* _io_IOBase_writable(AlifObject* self, AlifObject* ALIF_UNUSED(ignored)) { // 245
	return _io_IOBase_writableImpl(self);
}






// 307
#define _IO__IOBASE_READLINE_METHODDEF    \
    {"اقرا_سطر", ALIF_CPPFUNCTION_CAST(_io_IOBase_readline), METHOD_FASTCALL},

static AlifObject* _io_IOBase_readlineImpl(AlifObject*, AlifSizeT);

static AlifObject* _io_IOBase_readline(AlifObject* self,
	AlifObject* const* args, AlifSizeT nargs) { // 313
	AlifObject* returnValue = nullptr;
	AlifSizeT limit = -1;

	if (!_ALIFARG_CHECKPOSITIONAL("اقرا_سطر", nargs, 0, 1)) {
		goto exit;
	}
	if (nargs < 1) {
		goto skip_optional;
	}
	if (!_alifConvertOptional_toSizeT(args[0], &limit)) {
		goto exit;
	}
skip_optional:
	returnValue = _io_IOBase_readlineImpl(self, limit);

exit:
	return returnValue;
}
