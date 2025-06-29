

#include "AlifCore_ModSupport.h"


















static AlifObject* tb_newImpl(AlifTypeObject*, AlifObject*, AlifFrameObject*, AlifIntT, AlifIntT); // 17

static AlifObject* tb_new(AlifTypeObject* type, AlifObject* args, AlifObject* kwargs) { // 22
	AlifObject* return_value = nullptr;

#ifdef ALIF_BUILD_CORE
#else  // !ALIF_BUILD_CORE
#  define KWTUPLE nullptr
#endif  // !ALIF_BUILD_CORE
	static const char* const keywords[] = { "TBNext", "TBFrame", "TBLasti", "TBLineno", nullptr };
	static AlifArgParser parser = {
		.keywords = keywords,
		.fname = "traceback",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[4]{};
	AlifObject* const* fastargs{};
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);
	AlifObject* tb_next{};
	AlifFrameObject* tb_frame{};
	AlifIntT tb_lasti{};
	AlifIntT tb_lineno{};

	fastargs = ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(args)->item, nargs,
		kwargs, nullptr, &parser, 4, 4, 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	tb_next = fastargs[0];
	if (!ALIFOBJECT_TYPECHECK(fastargs[1], &_alifFrameType_)) {
		//_alifArg_badArgument("traceback", "argument 'tb_frame'", (&_alifFrameType_)->name, fastargs[1]);
		goto exit;
	}
	tb_frame = (AlifFrameObject*)fastargs[1];
	tb_lasti = alifLong_asInt(fastargs[2]);
	if (tb_lasti == -1 and alifErr_occurred()) {
		goto exit;
	}
	tb_lineno = alifLong_asInt(fastargs[3]);
	if (tb_lineno == -1 and alifErr_occurred()) {
		goto exit;
	}
	return_value = tb_newImpl(type, tb_next, tb_frame, tb_lasti, tb_lineno);

exit:
	return return_value;
}
