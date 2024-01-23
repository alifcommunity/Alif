#include "alif.h"

#include "AlifCore_AlifRun.h"
#include "AlifCore_AST.h"

#ifdef _WINDOWS
#include "windows.h"
#endif // _WINDOWS



int alifRun_fileObj(FILE* _fp, AlifObj* _fn, int _fClose) {

	int res{};
	if (_isatty(_fileno(_fp))) { // هذا يعني انه تفاعلي
		//res = alifRun_interactiveLoop(_fp, _fn);
		if (_close) {
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

	mod = alifRun_file(_fp, _fn, _fClose);

	exitcode = 0;

	return exitcode;
}





static AlifObj* alifRun_file(FILE* _fp, AlifObj* _fn, int _fClose) {
	Module modul{};
}
