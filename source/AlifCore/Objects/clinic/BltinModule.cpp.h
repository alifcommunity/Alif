


#include "AlifCore_ModSupport.h"






#define BUILTIN___IMPORT___METHODDEF    \
    {"__استورد__", ALIF_CPPFUNCTION_CAST(builtin___import__), METHOD_FASTCALL | METHOD_KEYWORDS}

static AlifObject* builtin___import__Impl(AlifObject*, AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifIntT); // 35


static AlifObject* builtin___import__(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwNames) { // 39
	AlifObject* returnValue = nullptr;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#define NUM_KEYWORDS 5
	static class {
	public:
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_STR(Name), &ALIF_STR(Globals), &ALIF_STR(Locals), &ALIF_STR(FromList), &ALIF_STR(Level), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else 
#  define KWTUPLE nullptr
#endif 

	static const char* const _keywords[] = { "اسم", "اسماء_عامة", "اسماء_محلية", "من_مصفوفة", "مستوى", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "__استورد__",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[5];
	AlifSizeT noptargs = _nargs + (_kwNames ? ALIFTUPLE_GET_SIZE(_kwNames) : 0) - 1;
	AlifObject* name;
	AlifObject* globals = nullptr;
	AlifObject* locals = nullptr;
	AlifObject* fromlist = nullptr;
	AlifIntT level = 0;

	_args = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwNames, &_parser,
		/*minpos*/ 1, /*maxpos*/ 5, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	name = _args[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (_args[1]) {
		globals = _args[1];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (_args[2]) {
		locals = _args[2];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (_args[3]) {
		fromlist = _args[3];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	level = alifLong_asInt(_args[4]);
	if (level == -1 and alifErr_occurred()) {
		goto exit;
	}
skip_optional_pos:
	returnValue = builtin___import__Impl(_module, name, globals, locals, fromlist, level);

exit:
	return returnValue;
}



#define BUILTIN_ALL_METHODDEF    \
    {"تحقق_الكل", (AlifCPPFunction)builtin_all, METHOD_O} // 131

#define BUILTIN_ANY_METHODDEF    \
    {"تحقق_اي", (AlifCPPFunction)builtin_any, METHOD_O} // 141



// 168 //* alif
#define BUILTIN_BIN_METHODDEF    \
    {"ثنائي", ALIF_CPPFUNCTION_CAST(builtin_bin), METHOD_FASTCALL}

static AlifObject* builtin_binImpl(AlifObject*, AlifObject*, AlifIntT);

AlifObject* builtin_bin(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs) { //* alif
	AlifObject* returnValue{};
	AlifObject* obj{};
	AlifIntT isAscii = 0;

	if (!_ALIFARG_CHECKPOSITIONAL("ثنائي", _nargs, 1, 2)) {
		goto exit;
	}
	obj = _args[0];
	if (_args[1]) {
		isAscii = alifNumber_asSizeT(_args[1], _alifExcValueError_);
		if (isAscii < 0) {
			goto exit;
		}
	}
	returnValue = builtin_binImpl(_module, obj, isAscii);

exit:
	return returnValue;
}

// 809 //* alif
#define BUILTIN_OCT_METHODDEF    \
    {"ثماني", ALIF_CPPFUNCTION_CAST(builtin_oct), METHOD_FASTCALL}

static AlifObject* builtin_octImpl(AlifObject*, AlifObject*, AlifIntT);

AlifObject* builtin_oct(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs) { //* alif
	AlifObject* returnValue{};
	AlifObject* obj{};
	AlifIntT isAscii = 0;

	if (!_ALIFARG_CHECKPOSITIONAL("ثماني", _nargs, 1, 2)) {
		goto exit;
	}
	obj = _args[0];
	if (_args[1]) {
		isAscii = alifNumber_asSizeT(_args[1], _alifExcValueError_);
		if (isAscii < 0) {
			goto exit;
		}
	}
	returnValue = builtin_octImpl(_module, obj, isAscii);

exit:
	return returnValue;
}

// 719 //* alif
#define BUILTIN_HEX_METHODDEF    \
    {"ست_عشري", ALIF_CPPFUNCTION_CAST(builtin_hex), METHOD_FASTCALL}

static AlifObject* builtin_hexImpl(AlifObject*, AlifObject*, AlifIntT);

AlifObject* builtin_hex(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs) { //* alif
	AlifObject* returnValue{};
	AlifObject* obj{};
	AlifIntT isAscii = 0;

	if (!_ALIFARG_CHECKPOSITIONAL("ست_عشري", _nargs, 1, 2)) {
		goto exit;
	}
	obj = _args[0];
	if (_args[1]) {
		isAscii = alifNumber_asSizeT(_args[1], _alifExcValueError_);
		if (isAscii < 0) {
			goto exit;
		}
	}
	returnValue = builtin_hexImpl(_module, obj, isAscii);

exit:
	return returnValue;
}



#define BUILTIN_CHR_METHODDEF    \
    {"حرف", (AlifCPPFunction)builtin_chr, METHOD_O} // 234



// 255
#define BUILTIN_COMPILE_METHODDEF    \
    {"ترجم", ALIF_CPPFUNCTION_CAST(builtin_compile), METHOD_FASTCALL|METHOD_KEYWORDS}

static AlifObject* builtin_compileImpl(AlifObject*, AlifObject*, AlifObject*,
	const char*, AlifIntT, AlifIntT, AlifIntT, AlifIntT);

static AlifObject* builtin_compile(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 263
	AlifObject* returnValue{};

#define KWTUPLE nullptr

	static const char* const _keywords[] = { "مصدر", "الملف", "الوضع", "الأعلام", "لا_ترث", "محسن", "نسخة_الميزات", nullptr };
	static AlifArgParser parser = {
		.keywords = _keywords,
		.fname = "ترجم",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[7]{};
	AlifSizeT noptargs = _nargs + (_kwnames ? ALIFTUPLE_GET_SIZE(_kwnames) : 0) - 3;
	AlifObject* source{};
	AlifObject* filename{};
	const char* mode{};
	AlifIntT flags = 0;
	AlifIntT dontInherit = 0;
	AlifIntT optimize = -1;
	AlifIntT featureVersion = -1;

	_args = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &parser,
		/*minpos*/ 3, /*maxpos*/ 6, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	source = _args[0];
	if (!alifUStr_fsDecoder(_args[1], &filename)) {
		goto exit;
	}
	if (!ALIFUSTR_CHECK(_args[2])) {
		//_alifArg_badArgument("ترجم", "المعامل 'الوضع'", "نص", _args[2]);
		goto exit;
	}
	AlifSizeT modeLength;
	mode = alifUStr_asUTF8AndSize(_args[2], &modeLength);
	if (mode == nullptr) {
		goto exit;
	}
	if (strlen(mode) != (size_t)modeLength) {
		alifErr_setString(_alifExcValueError_, "يوجد حرف فارغ");
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (_args[3]) {
		flags = alifLong_asInt(_args[3]);
		if (flags == -1 and alifErr_occurred()) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (_args[4]) {
		dontInherit = alifObject_isTrue(_args[4]);
		if (dontInherit < 0) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (_args[5]) {
		optimize = alifLong_asInt(_args[5]);
		if (optimize == -1 and alifErr_occurred()) {
			goto exit;
		}
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
skip_optional_pos:
	if (!noptargs) {
		goto skip_optional_kwonly;
	}
	featureVersion = alifLong_asInt(_args[6]);
	if (featureVersion == -1 and alifErr_occurred()) {
		goto exit;
	}
skip_optional_kwonly:
	returnValue = builtin_compileImpl(_module, source, filename, mode, flags, dontInherit, optimize, featureVersion);

exit:
	return returnValue;
}









#define BUILTIN_EXEC_METHODDEF    \
    {"نفذ", ALIF_CPPFUNCTION_CAST(builtin_exec), METHOD_FASTCALL|METHOD_KEYWORDS}

static AlifObject* builtin_execImpl(AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifObject*);

static AlifObject* builtin_exec(AlifObject* module,
	AlifObject* const* args, AlifSizeT nargs, AlifObject* kwnames) { // 497
	AlifObject* returnValue = nullptr;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#define NUM_KEYWORDS 3
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
			.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
			.item = { &ALIF_STR(Globals), &ALIF_STR(Locals), &ALIF_ID(Closure), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else
#  define KWTUPLE nullptr
#endif

	static const char* const _keywords[] = { "", "اسماء_عامة", "اسماء_محلية", "Closure", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "نفذ",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[4];
	AlifSizeT noptargs = nargs + (kwnames ? ALIFTUPLE_GET_SIZE(kwnames) : 0) - 1;
	AlifObject* source;
	AlifObject* globals = ALIF_NONE;
	AlifObject* locals = ALIF_NONE;
	AlifObject* closure = nullptr;

	args = _ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser,
		/*minpos*/ 1, /*maxpos*/ 3, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!args) {
		goto exit;
	}
	source = args[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	if (args[1]) {
		globals = args[1];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
	if (args[2]) {
		locals = args[2];
		if (!--noptargs) {
			goto skip_optional_pos;
		}
	}
skip_optional_pos:
	if (!noptargs) {
		goto skip_optional_kwonly;
	}
	closure = args[3];
skip_optional_kwonly:
	returnValue = builtin_execImpl(module, source, globals, locals, closure);

exit:
	return returnValue;
}







#define BUILTIN_GLOBALS_METHODDEF    \
    {"اسماء_عامة", (AlifCPPFunction)builtin_globals, METHOD_NOARGS} // 575

static AlifObject* builtin_globalsImpl(AlifObject*);

static AlifObject* builtin_globals(AlifObject* _module, AlifObject* ALIF_UNUSED(_ignored)) { // 581
	return builtin_globalsImpl(_module);
}



// 595
#define BUILTIN_HASATTR_METHODDEF    \
    {"يملك_صفة", ALIF_CPPFUNCTION_CAST(builtin_hasAttr), METHOD_FASTCALL}

static AlifObject* builtin_hasAttrImpl(AlifObject*, AlifObject*, AlifObject*);

static AlifObject* builtin_hasAttr(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs) {
	AlifObject* returnValue{};
	AlifObject* obj{};
	AlifObject* name{};

	if (!_ALIFARG_CHECKPOSITIONAL("يملك_صفة", _nargs, 2, 2)) {
		goto exit;
	}
	obj = _args[0];
	name = _args[1];
	returnValue = builtin_hasAttrImpl(_module, obj, name);

exit:
	return returnValue;
}



// 639
#define BUILTIN_SETATTR_METHODDEF    \
    {"اضف_صفة", ALIF_CPPFUNCTION_CAST(builtin_setAttr), METHOD_FASTCALL}

static AlifObject* builtin_setAttrImpl(AlifObject*, AlifObject*, AlifObject*, AlifObject*);

static AlifObject* builtin_setAttr(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs) { // 646
	AlifObject* returnValue{};
	AlifObject* obj{};
	AlifObject* name{};
	AlifObject* value{};

	if (!_ALIFARG_CHECKPOSITIONAL("اضف_صفة", _nargs, 3, 3)) {
		goto exit;
	}
	obj = _args[0];
	name = _args[1];
	value = _args[2];
	returnValue = builtin_setAttrImpl(_module, obj, name, value);

exit:
	return returnValue;
}




#define BUILTIN_DELATTR_METHODDEF    \
    {"احذف_صفة", ALIF_CPPFUNCTION_CAST(builtin_delAttr), METHOD_FASTCALL} // 674

static AlifObject* builtin_delAttrImpl(AlifObject*, AlifObject*, AlifObject*);

static AlifObject* builtin_delAttr(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs) { // 680
	AlifObject* returnValue{};
	AlifObject* obj{};
	AlifObject* name{};

	if (!_ALIFARG_CHECKPOSITIONAL("احذف_صفة", _nargs, 2, 2)) {
		goto exit;
	}
	obj = _args[0];
	name = _args[1];
	returnValue = builtin_delAttrImpl(_module, obj, name);

exit:
	return returnValue;
}


#define BUILTIN_LOCALS_METHODDEF    \
    {"اسماء_محلية", (AlifCPPFunction)builtin_locals, METHOD_NOARGS} // 788

static AlifObject* builtin_localsImpl(AlifObject*);

static AlifObject* builtin_locals(AlifObject* _module, AlifObject* ALIF_UNUSED(_ignored)) { // 795
	return builtin_localsImpl(_module);
}



#define BUILTIN_LEN_METHODDEF {"طول", (AlifCPPFunction)builtin_len, METHOD_O} // 771




#define BUILTIN_ORD_METHODDEF    \
    {"رمز", (AlifCPPFunction)builtin_ord, METHOD_O} // 818




#define BUILTIN_PRINT_METHODDEF {"اطبع", ALIF_CPPFUNCTION_CAST(builtin_print), METHOD_FASTCALL | METHOD_KEYWORDS} // 900



static AlifObject* builtin_printImpl(AlifObject*, AlifObject* const*,
	AlifSizeT, AlifObject*, AlifObject*, AlifObject*, AlifIntT); // 904

static AlifObject* builtin_print(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 907

	AlifObject* returnValue = nullptr;
#define NUM_KEYWORDS 4
	static class {
	public:
		AlifGCHead thisNotUsed{};
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_STR(Sep), &ALIF_STR(End), &ALIF_ID(File), &ALIF_STR(Flush), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)


	static const char* const keywords[] = { "الفاصل", "النهاية", "File", "مباشر", nullptr };
	static AlifArgParser _parser = {
		.keywords = keywords,
		.fname = "اطبع",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[4]{};
	AlifObject* const* fastargs{};
	AlifSizeT noptargs = 0 + (_kwnames ? ALIFTUPLE_GET_SIZE(_kwnames) : 0) - 0;
	AlifObject* const* __clinicArgs{};
	AlifSizeT argsLength{};
	AlifObject* sep = ALIF_NONE;
	AlifObject* end = ALIF_NONE;
	AlifObject* file = ALIF_NONE;
	AlifIntT flush = 0;

	fastargs = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwnames, &_parser,
		/*minpos*/ 0, /*maxpos*/ 0, /*minkw*/ 0, /*varpos*/ 1, argsbuf);
	if (!fastargs) {
		goto exit;
	}
	if (!noptargs) {
		goto skip_optional_kwonly;
	}
	if (fastargs[0]) {
		sep = fastargs[0];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	if (fastargs[1]) {
		end = fastargs[1];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	if (fastargs[2]) {
		file = fastargs[2];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	flush = alifObject_isTrue(fastargs[3]);
	if (flush < 0) {
		goto exit;
	}
skip_optional_kwonly:
	__clinicArgs = _args;
	argsLength = _nargs;
	returnValue = builtin_printImpl(_module, __clinicArgs, argsLength, sep, end, file, flush);

exit:
	return returnValue;
}






// 994
#define BUILTIN_INPUT_METHODDEF    \
    {"ادخل", ALIF_CPPFUNCTION_CAST(builtin_input), METHOD_FASTCALL /*, builtin_input__doc__*/}

static AlifObject* builtin_inputImpl(AlifObject*, AlifObject*); // 997

static AlifObject* builtin_input(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs) { // 1000
	AlifObject* return_value = nullptr;
	AlifObject* prompt = nullptr;

	if (!_ALIFARG_CHECKPOSITIONAL("ادخل", _nargs, 0, 1)) {
		goto exit;
	}
	if (_nargs < 1) {
		goto skip_optional;
	}
	prompt = _args[0];
skip_optional:
	return_value = builtin_inputImpl(_module, prompt);

exit:
	return return_value;
}


#define BUILTIN_ROUND_METHODDEF    \
    {"قرب", ALIF_CPPFUNCTION_CAST(builtin_round), METHOD_FASTCALL|METHOD_KEYWORDS}

static AlifObject* builtin_roundImpl(AlifObject*, AlifObject*, AlifObject*);

static AlifObject* builtin_round(AlifObject* module,
	AlifObject* const* args, AlifSizeT nargs, AlifObject* kwnames) { // 1055

	AlifObject* returnValue{};

#  define KWTUPLE nullptr

	static const char* const _keywords[] = { "رقم", "عدد_الارقام", nullptr };
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "قرب",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2]{};
	AlifSizeT noptargs = nargs + (kwnames ? ALIFTUPLE_GET_SIZE(kwnames) : 0) - 1;
	AlifObject* number{};
	AlifObject* ndigits = ALIF_NONE;

	args = _ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser,
		/*minpos*/ 1, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!args) {
		goto exit;
	}
	number = args[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	ndigits = args[1];
skip_optional_pos:
	returnValue = builtin_roundImpl(module, number, ndigits);

exit:
	return returnValue;
}



// 1106
#define BUILTIN_SUM_METHODDEF    \
    {"اجمع", ALIF_CPPFUNCTION_CAST(builtin_sum), METHOD_FASTCALL|METHOD_KEYWORDS}

static AlifObject* builtin_sumImpl(AlifObject*, AlifObject*, AlifObject*);

static AlifObject* builtin_sum(AlifObject* _module, AlifObject *const* _args,
	AlifSizeT _nargs, AlifObject* _kwNames) { // 1112
	AlifObject* return_value = nullptr;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#define NUM_KEYWORDS 1
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
			.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
			.item = { &ALIF_ID(Start), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else 
#  define KWTUPLE nullptr
#endif

	static const char * const _keywords[] = {"", "Start", nullptr};
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "اجمع",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2]{};
	AlifSizeT noptargs = _nargs + (_kwNames ? ALIFTUPLE_GET_SIZE(_kwNames) : 0) - 1;
	AlifObject* iterable{};
	AlifObject* start{};

	_args = _ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwNames, &_parser,
		/*minpos*/ 1, /*maxpos*/ 2, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	iterable = _args[0];
	if (!noptargs) {
		goto skip_optional_pos;
	}
	start = _args[1];
skip_optional_pos:
	return_value = builtin_sumImpl(_module, iterable, start);

exit:
	return return_value;
}



#define BUILTIN_ISINSTANCE_METHODDEF    \
    {"هل_نوع", ALIF_CPPFUNCTION_CAST(builtin_isInstance), METHOD_FASTCALL}

static AlifObject * builtin_isInstanceImpl(AlifObject*, AlifObject*, AlifObject*);

static AlifObject * builtin_isInstance(AlifObject *module, AlifObject *const *args,
	AlifSizeT nargs) { // 1172
	AlifObject *returnValue{};
	AlifObject *obj{};
	AlifObject *classOrTuple{};

	if (!_ALIFARG_CHECKPOSITIONAL("هل_نوع", nargs, 2, 2)) {
		goto exit;
	}
	obj = args[0];
	classOrTuple = args[1];
	returnValue = builtin_isInstanceImpl(module, obj, classOrTuple);

exit:
	return returnValue;
}
