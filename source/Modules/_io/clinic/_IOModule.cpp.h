

#include "AlifCore_ModSupport.h"






// 130
#define _IO_OPEN_METHODDEF    \
    {"فتح", ALIF_CPPFUNCTION_CAST(_io_open), METHOD_FASTCALL|METHOD_KEYWORDS}

static AlifObject* _io_openImpl(AlifObject*, AlifObject*, const char*, AlifIntT,
	const char*, const char*, const char*, AlifIntT, AlifObject*); // 133

static AlifObject* _io_open(AlifObject* module, AlifObject* const* args,
	AlifSizeT nargs, AlifObject* kwnames) { // 138
	AlifObject* returnValue{};

#define NUM_KEYWORDS 8
	static struct {
		AlifGCHead _thisNotUsed;
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS];
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_ID(File), &ALIF_ID(Mode), &ALIF_ID(Buffering), &ALIF_ID(Encoding), &ALIF_ID(Errors), &ALIF_ID(Newline), &ALIF_ID(CloseFD), &ALIF_ID(Opener), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

	static const char* const _keywords[] = { "file", "mode", "buffering", "encoding", "errors", "newline", "closefd", "opener", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "فتح",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[8]{};
	AlifSizeT noptargs = nargs + (kwnames ? ALIFTUPLE_GET_SIZE(kwnames) : 0) - 1;
	AlifObject* file{};
	const char* mode = "r";
	int buffering = -1;
	const char* encoding{};
	const char* errors{};
	const char* newline{};
	AlifIntT closefd = 1;
	AlifObject* opener = ALIF_NONE;

	args = ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser, 1, 8, 0, argsbuf);
	if (!args) {
		goto exit;
	}
	file = args[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (args[1]) {
		if (!ALIFUSTR_CHECK(args[1])) {
			//_alifArg_badArgument("open", "argument 'mode'", "str", args[1]);
			goto exit;
		}
		AlifSizeT modeLength{};
		mode = alifUStr_asUTF8AndSize(args[1], &modeLength);
		if (mode == nullptr) {
			goto exit;
		}
		if (strlen(mode) != (AlifUSizeT)modeLength) {
			alifErr_setString(_alifExcValueError_, "embedded null character");
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (args[2]) {
		buffering = alifLong_asInt(args[2]);
		if (buffering == -1 and alifErr_occurred()) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (args[3]) {
		if (args[3] == ALIF_NONE) {
			encoding = nullptr;
		}
		else if (ALIFUSTR_CHECK(args[3])) {
			AlifSizeT encodingLength{};
			encoding = alifUStr_asUTF8AndSize(args[3], &encodingLength);
			if (encoding == nullptr) {
				goto exit;
			}
			if (strlen(encoding) != (size_t)encodingLength) {
				alifErr_setString(_alifExcValueError_, "embedded null character");
				goto exit;
			}
		}
		else {
			//_alifArg_badArgument("open", "argument 'encoding'", "str or None", args[3]);
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (args[4]) {
		if (args[4] == ALIF_NONE) {
			errors = nullptr;
		}
		else if (ALIFUSTR_CHECK(args[4])) {
			AlifSizeT errorsLength{};
			errors = alifUStr_asUTF8AndSize(args[4], &errorsLength);
			if (errors == nullptr) {
				goto exit;
			}
			if (strlen(errors) != (AlifUSizeT)errorsLength) {
				alifErr_setString(_alifExcValueError_, "embedded null character");
				goto exit;
			}
		}
		else {
			//_alifArg_badArgument("open", "argument 'errors'", "str or None", args[4]);
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (args[5]) {
		if (args[5] == ALIF_NONE) {
			newline = nullptr;
		}
		else if (ALIFUSTR_CHECK(args[5])) {
			AlifSizeT newlineLength{};
			newline = alifUStr_asUTF8AndSize(args[5], &newlineLength);
			if (newline == nullptr) {
				goto exit;
			}
			if (strlen(newline) != (AlifUSizeT)newlineLength) {
				alifErr_setString(_alifExcValueError_, "embedded null character");
				goto exit;
			}
		}
		else {
			//_alifArg_badArgument("open", "argument 'newline'", "str or None", args[5]);
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (args[6]) {
		closefd = alifObject_isTrue(args[6]);
		if (closefd < 0) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	opener = args[7];
skip_optional_pos:
	returnValue = _io_openImpl(module, file, mode, buffering, encoding, errors, newline, closefd, opener);

exit:
	return returnValue;
}
