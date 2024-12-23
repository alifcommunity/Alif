#pragma once

#include "AlifCore_ModSupport.h"


// in BltinModule.c.h
#define BUILTIN_PRINT_METHODDEF {"اطبع", ALIF_CPPFUNCTION_CAST(builtin_print), METHOD_FASTCALL | METHOD_KEYWORDS} // 900



static AlifObject* builtin_printImpl(AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifObject*, AlifIntT); // 904

// in BltinModule.c.h
static AlifObject* builtin_print(AlifObject* module, AlifObject* const* args,
	AlifSizeT nargs, AlifObject* kwnames) { // 907
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
	AlifSizeT noptargs = 0 + (kwnames ? ALIFTUPLE_GET_SIZE(kwnames) : 0) - 0;
	AlifObject* __clinic_args = nullptr;
	AlifObject* sep = ALIF_NONE;
	AlifObject* end = ALIF_NONE;
	AlifObject* file = ALIF_NONE;
	AlifIntT flush = 0;

	args = _alifArg_unpackKeywordsWithVarArg(args, nargs, nullptr, kwnames, &parser, 0, 0, 0, 0, argsbuf);
	if (!args) {
		goto exit;
	}
	__clinic_args = args[0];
	if (!noptargs) {
		goto skip_optional_kwonly;
	}
	if (args[1]) {
		sep = args[1];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	if (args[2]) {
		end = args[2];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	if (args[3]) {
		file = args[3];
		if (!--noptargs) {
			goto skip_optional_kwonly;
		}
	}
	flush = alifObject_isTrue(args[4]);
	if (flush < 0) {
		goto exit;
	}
skip_optional_kwonly:
	returnValue = builtin_printImpl(module, __clinic_args, sep, end, file, flush);

exit:
	ALIF_XDECREF(__clinic_args);
	return returnValue;
}
