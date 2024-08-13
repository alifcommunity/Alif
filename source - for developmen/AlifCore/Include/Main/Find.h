#pragma once

#pragma warning(disable : 4996) // for disable unsafe functions error


template <typename STRINGLIB_CHAR>
int64_t find(const STRINGLIB_CHAR* _str, int64_t _strLen,
	const STRINGLIB_CHAR* _sub, int64_t _subLen,
	int64_t _offset)
{
	int64_t pos_;

	if (_subLen == 0)
		return _offset;

	pos_ = fastSearch(_str, _strLen, _sub, _subLen, -1, FAST_SEARCH);

	if (pos_ >= 0)
		pos_ += _offset;

	return pos_;
}

template <typename STRINGLIB_CHAR>
int64_t rfind(const STRINGLIB_CHAR* _str, int64_t _strLen,
	const STRINGLIB_CHAR* _sub, int64_t _subLen,
	int64_t _offset)
{
	int64_t pos_;

	if (_subLen == 0)
		return _strLen + _offset;

	pos_ = fastSearch(_str, _strLen, _sub, _subLen, -1, FAST_RSEARCH);

	if (pos_ >= 0)
		pos_ += _offset;

	return pos_;
}

template <typename STRINGLIB_CHAR>
int64_t find_slice(const STRINGLIB_CHAR* _str, int64_t _strLen,
	const STRINGLIB_CHAR* _sub, int64_t _subLen,
	int64_t _start, int64_t _end)
{
	return find(_str + _start, _end - _start, _sub, _subLen, _start);
}

template <typename STRINGLIB_CHAR>
int64_t rfind_slice(const STRINGLIB_CHAR* _str, int64_t _strLen,
	const STRINGLIB_CHAR* _sub, int64_t _subLen,
	int64_t _start, int64_t _end)
{
	return rfind(_str + _start, _end - _start, _sub, _subLen, _start);
}

#define FORMAT_BUFFER_SIZE 50

// in file ceval.c in 2369
int alifEval_sliceIndex(AlifObject* _v, int64_t* _pi)
{
	if (!ALIF_ISNONE(_v)) {
		int64_t x_;
		if ((_v->type_->asNumber != nullptr)) {
			x_ = alifInteger_asLongLong(_v);
		}
		else {
			return 0;
		}
		*_pi = x_;
	}
	return 1;
}

int parse_args_finds(const wchar_t* _functionName, AlifObject* _args,
	AlifObject** _subObj,
	int64_t* _start, int64_t* _end)
{
	AlifObject* tmpSubObj;
	int64_t tmpStart = 0;
	int64_t tmpEnd = LLONG_MAX;
	AlifObject* objStart = ALIF_NONE, * objEnd = ALIF_NONE;
	wchar_t format_[FORMAT_BUFFER_SIZE] = L"O|OO:";
	size_t len_ = wcslen(format_);

	wcsncpy(format_ + len_, _functionName, FORMAT_BUFFER_SIZE - len_);
	format_[FORMAT_BUFFER_SIZE - 1] = '\0';

	if (!alifArg_parseTuple(_args, format_, &tmpSubObj, &objStart, &objEnd))
		return 0;
	if (objStart != ALIF_NONE)
		if (!alifEval_sliceIndex(objStart, &tmpStart))
			return 0;
	if (objEnd != ALIF_NONE)
		if (!alifEval_sliceIndex(objEnd, &tmpEnd))
			return 0;

	*_start = tmpStart;
	*_end = tmpEnd;
	*_subObj = tmpSubObj;
	return 1;
}
