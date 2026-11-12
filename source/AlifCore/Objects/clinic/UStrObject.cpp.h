#include "AlifCore_Abstract.h"
#include "AlifCore_ModSupport.h"  





// 147
#define UNICODE_COUNT_METHODDEF    \
    {"كم", ALIF_CPPFUNCTION_CAST(uStr_count), METHOD_FASTCALL},

static AlifSizeT uStr_countImpl(AlifObject*, AlifObject*, AlifSizeT, AlifSizeT);

static AlifObject* uStr_count(AlifObject* str, AlifObject*const* args, AlifSizeT nargs) { // 154
	AlifObject* returnValue{};
	AlifObject* substr{};
	AlifSizeT start = 0;
	AlifSizeT end = ALIF_SIZET_MAX;
	AlifSizeT returnValue_;

	if (!_ALIFARG_CHECKPOSITIONAL("كم", nargs, 1, 3)) {
		goto exit;
	}
	if (!ALIFUSTR_CHECK(args[0])) {
		//_alifArg_badArgument("كم", "argument 1", "str", args[0]);
		goto exit;
	}
	substr = args[0];
	if (nargs < 2) {
		goto skip_optional;
	}
	if (!_alifEval_sliceIndex(args[1], &start)) {
		goto exit;
	}
	if (nargs < 3) {
		goto skip_optional;
	}
	if (!_alifEval_sliceIndex(args[2], &end)) {
		goto exit;
	}
skip_optional:
	returnValue_ = uStr_countImpl(str, substr, start, end);
	if ((returnValue_ == -1) and alifErr_occurred()) {
		goto exit;
	}
	returnValue = alifLong_fromSizeT(returnValue_);

exit:
	return returnValue;
}





// 368
#define UNICODE_FIND_METHODDEF    \
    {"اوجد", ALIF_CPPFUNCTION_CAST(uStr_find), METHOD_FASTCALL},

static AlifSizeT uStr_findImpl(AlifObject*, AlifObject*, AlifSizeT, AlifSizeT);

static AlifObject* uStr_find(AlifObject* str,
	AlifObject *const* args, AlifSizeT nargs) {
	AlifObject* returnValue{};
	AlifObject* substr{};
	AlifSizeT start = 0;
	AlifSizeT end = ALIF_SIZET_MAX;
	AlifSizeT returnValue_{};

	if (!_alifArg_checkPositional("اوجد", nargs, 1, 3)) {
		goto exit;
	}
	if (!ALIFUSTR_CHECK(args[0])) {
		//_alifArg_badArgument("find", "argument 1", "str", args[0]);
		goto exit;
	}
	substr = args[0];
	if (nargs < 2) {
		goto skip_optional;
	}
	if (!_alifEval_sliceIndex(args[1], &start)) {
		goto exit;
	}
	if (nargs < 3) {
		goto skip_optional;
	}
	if (!_alifEval_sliceIndex(args[2], &end)) {
		goto exit;
	}
skip_optional:
	returnValue_ = uStr_findImpl(str, substr, start, end);
	if ((returnValue_ == -1) and alifErr_occurred()) {
		goto exit;
	}
	returnValue = alifLong_fromSizeT(returnValue_);

exit:
	return returnValue;
}




// 482
#define UNICODE_ISASCII_METHODDEF    \
    {"هل_ترميز_بدائي", (AlifCPPFunction)uStr_isascii, METHOD_NOARGS},

static AlifObject* uStr_isasciiImpl(AlifObject*);

static AlifObject* uStr_isascii(AlifObject* self,
	AlifObject* ALIF_UNUSED(ignored)) {
	return uStr_isasciiImpl(self);
}





// 608
#define UNICODE_ISALNUM_METHODDEF    \
    {"هل_حرف_عدد", (AlifCPPFunction)uStr_isalnum, METHOD_NOARGS},

static AlifObject* uStr_isalnumImpl(AlifObject*);

static AlifObject* uStr_isalnum(AlifObject* self,
	AlifObject* ALIF_UNUSED(ignored)) { // 614
	return uStr_isalnumImpl(self);
}




// 734
#define UNICODE_JOIN_METHODDEF {"اربط", (AlifCPPFunction)uStr_join, METHOD_O},


// 794
#define UNICODE_LOWER_METHODDEF    \
    {"حرف_صغير", (AlifCPPFunction)uStr_lower, METHOD_NOARGS},

static AlifObject* uStr_lowerImpl(AlifObject*);

static AlifObject* uStr_lower(AlifObject* _self,
	AlifObject* ALIF_UNUSED(ignored)) {
	return uStr_lowerImpl(_self);
}


// 814
#define UNICODE_STRIP_METHODDEF    \
    {"جرد", ALIF_CPPFUNCTION_CAST(uStr_strip), METHOD_FASTCALL},

static AlifObject* uStr_stripImpl(AlifObject*, AlifObject*);

static AlifObject* uStr_strip(AlifObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs) { // 820
	AlifObject* returnValue{};
	AlifObject* chars = ALIF_NONE;

	if (!_ALIFARG_CHECKPOSITIONAL("جرد", _nargs, 0, 1)) {
		goto exit;
	}
	if (_nargs < 1) {
		goto skip_optional;
	}
	chars = _args[0];
skip_optional:
	returnValue = uStr_stripImpl(_self, chars);

exit:
	return returnValue;
}

// 848
#define UNICODE_LSTRIP_METHODDEF    \
    {"جرد_يسار", ALIF_CPPFUNCTION_CAST(uStr_lStrip), METHOD_FASTCALL},

static AlifObject* uStr_lStripImpl(AlifObject*, AlifObject*);

static AlifObject* uStr_lStrip(AlifObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs) { // 854
	AlifObject* returnValue{};
	AlifObject* chars = ALIF_NONE;

	if (!_ALIFARG_CHECKPOSITIONAL("جرد_يسار", _nargs, 0, 1)) {
		goto exit;
	}
	if (_nargs < 1) {
		goto skip_optional;
	}
	chars = _args[0];
skip_optional:
	returnValue = uStr_lStripImpl(_self, chars);

exit:
	return returnValue;
}

// 882
#define UNICODE_RSTRIP_METHODDEF    \
    {"جرد_يمين", ALIF_CPPFUNCTION_CAST(uStr_rStrip), METHOD_FASTCALL},

static AlifObject* uStr_rStripImpl(AlifObject*, AlifObject*);

static AlifObject* uStr_rStrip(AlifObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs) { // 888
	AlifObject* returnValue{};
	AlifObject* chars = ALIF_NONE;

	if (!_ALIFARG_CHECKPOSITIONAL("جرد_يمين", _nargs, 0, 1)) {
		goto exit;
	}
	if (_nargs < 1) {
		goto skip_optional;
	}
	chars = _args[0];
skip_optional:
	returnValue = uStr_rStripImpl(_self, chars);

exit:
	return returnValue;
}




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
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_ID(Count), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else 
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "", "", "التكرار", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "استبدل",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[3]{};
	AlifSizeT noptargs = nargs + (kwnames ? ALIFTUPLE_GET_SIZE(kwnames) : 0) - 2;
	AlifObject* old{};
	AlifObject* new_{};
	AlifSizeT count = -1;

	args = _ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser,
		/*minpos*/ 2, /*maxpos*/ 3, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
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
    {"افصل", ALIF_CPPFUNCTION_CAST(uStr_split), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* uStr_splitImpl(AlifObject*, AlifObject*, AlifSizeT);

static AlifObject* uStr_split(AlifObject* _self, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 1251
	AlifObject* returnValue = nullptr;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#define NUM_KEYWORDS 2
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_STR(Sep), &ALIF_STR(MaxSplit), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else 
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "الفاصل", "التكرار", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "افصل",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2];
	AlifSizeT noptargs = _nargs + (_kwnames ? ALIFTUPLE_GET_SIZE(_kwnames) : 0) - 0;
	AlifObject* sep = ALIF_NONE;
	AlifSizeT maxsplit = -1;

	_args = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &_parser,
		/*minpos*/ 0, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
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
    {"اقسم", (AlifCPPFunction)uStr_partition, METHOD_O},

#define UNICODE_RPARTITION_METHODDEF    \
    {"اقسم_ع", (AlifCPPFunction)uStr_rpartition, METHOD_O},


// 1367
#define UNICODE_RSPLIT_METHODDEF    \
    {"افصل_ع", ALIF_CPPFUNCTION_CAST(uStr_rsplit), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* uStr_rsplitImpl(AlifObject*, AlifObject*, AlifSizeT);

static AlifObject* uStr_rsplit(AlifObject* self, AlifObject *const* args,
	AlifSizeT nargs, AlifObject* kwnames) { // 1373
	AlifObject* returnValue = nullptr;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#define NUM_KEYWORDS 2
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
			.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
			.item = { &ALIF_STR(Sep), &ALIF_STR(MaxSplit), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)
#else 
#  define KWTUPLE nullptr
#endif

	static const char * const _keywords[] = {"الفاصل", "التكرار", nullptr};
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "افصل_ع",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject *argsbuf[2];
	AlifSizeT noptargs = nargs + (kwnames ? ALIFTUPLE_GET_SIZE(kwnames) : 0) - 0;
	AlifObject* sep = ALIF_NONE;
	AlifSizeT maxsplit = -1;

	args = _ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser,
		/*minpos*/ 0, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!args) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (args[0]) {
		sep = args[0];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	{
		AlifSizeT ival = -1;
		AlifObject* iobj = _alifNumber_index(args[1]);
		if (iobj != nullptr) {
			ival = alifLong_asSizeT(iobj);
			ALIF_DECREF(iobj);
		}
		if (ival == -1 and alifErr_occurred()) {
			goto exit;
		}
		maxsplit = ival;
	}
skip_optional_pos:
	returnValue = uStr_rsplitImpl(self, sep, maxsplit);

exit:
	return returnValue;
}







// 1670
#define UNICODE_STARTSWITH_METHODDEF    \
    {"يبدا_ب", ALIF_CPPFUNCTION_CAST(uStr_startsWith), METHOD_FASTCALL},

static AlifObject* uStr_startsWithImpl(AlifObject*, AlifObject*, AlifSizeT, AlifSizeT);

static AlifObject* uStr_startsWith(AlifObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs) { // 1677
	AlifObject* returnValue{};
	AlifObject* subobj{};
	AlifSizeT start = 0;
	AlifSizeT end = ALIF_SIZET_MAX;

	if (!_ALIFARG_CHECKPOSITIONAL("يبدا_ب", _nargs, 1, 3)) {
		goto exit;
	}
	subobj = _args[0];
	if (_nargs < 2) {
		goto skip_optional;
	}
	if (!_alifEval_sliceIndex(_args[1], &start)) {
		goto exit;
	}
	if (_nargs < 3) {
		goto skip_optional;
	}
	if (!_alifEval_sliceIndex(_args[2], &end)) {
		goto exit;
	}
skip_optional:
	returnValue = uStr_startsWithImpl(_self, subobj, start, end);

exit:
	return returnValue;
}

// 1721
#define UNICODE_ENDSWITH_METHODDEF    \
    {"ينتهي_ب", ALIF_CPPFUNCTION_CAST(uStr_endsWith), METHOD_FASTCALL},

static AlifObject* uStr_endsWithImpl(AlifObject*, AlifObject*, AlifSizeT, AlifSizeT);

static AlifObject* uStr_endsWith(AlifObject* self,
	AlifObject* const* args, AlifSizeT nargs) { // 1728
	AlifObject* returnValue{};
	AlifObject* subobj{};
	AlifSizeT start = 0;
	AlifSizeT end = ALIF_SIZET_MAX;

	if (!_ALIFARG_CHECKPOSITIONAL("ينتهي_ب", nargs, 1, 3)) {
		goto exit;
	}
	subobj = args[0];
	if (nargs < 2) {
		goto skip_optional;
	}
	if (!_alifEval_sliceIndex(args[1], &start)) {
		goto exit;
	}
	if (nargs < 3) {
		goto skip_optional;
	}
	if (!_alifEval_sliceIndex(args[2], &end)) {
		goto exit;
	}
skip_optional:
	returnValue = uStr_endsWithImpl(self, subobj, start, end);

exit:
	return returnValue;
}




// 1759
#define UNICODE___FORMAT___METHODDEF    \
    {"__تنسيق__", (AlifCPPFunction)uStr___format__, METHOD_O},

static AlifObject* uStr___format__Impl(AlifObject*, AlifObject*);

static AlifObject* uStr___format__(AlifObject* _self, AlifObject* _arg) { // 1765
	AlifObject* returnValue{};
	AlifObject* formatSpec{};

	if (!ALIFUSTR_CHECK(_arg)) {
		//_alifArg_badArgument("__تنسيق__", "وسيط", "نص", _arg);
		goto exit;
	}
	formatSpec = _arg;
	returnValue = uStr___format__Impl(_self, formatSpec);

exit:
	return returnValue;
}












// 1806
static AlifObject* uStr_newImpl(AlifTypeObject*, AlifObject*, const char*, const char*);

static AlifObject* uStr_new(AlifTypeObject* type, AlifObject* args, AlifObject* kwargs) { // 1810
	AlifObject* returnValue = nullptr;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#define NUM_KEYWORDS 3
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_STR(Object), &ALIF_ID(Encoding), &ALIF_ID(Errors), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else 
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "كائن", "Encoding", "Errors", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "نص",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[3]{};
	AlifObject* const* fastargs{};
	AlifSizeT nargs = ALIFTUPLE_GET_SIZE(args);
	AlifSizeT noptargs = nargs + (kwargs ? ALIFDICT_GET_SIZE(kwargs) : 0) - 0;
	AlifObject* x = nullptr;
	const char* encoding = nullptr;
	const char* errors = nullptr;

	fastargs = _ALIFARG_UNPACKKEYWORDS(ALIFTUPLE_CAST(args)->item, nargs, kwargs, nullptr, &_parser,
		/*minpos*/ 0, /*maxpos*/ 3, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (fastargs[0]) {
		x = fastargs[0];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (fastargs[1]) {
		if (!ALIFUSTR_CHECK(fastargs[1])) {
			//_alifArg_badArgument("نص", "وسيط 'ترميز'", "نص", fastargs[1]);
			goto exit;
		}
		AlifSizeT encodingLength{};
		encoding = alifUStr_asUTF8AndSize(fastargs[1], &encodingLength);
		if (encoding == nullptr) {
			goto exit;
		}
		if (strlen(encoding) != (size_t)encodingLength) {
			alifErr_setString(_alifExcValueError_, "يحتوي حرف فارغ");
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (!ALIFUSTR_CHECK(fastargs[2])) {
		//_alifArg_badArgument("نص", "وسيط 'اخطاء'", "نص", fastargs[2]);
		goto exit;
	}
	AlifSizeT errorsLength;
	errors = alifUStr_asUTF8AndSize(fastargs[2], &errorsLength);
	if (errors == nullptr) {
		goto exit;
	}
	if (strlen(errors) != (size_t)errorsLength) {
		alifErr_setString(_alifExcValueError_, "يحتوي حرف فارغ");
		goto exit;
	}
skip_optional_pos:
	returnValue = uStr_newImpl(type, x, encoding, errors);

exit:
	return returnValue;
}
