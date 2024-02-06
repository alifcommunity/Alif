#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_AST.h"
#include "AlifCore_AlifRun.h"

#include "ParserEngine.h"

#ifdef _WINDOWS
#include "windows.h"
#endif // _WINDOWS


#define ALIFSINGLE_INPUT 256
#define ALIFFILE_INPUT 257
#define ALIFEVAL_INPUT 258
#define ALIFFUNCTYPE_INPUT 345

// Forward Declaration
static AlifObj* alifRun_file(FILE*, AlifObj*, int, int);


int alifRun_fileObj(FILE* _fp, AlifObj* _fn, int _fClose) {

	int res{};
	if (_isatty(_fileno(_fp))) { // هذا يعني انه تفاعلي
		//res = alifRun_interactiveLoop(_fp, _fn);
		if (_fClose) {
			fclose(_fp);
		}
	}
	else {
		res = alifRun_simpleFileObj(_fp, _fn, _fClose);
	}


	return res;
}


int alifRun_simpleFileObj(FILE* _fp, AlifObj* _fn, int _fClose) {
	int exitcode = -1;

	AlifObj* mod{};

	mod = alifRun_file(_fp, _fn, ALIFFILE_INPUT, _fClose);

	exitcode = 0;

	return exitcode;
}





static AlifObj* alifRun_file(FILE* _fp, AlifObj* _fn, int _start, int _fClose) {

	/* يجب إسناد الذاكرة الى صنف داخلي وعدم تركها عامة */

	Module* module_{};
	module_ = alifParser_astFromFile(_fp, _fn, _start, &alifMem); // لا داعي لتمرير الذاكرة لانها متغير عام

	if (_fClose) {
		fclose(_fp);
	}

	AlifObj* res{};
	if (module_) {
		//res = alifRun_module(module_, _fn, alifMem);
	}
	else {
		res = nullptr;
	}


	return res; 
}
