#include "alif.h"

#include "AlifCore_AST.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_Object.h"
#include "AlifCore_Parser.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Run.h"


#ifdef _WINDOWS
#undef BYTE
#endif


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
		//ret = run_mod(mod, _filename, _globals, _locals, _flags, astMem, nullptr, 0);
	}
	else {
		ret = nullptr;
	}
	alifASTMem_free(astMem);

	return ret;
}
