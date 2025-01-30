#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Compile.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Object.h"
#include "AlifCore_Parser.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Run.h"


#ifdef _WINDOWS
#undef BYTE
#endif


static AlifObject* run_mod(ModuleTy, AlifObject*, AlifObject*,
	AlifObject*, AlifCompilerFlags*, AlifASTMem*, AlifObject*, AlifIntT); // 42
static AlifObject* alifRun_file(FILE*, AlifObject*, AlifIntT,
	AlifObject*, AlifObject*, AlifIntT, AlifCompilerFlags*); // 47





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
	//flush_io();
	if (v == nullptr) {
		ALIF_CLEAR(mainModule);
		//* alif
		const char* str{};
		AlifThread* thread = _alifThread_get();
		AlifObject* errorExc = thread->currentException;
		if (errorExc) {
			AlifBaseExceptionObject* exc = (AlifBaseExceptionObject*)errorExc;
			AlifTupleObject* errArgs = (AlifTupleObject*)exc->args;
			AlifObject* args = ALIFTUPLE_GET_ITEM(errArgs, 0);
			AlifObject* arg = ALIFTUPLE_GET_ITEM(args, 0);

			AlifObject* errMsg = ALIFTUPLE_GET_ITEM(arg, 0);

			AlifObject* errSecArg = ALIFTUPLE_GET_ITEM(arg, 1);
			AlifObject* errLineNum = ALIFTUPLE_GET_ITEM(errSecArg, 1);

			const char* errorType = ALIF_TYPE(errorExc)->name;
			AlifSizeT lineNum = alifLong_asSizeT(errLineNum);
			str = alifUStr_asUTF8(errMsg);
			printf("%s: %s , السطر: %d \n", errorType, str, lineNum);
		}
		else {
			str = "خطأ غير معروف";
			printf("%s \n", str);
		}
		//* alif
		//alifErr_print();
		goto done;
	}
	ALIF_DECREF(v);
	ret = 0;
done:
	if (setFileName) {
		if (alifDict_popString(dict, "__file__", nullptr) < 0) {
			//alifErr_print();
		}
		if (alifDict_popString(dict, "__cached__", nullptr) < 0) {
			//alifErr_print();
		}
	}
	ALIF_XDECREF(mainModule);
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


static AlifObject* runEval_codeObj(AlifThread* _thread, AlifCodeObject* _co,
	AlifObject* _globals, AlifObject* _locals) { // 1258
	AlifObject* v{};
	//_alifDureRun_.signals.unhandledKeyboardInterrupt = 0;

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
			_thread->interpreter->builtins) < 0)
		{
			return nullptr;
		}
	}

	v = alifEval_evalCode((AlifObject*)_co, _globals, _locals);
	//if (!v and _alifErr_Occurred(_thread) == _alifExcKeyboardInterrupt_) {
	//	_alifDureRun_.signals.unhandledKeyboardInterrupt = 1;
	//}
	return v;
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
