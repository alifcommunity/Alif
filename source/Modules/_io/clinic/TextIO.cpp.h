
#include "AlifCore_Abstract.h"
#include "AlifCore_CriticalSection.h"
#include "AlifCore_ModSupport.h"





















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


	static const char* const _keywords[] = { "buffer", "encoding", "errors", "newline", "line_buffering", "write_through", nullptr };
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

//static AlifObject* _ioTextIOWrapper_writeImpl(TextIO* self, AlifObject* text);

static AlifObject* _ioTextIOWrapper_write(TextIO* self, AlifObject* arg) { // 759
	AlifObject* returnValue = nullptr;
	AlifObject* text{};

	if (!ALIFUSTR_CHECK(arg)) {
		//_alifArg_badArgument("write", "argument", "str", arg);
		goto exit;
	}
	text = arg;
	ALIF_BEGIN_CRITICAL_SECTION(self);
	//returnValue = _ioTextIOWrapper_writeImpl(self, text);
	ALIF_END_CRITICAL_SECTION();

exit:
	return returnValue;
}
