

#include "AlifCore_ModSupport.h"


















static AlifObject* tb_newImpl(AlifTypeObject*, AlifObject*, AlifFrameObject*, AlifIntT, AlifIntT); // 17

static AlifObject* tb_new(AlifTypeObject* type, AlifObject* args, AlifObject* kwargs) { // 22
	AlifObject* return_value = nullptr;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)

#define NUM_KEYWORDS 4
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_ID(TBNext), &ALIF_ID(TBFrame), &ALIF_ID(TBLasti), &ALIF_ID(TBLineno), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else  // !ALIF_BUILD_CORE
#  define KWTUPLE nullptr
#endif  // !ALIF_BUILD_CORE
	static const char* const keywords[] = { "TBNext", "TBFrame", "TBLasti", "TBLineno", nullptr };
	static AlifArgParser parser = {
		.keywords = keywords,
		.fname = "تتبع_عكسي",
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

	fastargs = _ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(args)->item, nargs,
		kwargs, nullptr, &parser, /*minpos*/ 4, /*maxpos*/ 4, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
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
