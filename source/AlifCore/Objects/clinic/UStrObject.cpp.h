#include "AlifCore_Abstract.h"
#include "AlifCore_ModSupport.h"  











// 919
#define UNICODE_REPLACE_METHODDEF    \
    {"استبدل", ALIF_CPPFUNCTION_CAST(unicode_replace), METHOD_FASTCALL|METHOD_KEYWORDS},

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


// 1245
#define UNICODE_SPLIT_METHODDEF    \
    {"افصل", ALIF_CPPFUNCTION_CAST(unicode_split), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* uStr_splitImpl(AlifObject*, AlifObject*, AlifSizeT);

static AlifObject* unicode_split(AlifObject* _self, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 1251
	AlifObject* returnValue = nullptr;
#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)

#define NUM_KEYWORDS 2
	static struct {
		AlifGCHead thisIsNotUsed;
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS];
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_STR(Sep), &ALIF_ID(MaxSplit), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else 
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "Sep", "MaxSplit", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "split",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2];
	AlifSizeT noptargs = _nargs + (_kwnames ? ALIFTUPLE_GET_SIZE(_kwnames) : 0) - 0;
	AlifObject* sep = ALIF_NONE;
	AlifSizeT maxsplit = -1;

	_args = ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &_parser, 0, 2, 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (_args[0]) {
		sep = _args[0];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	{
		AlifSizeT ival = -1;
		AlifObject* iobj = _alifNumber_index(_args[1]);
		if (iobj != NULL) {
			ival = alifLong_asSizeT(iobj);
			ALIF_DECREF(iobj);
		}
		if (ival == -1 and alifErr_occurred()) {
			goto exit;
		}
		maxsplit = ival;
	}
skip_optional_pos:
	returnValue = uStr_splitImpl(_self, sep, maxsplit);

exit:
	return returnValue;
}




// 1330
#define UNICODE_PARTITION_METHODDEF    \
    {"قسم", (AlifCPPFunction)uStr_partition, METHOD_O},
