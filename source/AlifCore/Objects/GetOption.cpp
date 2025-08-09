#include "alif.h"
#include "AlifCore_GetOption.h"

// هذا المتغير يحتوي الاحرف التي قد يتم إستخدامها في سطر الطرفية
#define SHORT_OPTS L"ص:مك:ن"

static const LongOption _longOpts_[] = {
	{L"", 1, 0},
	{L"help-all", 0, 1},
	{L"help-env", 0, 2},
	{L"", 0, 3},
	{nullptr, 0, -1},
};

/*
	هنا يتم عمل تحليل لسطر الطرفية
*/

AlifIntT _alifOSOptErr_ = 1;
AlifSizeT _alifOSOptInd_ = 1;
static const wchar_t* _optPtr_ = L"";
const wchar_t* _alifOSOptArg_ = nullptr;


void _alifOS_resetGetOpt(void) { // 52
	_alifOSOptErr_ = 1;
	_alifOSOptInd_ = 1;
	_alifOSOptArg_ = nullptr;
	_optPtr_ = L"";
}


AlifIntT _alifOS_getOpt(AlifSizeT _argc, wchar_t* const* _argv, AlifIntT* _longIndex) { // 60
	wchar_t* ptr{};
	wchar_t option{};

	if (*_optPtr_ == '\0') {
		if (_alifOSOptInd_ >= _argc) {
			return -1;
		}
	#ifdef _WINDOWS
		else if (wcscmp(_argv[_alifOSOptInd_], L"/?") == 0) {
			++_alifOSOptInd_;
			return L'م';
		}
	#endif
		else if (_argv[_alifOSOptInd_][0] != L'-' or _argv[_alifOSOptInd_][1] == L'\0') {
			return -1;
		}
		else if (wcscmp(_argv[_alifOSOptInd_], L"--نسخة") == 0) {
			++_alifOSOptInd_;
			return L'ن';
		}
		else if (wcscmp(_argv[_alifOSOptInd_], L"--مساعدة") == 0) {
			++_alifOSOptInd_;
			return L'م';
		}

		_optPtr_ = &_argv[_alifOSOptInd_++][1];
	}

	if ((option = *_optPtr_++) == L'\0') {
		return -1;
	}

	if (option == L'-') {
		// Parse long option.
		if (*_optPtr_ == L'\0') {
			fprintf(stderr, "يتوقع وجود خيار طويل\n");
			return -1;
		}
		*_longIndex = 0;
		const LongOption* opt;
		for (opt = &_longOpts_[*_longIndex]; opt->name; opt = &_longOpts_[++(*_longIndex)]) {
			if (!wcscmp(opt->name, _optPtr_))
				break;
		}
		if (!opt->name) {
			fprintf(stderr, "خيار غير معروف %ls\n", _argv[_alifOSOptInd_ - 1]);
			return '_';
		}
		_optPtr_ = L"";
		if (!opt->hasArg) {
			return opt->val;
		}
		if (_alifOSOptInd_ >= _argc) {
			fprintf(stderr, "يتوقع وجود وسيط للخيار %ls\n", _argv[_alifOSOptInd_ - 1]);
			return '_';
		}
		_alifOSOptArg_ = _argv[_alifOSOptInd_++];
		return opt->val;
	}

	if ((ptr = (wchar_t*)wcschr(SHORT_OPTS, option)) == nullptr) {
		fwprintf(stderr, L"خيار غير معروف: -%wc\n", (wchar_t)option);
		return '-';
	}

	if (*(ptr + 1) == L':') {
		if (*_optPtr_ != L'\0') {
			_alifOSOptArg_ = _optPtr_;
			_optPtr_ = L"";
		}
		else {
			if (_alifOSOptInd_ >= _argc) {
				return '-';
			}

			_alifOSOptArg_ = _argv[_alifOSOptInd_++];
		}
	}

	return option;
}
