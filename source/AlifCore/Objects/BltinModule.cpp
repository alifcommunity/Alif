#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_State.h"
#include "AlifCore_SysModule.h"

#include "BltinModule.h"








static AlifObject* builtin_printImpl(AlifObject* _module, AlifObject* _args,
	AlifObject* _sep, AlifObject* _end, AlifObject* _file, AlifIntT _flush) { // 2058
	AlifIntT i{}, err{};

	if (_file == ALIF_NONE) {
		AlifThread* thread = _alifThread_get();
		_file = _alifSys_getAttr(thread, &ALIF_ID(Stdout));
		if (_file == nullptr) {
		//	//alifErr_setString(_alifExcRuntimeError_, "lost sys.stdout");
			return nullptr;
		}

		if (_file == ALIF_NONE) {
			return ALIF_NONE;
		}
	}

	if (_sep == ALIF_NONE) {
		_sep = nullptr;
	}
	else if (_sep and !ALIFUSTR_CHECK(_sep)) {
		//alifErr_format(_alifExcTypeError_,
		//	"sep must be None or a string, not %.200s",
		//	ALIF_TYPE(sep)->name);
		return nullptr;
	}
	if (_end == ALIF_NONE) {
		_end = nullptr;
	}
	else if (_end and !ALIFUSTR_CHECK(_end)) {
		//alifErr_format(_alifExcTypeError_,
		//	"end must be None or a string, not %.200s",
		//	ALIF_TYPE(end)->name);
		return nullptr;
	}

	for (i = 0; i < ALIFTUPLE_GET_SIZE(_args); i++) {
		if (i > 0) {
			if (_sep == nullptr) {
				err = alifFile_writeString(" ", _file);
			}
			else {
				err = alifFile_writeObject(_sep, _file, ALIF_PRINT_RAW);
			}
			if (err) {
				return nullptr;
			}
		}
		err = alifFile_writeObject(ALIFTUPLE_GET_ITEM(_args, i), _file, ALIF_PRINT_RAW);
		if (err) {
			return nullptr;
		}
	}

	if (_end == nullptr) {
		err = alifFile_writeString("\n", _file);
	}
	else {
		err = alifFile_writeObject(_end, _file, ALIF_PRINT_RAW);
	}
	if (err) {
		return nullptr;
	}

	if (_flush) {
		//if (_alifFile_flush(_file) < 0) {
		//	return nullptr;
		//}
	}

	return ALIF_NONE;
}



static AlifMethodDef builtinMethods[] = { // 3141
	BUILTIN_PRINT_METHODDEF,
	{nullptr, nullptr},
};


static AlifModuleDef _alifBuiltinsModule_ = { // 3202
	ALIFMODULEDEF_HEAD_INIT,
	"builtins",
	nullptr,
	-1,
	builtinMethods,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
};


AlifObject* alifBuiltin_init(AlifInterpreter* _interpreter) { // 3215
	AlifObject* mod{}, * dict{}, *debug{};

	const AlifConfig* config = alifInterpreter_getConfig(_interpreter);

	mod = alifModule_createInitialized(&_alifBuiltinsModule_);
	if (mod == nullptr) return nullptr;

#ifdef ALIF_GIL_DISABLED
	alifUnstable_moduleSetGIL(mod, ALIF_MOD_GIL_NOT_USED);
#endif
	dict = alifModule_getDict(mod);

#define SETBUILTIN(_name, _object) \
    if (alifDict_setItemString(dict, _name, (AlifObject *)_object) < 0)       \
        return nullptr;                                                    \

	SETBUILTIN("None", ALIF_NONE);
	SETBUILTIN("Ellipsis", ALIF_ELLIPSIS);
	SETBUILTIN("NotImplemented", ALIF_NOTIMPLEMENTED);
	SETBUILTIN("False", ALIF_FALSE);
	SETBUILTIN("True", ALIF_TRUE);
	SETBUILTIN("bool", &_alifBoolType_);
	//SETBUILTIN("memoryview", &_alifMemoryViewType_);
	SETBUILTIN("bytearray", &_alifByteArrayType_);
	SETBUILTIN("bytes", &_alifBytesType_);
	//SETBUILTIN("classmethod", &_alifClassMethodType_);
	SETBUILTIN("complex", &_alifComplexType_);
	SETBUILTIN("dict", &_alifDictType_);
	//SETBUILTIN("enumerate", &_alifEnumType_);
	//SETBUILTIN("filter", &_alifFilterType_);
	SETBUILTIN("float", &_alifFloatType_);
	SETBUILTIN("frozenset", &_alifFrozenSetType_);
	//SETBUILTIN("property", &_alifPropertyType_);
	SETBUILTIN("int", &_alifLongType_);
	SETBUILTIN("list", &_alifListType_);
	//SETBUILTIN("map", &_alifMapType_);
	SETBUILTIN("object", &_alifBaseObjectType_);
	//SETBUILTIN("range", &_alifRangeType_);
	//SETBUILTIN("reversed", &_alifReversedType_);
	SETBUILTIN("set", &_alifSetType_);
	SETBUILTIN("slice", &_alifSliceType_);
	//SETBUILTIN("staticmethod", &_alifStaticMethodType_);
	SETBUILTIN("str", &_alifUStrType_);
	//SETBUILTIN("super", &_alifSuperType_);
	SETBUILTIN("tuple", &_alifTupleType_);
	SETBUILTIN("type", &_alifTypeType_);
	debug = alifBool_fromLong(config->optimizationLevel == 0);
	if (alifDict_setItemString(dict, "__debug__", debug) < 0) {
		ALIF_DECREF(debug);
		return nullptr;
	}
	ALIF_DECREF(debug);

	return mod;
#undef ADD_TO_ALL
#undef SETBUILTIN
}
