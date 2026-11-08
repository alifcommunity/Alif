
#include "AlifCore_ModSupport.h"



static AlifIntT module___init__Impl(AlifModuleObject*, AlifObject*, AlifObject*);

static AlifIntT module___init__(AlifObject* _self,
	AlifObject* _args, AlifObject* _kwargs) {
	AlifIntT returnValue = -1;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#define NUM_KEYWORDS 2
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
			.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
			.item = { &ALIF_STR(Name), &ALIF_ID(Doc), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else 
#  define KWTUPLE nullptr
#endif 

	static const char* const _keywords[] = { "اسم", "Doc", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "وحدة",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2]{};
	AlifObject* const* fastargs{};
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(_args);
	AlifSizeT noptargs = nargs + (_kwargs ? ALIFDICT_GET_SIZE(_kwargs) : 0) - 1;
	AlifObject* name{};
	AlifObject* doc = ALIF_NONE;

	fastargs = _ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(_args)->item, nargs, _kwargs, nullptr, &_parser,
		/*minpos*/ 1, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	if (!ALIFUSTR_CHECK(fastargs[0])) {
		//_alifArg_badArgument("وحدة", "المعامل 'اسم'", "نص", fastargs[0]);
		goto exit;
	}
	name = fastargs[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	doc = fastargs[1];
skip_optional_pos:
	returnValue = module___init__Impl((AlifModuleObject*)_self, name, doc);

exit:
	return returnValue;
}
