#include "AlifCore_ModSupport.h"












static AlifIntT _ioBufferedReader___init__Impl(Buffered*, AlifObject*, AlifSizeT); // 936

static AlifIntT _ioBufferedReader___init__(AlifObject* self, AlifObject* args, AlifObject* kwargs) { // 940
	int return_value = -1;
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
	return_value = _ioBufferedReader___init__Impl((Buffered*)self, raw, buffer_size);

exit:
	return return_value;
}
