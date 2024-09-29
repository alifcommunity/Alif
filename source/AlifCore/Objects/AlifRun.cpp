#include "alif.h"

#include "AlifCore_Interpreter.h"
#include "AlifCore_Object.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Run.h"


#ifdef _WINDOWS
#undef BYTE
#endif







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
