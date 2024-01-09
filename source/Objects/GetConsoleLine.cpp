#include "alif.h"
#include "AlifCore_GetConsoleLine.h"

#define SHORT_OPTS L"c:m:v"


AlifSizeT optInd = 1;
static const wchar_t* optPtr = L"";
const wchar_t* optArg = nullptr;


void alif_resetConsoleLine() {
	optInd = 1;
	optArg = nullptr;
	optPtr = L"";
}


int alif_getConsoleLine(AlifSizeT _argc, wchar_t* const* _argv) {
	wchar_t* ptr{};
	wchar_t option{};

	if (*optPtr == '\0') {
		if (optInd >= _argc) {
			return -1;
		}
		else if (_argv[optInd][0] != L'-' or _argv[optInd][1] == L'\0') {
			return -1;
		}
		else if (wcscmp(_argv[optInd], L"--version") == 0) {
			++optInd;
			return 'v';
		}
		else if (wcscmp(_argv[optInd], L"--help") == 0) {
			++optInd;
			return 'h';
		}

		optPtr = &_argv[optInd++][1];
	}

	if ((option = *optPtr++) == L'\0') {
		return -1;
	}

	if ((ptr = (wchar_t*)wcschr(SHORT_OPTS, option)) == nullptr) {
		return '-';
	}

	if (*(ptr + 1) == L':') {
		if (*optPtr != L'\0') {

		}
		else {
			if (optInd >= _argc) {
				return '-';
			}

			optArg = _argv[optInd++];
		}
	}

	return option;
}
