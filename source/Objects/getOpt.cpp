
























#include <alif.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "alifCore_getOpt.h"





int alifOSOptErr = 1;
AlifSizeT alifOSOptind = 1;
const wchar_t* alifOSOptArg = nullptr;

static const wchar_t* optPtr = L"";



#define SHORT_OPTS L"bBc:dEhiIJm:OPqRsStuvVW:xX:?"

static const AlifOSLongOption longOpts[] = {
	{L"checkHashBasedAlifCS", 1, 0},
	{L"helpAll", 0, 1},
	{L"helpEnv", 0, 2},
	{L"helpXOptions", 0, 3},
	{nullptr, 0, -1},
};



void alifOS_resetGetOpt(void)
{
	alifOSOptErr = 1;
	alifOSOptind = 1;
	alifOSOptArg = nullptr;
	optPtr = L"";
}

int alifOS_getOpt(AlifSizeT _argc, wchar_t* const* _argv, int* _longIndex)
{
	wchar_t* ptr;
	wchar_t option;

	if (*optPtr == '\0') {

		if (alifOSOptind >= _argc)
			return -1;
#ifdef MS_WINDOWS
		else if (wcscmp(_argv[alifOSOptind], L"/?") == 0) {
			++alifOSOptind;
			return 'h';
		}
#endif

		else if (_argv[alifOSOptind][0] != L'-' ||
			_argv[alifOSOptind][1] == L'\0' /* lone dash */)
			return -1;

		else if (wcscmp(_argv[alifOSOptind], L"--") == 0) {
			++alifOSOptind;
			return -1;
		}

		else if (wcscmp(_argv[alifOSOptind], L"--help") == 0) {
			++alifOSOptind;
			return 'h';
		}

		else if (wcscmp(_argv[alifOSOptind], L"--version") == 0) {
			++alifOSOptind;
			return 'V';
		}

		optPtr = &_argv[alifOSOptind++][1];
	}

	if ((option = *optPtr++) == L'\0')
		return -1;

	if (option == L'-') {
		// Parse long option.
		if (*optPtr == L'\0') {
			if (alifOSOptErr) {
				fprintf(stderr, "expected long option\n");
			}
			return -1;
		}
		*_longIndex = 0;
		const AlifOSLongOption* opt{};
		for (opt = &longOpts[*_longIndex]; opt->name; opt = &longOpts[++(*_longIndex)]) {
			if (!wcscmp(opt->name, optPtr))
				break;
		}
		if (!opt->name) {
			if (alifOSOptErr) {
				fprintf(stderr, "unknown option %ls\n", _argv[alifOSOptind - 1]);
			}
			return '_';
		}
		optPtr = L"";
		if (!opt->hasArg) {
			return opt->val;
		}
		if (alifOSOptind >= _argc) {
			if (alifOSOptErr) {
				fprintf(stderr, "Argument expected for the %ls options\n",
					_argv[alifOSOptind - 1]);
			}
			return '_';
		}
		alifOSOptArg = _argv[alifOSOptind++];
		return opt->val;
	}

	if (option == 'J') {
		if (alifOSOptErr) {
			fprintf(stderr, "-J is reserved for Jalif\n");
		}
		return '_';
	}

	if ((ptr = (wchar_t*)wcschr(SHORT_OPTS, option)) == nullptr) { // تم تعديل النوع المرجع بسبب ظهور خطأ
		if (alifOSOptErr) {
			fprintf(stderr, "Unknown option: -%c\n", (char)option);
		}
		return '_';
	}

	if (*(ptr + 1) == L':') {
		if (*optPtr != L'\0') {
			alifOSOptArg = optPtr;
			optPtr = L"";
		}

		else {
			if (alifOSOptind >= _argc) {
				if (alifOSOptErr) {
					fprintf(stderr,
						"Argument expected for the -%c option\n", (char)option);
				}
				return '_';
			}

			alifOSOptArg = _argv[alifOSOptind++];
		}
	}

	return option;
}
