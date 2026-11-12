#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_State.h"
#include "AlifCore_Run.h"
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
			alifErr_setString(_alifExcTypeError_,
				"__mroEntries__ يجب أن ترجع مترابطة");
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

	AlifThread* thread{}; //* alif

	if (_nargs < 2) {
		alifErr_setString(_alifExcTypeError_,
			"__buildClass__: لم يتم تمرير معاملات كافية");
		return nullptr;
	}
	func = _args[0];   /* Better be callable */
	if (!ALIFFUNCTION_CHECK(func)) {
		alifErr_setString(_alifExcTypeError_,
			"__buildClass__: متغير func يجب أن يكون دالة");
		return nullptr;
	}
	name = _args[1];
	if (!ALIFUSTR_CHECK(name)) {
		alifErr_setString(_alifExcTypeError_,
			"__buildClass__: متغير name يجب أن يكون نص");
		return nullptr;
	}
	orig_bases = _alifTuple_fromArray(_args + 2, _nargs - 2);
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
		mkw = _alifStack_asDict(_args + _nargs, _kwnames);
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
		alifErr_format(_alifExcTypeError_,
			"%.200s.__prepare__() يجب أن ترجع فهرس, وليس %.200s",
			isclass ? ((AlifTypeObject*)meta)->name : "<metaclass>",
			ALIF_TYPE(ns)->name);
		goto error;
	}
	thread = _alifThread_get();
	cell = _alifEval_vector(thread, (AlifFunctionObject*)func, ns, nullptr, 0, nullptr);
	if (cell != nullptr) {
		if (bases != orig_bases) {
			if (alifMapping_setItemString(ns, "__origBases__", orig_bases) < 0) {
				goto error;
			}
		}
		AlifObject* margs[3] = { name, bases, ns };
		cls = alifObject_vectorCallDict(meta, margs, 3, mkw);
		if (cls != nullptr and ALIFTYPE_CHECK(cls) and ALIFCELL_CHECK(cell)) {
			AlifObject* cell_cls = ALIFCELL_GET(cell);
			if (cell_cls != cls) {
				if (cell_cls == nullptr) {
					const char* msg =
						"__صنف__ غير محدد تعريف %.200R كـ %.200R. "
						"هل تم نقل __خلية_صنف__ إلى نوع.__جديد__؟";
					alifErr_format(_alifExcRuntimeError_, msg, name, cls);
				}
				else {
					const char* msg =
						"__صنف__ مضبوط على %.200R مع تعريف %.200R على أنه %.200R";
					alifErr_format(_alifExcTypeError_, msg, cell_cls, name, cls);
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





static AlifObject* builtin___import__Impl(AlifObject* module, AlifObject* name, AlifObject* globals,
	AlifObject* locals, AlifObject* fromlist, AlifIntT level) { // 272
	return alifImport_importModuleLevelObject(name, globals, locals,
		fromlist, level);
}




static AlifObject* builtin_all(AlifObject* _module, AlifObject* _iterable) { // 309
	AlifObject* it{}, * item{};
	AlifObject* (*iternext)(AlifObject*);
	AlifIntT cmp{};

	it = alifObject_getIter(_iterable);
	if (it == nullptr)
		return nullptr;
	iternext = *ALIF_TYPE(it)->iterNext;

	for (;;) {
		item = iternext(it);
		if (item == nullptr)
			break;
		cmp = alifObject_isTrue(item);
		ALIF_DECREF(item);
		if (cmp < 0) {
			ALIF_DECREF(it);
			return nullptr;
		}
		if (cmp == 0) {
			ALIF_DECREF(it);
			ALIF_RETURN_FALSE;
		}
	}
	ALIF_DECREF(it);
	if (alifErr_occurred()) {
		if (alifErr_exceptionMatches(_alifExcStopIteration_))
			alifErr_clear();
		else
			return nullptr;
	}
	ALIF_RETURN_TRUE;
}

static AlifObject* builtin_any(AlifObject* _module, AlifObject* _iterable) { // 358
	AlifObject* it{}, * item{};
	AlifObject* (*iternext)(AlifObject*);
	AlifIntT cmp{};

	it = alifObject_getIter(_iterable);
	if (it == nullptr)
		return nullptr;
	iternext = *ALIF_TYPE(it)->iterNext;

	for (;;) {
		item = iternext(it);
		if (item == nullptr)
			break;
		cmp = alifObject_isTrue(item);
		ALIF_DECREF(item);
		if (cmp < 0) {
			ALIF_DECREF(it);
			return nullptr;
		}
		if (cmp > 0) {
			ALIF_DECREF(it);
			ALIF_RETURN_TRUE;
		}
	}
	ALIF_DECREF(it);
	if (alifErr_occurred()) {
		if (alifErr_exceptionMatches(_alifExcStopIteration_))
			alifErr_clear();
		else
			return nullptr;
	}
	ALIF_RETURN_FALSE;
}




static AlifObject* builtin_binImpl(AlifObject* _module,
	AlifObject* _number, AlifIntT isAscii) { // 430
	return alifNumber_toBase(_number, 2, isAscii);
}

static AlifObject* builtin_octImpl(AlifObject* _module,
	AlifObject* _number, AlifIntT isAscii) { // 2046
	return alifNumber_toBase(_number, 8, isAscii);
}

static AlifObject* builtin_hexImpl(AlifObject* _module,
	AlifObject* _number, AlifIntT _isAscii) { // 1743
	return alifNumber_toBase(_number, 16, _isAscii);
}



static AlifObject* builtin_chr(AlifObject* _module, AlifObject* _i) { // 709
	AlifIntT overflow{};
	long v = alifLong_asLongAndOverflow(_i, &overflow);
	if (v == -1 and alifErr_occurred()) {
		return nullptr;
	}
	if (overflow) {
		v = overflow < 0 ? INT_MIN : INT_MAX;
	}
#if SIZEOF_INT < SIZEOF_LONG
	else if (v < INT_MIN) {
		v = INT_MIN;
	}
	else if (v > INT_MAX) {
		v = INT_MAX;
	}
#endif
	return alifUStr_fromOrdinal(v);
}



static AlifObject* builtin_compileImpl(AlifObject* _module,
	AlifObject* _source, AlifObject* _filename, const char* _mode, AlifIntT _flags,
	AlifIntT _dontInherit, AlifIntT _optimize, AlifIntT _featureVersion) { // 760
	AlifObject* sourceCopy{};
	const char* str{};
	AlifIntT compileMode = -1;
	AlifIntT isAst{};
	AlifIntT start[] = { ALIF_FILE_INPUT, ALIF_EVAL_INPUT, ALIF_SINGLE_INPUT, ALIF_FUNC_TYPE_INPUT };
	AlifObject* result{};

	AlifCompilerFlags cf = ALIFCOMPILERFLAGS_INIT;
	cf.flags = _flags | ALIFCF_SOURCE_IS_UTF8;
	if (_featureVersion >= 0 and (_flags & ALIFCF_ONLY_AST)) {
		cf.featureVersion = _featureVersion;
	}

	if (_flags &
		~(ALIFCF_MASK | ALIFCF_MASK_OBSOLETE | ALIFCF_COMPILE_MASK)) {
		alifErr_setString(_alifExcValueError_,
			"ترجم(): أعلام غير معروفة");
		goto error;
	}

	if (_optimize < -1 or _optimize > 2) {
		alifErr_setString(_alifExcValueError_,
			"ترجم(): قيمة 'تحسين' غير صحيحة");
		goto error;
	}

	if (!_dontInherit) {
		alifEval_mergeCompilerFlags(&cf);
	}

	if (strcmp(_mode, "نفذ") == 0)
		compileMode = 0;
	else if (strcmp(_mode, "نفذ_تعبير") == 0)
		compileMode = 1;
	else if (strcmp(_mode, "نفذ_حالة") == 0)
		compileMode = 2;
	else if (strcmp(_mode, "نفذ_دالة") == 0) {
		if (!(_flags & ALIFCF_ONLY_AST)) {
			alifErr_setString(_alifExcValueError_,
				"ترجم() وضع 'نفذ_دالة' يتطلب علم ALIFCF_ONLY_AST");
			goto error;
		}
		compileMode = 3;
	}
	else {
		const char* msg{};
		if (_flags & ALIFCF_ONLY_AST)
			msg = "ترجم() الوضع يجب أن يكون 'نفذ', 'نفذ_تعبير', 'نفذ_حالة' او 'نفذ_دالة'";
		else
			msg = "ترجم() الوضع يجب أن يكون 'نفذ', 'نفذ_تعبير' او 'نفذ_حالة'";
		alifErr_setString(_alifExcValueError_, msg);
		goto error;
	}

	isAst = alifAST_check(_source);
	if (isAst == -1)
		goto error;
	if (isAst) {
		printf("%s", "هذه الميزة تحت التطوير");
		goto error; 
	//	if ((_flags & ALIFCF_OPTIMIZED_AST) == ALIFCF_ONLY_AST) {
	//		// return an un-optimized AST
	//		result = ALIF_NEWREF(_source);
	//	}
	//	else {
	//		// Return an optimized AST or code object

	//		AlifASTMem* astMem = alifASTMem_new();
	//		if (astMem == nullptr) {
	//			goto error;
	//		}

	//		if (_flags & ALIFCF_ONLY_AST) {
	//			ModuleTy mod = alifAST_obj2mod(_source, astMem, compileMode);
	//			if (mod == nullptr or !_alifAST_validate(mod)) {
	//				alifASTMem_free(astMem);
	//				goto error;
	//			}
	//			if (_alifCompile_astOptimize(mod, _filename, &cf, _optimize,
	//				astMem) < 0) {
	//				alifASTMem_free(astMem);
	//				goto error;
	//			}
	//			result = alifAST_mod2obj(mod);
	//		}
	//		else {
	//			ModuleTy mod = alifAST_obj2mod(_source, astMem, compileMode);
	//			if (mod == nullptr or !_alifAST_validate(mod)) {
	//				alifASTMem_free(astMem);
	//				goto error;
	//			}
	//			result = (AlifObject*)_alifAST_compile(mod, _filename,
	//				&cf, _optimize, astMem);
	//		}
	//		alifASTMem_free(astMem);
	//	}
	//	goto finally;
	}

	str = _alif_sourceAsString(_source, "ترجم", "نص او ثمانيات او شجرة_عقد", &cf, &sourceCopy);
	if (str == nullptr)
		goto error;


	AlifThreadImpl* thread; thread = (AlifThreadImpl*)_alifThread_get();
	thread->suppressCoConstImmortalization++;

	result = alif_compileStringObject(str, _filename, start[compileMode], &cf, _optimize);

	thread->suppressCoConstImmortalization--;

	ALIF_XDECREF(sourceCopy);
	goto finally;

error:
	result = nullptr;
finally:
	ALIF_DECREF(_filename);
	return result;
}



static AlifObject* builtin_execImpl(AlifObject* _module,
	AlifObject* _source, AlifObject* _globals,
	AlifObject* _locals, AlifObject* _closure) { // 1065
	AlifObject* v{};

	if (_globals == ALIF_NONE) {
		_globals = alifEval_getGlobals();
		if (_locals == ALIF_NONE) {
			_locals = _alifEval_getFrameLocals();
			if (_locals == nullptr)
				return nullptr;
		}
		else {
			ALIF_INCREF(_locals);
		}
		if (!_globals || !_locals) {
			alifErr_setString(_alifExcSystemError_,
				"اسماء_عامة واسماء_محلية لا يمكن أن تكون فارغة");
			return nullptr;
		}
	}
	else if (_locals == ALIF_NONE) {
		_locals = ALIF_NEWREF(_globals);
	}
	else {
		ALIF_INCREF(_locals);
	}

	if (!ALIFDICT_CHECK(_globals)) {
		alifErr_format(_alifExcTypeError_, "اسماء_عامة لدالة نفذ() الضمنية يجب أن تكون من نوع فهرس, وليس %.100s",
			ALIF_TYPE(_globals)->name);
		goto error;
	}
	if (!alifMapping_check(_locals)) {
		alifErr_format(_alifExcTypeError_,
			"اسماء_محلية يجب أن تكون من نوع خرائط او عدم, وليس %.100s",
			ALIF_TYPE(_locals)->name);
		goto error;
	}
	AlifIntT r; r = alifDict_contains(_globals, &ALIF_ID(__builtins__));
	if (r == 0) {
		r = alifDict_setItem(_globals, &ALIF_ID(__builtins__), alifEval_getBuiltins());
	}
	if (r < 0) {
		goto error;
	}

	if (_closure == ALIF_NONE) {
		_closure = nullptr;
	}

	if (ALIFCODE_CHECK(_source)) {
		AlifSizeT num_free = alifCode_getNumFree((AlifCodeObject*)_source);
		if (num_free == 0) {
			if (_closure) {
				alifErr_setString(_alifExcTypeError_,
					"لا يمكن إستخدام المغلق مع كائن الشيفرة هذا");
				goto error;
			}
		}
		else {
			int closure_is_ok =
				_closure
				and ALIFTUPLE_CHECKEXACT(_closure)
				and (ALIFTUPLE_GET_SIZE(_closure) == num_free);
			if (closure_is_ok) {
				for (AlifSizeT i = 0; i < num_free; i++) {
					AlifObject* cell = ALIFTUPLE_GET_ITEM(_closure, i);
					if (!ALIFCELL_CHECK(cell)) {
						closure_is_ok = 0;
						break;
					}
				}
			}
			if (!closure_is_ok) {
				alifErr_format(_alifExcTypeError_,
					"كائن الشيفرة يحتاج مغلق بطول يساوي %zd",
					num_free);
				goto error;
			}
		}

		//if (alifSys_audit("نفذ", "O", _source) < 0) {
		//	goto error;
		//}

		if (!_closure) {
			v = alifEval_evalCode(_source, _globals, _locals);
		}
		else {
			v = alifEval_evalCodeEx(_source, _globals, _locals,
				nullptr, 0,
				nullptr, 0,
				nullptr, 0,
				nullptr,
				_closure);
		}
	}
	else {
		if (_closure != nullptr) {
			alifErr_setString(_alifExcTypeError_,
				"يمكن إستخدام المغلق فقط عندما يكون المصدر كائن شيفرة");
		}
		AlifObject* sourceCopy{};
		const char* str{};
		AlifCompilerFlags cf = ALIFCOMPILERFLAGS_INIT;
		cf.flags = ALIFCF_SOURCE_IS_UTF8;
		str = _alif_sourceAsString(_source, "نفذ",
			"نص او ثمانيات او شيفرة", &cf,
			&sourceCopy);
		if (str == nullptr)
			goto error;
		if (alifEval_mergeCompilerFlags(&cf))
			v = alifRun_stringFlags(str, ALIF_FILE_INPUT, _globals,
				_locals, &cf);
		else
			v = ALIFRUN_STRING(str, ALIF_FILE_INPUT, _globals, _locals);
		ALIF_XDECREF(sourceCopy);
	}
	if (v == nullptr)
		goto error;
	ALIF_DECREF(_locals);
	ALIF_DECREF(v);
	return ALIF_NONE;

error:
	ALIF_XDECREF(_locals);
	return nullptr;
}


static AlifObject* builtin_getAttr(AlifObject* self,
	AlifObject* const* args, AlifSizeT nargs) { // 1197
	AlifObject* v{}, * name{}, * result{};

	if (!_ALIFARG_CHECKPOSITIONAL("احضر_صفة", nargs, 2, 3))
		return nullptr;

	v = args[0];
	name = args[1];
	if (nargs > 2) {
		if (alifObject_getOptionalAttr(v, name, &result) == 0) {
			AlifObject* dflt = args[2];
			return ALIF_NEWREF(dflt);
		}
	}
	else {
		result = alifObject_getAttr(v, name);
	}
	return result;
}


static AlifObject* builtin_globalsImpl(AlifObject* _module) { // 1236
	AlifObject* d{};

	d = alifEval_getGlobals();
	return ALIF_XNEWREF(d);
}


static AlifObject* builtin_hasAttrImpl(AlifObject* _module,
	AlifObject* _obj, AlifObject* _name) { // 1259
	AlifObject* v{};

	if (alifObject_getOptionalAttr(_obj, _name, &v) < 0) {
		return nullptr;
	}
	if (v == nullptr) {
		ALIF_RETURN_FALSE;
	}
	ALIF_DECREF(v);
	ALIF_RETURN_TRUE;
}



/* map object ************************************************************/

class MapObject { // 1310
public:
	ALIFOBJECT_HEAD{};
	AlifObject* iters{};
	AlifObject* func{};
	AlifIntT strict{};
};

static AlifObject* map_new(AlifTypeObject* _type,
	AlifObject* _args, AlifObject* _kwds) { // 1317
	AlifObject* it{}, * iters{}, * func{};
	MapObject* lz{};
	AlifSizeT numargs{}, i{};
	AlifIntT strict = 0;

	if (_kwds) {
		AlifObject* empty = alifTuple_new(0);
		if (empty == nullptr) {
			return nullptr;
		}
		static char* kwlist[] = { (char*)"strict", nullptr };
		AlifIntT parsed = alifArg_parseTupleAndKeywords(
			empty, _kwds, "|$p:طبق", kwlist, &strict);
		ALIF_DECREF(empty);
		if (!parsed) {
			return nullptr;
		}
	}

	numargs = alifTuple_size(_args);
	if (numargs < 2) {
		alifErr_setString(_alifExcTypeError_,
			"طبق() يجب أن يمرر لها معاملين على الأقل.");
		return nullptr;
	}

	iters = alifTuple_new(numargs - 1);
	if (iters == nullptr)
		return nullptr;

	for (i = 1; i < numargs; i++) {
		/* Get iterator. */
		it = alifObject_getIter(ALIFTUPLE_GET_ITEM(_args, i));
		if (it == nullptr) {
			ALIF_DECREF(iters);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(iters, i - 1, it);
	}

	/* create mapobject structure */
	lz = (MapObject*)_type->alloc(_type, 0);
	if (lz == nullptr) {
		ALIF_DECREF(iters);
		return nullptr;
	}
	lz->iters = iters;
	func = ALIFTUPLE_GET_ITEM(_args, 0);
	lz->func = ALIF_NEWREF(func);
	lz->strict = strict;

	return (AlifObject*)lz;
}

static AlifObject* map_vectorCall(AlifObject* type, AlifObject* const* args,
	AlifUSizeT nargsf, AlifObject* kwnames) { // 1374
	AlifTypeObject* tp = ALIFTYPE_CAST(type);

	AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);
	if (kwnames != nullptr and ALIFTUPLE_GET_SIZE(kwnames) != 0) {
		// Fallback to map_new()
		AlifThread* tstate = _alifThread_get();
		return alifObject_makeTpCall(tstate, type, args, nargs, kwnames);
	}

	if (nargs < 2) {
		alifErr_setString(_alifExcTypeError_,
			"طبق() يجب أن يمرر لها معاملين على الأقل.");
		return nullptr;
	}

	AlifObject* iters = alifTuple_new(nargs - 1);
	if (iters == nullptr) {
		return nullptr;
	}

	for (int i = 1; i < nargs; i++) {
		AlifObject* it = alifObject_getIter(args[i]);
		if (it == nullptr) {
			ALIF_DECREF(iters);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(iters, i - 1, it);
	}

	MapObject* lz = (MapObject*)tp->alloc(tp, 0);
	if (lz == nullptr) {
		ALIF_DECREF(iters);
		return nullptr;
	}
	lz->iters = iters;
	lz->func = ALIF_NEWREF(args[0]);
	lz->strict = 0;

	return (AlifObject*)lz;
}

static void map_dealloc(MapObject* lz) { // 1419
	alifObject_gcUnTrack(lz);
	ALIF_XDECREF(lz->iters);
	ALIF_XDECREF(lz->func);
	ALIF_TYPE(lz)->free(lz);
}

static AlifObject* map_next(MapObject* _lz) { // 1436
	AlifSizeT i{};
	AlifObject* smallStack[ALIF_FASTCALL_SMALL_STACK]{};
	AlifObject** stack{};
	AlifObject* result = nullptr;
	AlifThread* tstate = _alifThread_get();

	const AlifSizeT niters = ALIFTUPLE_GET_SIZE(_lz->iters);
	if (niters <= (AlifSizeT)ALIF_ARRAY_LENGTH(smallStack)) {
		stack = smallStack;
	}
	else {
		stack = (AlifObject**)alifMem_dataAlloc(niters * sizeof(stack[0]));
		if (stack == nullptr) {
			//_alifErr_noMemory(tstate);
			return nullptr;
		}
	}

	AlifSizeT nargs = 0;
	for (i = 0; i < niters; i++) {
		AlifObject* it = ALIFTUPLE_GET_ITEM(_lz->iters, i);
		AlifObject* val = ALIF_TYPE(it)->iterNext(it);
		if (val == nullptr) {
			if (_lz->strict) {
				goto check;
			}
			goto exit;
		}
		stack[i] = val;
		nargs++;
	}

	result = _alifObject_vectorCallThread(tstate, _lz->func, stack, nargs, nullptr);

exit:
	for (i = 0; i < nargs; i++) {
		ALIF_DECREF(stack[i]);
	}
	if (stack != smallStack) {
		alifMem_dataFree(stack);
	}
	return result;
check:
	if (alifErr_occurred()) {
		if (!alifErr_exceptionMatches(_alifExcStopIteration_)) {
			return nullptr;
		}
		alifErr_clear();
	}
	if (i) {
		const char* plural = i == 1 ? " " : "ات 1-";
		return alifErr_format(_alifExcValueError_,
			"طبق() المعامل %d اقصر من المعامل%s%d",
			i + 1, plural, i);
	}
	for (i = 1; i < niters; i++) {
		AlifObject* it = ALIFTUPLE_GET_ITEM(_lz->iters, i);
		AlifObject* val = (*ALIF_TYPE(it)->iterNext)(it);
		if (val) {
			ALIF_DECREF(val);
			const char* plural = i == 1 ? " " : "ات 1-";
			return alifErr_format(_alifExcValueError_,
				"طبق() المعامل %d اطول من المعامل%s%d",
				i + 1, plural, i);
		}
		if (alifErr_occurred()) {
			if (!alifErr_exceptionMatches(_alifExcStopIteration_)) {
				return nullptr;
			}
			alifErr_clear();
		}
	}
	goto exit;
}


AlifTypeObject _alifMapType_ = { // 1570
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "طبق",
	.basicSize = sizeof(MapObject),
	/* methods */
	.dealloc = (Destructor)map_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE,
	//.traverse = (TraverseProc)map_traverse,
	.iter = alifObject_selfIter,
	.iterNext = (IterNextFunc)map_next,
	//.methods = _mapMethods_,
	.alloc = alifType_genericAlloc,
	.new_ = map_new,
	.free = alifObject_gcDel,
	.vectorCall = (VectorCallFunc)map_vectorCall
};




static AlifObject* builtin_setAttrImpl(AlifObject* _module,
	AlifObject* _obj, AlifObject* _name,
	AlifObject* value) { // 1672
	if (alifObject_setAttr(_obj, _name, value) != 0)
		return nullptr;
	return ALIF_NONE;
}



static AlifObject* builtin_delAttrImpl(AlifObject* _module,
	AlifObject* _obj, AlifObject* _name) { // 1695
	if (alifObject_delAttr(_obj, _name) < 0) {
		return nullptr;
	}
	return ALIF_NONE;
}



static AlifObject* builtin_len(AlifObject* _module, AlifObject* _obj) { // 1762
	AlifSizeT res{};

	res = alifObject_size(_obj);
	if (res < 0) {
		return nullptr;
	}
	return alifLong_fromSizeT(res);
}



static AlifObject* builtin_localsImpl(AlifObject* _module) { // 1873
	return _alifEval_getFrameLocals();
}




static AlifObject* min_max(AlifObject* const* args, AlifSizeT nargs,
	AlifObject* kwnames, AlifIntT op) { // 1795
	AlifObject* it = nullptr, * item{}, * val{}, * maxitem{}, * maxval{}, * keyfunc = nullptr;
	AlifObject* defaultval = nullptr;
	static const char* const keywords[] = { "مفتاح", "إفتراضي", nullptr };
	static AlifArgParser parserMin = { "|$OO:min", keywords, 0 };
	static AlifArgParser parserMax = { "|$OO:max", keywords, 0 };
	const char* name = (op == ALIF_LT) ? "ادنى" : "اقصى";
	AlifArgParser* parser_ = (op == ALIF_LT) ? &parserMin : &parserMax;

	if (nargs == 0) {
		alifErr_format(_alifExcTypeError_, "%s يجب تمرير معامل واحد على الأقل, ولكن تم تمرير 0", name);
		return nullptr;
	}

	if (kwnames != nullptr and !_alifArg_parseStackAndKeywords(args + nargs, 0, kwnames, parser_,
		&keyfunc, &defaultval)) {
		return nullptr;
	}

	const AlifIntT positional = nargs > 1;
	if (positional and defaultval != nullptr) {
		alifErr_format(_alifExcTypeError_,
			"لا يمكن تحديد قيمة إفتراضية لـ %s() مع "
			"معاملات مكانية متعددة", name);
		return nullptr;
	}

	if (!positional) {
		it = alifObject_getIter(args[0]);
		if (it == nullptr) {
			return nullptr;
		}
	}

	if (keyfunc == ALIF_NONE) {
		keyfunc = nullptr;
	}

	maxitem = nullptr; /* the result */
	maxval = nullptr;  /* the value associated with the result */
	while (1) {
		if (it == nullptr) {
			if (nargs-- <= 0) {
				break;
			}
			item = *args++;
			ALIF_INCREF(item);
		}
		else {
			item = alifIter_next(it);
			if (item == nullptr) {
				if (alifErr_occurred()) {
					goto Fail_it;
				}
				break;
			}
		}

		/* get the value from the key function */
		if (keyfunc != nullptr) {
			val = alifObject_callOneArg(keyfunc, item);
			if (val == nullptr)
				goto Fail_it_item;
		}
		/* no key function; the value is the item */
		else {
			val = ALIF_NEWREF(item);
		}

		/* maximum value and item are unset; set them */
		if (maxval == nullptr) {
			maxitem = item;
			maxval = val;
		}
		/* maximum value and item are set; update them as necessary */
		else {
			AlifIntT cmp = alifObject_richCompareBool(val, maxval, op);
			if (cmp < 0)
				goto Fail_it_item_and_val;
			else if (cmp > 0) {
				ALIF_DECREF(maxval);
				ALIF_DECREF(maxitem);
				maxval = val;
				maxitem = item;
			}
			else {
				ALIF_DECREF(item);
				ALIF_DECREF(val);
			}
		}
	}
	if (maxval == nullptr) {
		if (defaultval != nullptr) {
			maxitem = ALIF_NEWREF(defaultval);
		}
		else {
			alifErr_format(_alifExcValueError_,
				"%s() معامل التكرار فارغ", name);
		}
	}
	else {
		ALIF_DECREF(maxval);
	}

	ALIF_XDECREF(it);
	return maxitem;

Fail_it_item_and_val:
	ALIF_DECREF(val);
Fail_it_item:
	ALIF_DECREF(item);
Fail_it:
	ALIF_XDECREF(maxval);
	ALIF_XDECREF(maxitem);
	ALIF_XDECREF(it);
	return nullptr;
}



static AlifObject* builtin_min(AlifObject* _self, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 1914
	return min_max(_args, _nargs, _kwnames, ALIF_LT);
}

static AlifObject* builtin_max(AlifObject* _self, AlifObject* const* _args,
	AlifSizeT _nargs, AlifObject* _kwnames) { // 1931
	return min_max(_args, _nargs, _kwnames, ALIF_GT);
}


static AlifObject* builtin_ord(AlifObject* _module,
	AlifObject* _c) { // 2062
	long ord{};
	AlifSizeT size{};

	if (ALIFBYTES_CHECK(_c)) {
		size = ALIFBYTES_GET_SIZE(_c);
		if (size == 1) {
			ord = (long)((unsigned char)*ALIFBYTES_AS_STRING(_c));
			return alifLong_fromLong(ord);
		}
	}
	else if (ALIFUSTR_CHECK(_c)) {
		size = ALIFUSTR_GET_LENGTH(_c);
		if (size == 1) {
			ord = (long)ALIFUSTR_READ_CHAR(_c, 0);
			return alifLong_fromLong(ord);
		}
	}
	else if (ALIFBYTEARRAY_CHECK(_c)) {
		/* XXX Hopefully this is temporary */
		size = ALIFBYTEARRAY_GET_SIZE(_c);
		if (size == 1) {
			ord = (long)((unsigned char)*ALIFBYTEARRAY_AS_STRING(_c));
			return alifLong_fromLong(ord);
		}
	}
	else {
		alifErr_format(_alifExcTypeError_,
			"رمز() يتوقع تمرير نص مكون من حرف واحد, ولكن " \
			"تم تمرير %.200s", ALIF_TYPE(_c)->name);
		return nullptr;
	}

	alifErr_format(_alifExcTypeError_,
		"رمز() يتوقع تمرير حرف واحد, "
		"ولكن تم تمرير نص بطول %zd",
		size);
	return nullptr;
}



static AlifObject* builtin_printImpl(AlifObject* _module, AlifObject* const* _args,
	AlifSizeT _argsLength, AlifObject* _sep, AlifObject* _end,
	AlifObject* _file, AlifIntT _flush) { // 2144
	AlifIntT i{}, err{};

	if (_file == ALIF_NONE) {
		AlifThread* thread = _alifThread_get();
		_file = _alifSys_getAttr(thread, &ALIF_STR(Stdout));
		if (_file == nullptr) {
			alifErr_setString(_alifExcRuntimeError_, "مفقود النظام.مخرجات");
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
		alifErr_format(_alifExcTypeError_,
			"الفاصلة يجب أن تكون عدم او نص, وليس %.200s",
			ALIF_TYPE(_sep)->name);
		return nullptr;
	}
	if (_end == ALIF_NONE) {
		_end = nullptr;
	}
	else if (_end and !ALIFUSTR_CHECK(_end)) {
		alifErr_format(_alifExcTypeError_,
			"النهاية يجب أن تكون عدم او نص, وليس %.200s",
			ALIF_TYPE(_end)->name);
		return nullptr;
	}

	for (i = 0; i < _argsLength; i++) {
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
		err = alifFile_writeObject(_args[i], _file, ALIF_PRINT_RAW);
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
		if (_alifFile_flush(_file) < 0) {
			return nullptr;
		}
	}

	return ALIF_NONE;
}



static AlifObject* builtin_inputImpl(AlifObject* module, AlifObject* prompt) { // 2151
	AlifThread* thread = _alifThread_get();
	AlifObject* fin = _alifSys_getAttr(thread, &ALIF_ID(Stdin));
	AlifObject* fout = _alifSys_getAttr(thread, &ALIF_STR(Stdout));
	AlifObject* ferr = _alifSys_getAttr(thread, &ALIF_ID(Stderr));
	AlifObject* tmp{};
	long fd{};
	AlifIntT tty{};

	/* Check that stdin/out/err are intact */
	if (fin == nullptr or fin == ALIF_NONE) {
		alifErr_setString(_alifExcRuntimeError_,
			"ادخل(): مفقود النظام.الإدخال");
		return nullptr;
	}
	if (fout == nullptr or fout == ALIF_NONE) {
		alifErr_setString(_alifExcRuntimeError_,
			"ادخل(): مفقود النظام.الإخراج");
		return nullptr;
	}
	if (ferr == nullptr or ferr == ALIF_NONE) {
		alifErr_setString(_alifExcRuntimeError_,
			"ادخل(): مفقود النظام.الخطأ");
		return nullptr;
	}

	//if (alifSys_audit("builtins.input", "O", prompt ? prompt : ALIF_NONE) < 0) {
	//	return nullptr;
	//}

	/* First of all, flush stderr */
	if (_alifFile_flush(ferr) < 0) {
		alifErr_clear();
	}

	/* We should only use (GNU) readline if Alif's sys.stdin and
	   sys.stdout are the same as CPP's stdin and stdout, because we
	   need to pass it those. */
	tmp = alifObject_callMethodNoArgs(fin, &ALIF_ID(Fileno));
	if (tmp == nullptr) {
		alifErr_clear();
		tty = 0;
	}
	else {
		fd = alifLong_asLong(tmp);
		ALIF_DECREF(tmp);
		if (fd < 0 and alifErr_occurred())
			return nullptr;
		tty = fd == fileno(stdin) and isatty(fd);
	}
	if (tty) {
		tmp = alifObject_callMethodNoArgs(fout, &ALIF_ID(Fileno));
		if (tmp == nullptr) {
			alifErr_clear();
			tty = 0;
		}
		else {
			fd = alifLong_asLong(tmp);
			ALIF_DECREF(tmp);
			if (fd < 0 and alifErr_occurred())
				return nullptr;
			tty = fd == fileno(stdout) and isatty(fd);
		}
	}

	/* If we're interactive, use (GNU) readline */
	if (tty) {
		AlifObject* po = nullptr;
		const char* promptstr{};
		char* s = nullptr;
		AlifObject* stdinEncoding = nullptr, * stdinErrors = nullptr;
		AlifObject* stdoutEncoding = nullptr, * stdoutErrors = nullptr;
		const char* stdinEncodingStr, * stdinErrorsStr;
		AlifObject* result{};
		AlifUSizeT len{};

		/* stdin is a text stream, so it must have an encoding. */
		stdinEncoding = alifObject_getAttr(fin, &ALIF_ID(Encoding));
		if (stdinEncoding == nullptr) {
			tty = 0;
			goto _readline_errors;
		}
		stdinErrors = alifObject_getAttr(fin, &ALIF_ID(Errors));
		if (stdinErrors == nullptr) {
			tty = 0;
			goto _readline_errors;
		}
		if (!ALIFUSTR_CHECK(stdinEncoding) or
			!ALIFUSTR_CHECK(stdinErrors)) {
			tty = 0;
			goto _readline_errors;
		}
		stdinEncodingStr = alifUStr_asUTF8(stdinEncoding);
		if (stdinEncodingStr == nullptr) {
			goto _readline_errors;
		}
		stdinErrorsStr = alifUStr_asUTF8(stdinErrors);
		if (stdinErrorsStr == nullptr) {
			goto _readline_errors;
		}
		if (_alifFile_flush(fout) < 0) {
			alifErr_clear();
		}
		if (prompt != nullptr) {
			/* We have a prompt, encode it as stdout would */
			const char* stdoutEncodingStr{}, * stdoutErrorsStr{};
			AlifObject* stringpo{};
			stdoutEncoding = alifObject_getAttr(fout, &ALIF_ID(Encoding));
			if (stdoutEncoding == nullptr) {
				tty = 0;
				goto _readline_errors;
			}
			stdoutErrors = alifObject_getAttr(fout, &ALIF_ID(Errors));
			if (stdoutErrors == nullptr) {
				tty = 0;
				goto _readline_errors;
			}
			if (!ALIFUSTR_CHECK(stdoutEncoding) ||
				!ALIFUSTR_CHECK(stdoutErrors)) {
				tty = 0;
				goto _readline_errors;
			}
			stdoutEncodingStr = alifUStr_asUTF8(stdoutEncoding);
			if (stdoutEncodingStr == nullptr) {
				goto _readline_errors;
			}
			stdoutErrorsStr = alifUStr_asUTF8(stdoutErrors);
			if (stdoutErrorsStr == nullptr) {
				goto _readline_errors;
			}
			stringpo = alifObject_str(prompt);
			if (stringpo == nullptr)
				goto _readline_errors;
			po = alifUStr_asEncodedString(stringpo,
				stdoutEncodingStr, stdoutErrorsStr);
			ALIF_CLEAR(stdoutEncoding);
			ALIF_CLEAR(stdoutErrors);
			ALIF_CLEAR(stringpo);
			if (po == nullptr)
				goto _readline_errors;
			promptstr = ALIFBYTES_AS_STRING(po);
			if ((AlifSizeT)strlen(promptstr) != ALIFBYTES_GET_SIZE(po)) {
				alifErr_setString(_alifExcValueError_,
					"ادخل: النص المدخل لا يمكن أن يحتوي على حروف فارغة");
				goto _readline_errors;
			}
		}
		else {
			po = nullptr;
			promptstr = "";
		}
		s = alifOS_readline(stdin, stdout, promptstr);
		if (s == nullptr) {
			//alifErr_checkSignals();
			if (!alifErr_occurred())
				//alifErr_setNone(_alifExcKeyboardInterrupt_);
				goto _readline_errors;
		}

		len = strlen(s);
		if (len == 0) {
			//alifErr_setNone(_alifExcEOFError_);
			result = nullptr;
		}
		else {
			if (len > ALIF_SIZET_MAX) {
				alifErr_setString(_alifExcOverflowError_,
					"ادخل: النص المدخل طويل جداً");
				result = nullptr;
			}
			else {
				len--;   /* strip trailing '\n' */
				if (len != 0 and s[len - 1] == '\r')
					len--;   /* strip trailing '\r' */
				result = alifUStr_decode(s, len, stdinEncodingStr,
					stdinErrorsStr);
			}
		}
		ALIF_DECREF(stdinEncoding);
		ALIF_DECREF(stdinErrors);
		ALIF_XDECREF(po);
		alifMem_dataFree(s);

		if (result != nullptr) {
			//if (alifSys_audit("builtins.input/result", "O", result) < 0) {
			//	return nullptr;
			//}
		}

		return result;

_readline_errors:
		ALIF_XDECREF(stdinEncoding);
		ALIF_XDECREF(stdoutEncoding);
		ALIF_XDECREF(stdinErrors);
		ALIF_XDECREF(stdoutErrors);
		ALIF_XDECREF(po);
		if (tty)
			return nullptr;

		alifErr_clear();
	}

	/* Fallback if we're not interactive */
	if (prompt != nullptr) {
		if (alifFile_writeObject(prompt, fout, ALIF_PRINT_RAW) != 0)
			return nullptr;
	}
	if (_alifFile_flush(fout) < 0) {
		alifErr_clear();
	}
	return alifFile_getLine(fin, -1);
}



static AlifObject* builtin_roundImpl(AlifObject* _module,
	AlifObject* _number, AlifObject* _ndigits) { // 2493
	AlifObject* round{}, * result{};

	round = _alifObject_lookupSpecial(_number, &ALIF_STR(__round__));
	if (round == nullptr) {
		if (!alifErr_occurred())
			alifErr_format(_alifExcTypeError_,
				"النوع %.100s لا يعرف وظيفة __قرب__",
				ALIF_TYPE(_number)->name);
		return nullptr;
	}

	if (_ndigits == ALIF_NONE)
		result = _alifObject_callNoArgs(round);
	else
		result = alifObject_callOneArg(round, _ndigits);
	ALIF_DECREF(round);
	return result;
}



// 2543
#define BUILTIN_SORTED_METHODDEF    \
    {"مرتب", ALIF_CPPFUNCTION_CAST(builtin_sorted), METHOD_FASTCALL | METHOD_KEYWORDS}

static AlifObject* builtin_sorted(AlifObject* _self,
	AlifObject* const* _args, AlifSizeT _nargs, AlifObject* _kwnames) { // 2546
	AlifObject* newlist{}, * v{}, * seq{}, * callable{};

	/* Keyword arguments are passed through مصفوفة.رتب() which will check
	them. */
	if (!_alifArg_unpackStack(_args, _nargs, "مرتب", 1, 1, &seq))
		return nullptr;

	newlist = alifSequence_list(seq);
	if (newlist == nullptr)
		return nullptr;

	callable = alifObject_getAttr(newlist, &ALIF_STR(Sort));
	if (callable == nullptr) {
		ALIF_DECREF(newlist);
		return nullptr;
	}

	v = alifObject_vectorCall(callable, _args + 1, _nargs - 1, _kwnames);
	ALIF_DECREF(callable);
	if (v == nullptr) {
		ALIF_DECREF(newlist);
		return nullptr;
	}
	ALIF_DECREF(v);
	return newlist;
}





class CompensatedSum { // 2534
public:
	double hi{};     /* high-order bits for a running sum */
	double lo{};     /* a running compensation for lost low-order bits */
};

static inline CompensatedSum cs_fromDouble(double _x) { // 2539
	return {_x};
}

static inline CompensatedSum cs_add(CompensatedSum _total,
	double _x) {
	double t = _total.hi + _x;
	if (fabs(_total.hi) >= fabs(_x)) {
		_total.lo += (_total.hi - t) + _x;
	}
	else {
		_total.lo += (_x - t) + _total.hi;
	}
	return {t, _total.lo};
}

static inline double cs_toDouble(CompensatedSum _total) {
	/* Avoid losing the sign on a negative result,
	and don't let adding the compensation convert
	an infinite or overflowed sum to a NaN (not a number). */
	if (_total.lo and isfinite(_total.lo)) {
		return _total.hi + _total.lo;
	}
	return _total.hi;
}

static AlifObject* builtin_sumImpl(AlifObject* module,
	AlifObject* iterable, AlifObject* start) { // 2584
	AlifObject* result = start;
	AlifObject* temp{}, * item{}, * iter{};

	iter = alifObject_getIter(iterable);
	if (iter == nullptr)
		return nullptr;

	if (result == nullptr) {
		result = alifLong_fromLong(0);
		if (result == nullptr) {
			ALIF_DECREF(iter);
			return nullptr;
		}
	} else {
		/* reject string values for 'start' parameter */
		if (ALIFUSTR_CHECK(result)) {
			alifErr_setString(_alifExcTypeError_,
				"اجمع() لا تستطيع جمع النصوص [بدل من ذلك استخدم ''.ضم(سلسلة)]");
			ALIF_DECREF(iter);
			return nullptr;
		}
		if (ALIFBYTES_CHECK(result)) {
			//alifErr_setString(_alifExcTypeError_,
			//	"اجمع() لا تستطيع جمع الثمانيات [بدل من ذلك استخدم ب''.ضم(سلسلة)]");
			ALIF_DECREF(iter);
			return nullptr;
		}
		if (ALIFBYTEARRAY_CHECK(result)) {
			//alifErr_setString(_alifExcTypeError_,
			//	"اجمع() لا تستطيع جمع مصفوفة الثمانيات [بدل من ذلك استخدم ب''.ضم(سلسلة)]");
			ALIF_DECREF(iter);
			return nullptr;
		}
		ALIF_INCREF(result);
	}

#ifndef SLOW_SUM
	/* Fast addition by keeping temporary sums in CPP instead of new Alif objects.
	Assumes all inputs are the same type.  If the assumption fails, default
	to the more general routine.
	*/
	if (ALIFLONG_CHECKEXACT(result)) {
		AlifIntT overflow{};
		AlifSizeT i_result = alifLong_asLongAndOverflow(result, &overflow);
		/* If this already overflowed, don't even enter the loop. */
		if (overflow == 0) {
			ALIF_SETREF(result, nullptr);
		}
		while(result == nullptr) {
			item = alifIter_next(iter);
			if (item == nullptr) {
				ALIF_DECREF(iter);
				if (alifErr_occurred())
					return nullptr;
				return alifLong_fromSizeT(i_result);
			}
			if (ALIFLONG_CHECKEXACT(item) or ALIFBOOL_CHECK(item)) {
				AlifSizeT b{};
				overflow = 0;
				/* Single digits are common, fast, and cannot overflow on unpacking. */
				if (alifLong_isCompact((AlifLongObject*)item)) {
					b = alifLong_compactValue((AlifLongObject*)item);
				}
				else {
					b = alifLong_asLongAndOverflow(item, &overflow);
				}
				if (overflow == 0 and
					(i_result >= 0 ? (b <= ALIF_SIZET_MAX - i_result)
						: (b >= ALIF_SIZET_MIN - i_result)))
				{
					i_result += b;
					ALIF_DECREF(item);
					continue;
				}
			}
			/* Either overflowed or is not an int. Restore real objects and process normally */
			result = alifLong_fromSizeT(i_result);
			if (result == nullptr) {
				ALIF_DECREF(item);
				ALIF_DECREF(iter);
				return nullptr;
			}
			temp = alifNumber_add(result, item);
			ALIF_DECREF(result);
			ALIF_DECREF(item);
			result = temp;
			if (result == nullptr) {
				ALIF_DECREF(iter);
				return nullptr;
			}
		}
	}

	if (ALIFFLOAT_CHECKEXACT(result)) {
		CompensatedSum re_sum = cs_fromDouble(ALIFFLOAT_AS_DOUBLE(result));
		ALIF_SETREF(result, nullptr);
		while(result == nullptr) {
			item = alifIter_next(iter);
			if (item == nullptr) {
				ALIF_DECREF(iter);
				if (alifErr_occurred())
					return nullptr;
				return alifFloat_fromDouble(cs_toDouble(re_sum));
			}
			if (ALIFFLOAT_CHECKEXACT(item)) {
				re_sum = cs_add(re_sum, ALIFFLOAT_AS_DOUBLE(item));
				_alif_decrefSpecialized(item, _alifFloat_exactDealloc);
				continue;
			}
			if (ALIFLONG_CHECK(item)) {
				double value = alifLong_asDouble(item);
				if (value != -1.0 or !alifErr_occurred()) {
					re_sum = cs_add(re_sum, value);
					ALIF_DECREF(item);
					continue;
				}
				else {
					ALIF_DECREF(item);
					ALIF_DECREF(iter);
					return nullptr;
				}
			}
			result = alifFloat_fromDouble(cs_toDouble(re_sum));
			if (result == nullptr) {
				ALIF_DECREF(item);
				ALIF_DECREF(iter);
				return nullptr;
			}
			temp = alifNumber_add(result, item);
			ALIF_DECREF(result);
			ALIF_DECREF(item);
			result = temp;
			if (result == nullptr) {
				ALIF_DECREF(iter);
				return nullptr;
			}
		}
	}

	/*if (ALIFCOMPLEX_CHECKEXACT(result)) {
		AlifComplex z = alifComplex_asCComplex(result);
		CompensatedSum re_sum = cs_fromDouble(z.real);
		CompensatedSum im_sum = cs_fromDouble(z.imag);
		ALIF_SETREF(result, nullptr);
		while (result == nullptr) {
			item = alifIter_next(iter);
			if (item == nullptr) {
				ALIF_DECREF(iter);
				if (alifErr_occurred()) {
					return nullptr;
				}
				return alifComplex_fromDoubles(cs_toDouble(re_sum),
					cs_toDouble(im_sum));
			}
			if (ALIFCOMPLEX_CHECKEXACT(item)) {
				z = alifComplex_asCComplex(item);
				re_sum = cs_add(re_sum, z.real);
				im_sum = cs_add(im_sum, z.imag);
				ALIF_DECREF(item);
				continue;
			}
			if (ALIFLONG_CHECK(item)) {
				double value = alifLong_asDouble(item);
				if (value != -1.0 or !alifErr_occurred()) {
					re_sum = cs_add(re_sum, value);
					im_sum.hi += 0.0;
					ALIF_DECREF(item);
					continue;
				}
				else {
					ALIF_DECREF(item);
					ALIF_DECREF(iter);
					return nullptr;
				}
			}
			if (ALIFFLOAT_CHECK(item)) {
				double value = alifFLOAT_AS_DOUBLE(item);
				re_sum = cs_add(re_sum, value);
				im_sum.hi += 0.0;
				_alif_decRefSpecialized(item, _alifFloat_exactDealloc);
				continue;
			}
			result = alifComplex_fromDoubles(cs_toDouble(re_sum),
				cs_toDouble(im_sum));
			if (result == nullptr) {
				ALIF_DECREF(item);
				ALIF_DECREF(iter);
				return nullptr;
			}
			temp = alifNumber_add(result, item);
			ALIF_DECREF(result);
			ALIF_DECREF(item);
			result = temp;
			if (result == nullptr) {
				ALIF_DECREF(iter);
				return nullptr;
			}
		}
	}*/
#endif

	for(;;) {
		item = alifIter_next(iter);
		if (item == nullptr) {
			/* error, or end-of-sequence */
			if (alifErr_occurred()) {
				ALIF_SETREF(result, nullptr);
			}
			break;
		}
		/* It's tempting to use alifNumber_inPlaceAdd instead of
		alifNumber_add here, to avoid quadratic running time
		when doing 'sum(list_of_lists, [])'.  However, this
		would produce a change in behaviour: a snippet like

		empty = []
		sum([[x] for x in range(10)], empty)

		would change the value of empty. In fact, using
		in-place addition rather that binary addition for
		any of the steps introduces subtle behavior changes: */
		temp = alifNumber_add(result, item);
		ALIF_DECREF(result);
		ALIF_DECREF(item);
		result = temp;
		if (result == nullptr)
			break;
	}
	ALIF_DECREF(iter);
	return result;
}


static AlifObject * builtin_isInstanceImpl(AlifObject *_module, AlifObject *_obj,
	AlifObject *_classOrTuple) { // 2837
	AlifIntT retval{};

	retval = alifObject_isInstance(_obj, _classOrTuple);
	if (retval < 0)
		return nullptr;
	return alifBool_fromLong(retval);
}


class ZipObject { // 2878
public:
	ALIFOBJECT_HEAD{};
	AlifSizeT tupleSize{};
	AlifObject* itTuple{};     /* tuple of iterators */
	AlifObject* result;
	AlifIntT strict{};
};

static AlifObject* zip_new(AlifTypeObject* type,
	AlifObject* args, AlifObject* kwds) { // 2886
	ZipObject* lz{};
	AlifSizeT i{};
	AlifObject* ittuple{};  /* tuple of iterators */
	AlifObject* result{};
	AlifSizeT tupleSize{};
	AlifIntT strict = 0;

	if (kwds) {
		AlifObject* empty = alifTuple_new(0);
		if (empty == nullptr) {
			return nullptr;
		}
		static char* kwlist[] = { (char*)"دقيق", nullptr };
		AlifIntT parsed = alifArg_parseTupleAndKeywords(
			empty, kwds, "|$p:مقرون", kwlist, &strict);
		ALIF_DECREF(empty);
		if (!parsed) {
			return nullptr;
		}
	}

	/* args must be a tuple */
	tupleSize = ALIFTUPLE_GET_SIZE(args);

	/* obtain iterators */
	ittuple = alifTuple_new(tupleSize);
	if (ittuple == nullptr)
		return nullptr;
	for (i=0; i < tupleSize; ++i) {
		AlifObject* item = ALIFTUPLE_GET_ITEM(args, i);
		AlifObject* it = alifObject_getIter(item);
		if (it == nullptr) {
			ALIF_DECREF(ittuple);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(ittuple, i, it);
	}

	/* create a result holder */
	result = alifTuple_new(tupleSize);
	if (result == nullptr) {
		ALIF_DECREF(ittuple);
		return nullptr;
	}
	for (i=0 ; i < tupleSize ; i++) {
		ALIFTUPLE_SET_ITEM(result, i, ALIF_NEWREF(ALIF_NONE));
	}

	/* create zipobject structure */
	lz = (ZipObject*)type->alloc(type, 0);
	if (lz == nullptr) {
		ALIF_DECREF(ittuple);
		ALIF_DECREF(result);
		return nullptr;
	}
	lz->itTuple = ittuple;
	lz->tupleSize = tupleSize;
	lz->result = result;
	lz->strict = strict;

	return (AlifObject*)lz;
}

static void zip_dealloc(ZipObject* lz) { // 2953
	alifObject_gcUnTrack(lz);
	ALIF_XDECREF(lz->itTuple);
	ALIF_XDECREF(lz->result);
	ALIF_TYPE(lz)->free(lz);
}


static AlifObject* zip_next(ZipObject* lz) { // 2970
	AlifSizeT i{};
	AlifSizeT tuplesize = lz->tupleSize;
	AlifObject *result = lz->result;
	AlifObject *it{};
	AlifObject *item{};
	AlifObject *olditem{};

	if (tuplesize == 0)
		return nullptr;

	if (_alifObject_isUniquelyReferenced(result)) {
		ALIF_INCREF(result);
		for (i=0 ; i < tuplesize ; i++) {
			it = ALIFTUPLE_GET_ITEM(lz->itTuple, i);
			item = (*ALIF_TYPE(it)->iterNext)(it);
			if (item == nullptr) {
				ALIF_DECREF(result);
				if (lz->strict) {
					goto check;
				}
				return nullptr;
			}
			olditem = ALIFTUPLE_GET_ITEM(result, i);
			ALIFTUPLE_SET_ITEM(result, i, item);
			ALIF_DECREF(olditem);
		}
		if (!ALIFOBJECT_GC_IS_TRACKED(result)) {
			ALIFOBJECT_GC_TRACK(result);
		}
	} else {
		result = alifTuple_new(tuplesize);
		if (result == nullptr)
			return nullptr;
		for (i=0 ; i < tuplesize ; i++) {
			it = ALIFTUPLE_GET_ITEM(lz->itTuple, i);
			item = (*ALIF_TYPE(it)->iterNext)(it);
			if (item == nullptr) {
				ALIF_DECREF(result);
				if (lz->strict) {
					goto check;
				}
				return nullptr;
			}
			ALIFTUPLE_SET_ITEM(result, i, item);
		}
	}
	return result;
check:
	if (alifErr_occurred()) {
		if (!alifErr_exceptionMatches(_alifExcStopIteration_)) {
			// next() on argument i raised an exception (not StopIteration)
			return nullptr;
		}
		alifErr_clear();
	}
	if (i) {
		// خطأ_قيمة: zip() argument 2 is shorter than argument 1
		// خطأ_قيمة: zip() argument 3 is shorter than arguments 1-2
		const char* plural = i == 1 ? " " : "ات 1-";
		return alifErr_format(_alifExcValueError_,
			"مقرون() المعامل %d اقصر من المعامل%s%d",
			i + 1, plural, i);
	}
	for (i = 1; i < tuplesize; i++) {
		it = ALIFTUPLE_GET_ITEM(lz->itTuple, i);
		item = (*ALIF_TYPE(it)->iterNext)(it);
		if (item) {
			ALIF_DECREF(item);
			const char* plural = i == 1 ? " " : "ات 1-";
			return alifErr_format(_alifExcValueError_,
				"مقرون() المعامل %d اطول من المعامل%s%d",
				i + 1, plural, i);
		}
		if (alifErr_occurred()) {
			if (!alifErr_exceptionMatches(_alifExcStopIteration_)) {
				// next() on argument i raised an exception (not StopIteration)
				return nullptr;
			}
			alifErr_clear();
		}
		// Argument i is exhausted. So far so good...
	}
	// All arguments are exhausted. Success!
	return nullptr;
}

AlifTypeObject _alifZipType_ = { // 3105
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مقرون",
	.basicSize = sizeof(ZipObject),
	/* methods */
	.dealloc = (Destructor)zip_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE,
	//.traverse = (TraverseProc)zip_traverse,
	.iter = alifObject_selfIter,
	.iterNext = (IterNextFunc)zip_next,
	//.methods = _zipMethods_,
	.alloc = alifType_genericAlloc,
	.new_ = zip_new,
	.free = alifObject_gcDel,
};

static AlifMethodDef _builtinMethods_[] = { // 3141
	{"__buildClass__", ALIF_CPPFUNCTION_CAST(builtin___buildClass__),
	 METHOD_FASTCALL | METHOD_KEYWORDS},
	BUILTIN___IMPORT___METHODDEF,
	BUILTIN_ALL_METHODDEF,
	BUILTIN_ANY_METHODDEF,
	BUILTIN_BIN_METHODDEF,
	BUILTIN_HEX_METHODDEF,
	BUILTIN_OCT_METHODDEF,
	BUILTIN_CHR_METHODDEF,
	BUILTIN_COMPILE_METHODDEF,
	BUILTIN_EXEC_METHODDEF,
	BUILTIN_DELATTR_METHODDEF,
	{"احضر_صفة", ALIF_CPPFUNCTION_CAST(builtin_getAttr), METHOD_FASTCALL},
	BUILTIN_HASATTR_METHODDEF,
	BUILTIN_SETATTR_METHODDEF,
	BUILTIN_GLOBALS_METHODDEF,
	BUILTIN_LOCALS_METHODDEF,
	BUILTIN_INPUT_METHODDEF,
	BUILTIN_ISINSTANCE_METHODDEF,
	BUILTIN_LEN_METHODDEF,
	{"اقصى", ALIF_CPPFUNCTION_CAST(builtin_max), METHOD_FASTCALL | METHOD_KEYWORDS},
	{"ادنى", ALIF_CPPFUNCTION_CAST(builtin_min), METHOD_FASTCALL | METHOD_KEYWORDS},
	BUILTIN_ORD_METHODDEF,
	BUILTIN_PRINT_METHODDEF,
	BUILTIN_SORTED_METHODDEF,
	BUILTIN_SUM_METHODDEF,
	BUILTIN_ROUND_METHODDEF,
	{nullptr, nullptr},
};


static AlifModuleDef _alifBuiltinsModule_ = { // 3202
	ALIFMODULEDEF_HEAD_INIT,
	"الضمنيات",
	nullptr,
	-1,
	_builtinMethods_,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
};


AlifObject* alifBuiltin_init(AlifInterpreter* _interpreter) { // 3215
	AlifObject* mod{}, * dict{}, * debug{};

	const AlifConfig* config = _alifInterpreterState_getConfig(_interpreter);

	mod = alifModule_createInitialized(&_alifBuiltinsModule_);
	if (mod == nullptr) return nullptr;

	alifUnstable_moduleSetGIL(mod, ALIF_MOD_GIL_NOT_USED);
	dict = alifModule_getDict(mod);

#define SETBUILTIN(_name, _object) \
    if (alifDict_setItemString(dict, _name, (AlifObject *)_object) < 0)       \
        return nullptr;                                                    \

	SETBUILTIN("عدم", ALIF_NONE);
	SETBUILTIN("Ellipsis", ALIF_ELLIPSIS);
	SETBUILTIN("غير_منتهي", ALIF_NOTIMPLEMENTED);
	SETBUILTIN("خطأ", ALIF_FALSE);
	SETBUILTIN("صح", ALIF_TRUE);
	SETBUILTIN("منطق", &_alifBoolType_);
	//SETBUILTIN("memoryview", &_alifMemoryViewType_);
	SETBUILTIN("مصفوفة_ثمانية", &_alifByteArrayType_);
	SETBUILTIN("ثمانيات", &_alifBytesType_);
	SETBUILTIN("وظيفة_صنف", &_alifClassMethodType_);
	SETBUILTIN("complex", &_alifComplexType_);
	SETBUILTIN("قاموس", &_alifDictType_);
	SETBUILTIN("تعداد", &_alifEnumType_);
	//SETBUILTIN("filter", &_alifFilterType_);
	SETBUILTIN("عشري", &_alifFloatType_);
	SETBUILTIN("frozenset", &_alifFrozenSetType_);
	SETBUILTIN("خاصية", &_alifPropertyType_);
	SETBUILTIN("صحيح", &_alifLongType_);
	SETBUILTIN("مصفوفة", &_alifListType_);
	SETBUILTIN("طبق", &_alifMapType_);
	SETBUILTIN("كائن", &_alifBaseObjectType_);
	SETBUILTIN("مدى", &_alifRangeType_);
	SETBUILTIN("معكوس", &_alifReversedType_);
	SETBUILTIN("مميزة", &_alifSetType_);
	SETBUILTIN("قطع", &_alifSliceType_);
	SETBUILTIN("وظيفة_ساكنة", &_alifStaticMethodType_);
	SETBUILTIN("نص", &_alifUStrType_);
	SETBUILTIN("اصل", &_alifSuperType_);
	SETBUILTIN("مترابطة", &_alifTupleType_);
	SETBUILTIN("نوع", &_alifTypeType_);
	SETBUILTIN("مقرون", &_alifZipType_);
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
