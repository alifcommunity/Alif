#include "AlifCore_ModSupport.h"



// 122
#define _IMP_CREATE_BUILTIN_METHODDEF    \
    {"انشئ_ضمني", (AlifCPPFunction)_imp_createBuiltin, METHOD_O},


// 131
#define _IMP_EXTENSION_SUFFIXES_METHODDEF    \
    {"لاحقات_الامتداد", (AlifCPPFunction)_imp_extensionSuffixes, METHOD_NOARGS},

static AlifObject* _imp_extensionSuffixesImpl(AlifObject*);

static AlifObject* _imp_extensionSuffixes(AlifObject* _module,
	AlifObject* ALIF_UNUSED(ignored)) { // 137
	return _imp_extensionSuffixesImpl(_module);
}



// 186
#define _IMP_FIND_FROZEN_METHODDEF    \
    {"اوجد_مصرف", ALIF_CPPFUNCTION_CAST(_imp_findFrozen), METHOD_FASTCALL|METHOD_KEYWORDS},

static AlifObject* _imp_findFrozenImpl(AlifObject*, AlifObject*, AlifIntT);

static AlifObject* _imp_findFrozen(AlifObject* module, AlifObject* const* args,
	AlifSizeT nargs, AlifObject* kwnames) { // 192
	AlifObject* returnValue = nullptr;

#if defined(ALIF_BUILD_CORE) and !defined(ALIF_BUILD_CORE_MODULE)
#define NUM_KEYWORDS 1
	static struct {
		AlifGCHead thisIsNotUsed{};
		ALIFOBJECT_VAR_HEAD{};
		AlifObject* item[NUM_KEYWORDS]{};
	} _kwtuple = {
			.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTupleType_, NUM_KEYWORDS),
			.item = { &ALIF_ID(WithData), },
	};
#undef NUM_KEYWORDS
#define KWTUPLE (&_kwtuple.objBase.objBase)

#else  // !ALIF_BUILD_CORE
#  define KWTUPLE nullptr
#endif  // !ALIF_BUILD_CORE

	static const char * const _keywords[] = {"", "WithData", nullptr};
	static AlifArgParser _parser = {
		.keywords = _keywords,
		.fname = "اوجد_مصرف",
		.kwTuple = KWTUPLE,
	};
#undef KWTUPLE
	AlifObject* argsbuf[2]{};
	AlifSizeT noptargs = nargs + (kwnames ? ALIFTUPLE_GET_SIZE(kwnames) : 0) - 1;
	AlifObject* name{};
	AlifIntT withdata = 0;

	args = _ALIFARG_UNPACKKEYWORDS(args, nargs, nullptr, kwnames, &_parser,
		/*minpos*/ 1, /*maxpos*/ 1, /*minkw*/ 0, /*varpos*/ 0, argsbuf);
	if (!args) {
		goto exit;
	}
	if (!ALIFUSTR_CHECK(args[0])) {
		//_alifArg_badArgument("اوجد_مصرف", "argument 1", "str", args[0]);
		goto exit;
	}
	name = args[0];
	if (!noptargs) {
		goto skip_optional_kwonly;
	}
	withdata = alifObject_isTrue(args[1]);
	if (withdata < 0) {
		goto exit;
	}
skip_optional_kwonly:
	returnValue = _imp_findFrozenImpl(module, name, withdata);

exit:
	return returnValue;
}



// 256
#define _IMP_GET_FROZEN_OBJECT_METHODDEF    \
    {"اجلب_كائن_مصرف", ALIF_CPPFUNCTION_CAST(_imp_getFrozenObject), METHOD_FASTCALL},

static AlifObject* _imp_getFrozenObjectImpl(AlifObject*, AlifObject*, AlifObject*);

static AlifObject* _imp_getFrozenObject(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs) { // 263
	AlifObject* returnValue = nullptr;
	AlifObject* name{};
	AlifObject* dataobj = ALIF_NONE;

	if (!_ALIFARG_CHECKPOSITIONAL("اجلب_كائن_مصرف", _nargs, 1, 2)) {
		goto exit;
	}
	if (!ALIFUSTR_CHECK(_args[0])) {
		//_alifArg_badArgument("اجلب_كائن_مصرف", "argument 1", "نص", _args[0]);
		goto exit;
	}
	name = _args[0];
	if (_nargs < 2) {
		goto skip_optional;
	}
	dataobj = _args[1];
skip_optional:
	returnValue = _imp_getFrozenObjectImpl(_module, name, dataobj);

exit:
	return returnValue;
}








// 324
#define _IMP_IS_BUILTIN_METHODDEF    \
    {"هل_ضمني", (AlifCPPFunction)_imp_isBuiltin, METHOD_O},

static AlifObject* _imp_isBuiltinImpl(AlifObject*, AlifObject*);

static AlifObject* _imp_isBuiltin(AlifObject* _module, AlifObject* _arg) { // 330
	AlifObject* returnValue = nullptr;
	AlifObject* name{};

	if (!ALIFUSTR_CHECK(_arg)) {
		//_alifArg_badArgument("is_builtin", "argument", "str", _arg);
		goto exit;
	}
	name = _arg;
	returnValue = _imp_isBuiltinImpl(_module, name);

exit:
	return returnValue;
}

// 353
#define _IMP_IS_FROZEN_METHODDEF    \
    {"هل_مصرف", (AlifCPPFunction)_imp_isFrozen, METHOD_O},

static AlifObject* _imp_isFrozenImpl(AlifObject*, AlifObject*);

static AlifObject* _imp_isFrozen(AlifObject* _module,
	AlifObject* _arg) { // 359
	AlifObject* returnValue = nullptr;
	AlifObject* name{};

	if (!ALIFUSTR_CHECK(_arg)) {
		//_alifArg_badArgument("is_frozen", "argument", "str", _arg);
		goto exit;
	}
	name = _arg;
	returnValue = _imp_isFrozenImpl(_module, name);

exit:
	return returnValue;
}



#if defined(HAVE_DYNAMIC_LOADING) // 456


#define _IMP_CREATE_DYNAMIC_METHODDEF    \
    {"انشئ_حيوي", ALIF_CPPFUNCTION_CAST(_imp_createDynamic), METHOD_FASTCALL},

static AlifObject* _imp_createDynamicImpl(AlifObject*, AlifObject*, AlifObject*);

static AlifObject* _imp_createDynamic(AlifObject* _module,
	AlifObject* const* _args, AlifSizeT _nargs) { // 470
	AlifObject* returnValue{};
	AlifObject* spec{};
	AlifObject* file{};

	if (!_ALIFARG_CHECKPOSITIONAL("انشئ_حيوي", _nargs, 1, 2)) {
		goto exit;
	}
	spec = _args[0];
	if (_nargs < 2) {
		goto skip_optional;
	}
	file = _args[1];
skip_optional:
	returnValue = _imp_createDynamicImpl(_module, spec, file);

exit:
	return returnValue;
}

#endif /* defined(HAVE_DYNAMIC_LOADING) */



// 502
#define _IMP_EXEC_DYNAMIC_METHODDEF    \
    {"نفذ_حيوي", (AlifCPPFunction)_imp_execDynamic, METHOD_O},

static AlifIntT _imp_execDynamicImpl(AlifObject*, AlifObject*);

static AlifObject* _imp_execDynamic(AlifObject* _module, AlifObject* _mod) { // 508
	AlifObject* returnValue{};
	AlifIntT _returnValue{};

	_returnValue = _imp_execDynamicImpl(_module, _mod);
	if ((_returnValue == -1) and alifErr_occurred()) {
		goto exit;
	}
	returnValue = alifLong_fromLong((long)_returnValue);

exit:
	return returnValue;
}


// 530
#define _IMP_EXEC_BUILTIN_METHODDEF    \
    {"نفذ_ضمني", (AlifCPPFunction)_imp_execBuiltin, METHOD_O},

static AlifIntT _imp_execBuiltinImpl(AlifObject*, AlifObject*);

static AlifObject* _imp_execBuiltin(AlifObject* _module, AlifObject* _mod) { // 538
	AlifObject* returnValue{};
	AlifIntT _returnValue{};

	_returnValue = _imp_execBuiltinImpl(_module, _mod);
	if ((_returnValue == -1) and alifErr_occurred()) {
		goto exit;
	}
	returnValue = alifLong_fromLong((long)_returnValue);

exit:
	return returnValue;
}


//* alif //* delete //* todo
#define _IMP_PRINT   \
    {"طبع", (AlifCPPFunction)_imp_print, METHOD_O},

static AlifObject* _imp_printImpl(AlifObject*, AlifObject*);

static AlifObject* _imp_print(AlifObject* _module, AlifObject* _arg) { //* alif //* delete //* todo
	AlifObject* returnValue = nullptr;
	AlifObject* name{};

	if (!ALIFUSTR_CHECK(_arg)) {
		//_alifArg_badArgument("is_builtin", "argument", "str", _arg);
		goto exit;
	}
	name = _arg;
	returnValue = _imp_printImpl(_module, name);

exit:
	return returnValue;
}
