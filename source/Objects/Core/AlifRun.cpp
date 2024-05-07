#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_AST.h"
#include "AlifCore_AlifRun.h"

#include "ParserEngine.h"

#ifdef _WINDOWS
#include "windows.h"
#endif // _WINDOWS


// Forward Declaration
static AlifObj* alifRun_file(FILE*, AlifObj*, int, int);


int alifRun_fileObj(FILE* _fp, AlifObj* _fn, int _fClose) {

	int res_{};
	if (_isatty(_fileno(_fp))) { // هذا يعني انه تفاعلي
		//res_ = alifRun_interactiveLoop(_fp, _fn);
		if (_fClose) {
			fclose(_fp);
		}
	}
	else {
		res_ = alifRun_simpleFileObj(_fp, _fn, _fClose);
	}


	return res_;
}


int alifRun_simpleFileObj(FILE* _fp, AlifObj* _fn, int _fClose) {
	int exitCode = -1;

	AlifObj* mod_{};

	mod_ = alifRun_file(_fp, _fn, ALIFFILE_INPUT, _fClose);

	exitCode = 0;

	return exitCode;
}





static AlifObj* alifRun_file(FILE* _fp, AlifObj* _fn, int _start, int _fClose) {

	/* يجب إسناد الذاكرة الى صنف داخلي وعدم تركها عامة */

	Module* module_{};
	module_ = alifParser_astFromFile(_fp, _fn, _start, &_alifMem_); // لا داعي لتمرير الذاكرة لانها متغير عام

	if (_fClose) {
		fclose(_fp);
	}

	AlifObj* res_{};
	if (module_) {
		//res_ = alifRun_module(module_, _fn, _alifMem_);
	}
	else {
		res_ = nullptr;
	}


	return res_; 
}
