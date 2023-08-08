#include "alif.h"
#include "alifCore_getOpt.h"


int alifOsOptErr = 1;                 /* generate error messages */
AlifSizeT alifOsOptInd = 1;          /* index into argv array   */
const wchar_t* alifOsOptArg = nullptr;   /* optional argument       */

static const wchar_t* optPtr = L"";

/* Alif command line short and long options */

#define SHORT_OPTS L"bBc:dEhiIJm:OPqRsStuvVW:xX:?"

static const AlifOSLongOption longOpts[] = {
	/* name, hasArg, val (used in switch in initConfig.cpp) */
	{L"check-hash-based-alcs", 1, 0},
	{L"help-all", 0, 1},
	{L"help-env", 0, 2},
	{L"help-xoptions", 0, 3},
	{nullptr, 0, -1},
};


void alifOS_resetGetOpt()
{
	alifOsOptErr = 1;
	alifOsOptInd = 1;
	alifOsOptArg = nullptr;
	optPtr = L"";
}


int alifOS_getOpt(AlifSizeT _argc, wchar_t* const* _argv, int* _longIndex)
{
	wchar_t* ptr;
	wchar_t option;

	if (*optPtr == '\0') {

		if (alifOsOptInd >= _argc)
			return -1;
#ifdef MS_WINDOWS
		else if (wcscmp(_argv[alifOsOptInd], L"/?") == 0) {
			++alifOsOptInd;
			return 'h';
		}
#endif

		else if (_argv[alifOsOptInd][0] != L'-' ||
			_argv[alifOsOptInd][1] == L'\0' /* lone dash */)
			return -1;

		else if (wcscmp(_argv[alifOsOptInd], L"--") == 0) {
			++alifOsOptInd;
			return -1;
		}

		else if (wcscmp(_argv[alifOsOptInd], L"--help") == 0) {
			++alifOsOptInd;
			return 'h';
		}

		else if (wcscmp(_argv[alifOsOptInd], L"--version") == 0) {
			++alifOsOptInd;
			return 'V';
		}

		optPtr = &_argv[alifOsOptInd++][1];
	}

	if ((option = *optPtr++) == L'\0')
		return -1;

	if (option == L'-') {
		// Parse long option.
		if (*optPtr == L'\0') {
			if (alifOsOptErr) {
				fprintf(stderr, "expected long option\n");
			}
			return -1;
		}
		*_longIndex = 0;
		const AlifOSLongOption* opt;
		for (opt = &longOpts[*_longIndex]; opt->name; opt = &longOpts[++(*_longIndex)]) {
			if (!wcscmp(opt->name, optPtr))
				break;
		}
		if (!opt->name) {
			if (alifOsOptErr) {
				fprintf(stderr, "unknown option %ls\n", _argv[alifOsOptInd - 1]);
			}
			return '_';
		}
		optPtr = L"";
		if (!opt->hasArg) {
			return opt->val;
		}
		if (alifOsOptInd >= _argc) {
			if (alifOsOptErr) {
				fprintf(stderr, "Argument expected for the %ls options\n",
					_argv[alifOsOptInd - 1]);
			}
			return '_';
		}
		alifOsOptArg = _argv[alifOsOptInd++];
		return opt->val;
	}

	if (option == 'J') {
		if (alifOsOptErr) {
			fprintf(stderr, "-J is reserved for Jython\n");
		}
		return '_';
	}

	if ((ptr = (wchar_t*)wcschr(SHORT_OPTS, option)) == nullptr) { // تم تعديل القيمة المرجعة - يجب المراجعة قبل الإعتماد -
		if (alifOsOptErr) {
			fprintf(stderr, "Unknown option: -%c\n", (char)option);
		}
		return '_';
	}

	if (*(ptr + 1) == L':') {
		if (*optPtr != L'\0') {
			alifOsOptArg = optPtr;
			optPtr = L"";
		}

		else {
			if (alifOsOptInd >= _argc) {
				if (alifOsOptErr) {
					fprintf(stderr,
						"Argument expected for the -%c option\n", (char)option);
				}
				return '_';
			}

			alifOsOptArg = _argv[alifOsOptInd++];
		}
	}

	return option;
}
