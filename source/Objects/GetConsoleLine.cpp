#include "alif.h"
#include "AlifCore_GetConsoleLine.h"


AlifSizeT optInd = 1;
static const wchar_t* optPtr = L"";


void alif_resetConsoleLine() {
	optInd = 1;
	optPtr = L"";
}


int alif_getConsoleLine(AlifSizeT _argc, wchar_t* const* _argv, int* _index) {
	if (*optPtr == '\0') {
		if (optInd >= _argc) {
			return -1;
		}
		else if (_argv[optInd][0] != L'-' or _argv[optInd][1] == L'\0') {
			return -1;
		}
		else if (wcscmp(_argv[optInd], L"--version") == 0) {
			++optInd;
			return 'V';
		}

		optPtr = &_argv[optInd++][1];
	}
}
