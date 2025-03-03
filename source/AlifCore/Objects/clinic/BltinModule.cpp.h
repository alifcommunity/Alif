


#include "AlifCore_ModSupport.h"






#define BUILTIN___IMPORT___METHODDEF    \
    {"_استورد_", ALIF_CPPFUNCTION_CAST(builtin___import__), METHOD_FASTCALL | METHOD_KEYWORDS}

static AlifObject* builtin___import__Impl(AlifObject*, AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifIntT); // 35


static AlifObject* builtin___import__(AlifObject *_module, AlifObject *const* _args,
	AlifSizeT _nargs, AlifObject* _kwNames) { // 39
    AlifObject *returnValue = nullptr;

    #define NUM_KEYWORDS 5
    static class {
	public:
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS]{};
    } _kwtuple = {
        .objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
        //.item = { &ALIF_ID(Name), &ALIF_ID(Globals), &ALIF_ID(Locals), &ALIF_ID(Fromlist), &ALIF_ID(Level), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.objBase.objBase)

    static const char * const _keywords[] = {"name", "globals", "locals", "fromlist", "level", nullptr};
    static AlifArgParser _parser = {
        .keywords = _keywords,
        .fname = "__import__",
        .kwTuple = KWTUPLE,
    };
    #undef KWTUPLE
    AlifObject *argsbuf[5];
    AlifSizeT noptargs = _nargs + (_kwNames ? ALIFTUPLE_GET_SIZE(_kwNames) : 0) - 1;
    AlifObject *name;
    AlifObject *globals = nullptr;
    AlifObject *locals = nullptr;
    AlifObject *fromlist = nullptr;
    AlifIntT level = 0;

    _args = ALIFARG_UNPACKKEYWORDS(_args, _nargs, nullptr, _kwNames, &_parser, 1, 5, 0, argsbuf);
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




#define BUILTIN_LEN_METHODDEF {"طول", (AlifCPPFunction)builtin_len, METHOD_O} // 771









#define BUILTIN_PRINT_METHODDEF {"اطبع", ALIF_CPPFUNCTION_CAST(builtin_print), METHOD_FASTCALL | METHOD_KEYWORDS} // 900



static AlifObject* builtin_printImpl(AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifObject*, AlifIntT); // 904

static AlifObject* builtin_print(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 907

	// alif // print
	ReprFunc func{};
	for (AlifIntT i = 0; i < _nargs; i++) {
		func = ALIF_TYPE(_args[i])->repr;
		AlifObject* res = func(_args[i]);
		if (alifUStr_isASCII(res)) {
			char* buf = (char*)ALIFUSTR_DATA(res);
			printf("%s \n", buf);
		}
		else {
			char* buf = (char*)alifUStr_asUTF8(res);
			printf("%s \n", buf);
		}
	}
	return ALIF_NONE;
	// alif

	AlifObject* returnValue = nullptr;
#define NUM_KEYWORDS 4
	static class {
	public:
		AlifGCHead thisNotUsed{};
		ALIFOBJECT_VAR_HEAD;
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
		.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
		.item = { &ALIF_ID(Sep), &ALIF_ID(End), &ALIF_ID(File), &ALIF_ID(Flush), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)


	static const char* const keywords[] = { "Sep", "End", "File", "Flush", nullptr };
	static AlifArgParser parser = {
		.keywords = keywords,
		.fname = "print",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[5]{};
	AlifSizeT noptargs = 0 + (_kwnames ? ALIFTUPLE_GET_SIZE(_kwnames) : 0) - 0;
	AlifObject* __clinic_args = nullptr;
	AlifObject* sep = ALIF_NONE;
	AlifObject* end = ALIF_NONE;
	AlifObject* file = ALIF_NONE;
	AlifIntT flush = 0;

	_args = _alifArg_unpackKeywordsWithVarArg(_args, _nargs, nullptr, _kwnames, &parser, 0, 0, 0, 0, argsbuf);
	if (!_args) {
		goto exit;
	}
	__clinic_args = _args[0];
	if (!noptargs) {
		goto skip_optional_kwonly;
	}
	if (_args[1]) {
		sep = _args[1];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	if (_args[2]) {
		end = _args[2];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	if (_args[3]) {
		file = _args[3];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	flush = alifObject_isTrue(_args[4]);
	if (flush < 0) {
		goto exit;
	}
skip_optional_kwonly:
	returnValue = builtin_printImpl(_module, __clinic_args, sep, end, file, flush);

exit:
	ALIF_XDECREF(__clinic_args);
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
