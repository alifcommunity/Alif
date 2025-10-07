#include "AlifCore_Abstract.h"
#include "AlifCore_ModSupport.h"  











// 919
#define UNICODE_REPLACE_METHODDEF    \
    {"Replace", ALIF_CPPFUNCTION_CAST(unicode_replace), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* unicode_replaceImpl(AlifObject*, AlifObject*, AlifObject*, AlifSizeT);

static AlifObject* unicode_replace(AlifObject* self, AlifObject* const* args,
	AlifSizeT nargs, AlifObject* kwnames) { // 926
	AlifObject* return_value = nullptr;
#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)

#define NUM_KEYWORDS 1
	static struct {
		AlifGCHead _thisIsNotUsed;
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS];
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS)
		.item = { &ALIF_ID(Count), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else 
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "", "", "count", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "replace",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[3]{};
	AlifSizeT noptargs = nargs + (kwnames ? ALIFTUPLE_GET_SIZE(kwnames) : 0) - 2;
	AlifObject* old{};
	AlifObject* new_{};
	AlifSizeT count = -1;

	args = ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser, 2, 3, 0, argsbuf);
	if (!args) {
		goto exit;
	}
	if (!ALIFUSTR_CHECK(args[0])) {
		//_alifArg_badArgument("replace", "argument 1", "str", args[0]);
		goto exit;
	}
	old = args[0];
	if (!ALIFUSTR_CHECK(args[1])) {
		//_alifArg_badArgument("replace", "argument 2", "str", args[1]);
		goto exit;
	}
	new_ = args[1];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	{
		AlifSizeT ival = -1;
		AlifObject* iobj = _alifNumber_index(args[2]);
		if (iobj != nullptr) {
			ival = alifLong_asSizeT(iobj);
			ALIF_DECREF(iobj);
		}
		if (ival == -1 and alifErr_occurred()) {
			goto exit;
		}
		count = ival;
	}
skip_optional_pos:
	return_value = unicode_replaceImpl(self, old, new_, count);

exit:
	return return_value;
}
