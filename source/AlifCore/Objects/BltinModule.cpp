#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_SysModule.h"

#include "clinic/BltinModule.cpp.h"



static AlifObject* update_bases(AlifObject* _bases, AlifObject* const* _args, AlifSizeT _nargs) { // 25
	AlifSizeT i{}, j{};
	AlifObject* base{}, * meth{}, * new_base{}, * result{}, * new_bases = nullptr;

	for (i = 0; i < _nargs; i++) {
		base = _args[i];
		if (ALIFTYPE_CHECK(base)) {
			if (new_bases) {
				/* If we already have made a replacement, then we append every normal base,
				   otherwise just skip it. */
				if (alifList_append(new_bases, base) < 0) {
					goto error;
				}
			}
			continue;
		}
		if (alifObject_getOptionalAttr(base, &ALIF_ID(__mroEntries__), &meth) < 0) {
			goto error;
		}
		if (!meth) {
			if (new_bases) {
				if (alifList_append(new_bases, base) < 0) {
					goto error;
				}
			}
			continue;
		}
		new_base = alifObject_callOneArg(meth, _bases);
		ALIF_DECREF(meth);
		if (!new_base) {
			goto error;
		}
		if (!ALIFTUPLE_CHECK(new_base)) {
			//alifErr_setString(_alifExcTypeError_,
			//	"__mro_entries__ must return a tuple");
			ALIF_DECREF(new_base);
			goto error;
		}
		if (!new_bases) {
			/* If this is a first successful replacement, create new_bases list and
			   copy previously encountered bases. */
			if (!(new_bases = alifList_new(i))) {
				ALIF_DECREF(new_base);
				goto error;
			}
			for (j = 0; j < i; j++) {
				base = _args[j];
				ALIFLIST_SET_ITEM(new_bases, j, ALIF_NEWREF(base));
			}
		}
		j = ALIFLIST_GET_SIZE(new_bases);
		if (alifList_setSlice(new_bases, j, j, new_base) < 0) {
			ALIF_DECREF(new_base);
			goto error;
		}
		ALIF_DECREF(new_base);
	}
	if (!new_bases) {
		return _bases;
	}
	result = alifList_asTuple(new_bases);
	ALIF_DECREF(new_bases);
	return result;

error:
	ALIF_XDECREF(new_bases);
	return nullptr;
}

static AlifObject* builtin___buildClass__(AlifObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs, AlifObject* _kwnames) { // 97
	AlifObject* func, * name, * winner, * prep;
	AlifObject* cls = nullptr, * cell = nullptr, * ns = nullptr, * meta = nullptr, * orig_bases = nullptr;
	AlifObject* mkw = nullptr, * bases = nullptr;
	AlifIntT isclass = 0;   /* initialize to prevent gcc warning */

	AlifThread* thread{}; // alif

	if (_nargs < 2) {
		//alifErr_setString(_alifExcTypeError_,
		//	"__buildClass__: not enough arguments");
		return nullptr;
	}
	func = _args[0];   /* Better be callable */
	if (!ALIFFUNCTION_CHECK(func)) {
		//alifErr_setString(_alifExcTypeError_,
		//	"__buildClass__: func must be a function");
		return nullptr;
	}
	name = _args[1];
	if (!ALIFUSTR_CHECK(name)) {
		//alifErr_setString(_alifExcTypeError_,
		//	"__buildClass__: name is not a string");
		return nullptr;
	}
	orig_bases = alifTuple_fromArray(_args + 2, _nargs - 2);
	if (orig_bases == nullptr)
		return nullptr;

	bases = update_bases(orig_bases, _args + 2, _nargs - 2);
	if (bases == nullptr) {
		ALIF_DECREF(orig_bases);
		return nullptr;
	}

	if (_kwnames == nullptr) {
		meta = nullptr;
		mkw = nullptr;
	}
	else {
		mkw = alifStack_asDict(_args + _nargs, _kwnames);
		if (mkw == nullptr) {
			goto error;
		}

		if (alifDict_pop(mkw, &ALIF_ID(MetaClass), &meta) < 0) {
			goto error;
		}
		if (meta != nullptr) {
			/* metaclass is explicitly given, check if it's indeed a class */
			isclass = ALIFTYPE_CHECK(meta);
		}
	}
	if (meta == nullptr) {
		/* if there are no bases, use type: */
		if (ALIFTUPLE_GET_SIZE(bases) == 0) {
			meta = (AlifObject*)(&_alifTypeType_);
		}
		/* else get the type of the first base */
		else {
			AlifObject* base0 = ALIFTUPLE_GET_ITEM(bases, 0);
			meta = (AlifObject*)ALIF_TYPE(base0);
		}
		ALIF_INCREF(meta);
		isclass = 1;  /* meta is really a class */
	}

	if (isclass) {
		/* meta is really a class, so check for a more derived
		   metaclass, or possible metaclass conflicts: */
		winner = (AlifObject*)_alifType_calculateMetaclass((AlifTypeObject*)meta,
			bases);
		if (winner == nullptr) {
			goto error;
		}
		if (winner != meta) {
			ALIF_SETREF(meta, ALIF_NEWREF(winner));
		}
	}
	/* else: meta is not a class, so we cannot do the metaclass
	   calculation, so we will use the explicitly given object as it is */
	if (alifObject_getOptionalAttr(meta, &ALIF_ID(__prepare__), &prep) < 0) {
		ns = nullptr;
	}
	else if (prep == nullptr) {
		ns = alifDict_new();
	}
	else {
		AlifObject* pargs[2] = { name, bases };
		ns = alifObject_vectorCallDict(prep, pargs, 2, mkw);
		ALIF_DECREF(prep);
	}
	if (ns == nullptr) {
		goto error;
	}
	if (!alifMapping_check(ns)) {
		//alifErr_format(_alifExcTypeError_,
		//	"%.200s.__prepare__() must return a mapping, not %.200s",
		//	isclass ? ((AlifTypeObject*)meta)->name : "<metaclass>",
		//	ALIF_TYPE(ns)->name);
		goto error;
	}
	thread = _alifThread_get();
	cell = alifEval_vector(thread, (AlifFunctionObject*)func, ns, nullptr, 0, nullptr);
	if (cell != nullptr) {
		if (bases != orig_bases) {
			//if (alifMapping_setItemString(ns, "__origBases__", orig_bases) < 0) {
			//	goto error;
			//}
			printf("تعليق : BltinModule.cpp - builtin___buildClass__"); // alif
		}
		AlifObject* margs[3] = { name, bases, ns };
		cls = alifObject_vectorCallDict(meta, margs, 3, mkw);
		if (cls != nullptr and ALIFTYPE_CHECK(cls) and ALIFCELL_CHECK(cell)) {
			AlifObject* cell_cls = ALIFCELL_GET(cell);
			if (cell_cls != cls) {
				if (cell_cls == nullptr) {
					//const char* msg =
					//	"__class__ not set defining %.200R as %.200R. "
					//	"Was __classCell__ propagated to type.__new__?";
					//alifErr_format(_alifExcRuntimeError_, msg, name, cls);
				}
				else {
					//const char* msg =
					//	"__class__ set to %.200R defining %.200R as %.200R";
					//alifErr_format(_alifExcTypeError_, msg, cell_cls, name, cls);
				}
				ALIF_SETREF(cls, nullptr);
				goto error;
			}
		}
	}
error:
	ALIF_XDECREF(cell);
	ALIF_XDECREF(ns);
	ALIF_XDECREF(meta);
	ALIF_XDECREF(mkw);
	if (bases != orig_bases) {
		ALIF_DECREF(orig_bases);
	}
	ALIF_DECREF(bases);
	return cls;
}










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
	{"__buildClass__", ALIF_CPPFUNCTION_CAST(builtin___buildClass__),
	 METHOD_FASTCALL | METHOD_KEYWORDS},
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

	alifUnstable_moduleSetGIL(mod, ALIF_MOD_GIL_NOT_USED);
	dict = alifModule_getDict(mod);

#define SETBUILTIN(_name, _object) \
    if (alifDict_setItemString(dict, _name, (AlifObject *)_object) < 0)       \
        return nullptr;                                                    \

	SETBUILTIN("عدم", ALIF_NONE);
	SETBUILTIN("Ellipsis", ALIF_ELLIPSIS);
	SETBUILTIN("NotImplemented", ALIF_NOTIMPLEMENTED);
	SETBUILTIN("خطأ", ALIF_FALSE);
	SETBUILTIN("صح", ALIF_TRUE);
	SETBUILTIN("منطق", &_alifBoolType_);
	//SETBUILTIN("memoryview", &_alifMemoryViewType_);
	SETBUILTIN("bytearray", &_alifByteArrayType_);
	SETBUILTIN("bytes", &_alifBytesType_);
	SETBUILTIN("classmethod", &_alifClassMethodType_);
	SETBUILTIN("complex", &_alifComplexType_);
	SETBUILTIN("قاموس", &_alifDictType_);
	//SETBUILTIN("enumerate", &_alifEnumType_);
	//SETBUILTIN("filter", &_alifFilterType_);
	SETBUILTIN("عدد_عشري", &_alifFloatType_);
	SETBUILTIN("frozenset", &_alifFrozenSetType_);
	//SETBUILTIN("property", &_alifPropertyType_);
	SETBUILTIN("عدد_صحيح", &_alifLongType_);
	SETBUILTIN("list", &_alifListType_);
	//SETBUILTIN("map", &_alifMapType_);
	SETBUILTIN("كائن", &_alifBaseObjectType_);
	SETBUILTIN("مدى", &_alifRangeType_);
	//SETBUILTIN("reversed", &_alifReversedType_);
	SETBUILTIN("set", &_alifSetType_);
	SETBUILTIN("slice", &_alifSliceType_);
	//SETBUILTIN("staticmethod", &_alifStaticMethodType_);
	SETBUILTIN("نص", &_alifUStrType_);
	SETBUILTIN("super", &_alifSuperType_);
	SETBUILTIN("مترابطة", &_alifTupleType_);
	SETBUILTIN("نوع", &_alifTypeType_);
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
