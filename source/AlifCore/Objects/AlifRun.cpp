#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Eval.h"
#include "AlifCore_Compile.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Object.h"
#include "AlifCore_Parser.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Errors.h"
#include "AlifCore_Run.h"
#include "AlifCore_SysModule.h"
#include "AlifCore_Traceback.h"


#ifdef _WINDOWS
#undef BYTE
#endif

static void flush_io(void); // 41
static AlifObject* run_mod(ModuleTy, AlifObject*, AlifObject*,
	AlifObject*, AlifCompilerFlags*, AlifASTMem*, AlifObject*, AlifIntT); // 42
static AlifObject* alifRun_file(FILE*, AlifObject*, AlifIntT,
	AlifObject*, AlifObject*, AlifIntT, AlifCompilerFlags*); // 47


static AlifObject* _alifRun_stringFlagsWithName(const char*, AlifObject*, AlifIntT,
	AlifObject*, AlifObject*, AlifCompilerFlags*, AlifIntT); // 50


AlifIntT alifRun_fileObject(FILE* _fp, AlifObject* _filename,
	AlifIntT _closeIt, AlifCompilerFlags* _flags) { // 55

	AlifIntT decRefFileName = 0;
	if (_filename == nullptr) {
		_filename = alifUStr_fromString("???");
		if (_filename == nullptr) {
			//alifErr_print();
			return -1;
		}
		decRefFileName = 1;
	}

	AlifIntT res{};
	//if (alif_fdIsInteractive(_fp, _filename)) {
	//	res = alifRun_interactiveLoopObject(_fp, _filename, _flags);
	//	if (_closeIt) {
	//		fclose(_fp);
	//	}
	//}
	//else {
	res = alifRun_simpleFileObject(_fp, _filename, _closeIt, _flags);
	//}

	if (decRefFileName) {
		ALIF_DECREF(_filename);
	}
	return res;
}

















AlifIntT alifRun_simpleFileObject(FILE* _fp, AlifObject* _filename,
	AlifIntT _closeIt, AlifCompilerFlags* _flags) { // 399
	AlifIntT ret = -1;
	AlifObject* v{};
	//AlifIntT alifc{};

	AlifObject* mainModule = alifImport_addModuleRef("__main__");
	if (mainModule == nullptr)	return -1;
	AlifObject* dict = alifModule_getDict(mainModule);  // borrowed ref

	AlifIntT setFileName = 0;
	AlifIntT hasFile = alifDict_containsString(dict, "__file__");
	if (hasFile < 0) {
		goto done;
	}
	if (!hasFile) {
		if (alifDict_setItemString(dict, "__file__", _filename) < 0) {
			goto done;
		}
		if (alifDict_setItemString(dict, "__cached__", ALIF_NONE) < 0) {
			goto done;
		}
		setFileName = 1;
	}

	//alifc = maybe_alifcFile(_fp, _filename, _closeIt);
	//if (alifc < 0) {
	//	goto done;
	//}

	//if (alifc) {
	//	FILE* alifcFP{};
	//	if (_closeIt) {
	//		fclose(_fp);
	//	}

	//	alifcFP = alif_fOpenObj(_filename, "rb");
	//	if (alifcFP == nullptr) {
	//		fprintf(stderr, "alif: Can't reopen .alifc file\n");
	//		goto done;
	//	}

	//	if (set_mainLoader(dict, _filename, "SourcelessFileLoader") < 0) {
	//		fprintf(stderr, "alif: failed to set __main__.__loader__\n");
	//		ret = -1;
	//		fclose(alifcFP);
	//		goto done;
	//	}
	//	v = run_alifcFile(alifcFP, dict, dict, _flags);
	//}
	//else {
		//if ((!ALIFUSTR_CHECK(_filename)
		//	or !alifUStr_equalToUTF8(_filename, "<stdin>"))
		//	and set_mainLoader(dict, _filename, "SourceFileLoader") < 0) {
		//	fprintf(stderr, "alif: failed to set __main__.__loader__\n");
		//	ret = -1;
		//	goto done;
		//}
	v = alifRun_file(_fp, _filename, ALIF_FILE_INPUT, dict, dict,
		_closeIt, _flags);
	//}
	flush_io();
	if (v == nullptr) {
		ALIF_CLEAR(mainModule);
		alifErr_print();
		goto done;
	}
	ALIF_DECREF(v);
	ret = 0;
done:
	if (setFileName) {
		if (alifDict_popString(dict, "__file__", nullptr) < 0) {
			alifErr_print();
		}
		if (alifDict_popString(dict, "__cached__", nullptr) < 0) {
			alifErr_print();
		}
	}
	ALIF_XDECREF(mainModule);
	return ret;
}



AlifIntT _alifRun_simpleStringFlagsWithName(const char* _command,
	const char* _name, AlifCompilerFlags* _flags) { // 530
	AlifObject* mainModule = alifImport_addModuleRef("__main__");
	if (mainModule == nullptr) {
		return -1;
	}
	AlifObject* dict = alifModule_getDict(mainModule);  // borrowed ref

	AlifObject* res = nullptr;
	if (_name == nullptr) {
		res = alifRun_stringFlags(_command, ALIF_FILE_INPUT, dict, dict, _flags);
	}
	else {
		AlifObject* theName = alifUStr_fromString(_name);
		if (!theName) {
			alifErr_print();
			return -1;
		}
		res = _alifRun_stringFlagsWithName(_command, theName,
			ALIF_FILE_INPUT, dict, dict, _flags, 0);
		ALIF_DECREF(theName);
	}
	ALIF_DECREF(mainModule);
	if (res == nullptr) {
		alifErr_print();
		return -1;
	}

	ALIF_DECREF(res);
	return 0;
}



static AlifIntT parse_exitCode(AlifObject* _code, AlifIntT* _exitcodeP) { // 567
	if (ALIFLONG_CHECK(_code)) {
		AlifIntT exitcode = (AlifIntT)alifLong_asLongLong(_code);
		if (exitcode == -1 and alifErr_occurred()) {
			alifErr_clear();
			*_exitcodeP = -1;
			return 1;
		}
		*_exitcodeP = exitcode;
		return 1;
	}
	else if (_code == ALIF_NONE) {
		*_exitcodeP = 0;
		return 1;
	}
	return 0;
}

AlifIntT _alif_handleSystemExit(AlifIntT* _exitcodeP) { // 591
	AlifIntT inspect = alif_getConfig()->inspect;
	if (inspect) {
		return 0;
	}

	//if (!alifErr_exceptionMatches(_alifExcSystemExit_)) {
	//	return 0;
	//}

	fflush(stdout);

	AlifObject* exc = alifErr_getRaisedException();

	AlifObject* code; code = alifObject_getAttr(exc, &ALIF_ID(Code));
	if (code == nullptr) {
		alifErr_clear();
	}
	else if (parse_exitCode(code, _exitcodeP)) {
		ALIF_DECREF(code);
		ALIF_CLEAR(exc);
		return 1;
	}
	else {
		ALIF_SETREF(exc, code);
	}

	AlifThread* thread = _alifThread_get();
	AlifObject* sysStderr = _alifSys_getAttr(thread, &ALIF_ID(Stderr));
	if (sysStderr != nullptr and sysStderr != ALIF_NONE) {
		if (alifFile_writeObject(exc, sysStderr, ALIF_PRINT_RAW) < 0) {
			alifErr_clear();
		}
	}
	else {
		if (alifObject_print(exc, stderr, ALIF_PRINT_RAW) < 0) {
			alifErr_clear();
		}
		fflush(stderr);
	}
	//alifSys_writeStderr("\n");
	ALIF_CLEAR(exc);
	*_exitcodeP = 1;
	return 1;
}





static void handle_systemExit(void) { // 630
	AlifIntT exitcode;
	if (_alif_handleSystemExit(&exitcode)) {
		alif_exit(exitcode);
	}
}

static void _alifErr_printEx(AlifThread* _thread, AlifIntT _setSysLastVars) { // 640
	AlifObject* typ{}, * tb{};
	handle_systemExit();

	AlifObject* exc = _alifErr_getRaisedException(_thread);
	if (exc == nullptr) {
		goto done;
	}
	typ = ALIF_NEWREF(ALIF_TYPE(exc));
	tb = alifException_getTraceback(exc);
	if (tb == nullptr) {
		tb = ALIF_NEWREF(ALIF_NONE);
	}

	if (_setSysLastVars) {
		if (_alifSys_setAttr(&ALIF_ID(LastExc), exc) < 0) {
			_alifErr_clear(_thread);
		}
		/* Legacy version: */
		if (_alifSys_setAttr(&ALIF_ID(LastType), typ) < 0) {
			_alifErr_clear(_thread);
		}
		if (_alifSys_setAttr(&ALIF_ID(LastValue), exc) < 0) {
			_alifErr_clear(_thread);
		}
		if (_alifSys_setAttr(&ALIF_ID(LastTraceback), tb) < 0) {
			_alifErr_clear(_thread);
		}
	}
	AlifObject* hook;
	hook = _alifSys_getAttr(_thread, &ALIF_STR(Excepthook));
	//if (_alifSys_audit(_thread, "sys.excepthook", "OOOO", hook ? hook : ALIF_NONE,
	//	typ, exc, tb) < 0) {
	//	if (alifErr_exceptionMatches(_alifExcRuntimeError_)) {
	//		alifErr_clear();
	//		goto done;
	//	}
	//	_alifErr_writeUnraisableMsg("in audit hook", nullptr);
	//}
	if (hook) {
		AlifObject* args[3] = { typ, exc, tb };
		AlifObject* result = alifObject_vectorCall(hook, args, 3, nullptr);
		if (result == nullptr) {
			handle_systemExit();

			AlifObject* exc2 = _alifErr_getRaisedException(_thread);
			fflush(stdout);
			//alifSys_writeStderr("Error in sys.excepthook:\n");
			alifErr_displayException(exc2);
			//alifSys_writeStderr("\nOriginal exception was:\n");
			alifErr_displayException(exc);
			ALIF_DECREF(exc2);
		}
		else {
			ALIF_DECREF(result);
		}
	}
	else {
		//alifSys_writeStderr("sys.excepthook is missing\n");
		alifErr_displayException(exc);
	}

done:
	ALIF_XDECREF(typ);
	ALIF_XDECREF(exc);
	ALIF_XDECREF(tb);
}


void _alifErr_print(AlifThread* _thread) { // 711
	_alifErr_printEx(_thread, 1);
}

void alifErr_printEx(AlifIntT _setSysLastVars) { // 717
	AlifThread* thread = _alifThread_get();
	_alifErr_printEx(thread, _setSysLastVars);
}

void alifErr_print(void) { // 724
	alifErr_printEx(1);
}

class ExceptionPrintContext { // 730
public:
	AlifObject* file{};
	AlifObject* seen{};            // Prevent cycles in recursion
};


static AlifIntT printException_traceback(ExceptionPrintContext* ctx,
	AlifObject* value) { // 755
	AlifObject* f = ctx->file;
	AlifIntT err = 0;

	AlifObject* tb = alifException_getTraceback(value);
	if (tb and tb != ALIF_NONE) {
		const char* header = EXCEPTION_TB_HEADER;
		err = _alifTraceBack_print(tb, header, f);
	}
	ALIF_XDECREF(tb);
	return err;
}


static AlifIntT printException_fileAndLine(ExceptionPrintContext* ctx,
	AlifObject** value_p) { // 770
	AlifObject* f = ctx->file;

	//AlifObject* tmp;
	//AlifIntT res = alifObject_getOptionalAttr(*value_p, &ALIF_ID(PrintFileAndLine), &tmp);
	//if (res <= 0) {
	//	if (res < 0) {
	//		alifErr_clear();
	//	}
	//	return 0;
	//}
	//ALIF_DECREF(tmp);

	AlifObject* filename = nullptr;
	AlifSizeT lineno = 0;
	AlifObject* v = alifObject_getAttr(*value_p, &ALIF_ID(Filename));
	if (!v) {
		return -1;
	}
	if (v == ALIF_NONE) {
		ALIF_DECREF(v);
		ALIF_DECLARE_STR(anon_string, "<string>");
		filename = ALIF_NEWREF(&ALIF_STR(AnonString));
	}
	else {
		filename = v;
	}

	AlifObject* line = alifUStr_fromFormat("  الملف \"%S\", السطر %zd\n",
		filename, lineno);
	ALIF_DECREF(filename);
	if (line == nullptr) {
		goto error;
	}
	if (alifFile_writeObject(line, f, ALIF_PRINT_RAW) < 0) {
		goto error;
	}
	ALIF_CLEAR(line);

	return 0;

error:
	ALIF_XDECREF(line);
	return -1;
}


static AlifIntT printException_message(ExceptionPrintContext* ctx, AlifObject* type,
	AlifObject* value) { // 821
	AlifObject* f = ctx->file;

	//if (alifErr_givenExceptionMatches(value, _alifExcMemoryError_)) {
	//	return -1;
	//}

	//AlifObject* modulename = alifObject_getAttr(type, &ALIF_ID(__module__));
	//if (modulename == nullptr or !ALIFUSTR_CHECK(modulename)) {
	//	ALIF_XDECREF(modulename);
	//	alifErr_clear();
	//	if (alifFile_writeString("<unknown>.", f) < 0) {
	//		return -1;
	//	}
	//}
	//else {
	//	if (!_alifUStr_equal(modulename, &ALIF_ID(Builtins)) and
	//		!_alifUStr_equal(modulename, &ALIF_ID(__main__)))
	//	{
	//		AlifIntT res = alifFile_writeObject(modulename, f, ALIF_PRINT_RAW);
	//		ALIF_DECREF(modulename);
	//		if (res < 0) {
	//			return -1;
	//		}
	//		if (alifFile_writeString(".", f) < 0) {
	//			return -1;
	//		}
	//	}
	//	else {
	//		ALIF_DECREF(modulename);
	//	}
	//}

	AlifObject* qualname = alifType_getQualName((AlifTypeObject*)type);
	if (qualname == nullptr or !ALIFUSTR_CHECK(qualname)) {
		ALIF_XDECREF(qualname);
		alifErr_clear();
		if (alifFile_writeString("<unknown>", f) < 0) {
			return -1;
		}
	}
	else {
		AlifIntT res = alifFile_writeObject(qualname, f, ALIF_PRINT_RAW);
		ALIF_DECREF(qualname);
		if (res < 0) {
			return -1;
		}
	}

	if (ALIF_ISNONE(value)) {
		return 0;
	}

	AlifObject* s = alifObject_str(value);
	if (s == nullptr) {
		alifErr_clear();
		if (alifFile_writeString(": <exception str() failed>", f) < 0) {
			return -1;
		}
	}
	else {
		if (!ALIFUSTR_CHECK(s) or alifUStr_getLength(s) != 0) {
			if (alifFile_writeString(": ", f) < 0) {
				ALIF_DECREF(s);
				return -1;
			}
		}
		AlifIntT res = alifFile_writeObject(s, f, ALIF_PRINT_RAW);
		ALIF_DECREF(s);
		if (res < 0) {
			return -1;
		}
	}

	return 0;
}


static AlifIntT print_exception(ExceptionPrintContext* ctx, AlifObject* value) { // 908
	AlifObject* f = ctx->file;

	if (!ALIFEXCEPTIONINSTANCE_CHECK(value)) {
		//return printException_invalidType(ctx, value);
	}

	ALIF_INCREF(value);
	fflush(stdout);

	if (printException_traceback(ctx, value) < 0) {
		goto error;
	}

	/* grab the type and notes now because value can change below */
	AlifObject* type;
	type = (AlifObject*)ALIF_TYPE(value);

	if (printException_fileAndLine(ctx, &value) < 0) {
		goto error;
	}
	if (printException_message(ctx, type, value) < 0) {
		goto error;
	}
	if (alifFile_writeString("\n", f) < 0) {
		goto error;
	}
	ALIF_DECREF(value);
	return 0;
error:
	ALIF_DECREF(value);
	return -1;
}

static AlifIntT printException_causeAndContext(ExceptionPrintContext* ctx,
	AlifObject* value) { // 1011
	AlifObject* value_id = alifLong_fromVoidPtr(value);
	if (value_id == nullptr or alifSet_add(ctx->seen, value_id) == -1) {
		alifErr_clear();
		ALIF_XDECREF(value_id);
		return 0;
	}
	ALIF_DECREF(value_id);

	if (!ALIFEXCEPTIONINSTANCE_CHECK(value)) {
		return 0;
	}

	//AlifObject* cause = alifException_getCause(value);
	//if (cause) {
	//	int err = 0;
	//	if (!printException_seenLookup(ctx, cause)) {
	//		err = print_chained(ctx, cause, _causeMessage_, "cause");
	//	}
	//	ALIF_DECREF(cause);
	//	return err;
	//}
	//if (((AlifBaseExceptionObject*)value)->suppressContext) {
	//	return 0;
	//}
	//AlifObject* context = alifException_getContext(value);
	//if (context) {
	//	AlifIntT err = 0;
	//	if (!printException_seenLookup(ctx, context)) {
	//		err = print_chained(ctx, context, _contextMessage_, "context");
	//	}
	//	ALIF_DECREF(context);
	//	return err;
	//}
	return 0;
}

static AlifIntT printException_recursive(ExceptionPrintContext* ctx, AlifObject* value) { // 1051
	if (_alif_enterRecursiveCall(" in printException_recursive")) {
		return -1;
	}
	if (ctx->seen != nullptr) {
		/* Exception chaining */
		if (printException_causeAndContext(ctx, value) < 0) {
			goto error;
		}
	}
	if (print_exception(ctx, value) < 0) {
		goto error;
	}

	_alif_leaveRecursiveCall();
	return 0;
error:
	_alif_leaveRecursiveCall();
	return -1;
}

void _alifErr_display(AlifObject* file, AlifObject* unused,
	AlifObject* value, AlifObject* tb) { // 1075
	if (ALIFEXCEPTIONINSTANCE_CHECK(value)
		and tb != nullptr and ALIFTRACEBACK_CHECK(tb)) {
		AlifObject* cur_tb = alifException_getTraceback(value);
		if (cur_tb == nullptr) {
			alifException_setTraceback(value, tb);
		}
		else {
			ALIF_DECREF(cur_tb);
		}
	}

	//	AlifIntT unhandled_keyboard_interrupt = _alifRuntime_.signals.unhandledKeyboardInterrupt;
	//
	//	// Try first with the stdlib traceback module
	//	AlifObject* traceback_module = alifImport_importModule("traceback");
	//
	//	if (traceback_module == nullptr) {
	//		goto fallback;
	//	}
	//
	//	AlifObject* print_exception_fn = alifObject_getAttrString(traceback_module, "_print_exception_bltin");
	//
	//	if (print_exception_fn == nullptr or !ALIFCALLABLE_CHECK(print_exception_fn)) {
	//		ALIF_DECREF(traceback_module);
	//		goto fallback;
	//	}
	//
	//	AlifObject* result = alifObject_callOneArg(print_exception_fn, value);
	//
	//	ALIF_DECREF(traceback_module);
	//	ALIF_XDECREF(print_exception_fn);
	//	if (result) {
	//		ALIF_DECREF(result);
	//		_alifRuntime_.signals.unhandledKeyboardInterrupt = unhandled_keyboard_interrupt;
	//		return;
	//	}
	//fallback:
	//	_alifRuntime_.signals.unhandledKeyboardInterrupt = unhandled_keyboard_interrupt;
	//	alifErr_clear();
	ExceptionPrintContext ctx{};
	ctx.file = file;

	ctx.seen = alifSet_new(nullptr);
	if (ctx.seen == nullptr) {
		alifErr_clear();
	}
	if (printException_recursive(&ctx, value) < 0) {
		alifErr_clear();
		//_alifObject_dump(value);
		fprintf(stderr, "النظام.إخراج_الخطأ مفقود\n");
	}
	ALIF_XDECREF(ctx.seen);

	/* Call file.flush() */
	if (_alifFile_flush(file) < 0) {
		alifErr_clear();
	}
}

void alifErr_display(AlifObject* unused, AlifObject* value, AlifObject* tb) { // 1151
	AlifThread* tstate = _alifThread_get();
	AlifObject* file = _alifSys_getAttr(tstate, &ALIF_ID(Stderr));
	//if (file == nullptr) {
	//	_alifObject_dump(value);
	//	fprintf(stderr, "lost sys.stderr\n");
	//	return;
	//}
	//if (file == ALIF_NONE) {
	//	return;
	//}
	//ALIF_INCREF(file);
	_alifErr_display(file, nullptr, value, tb);
	//ALIF_DECREF(file);
}


void alifErr_displayException(AlifObject* _exc) { // 1174
	alifErr_display(nullptr, _exc, nullptr);
}


static AlifObject* _alifRun_stringFlagsWithName(const char* _str, AlifObject* _name,
	AlifIntT _start, AlifObject* _globals, AlifObject* _locals,
	AlifCompilerFlags* _flags, AlifIntT _generateNewSource) { // 1179
	AlifObject* ret = nullptr;
	ModuleTy mod{};
	AlifASTMem* astMem{};

	astMem = alifASTMem_new();
	if (astMem == nullptr)
		return nullptr;

	AlifObject* source = nullptr;
	ALIF_DECLARE_STR(AnonString, "<string>");

	if (_name) {
		source = alifUStr_fromString(_str);
		if (!source) {
			alifErr_clear();
		}
	}
	else {
		_name = &ALIF_STR(AnonString);
	}

	mod = _alifParser_astFromString(_str, _name, _start, _flags, astMem);

	if (mod != nullptr) {
		ret = run_mod(mod, _name, _globals, _locals, _flags, astMem, source, _generateNewSource);
	}
	ALIF_XDECREF(source);
	alifASTMem_free(astMem);
	return ret;
}


static AlifObject* alifRun_file(FILE* _fp, AlifObject* _filename,
	AlifIntT _start, AlifObject* _globals, AlifObject* _locals,
	AlifIntT _closeIt, AlifCompilerFlags* _flags) { // 1191
	AlifASTMem* astMem = alifASTMem_new();
	if (astMem == nullptr) {
		return nullptr;
	}

	ModuleTy mod{};
	mod = alifParser_astFromFile(_fp, _start, _filename, nullptr, nullptr,
		nullptr, _flags, nullptr, nullptr, astMem);

	if (_closeIt) {
		fclose(_fp);
	}

	AlifObject* ret{};
	if (mod != nullptr) {
		ret = run_mod(mod, _filename, _globals, _locals, _flags, astMem, nullptr, 0);
	}
	else {
		ret = nullptr;
	}
	alifASTMem_free(astMem);

	return ret;
}


AlifObject* alifRun_stringFlags(const char* _str, AlifIntT _start, AlifObject* _globals,
	AlifObject* _locals, AlifCompilerFlags* _flags) { // 1215

	return _alifRun_stringFlagsWithName(_str, nullptr, _start, _globals, _locals, _flags, 0);
}


static AlifObject* runEval_codeObj(AlifThread* _thread, AlifCodeObject* _co,
	AlifObject* _globals, AlifObject* _locals) { // 1258
	AlifObject* v{};
	//_alifRuntime_.signals.unhandledKeyboardInterrupt = 0;

	if (!_globals or !ALIFDICT_CHECK(_globals)) {
		//alifErr_setString(_alifExcSystemError_, "globals must be a real dict");
		return nullptr;
	}
	AlifIntT hasBuiltins = alifDict_containsString(_globals, "__builtins__");
	if (hasBuiltins < 0) {
		return nullptr;
	}
	if (!hasBuiltins) {
		if (alifDict_setItemString(_globals, "__builtins__",
			_thread->interpreter->builtins) < 0) {
			return nullptr;
		}
	}

	v = alifEval_evalCode((AlifObject*)_co, _globals, _locals);
	//if (!v and _alifErr_Occurred(_thread) == _alifExcKeyboardInterrupt_) {
	//	_alifRuntime_.signals.unhandledKeyboardInterrupt = 1;
	//}
	return v;
}

static void flush_ioStream(AlifThread* _thread, AlifObject* _name) { // 1268
	AlifObject* f = _alifSys_getAttr(_thread, _name);
	if (f != nullptr) {
		if (_alifFile_flush(f) < 0) {
			alifErr_clear();
		}
	}
}

static void flush_io(void) { // 1279
	AlifThread* thread = _alifThread_get();
	AlifObject* exc = _alifErr_getRaisedException(thread);
	flush_ioStream(thread, &ALIF_ID(Stderr));
	flush_ioStream(thread, &ALIF_ID(Stdout));
	_alifErr_setRaisedException(thread, exc);
}

static AlifObject* run_mod(ModuleTy _mod, AlifObject* _filename,
	AlifObject* _globals, AlifObject* _locals, AlifCompilerFlags* _flags,
	AlifASTMem* _astMem, AlifObject* _interactiveSrc, AlifIntT _generateNewSource) { // 1299
	AlifThread* thread = _alifThread_get();
	AlifObject* interactiveFilename = _filename;
	if (_interactiveSrc) {
		AlifInterpreter* interp = thread->interpreter;
		if (_generateNewSource) {
			//interactiveFilename = alifUStr_fromFormat(
			//	"%U-%d", _filename, interp->interactiveSrcCount++);
		}
		else {
			ALIF_INCREF(interactiveFilename);
		}
		if (interactiveFilename == nullptr) {
			return nullptr;
		}
	}

	AlifCodeObject* co = _alifAST_compile(_mod, interactiveFilename, _flags, -1, _astMem);
	if (co == nullptr) {
		if (_interactiveSrc) {
			ALIF_DECREF(interactiveFilename);
		}
		return nullptr;
	}

	//if (_interactiveSrc) {
	//	AlifObject* lineCacheModule = alifImport_importModule("linecache");

	//	if (lineCacheModule == nullptr) {
	//		ALIF_DECREF(co);
	//		ALIF_DECREF(interactiveFilename);
	//		return nullptr;
	//	}

	//	AlifObject* printTBFunc = alifObject_getAttrString(lineCacheModule, "_register_code");

	//	if (printTBFunc == nullptr) {
	//		ALIF_DECREF(co);
	//		ALIF_DECREF(interactiveFilename);
	//		ALIF_DECREF(lineCacheModule);
	//		return nullptr;
	//	}

	//	if (!alifCallable_check(printTBFunc)) {
	//		ALIF_DECREF(co);
	//		ALIF_DECREF(interactiveFilename);
	//		ALIF_DECREF(lineCacheModule);
	//		ALIF_DECREF(printTBFunc);
	//		alifErr_setString(_alifExcValueError_, "linecache._register_code is not callable");
	//		return nullptr;
	//	}

	//	AlifObject* result = alifObject_callFunction(
	//		printTBFunc, "OOO",
	//		interactiveFilename,
	//		_interactiveSrc,
	//		_filename
	//	);

	//	ALIF_DECREF(interactiveFilename);

	//	ALIF_DECREF(lineCacheModule);
	//	ALIF_XDECREF(printTBFunc);
	//	ALIF_XDECREF(result);
	//	if (!result) {
	//		ALIF_DECREF(co);
	//		return nullptr;
	//	}
	//}

	//if (_alifSys_audit(thread, "exec", "O", co) < 0) {
	//	ALIF_DECREF(co);
	//	return nullptr;
	//}

	AlifObject* v = runEval_codeObj(thread, co, _globals, _locals);
	ALIF_DECREF(co);
	return v;
}




AlifObject* alif_compileStringObject(const char* _str, AlifObject* _filename,
	AlifIntT _start, AlifCompilerFlags* _flags, AlifIntT _optimize) { // 1456
	AlifCodeObject* co{};
	ModuleTy mod{};
	AlifASTMem* astMem = alifASTMem_new();
	if (astMem == nullptr)
		return nullptr;

	mod = _alifParser_astFromString(_str, _filename, _start, _flags, astMem);
	if (mod == nullptr) {
		alifASTMem_free(astMem);
		return nullptr;
	}
	if (_flags and (_flags->flags & ALIFCF_ONLY_AST)) {
		if ((_flags->flags & ALIFCF_OPTIMIZED_AST) == ALIFCF_OPTIMIZED_AST) {
			if (_alifCompile_astOptimize(mod, _filename, _flags, _optimize, astMem) < 0) {
				return nullptr;
			}
		}
		AlifObject* result = alifAST_mod2obj(mod);
		alifASTMem_free(astMem);
		return result;
	}
	co = _alifAST_compile(mod, _filename, _flags, _optimize, astMem);
	alifASTMem_free(astMem);
	return (AlifObject*)co;
}

AlifObject* alif_compileStringExFlags(const char* _str, const char* _filenameStr,
	AlifIntT _start, AlifCompilerFlags* _flags, AlifIntT _optimize) { // 1486
	AlifObject* filename{}, * co{};
	filename = alifUStr_decodeFSDefault(_filenameStr);
	if (filename == nullptr)
		return nullptr;
	co = alif_compileStringObject(_str, filename, _start, _flags, _optimize);
	ALIF_DECREF(filename);
	return co;
}
