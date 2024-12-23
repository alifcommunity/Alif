


#include "AlifCore_ModSupport.h"



#define BUILTIN_PRINT_METHODDEF {"اطبع", ALIF_CPPFUNCTION_CAST(builtin_print), METHOD_FASTCALL | METHOD_KEYWORDS} // 900



static AlifObject* builtin_printImpl(AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifObject*, AlifIntT); // 904

static AlifObject* builtin_print(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 907

	// alif // print
	ReprFunc func{};
	for (AlifIntT i = 0; i < _nargs; i++) {
		func = ALIF_TYPE(*_args)->repr;
		AlifObject* res = func(*_args);
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
