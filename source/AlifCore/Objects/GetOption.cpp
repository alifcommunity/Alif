#include "alif.h"
#include "AlifCore_GetOption.h"

// هذا المتغير يحتوي الاحرف التي قد يتم إستخدامها في سطر الطرفية
#define SHORT_OPTS L"c:hm:v"

static const LongOption _longOpts_[] = {
	{L"", 1, 0},
	{L"help-all", 0, 1},
	{L"help-env", 0, 2},
	{L"", 0, 3},
	{NULL, 0, -1},
};

/*
	هنا يتم عمل تحليل لسطر الطرفية
*/


AlifSizeT _optIdx_ = 1;
static const wchar_t* _optPtr_ = L"";
const wchar_t* _optArg_ = nullptr;


void alif_resetGetOption() { // 52
	_optIdx_ = 1;
	_optArg_ = nullptr;
	_optPtr_ = L"";
}


AlifIntT alif_getOption(AlifSizeT _argc, wchar_t* const* _argv, AlifIntT* _longIndex) { // 60
	wchar_t* ptr{};
	wchar_t option{};

	if (*_optPtr_ == '\0') {
		if (_optIdx_ >= _argc) {
			return -1;
		}
		else if (_argv[_optIdx_][0] != L'-' or _argv[_optIdx_][1] == L'\0') {
			return -1;
		}
		else if (wcscmp(_argv[_optIdx_], L"--version") == 0) {
			++_optIdx_;
			return 'v';
		}
		else if (wcscmp(_argv[_optIdx_], L"--help") == 0) {
			++_optIdx_;
			return 'h';
		}

		_optPtr_ = &_argv[_optIdx_++][1];
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
			fprintf(stderr, "خيار غير معروف %ls\n", _argv[_optIdx_ - 1]);
			return '_';
		}
		_optPtr_ = L"";
		if (!opt->hasArg) {
			return opt->val;
		}
		if (_optIdx_ >= _argc) {
			fprintf(stderr, "يتوقع وجود وسيط للخيار %ls\n", _argv[_optIdx_ - 1]);
			return '_';
		}
		_optArg_ = _argv[_optIdx_++];
		return opt->val;
	}

	if ((ptr = (wchar_t*)wcschr(SHORT_OPTS, option)) == nullptr) {
		fprintf(stderr, "خيار غير معروف: -%c\n", (char)option);
		return '-';
	}

	if (*(ptr + 1) == L':') {
		if (*_optPtr_ != L'\0') {
			_optArg_ = _optPtr_;
			_optPtr_ = L"";
		}
		else {
			if (_optIdx_ >= _argc) {
				return '-';
			}

			_optArg_ = _argv[_optIdx_++];
		}
	}

	return option;
}
