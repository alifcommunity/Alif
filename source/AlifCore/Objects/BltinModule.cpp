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

	AlifThread* thread{}; //* alif

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
			printf("تعليق : BltinModule.cpp - builtin___buildClass__"); //* alif
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





static AlifObject* builtin___import__Impl(AlifObject* module, AlifObject* name, AlifObject* globals,
	AlifObject* locals, AlifObject* fromlist, AlifIntT level) { // 272
	return alifImport_importModuleLevelObject(name, globals, locals,
		fromlist, level);
}





static AlifObject* builtin_len(AlifObject* _module, AlifObject* _obj) { // 1762
	AlifSizeT res{};

	res = alifObject_size(_obj);
	if (res < 0) {
		return nullptr;
	}
	return alifLong_fromSizeT(res);
}






static AlifObject* min_max(AlifObject* const* args, AlifSizeT nargs,
	AlifObject* kwnames, AlifIntT op) { // 1795
	AlifObject* it = nullptr, * item{}, * val{}, * maxitem{}, * maxval{}, * keyfunc = nullptr;
	AlifObject* defaultval = nullptr;
	static const char* const keywords[] = { "key", "default", nullptr };
	static AlifArgParser parserMin = { "|$OO:min", keywords, 0 };
	static AlifArgParser parserMax = { "|$OO:max", keywords, 0 };
	const char* name = (op == ALIF_LT) ? "min" : "max";
	AlifArgParser* parser_ = (op == ALIF_LT) ? &parserMin : &parserMax;

	if (nargs == 0) {
		//alifErr_format(_alifExcTypeError_, "%s expected at least 1 argument, got 0", name);
		return nullptr;
	}

	if (kwnames != nullptr and !_alifArg_parseStackAndKeywords(args + nargs, 0, kwnames, parser_,
		&keyfunc, &defaultval)) {
		return nullptr;
	}

	const AlifIntT positional = nargs > 1;
	if (positional and defaultval != nullptr) {
		//alifErr_format(_alifExcTypeError_,
		//	"Cannot specify a default for %s() with multiple "
		//	"positional arguments", name);
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
			//alifErr_format(_alifExcValueError_,
			//	"%s() iterable argument is empty", name);
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



static AlifObject* builtin_inputImpl(AlifObject* module, AlifObject* prompt) { // 2151

	//* alif
	AlifObject* res{};
	const char* promptstr{};

	if (prompt) { //* review
		if (alifUStr_isASCII(prompt)) {
			char* promptstr = (char*)ALIFUSTR_DATA(prompt);
			printf("%s", promptstr);
		}
		else {
			char* promptstr = (char*)alifUStr_asUTF8(prompt);
			printf("%s", promptstr);
		}
	}

	char* buff = alifOS_readline(stdin, stdout, promptstr);


	if (buff == nullptr) {
		return ALIF_NONE;
	}

	AlifSizeT len = strlen(buff);
	if (len == 0) {
		res = nullptr;
	}
	else {
		if (len > ALIF_SIZET_MAX) {
			//alifErr_setString(_alifExcOverflowError_,
			//	"input: input too long");
			res = nullptr;
		}
		else {
			len--;   /* strip trailing '\n' */
			if (len != 0 and buff[len - 1] == '\r')
				len--;   /* strip trailing '\r' */
			res = alifUStr_decode(buff, len, nullptr, nullptr);
		}
	}
	alifMem_dataFree(buff);

	return res;
	//* alif


	AlifThread* thread = _alifThread_get();
	AlifObject* fin = _alifSys_getAttr(thread, &ALIF_ID(Stdin));
	AlifObject* fout = _alifSys_getAttr(thread, &ALIF_ID(Stdout));
	AlifObject* ferr = _alifSys_getAttr(thread, &ALIF_ID(Stderr));
	AlifObject* tmp{};
	long fd{};
	AlifIntT tty{};

	/* Check that stdin/out/err are intact */
	if (fin == nullptr or fin == ALIF_NONE) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"input(): lost sys.stdin");
		return nullptr;
	}
	if (fout == nullptr or fout == ALIF_NONE) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"input(): lost sys.stdout");
		return nullptr;
	}
	if (ferr == nullptr or ferr == ALIF_NONE) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"input(): lost sys.stderr");
		return nullptr;
	}

	//if (alifSys_audit("builtins.input", "O", prompt ? prompt : ALIF_NONE) < 0) {
	//	return nullptr;
	//}

	/* First of all, flush stderr */
	//if (_alifFile_flush(ferr) < 0) {
	//	alifErr_clear();
	//}

	/* We should only use (GNU) readline if Alif's sys.stdin and
	   sys.stdout are the same as CPP's stdin and stdout, because we
	   need to pass it those. */
	tmp = alifObject_callMethodNoArgs(fin, &ALIF_ID(Fileno));
	if (tmp == nullptr) {
		//alifErr_clear();
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
			//alifErr_clear();
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
		AlifObject* stdin_encoding = nullptr, * stdin_errors = nullptr;
		AlifObject* stdout_encoding = nullptr, * stdout_errors = nullptr;
		const char* stdin_encoding_str, * stdin_errors_str;
		AlifObject* result{};
		AlifUSizeT len{};

		/* stdin is a text stream, so it must have an encoding. */
		stdin_encoding = alifObject_getAttr(fin, &ALIF_ID(Encoding));
		if (stdin_encoding == nullptr) {
			tty = 0;
			goto _readline_errors;
		}
		stdin_errors = alifObject_getAttr(fin, &ALIF_ID(Errors));
		if (stdin_errors == nullptr) {
			tty = 0;
			goto _readline_errors;
		}
		if (!ALIFUSTR_CHECK(stdin_encoding) ||
			!ALIFUSTR_CHECK(stdin_errors))
		{
			tty = 0;
			goto _readline_errors;
		}
		stdin_encoding_str = alifUStr_asUTF8(stdin_encoding);
		if (stdin_encoding_str == nullptr) {
			goto _readline_errors;
		}
		stdin_errors_str = alifUStr_asUTF8(stdin_errors);
		if (stdin_errors_str == nullptr) {
			goto _readline_errors;
		}
		//if (_alifFile_flush(fout) < 0) {
		//	alifErr_clear();
		//}
		if (prompt != nullptr) {
			/* We have a prompt, encode it as stdout would */
			const char* stdout_encoding_str{}, * stdout_errors_str{};
			AlifObject* stringpo{};
			stdout_encoding = alifObject_getAttr(fout, &ALIF_ID(Encoding));
			if (stdout_encoding == nullptr) {
				tty = 0;
				goto _readline_errors;
			}
			stdout_errors = alifObject_getAttr(fout, &ALIF_ID(Errors));
			if (stdout_errors == nullptr) {
				tty = 0;
				goto _readline_errors;
			}
			if (!ALIFUSTR_CHECK(stdout_encoding) ||
				!ALIFUSTR_CHECK(stdout_errors))
			{
				tty = 0;
				goto _readline_errors;
			}
			stdout_encoding_str = alifUStr_asUTF8(stdout_encoding);
			if (stdout_encoding_str == nullptr) {
				goto _readline_errors;
			}
			stdout_errors_str = alifUStr_asUTF8(stdout_errors);
			if (stdout_errors_str == nullptr) {
				goto _readline_errors;
			}
			stringpo = alifObject_str(prompt);
			if (stringpo == nullptr)
				goto _readline_errors;
			po = alifUStr_asEncodedString(stringpo,
				stdout_encoding_str, stdout_errors_str);
			ALIF_CLEAR(stdout_encoding);
			ALIF_CLEAR(stdout_errors);
			ALIF_CLEAR(stringpo);
			if (po == nullptr)
				goto _readline_errors;
			promptstr = ALIFBYTES_AS_STRING(po);
			if ((AlifSizeT)strlen(promptstr) != ALIFBYTES_GET_SIZE(po)) {
				//alifErr_setString(_alifExcValueError_,
				//	"input: prompt string cannot contain null characters");
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
				//alifErr_setString(_alifExcOverflowError_,
				//	"input: input too long");
				result = nullptr;
			}
			else {
				len--;   /* strip trailing '\n' */
				if (len != 0 and s[len - 1] == '\r')
					len--;   /* strip trailing '\r' */
				result = alifUStr_decode(s, len, stdin_encoding_str,
					stdin_errors_str);
			}
		}
		ALIF_DECREF(stdin_encoding);
		ALIF_DECREF(stdin_errors);
		ALIF_XDECREF(po);
		alifMem_dataFree(s);

		if (result != nullptr) {
			//if (alifSys_audit("builtins.input/result", "O", result) < 0) {
			//	return nullptr;
			//}
		}

		return result;

	_readline_errors:
		ALIF_XDECREF(stdin_encoding);
		ALIF_XDECREF(stdout_encoding);
		ALIF_XDECREF(stdin_errors);
		ALIF_XDECREF(stdout_errors);
		ALIF_XDECREF(po);
		if (tty)
			return nullptr;

		//alifErr_clear();
	}

	/* Fallback if we're not interactive */
	if (prompt != nullptr) {
		if (alifFile_writeObject(prompt, fout, ALIF_PRINT_RAW) != 0)
			return nullptr;
	}
	//if (_alifFile_flush(fout) < 0) {
	//	alifErr_clear();
	//}
	//return alifFile_getLine(fin, -1);
}


static AlifMethodDef _builtinMethods_[] = { // 3141
	{"__buildClass__", ALIF_CPPFUNCTION_CAST(builtin___buildClass__),
	 METHOD_FASTCALL | METHOD_KEYWORDS},
	BUILTIN___IMPORT___METHODDEF,
	BUILTIN_INPUT_METHODDEF,
	BUILTIN_LEN_METHODDEF,
	{"اقصى", ALIF_CPPFUNCTION_CAST(builtin_max), METHOD_FASTCALL | METHOD_KEYWORDS},
	{"ادنى", ALIF_CPPFUNCTION_CAST(builtin_min), METHOD_FASTCALL | METHOD_KEYWORDS},
	BUILTIN_PRINT_METHODDEF,
	{nullptr, nullptr},
};


static AlifModuleDef _alifBuiltinsModule_ = { // 3202
	ALIFMODULEDEF_HEAD_INIT,
	"builtins",
	nullptr,
	-1,
	_builtinMethods_,
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
	SETBUILTIN("عشري", &_alifFloatType_);
	SETBUILTIN("frozenset", &_alifFrozenSetType_);
	//SETBUILTIN("property", &_alifPropertyType_);
	SETBUILTIN("صحيح", &_alifLongType_);
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
