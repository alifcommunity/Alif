
#include "AlifCore_Abstract.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"







// 271
#if defined(_IO__TEXTIOBASE_ERRORS_GETSETDEF)
#  undef _IO__TEXTIOBASE_ERRORS_GETSETDEF
#  define _IO__TEXTIOBASE_ERRORS_GETSETDEF {"Errors", (Getter)_io_TextIOBase_errorsGet, (Setter)_io_TextIOBase_errorsSet},
#else
#  define _IO__TEXTIOBASE_ERRORS_GETSETDEF {"Errors", (Getter)_io_TextIOBase_errorsGet, nullptr},
#endif

static AlifObject* _io_TextIOBase_errorsGetImpl(AlifObject*);

static AlifObject* _io_TextIOBase_errorsGet(AlifObject* self,
	void* ALIF_UNUSED(context)) { // 281
	return _io_TextIOBase_errorsGetImpl(self);
}



// 300
static AlifIntT _ioIncrementalNewlineDecoder___init__Impl(NLDecoderObject*,
	AlifObject*, AlifIntT, AlifObject*);

static AlifIntT _ioIncrementalNewlineDecoder___init__(AlifObject* self,
	AlifObject* args, AlifObject* kwargs) { // 305
	AlifIntT return_value = -1;
#define NUM_KEYWORDS 3
	static struct {
		AlifGCHead _thisNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_ID(Decoder), &ALIF_ID(Translate), &ALIF_ID(Errors), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

	static const char* const _keywords[] = { "decoder", "translate", "errors", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "IncrementalNewlineDecoder",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[3];
	AlifObject* const* fastargs;
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);
	AlifSizeT noptargs = nargs + (kwargs ? ALIFDICT_GET_SIZE(kwargs) : 0) - 2;
	AlifObject* decoder{};
	AlifIntT translate{};
	AlifObject* errors = nullptr;

	fastargs = ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(args)->item, nargs, kwargs, nullptr, &_parser, 2, 3, 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	decoder = fastargs[0];
	translate = alifObject_isTrue(fastargs[1]);
	if (translate < 0) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	errors = fastargs[2];
skip_optional_pos:
	return_value = _ioIncrementalNewlineDecoder___init__Impl((NLDecoderObject*)self, decoder, translate, errors);

exit:
	return return_value;
}







static AlifIntT _ioTextIOWrapper___init__Impl(TextIO*, AlifObject*,
	const char*, AlifObject*, const char*, AlifIntT, AlifIntT); // 503

static AlifIntT _ioTextIOWrapper___init__(AlifObject* self,
	AlifObject* args, AlifObject* kwargs) { // 509
	AlifIntT return_value = -1;
#define NUM_KEYWORDS 6
	static struct {
		AlifGCHead thisNotUsed;
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_ID(Buffer), &ALIF_ID(Encoding), &ALIF_ID(Errors), &ALIF_ID(Newline), &ALIF_ID(LineBuffering), &ALIF_ID(WriteThrough), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)


	static const char* const _keywords[] = { "Buffer", "Encoding", "Errors", "Newline", "LineBuffering", "write_through", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "TextIOWrapper",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[6];
	AlifObject* const* fastargs;
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);
	AlifSizeT noptargs = nargs + (kwargs ? ALIFDICT_GET_SIZE(kwargs) : 0) - 1;
	AlifObject* buffer;
	const char* encoding = nullptr;
	AlifObject* errors = ALIF_NONE;
	const char* newline = nullptr;
	AlifIntT line_buffering = 0;
	AlifIntT write_through = 0;

	fastargs = ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(args)->item, nargs, kwargs, nullptr, &_parser, 1, 6, 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	buffer = fastargs[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (fastargs[1]) {
		if (fastargs[1] == ALIF_NONE) {
			encoding = nullptr;
		}
		else if (ALIFUSTR_CHECK(fastargs[1])) {
			AlifSizeT encoding_length{};
			encoding = alifUStr_asUTF8AndSize(fastargs[1], &encoding_length);
			if (encoding == nullptr) {
				goto exit;
			}
			if (strlen(encoding) != (size_t)encoding_length) {
				alifErr_setString(_alifExcValueError_, "embedded null character");
				goto exit;
			}
		}
		else {
			//_alifArg_badArgument("TextIOWrapper", "argument 'encoding'", "str or None", fastargs[1]);
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (fastargs[2]) {
		errors = fastargs[2];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (fastargs[3]) {
		if (fastargs[3] == ALIF_NONE) {
			newline = nullptr;
		}
		else if (ALIFUSTR_CHECK(fastargs[3])) {
			AlifSizeT newline_length{};
			newline = alifUStr_asUTF8AndSize(fastargs[3], &newline_length);
			if (newline == nullptr) {
				goto exit;
			}
			if (strlen(newline) != (size_t)newline_length) {
				alifErr_setString(_alifExcValueError_, "embedded null character");
				goto exit;
			}
		}
		else {
			//_alifArg_badArgument("TextIOWrapper", "argument 'newline'", "str or None", fastargs[3]);
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (fastargs[4]) {
		line_buffering = alifObject_isTrue(fastargs[4]);
		if (line_buffering < 0) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	write_through = alifObject_isTrue(fastargs[5]);
	if (write_through < 0) {
		goto exit;
	}
skip_optional_pos:
	return_value = _ioTextIOWrapper___init__Impl((TextIO*)self, buffer, encoding, errors, newline, line_buffering, write_through);

exit:
	return return_value;
}







#define _IO_TEXTIOWRAPPER_WRITE_METHODDEF    \
    {"اكتب", (AlifCPPFunction)_ioTextIOWrapper_write, METHOD_O},

static AlifObject* _ioTextIOWrapper_writeImpl(TextIO* self, AlifObject* text);

static AlifObject* _ioTextIOWrapper_write(TextIO* self, AlifObject* arg) { // 759
	AlifObject* returnValue = nullptr;
	AlifObject* text{};

	if (!ALIFUSTR_CHECK(arg)) {
		//_alifArg_badArgument("write", "argument", "str", arg);
		goto exit;
	}
	text = arg;
	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _ioTextIOWrapper_writeImpl(self, text);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}

// 783
#define _IO_TEXTIOWRAPPER_READ_METHODDEF    \
    {"اقرا", ALIF_CPPFUNCTION_CAST(_ioTextIOWrapper_read), METHOD_FASTCALL},

static AlifObject* _ioTextIOWrapper_readImpl(TextIO*, AlifSizeT);

static AlifObject* _ioTextIOWrapper_read(TextIO* self,
	AlifObject* const* args, AlifSizeT nargs) { // 789
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
	returnValue = _ioTextIOWrapper_readImpl(self, n);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}



// 818
#define _IO_TEXTIOWRAPPER_READLINE_METHODDEF    \
    {"اقرا_سطر", ALIF_CPPFUNCTION_CAST(_ioTextIOWrapper_readline), METHOD_FASTCALL},

static AlifObject* _ioTextIOWrapper_readlineImpl(TextIO*, AlifSizeT);

static AlifObject* _ioTextIOWrapper_readline(TextIO* self, AlifObject* const* args, AlifSizeT nargs) { // 824
	AlifObject* returnValue = nullptr;
	AlifSizeT size = -1;

	if (!_ALIFARG_CHECKPOSITIONAL("اقرا_سطر", nargs, 0, 1)) {
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
		size = ival;
	}
skip_optional:
	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _ioTextIOWrapper_readlineImpl(self, size);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}

// 978
#define _IO_TEXTIOWRAPPER_FILENO_METHODDEF    \
    {"Fileno", (AlifCPPFunction)_ioTextIOWrapper_fileno, METHOD_NOARGS},

static AlifObject* _ioTextIOWrapper_filenoImpl(TextIO*);

static AlifObject* _ioTextIOWrapper_fileno(TextIO* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 984
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _ioTextIOWrapper_filenoImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}

// 1093
#define _IO_TEXTIOWRAPPER_FLUSH_METHODDEF    \
    {"Flush", (AlifCPPFunction)_ioTextIOWrapper_flush, METHOD_NOARGS},

static AlifObject* _ioTextIOWrapper_flushImpl(TextIO*);

static AlifObject* _ioTextIOWrapper_flush(TextIO* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 1099
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _ioTextIOWrapper_flushImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}


// 1116
#define _IO_TEXTIOWRAPPER_CLOSE_METHODDEF    \
    {"اغلق", (AlifCPPFunction)_ioTextIOWrapper_close, METHOD_NOARGS},

static AlifObject* _ioTextIOWrapper_closeImpl(TextIO*);

static AlifObject* _ioTextIOWrapper_close(TextIO* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 1122
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _ioTextIOWrapper_closeImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}




// 1220
#if defined(_IO_TEXTIOWRAPPER_ERRORS_GETSETDEF)
#  undef _IO_TEXTIOWRAPPER_ERRORS_GETSETDEF
#  define _IO_TEXTIOWRAPPER_ERRORS_GETSETDEF {"Errors", (Getter)_ioTextIOWrapper_errorsGet, (Setter)_ioTextIOWrapper_errorsSet},
#else
#  define _IO_TEXTIOWRAPPER_ERRORS_GETSETDEF {"Errors", (Getter)_ioTextIOWrapper_errorsGet, nullptr},
#endif

static AlifObject* _ioTextIOWrapper_errorsGetImpl(TextIO*);

static AlifObject* _ioTextIOWrapper_errorsGet(TextIO* self, void* ALIF_UNUSED(context)) { // 1230
	AlifObject* returnValue = nullptr;

	ALIF_BEGIN_CRITICAL_SECTION(self);
	returnValue = _ioTextIOWrapper_errorsGetImpl(self);
	ALIF_END_CRITICAL_SECTION();

	return returnValue;
}
