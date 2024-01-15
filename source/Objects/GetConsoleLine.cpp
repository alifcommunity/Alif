#include "alif.h"
#include "AlifCore_GetConsoleLine.h"

// هذا المتغير يحتوي الاحرف التي قد يتم إستخدامها في سطر الطرفية
#define SHORT_OPTS L"c:hm:v"

/*
	هنا يتم عمل تحليل لسطر الطرفية
*/


AlifSizeT optIdx = 1;
static const wchar_t* optPtr = L"";
const wchar_t* optArg = nullptr;


void alif_resetConsoleLine() {
	optIdx = 1;
	optArg = nullptr;
	optPtr = L"";
}


int alif_getConsoleLine(AlifSizeT _argc, wchar_t* const* _argv) {
	wchar_t* ptr{};
	wchar_t option{};

	if (*optPtr == '\0') {
		if (optIdx >= _argc) {
			return -1;
		}
		else if (_argv[optIdx][0] != L'-' or _argv[optIdx][1] == L'\0') {
			return -1;
		}
		else if (wcscmp(_argv[optIdx], L"--version") == 0) {
			++optIdx;
			return 'v';
		}
		else if (wcscmp(_argv[optIdx], L"--help") == 0) {
			++optIdx;
			return 'h';
		}

		optPtr = &_argv[optIdx++][1];
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
			if (optIdx >= _argc) {
				return '-';
			}

			optArg = _argv[optIdx++];
		}
	}

	return option;
}
