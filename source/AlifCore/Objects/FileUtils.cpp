#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_DureRun.h"

#include "OSDefs.h"


#include <stdlib.h>               // mbstowcs()
#ifdef HAVE_UNISTD_H
#include <unistd.h>             // getcwd()
#endif

#ifdef _WINDOWS
#include <pathcch.h>
#endif


#define MAX_UNICODE 0x10ffff // 53

// mbstowcs() and mbrtowc() errors
static const AlifUSizeT _decodeError_ = ((size_t)-1); // 56
static const AlifUSizeT _incompleteCharActer_ = (size_t)-2;



static AlifIntT get_surrogateEscape(AlifErrorHandler_ _errors, AlifIntT* _surrogateEscape) { // 60
	switch (_errors) {
	case AlifErrorHandler_::Alif_Error_Strict:
		*_surrogateEscape = 0;
		return 0;
	case AlifErrorHandler_::Alif_Error_SurrogateEscape:
		*_surrogateEscape = 1;
		return 0;
	default:
		return -1;
	}
}


static AlifIntT isValid_wideChar(wchar_t _ch) { // 118
#ifdef HAVE_NON_UNICODE_WCHAR_T_REPRESENTATION
	/* Oracle Solaris doesn't use Unicode code points as wchar_t encoding
	   for non-Unicode locales, which makes values higher than MAX_UNICODE
	   possibly valid. */
	return 1;
#endif
	if (alifUnicode_isSurrogate(_ch)) {
		// Reject lone surrogate characters
		return 0;
	}
	if (_ch > MAX_UNICODE) {
		// Reject characters outside [U+0000; U+10ffff] range.
		// The glibc mbstowcs() UTF-8 decoder does not respect the RFC 3629,
		// it creates characters outside the [U+0000; U+10ffff] range:
		// https://sourceware.org/bugzilla/show_bug.cgi?id=2373
		return 0;
	}
	return 1;
}

static AlifUSizeT alif_mbstowcs(wchar_t* _dest, const char* _src, AlifUSizeT _n) { // 142
	AlifUSizeT count = mbstowcs(_dest, _src, _n);
	if (_dest != nullptr and count != _decodeError_) {
		for (AlifUSizeT i = 0; i < count; i++) {
			wchar_t ch = _dest[i];
			if (!isValid_wideChar(ch)) {
				return _decodeError_;
			}
		}
	}
	return count;
}


static AlifIntT decode_currentLocale(const char* _arg, wchar_t** _wstr, size_t* _wlen,
	const char** _reason, AlifErrorHandler_ _errors) { // 454
	wchar_t* res{};
	AlifUSizeT argSize{};
	AlifUSizeT count{};
#ifdef HAVE_MBRTOWC
	unsigned char* in_{};
	wchar_t* out{};
	mbstate_t mbs{};
#endif

	AlifIntT surrogateescape{};
	if (get_surrogateEscape(_errors, &surrogateescape) < 0) {
		return -3;
	}

#ifdef HAVE_BROKEN_MBSTOWCS
	/* Some platforms have a broken implementation of
	 * mbstowcs which does not count the characters that
	 * would result from conversion.  Use an upper bound.
	 */
	argSize = strlen(_arg);
#else
	argSize = alif_mbstowcs(nullptr, _arg, 0);
#endif
	if (argSize != _decodeError_) {
		if (argSize > ALIF_SIZET_MAX / sizeof(wchar_t) - 1) {
			return -1;
		}
		res = (wchar_t*)alifMem_dataAlloc((argSize + 1) * sizeof(wchar_t));
		if (!res) {
			return -1;
		}

		count = alif_mbstowcs(res, _arg, argSize + 1);
		if (count != _decodeError_) {
			*_wstr = res;
			if (_wlen != nullptr) {
				*_wlen = count;
			}
			return 0;
		}
		alifMem_dataFree(res);
	}

	/* Conversion failed. Fall back to escaping with surrogateescape. */
#ifdef HAVE_MBRTOWC
	/* Try conversion with mbrtwoc (C99), and escape non-decodable bytes. */

	/* Overallocate; as multi-byte characters are in the argument, the
	   actual output could use less memory. */
	argSize = strlen(_arg) + 1;
	if (argSize > ALIF_SIZET_MAX / sizeof(wchar_t)) {
		return -1;
	}
	res = (wchar_t*)alifMem_dataAlloc(argSize * sizeof(wchar_t));
	if (!res) {
		return -1;
	}

	in_ = (unsigned char*)_arg;
	out = res;
	memset(&mbs, 0, sizeof mbs);
	while (argSize) {
		AlifUSizeT converted = alif_mbrtowc(out, (char*)in_, argSize, &mbs);
		if (converted == 0) {
			/* Reached end of string; null char stored. */
			break;
		}

		if (converted == _incompleteCharActer_) {
			/* Incomplete character. This should never happen,
			   since we provide everything that we have -
			   unless there is a bug in the Cpp library, or I
			   misunderstood how mbrtowc works. */
			goto decode_error;
		}

		if (converted == _decodeError_) {
			if (!surrogateescape) {
				goto decode_error;
			}

			/* Decoding error. Escape as UTF-8b, and start over in the initial
			   shift state. */
			*out++ = 0xdc00 + *in_++;
			argSize--;
			memset(&mbs, 0, sizeof mbs);
			continue;
		}

		/* successfully converted some bytes */
		in_ += converted;
		argSize -= converted;
		out++;
	}
	if (_wlen != nullptr) {
		*_wlen = out - res;
	}
	*_wstr = res;
	return 0;

decode_error:
	alifMem_dataFree(res);
	if (_wlen) {
		*_wlen = in_ - (unsigned char*)_arg;
	}
	if (_reason) {
		*_reason = "decoding error";
	}
	return -2;
#else   /* HAVE_MBRTOWC */
	/*
		لا يمكن ترميز النص على هذه المنصة
		لأنها لا تحتوي mbrtowc
	*/
	// خطأ ترميز هنا
	return -2;
#endif   /* HAVE_MBRTOWC */
}


AlifIntT alif_decodeLocaleEx(const char* _arg, wchar_t** _wstr, AlifUSizeT* _wlen,
	const char** _reason, AlifIntT _currentLocale, AlifErrorHandler_ _errors) { // 601
	if (_currentLocale) {
#ifdef ALIF_FORCE_UTF8_LOCALE
		return alif_decodeUTF8Ex(_arg, strlen(_arg), _wstr, _wlen, _reason, _errors);
#else
		return decode_currentLocale(_arg, _wstr, _wlen, _reason, _errors);
#endif
	}

//#ifdef ALIF_FORCE_UTF8_FS_ENCODING
//	return _alif_decodeUTF8Ex(_arg, strlen(_arg), _wstr, _wlen, _reason, _errors);
//#else
//	AlifIntT useUTF8 = (_alifDureRun_.preConfig.utf8Mode >= 1);
//#ifdef _WINDOWS
//	useUTF8 |= (_alifDureRun_.preConfig.legacyWindowsFSEncoding == 0);
//#endif
//	if (useUTF8) {
//		return _alif_decodeUTF8Ex(_arg, strlen(_arg), _wstr, _wlen, _reason, _errors);
//	}
//
//#ifdef USE_FORCE_ASCII
//	if (FORCE_ASCII == -1) {
//		FORCE_ASCII = check_forceASCII();
//	}
//
//	if (FORCE_ASCII) {
//		/* force ASCII encoding to workaround mbstowcs() issue */
//		return decode_ascii(_arg, _wstr, _wlen, _reason, _errors);
//	}
//#endif

	return alif_decodeUTF8Ex(_arg, strlen(_arg), _wstr, _wlen, _reason, _errors);

//#endif
}

wchar_t* alif_decodeLocale(const char* _arg, AlifUSizeT* _wlen) { // 663
	wchar_t* wstr{};
	AlifIntT res = alif_decodeLocaleEx(_arg, &wstr, _wlen, nullptr,
		0, AlifErrorHandler_::Alif_Error_SurrogateEscape);

	if (res != 0) {
		if (_wlen != nullptr) {
			*_wlen = (size_t)res;
		}
		return nullptr;
	}
	return wstr;
}


static AlifIntT encode_currentLocale(const wchar_t* _text, char** _str,
	AlifUSizeT* _errorPos, const char** _reason,
	AlifIntT _rawMalloc, AlifErrorHandler_ _errors) { // 681
	const AlifUSizeT len = wcslen(_text);
	char* result = nullptr, * bytes = nullptr;
	AlifUSizeT i_{}, size{}, converted{};
	wchar_t c_{}, buf[2]{};

	AlifIntT surrogateescape{};
	if (get_surrogateEscape(_errors, &surrogateescape) < 0) {
		return -3;
	}

	/* The function works in two steps:
	   1. compute the length of the output buffer in bytes (size)
	   2. outputs the bytes */
	size = 0;
	buf[1] = 0;
	while (1) {
		for (i_ = 0; i_ < len; i_++) {
			c_ = _text[i_];
			if (c_ >= 0xdc80 and c_ <= 0xdcff) {
				if (!surrogateescape) {
					goto encode_error;
				}
				/* UTF-8b surrogate */
				if (bytes != nullptr) {
					*bytes++ = c_ - 0xdc00;
					size--;
				}
				else {
					size++;
				}
				continue;
			}
			else {
				buf[0] = c_;
				if (bytes != nullptr) {
					converted = wcstombs(bytes, buf, size);
				}
				else {
					converted = wcstombs(nullptr, buf, 0);
				}
				if (converted == _decodeError_) {
					goto encode_error;
				}
				if (bytes != nullptr) {
					bytes += converted;
					size -= converted;
				}
				else {
					size += converted;
				}
			}
		}
		if (result != nullptr) {
			*bytes = '\0';
			break;
		}

		size += 1; /* nul byte at the end */
		if (_rawMalloc) {
			result = (char*)malloc(size);
		}
		else {
			result = (char*)alifMem_dataAlloc(size);
		}
		if (result == nullptr) {
			return -1;
		}
		bytes = result;
	}
	*_str = result;
	return 0;

encode_error:
	if (_rawMalloc) {
		free(result);
	}
	else {
		alifMem_dataFree(result);
	}
	if (_errorPos != nullptr) {
		*_errorPos = i_;
	}
	if (_reason) {
		*_reason = "encoding error";
	}
	return -2;
}



static AlifIntT encode_localeEX(const wchar_t* _text, char** _str, AlifUSizeT* _errorPos,
	const char** _reason, AlifIntT _rawMalloc,
	AlifIntT _currentLocale, AlifErrorHandler_ _errors) { // 792

	if (_currentLocale) {
#ifdef ALIF_FORCE_UTF8_LOCALE
		return _alif_encodeUTF8Ex(_text, _str, _errorPos, _reason,
			_rawMalloc, _errors);
#else
		return encode_currentLocale(_text, _str, _errorPos, _reason,
			_rawMalloc, _errors);
#endif
	}

//#ifdef ALIF_FORCE_UTF8_FS_ENCODING
//	return _alif_encodeUTF8Ex(_text, _str, _errorpos, _reason,
//		_rawMalloc, _errors);
//#else
//	AlifIntT useUTF8 = (_alifDureRun_.preConfig.utf8Mode >= 1);
//#ifdef _WINDOWS
//	useUTF8 |= (_alifDureRun_.preConfig.legacyWindowsFSEncoding == 0);
//#endif
//	if (useUTF8) {
//		return _alif_encodeUTF8Ex(_text, _str, _errorPos, _reason,
//			_rawMalloc, _errors);
//	}
//
//#ifdef USE_FORCE_ASCII
//	if (FORCE_ASCII == -1) {
//		FORCE_ASCII = check_forceASCII();
//	}
//
//	if (FORCE_ASCII) {
//		return encode_ascii(_text, _str, _errorPos, _reason,
//			_rawMalloc, _errors);
//	}
//#endif

	return encode_currentLocale(_text, _str, _errorPos, _reason, _rawMalloc, _errors);

//#endif
}


AlifIntT _alif_encodeLocaleEx(const wchar_t* _text, char** _str,
	AlifUSizeT* _error_pos, const char** _reason,
	AlifIntT _currentLocale, AlifErrorHandler_ _errors) { // 881
	return encode_localeEX(_text, _str, _error_pos, _reason, 1,
		_currentLocale, _errors);
}





FILE* alif_fOpenObj(AlifObject* _path, const char* _mode) { // 1764
	FILE* f{};
    AlifIntT asyncErr = 0;
#ifdef _WINDOWS
	wchar_t wmode[10]{};
	AlifIntT uSize{};

	if (!ALIFUSTR_CHECK(_path)) {
		//alifErr_format(_alifExcTypeError_,
		//	"str file path expected under Windows, got %R",
		//	ALIF_TYPE(_path));
		return nullptr;
	}

	wchar_t* wpath = alifUStr_asWideCharString(_path, nullptr);
	if (wpath == nullptr) return nullptr;

	uSize = MultiByteToWideChar(CP_ACP, 0, _mode, -1,
		wmode, ALIF_ARRAY_LENGTH(wmode));

	if (uSize == 0) {
		//alifErr_setFromWindowsErr(0);
		//alifMem_free(wpath);
		return nullptr;
	}

	do {
		ALIF_BEGIN_ALLOW_THREADS
			f = _wfopen(wpath, wmode);
		ALIF_END_ALLOW_THREADS
	} while (f == nullptr and errno == EINTR /*and !(async_err = alifErr_checkSignals())*/);
	AlifIntT savedErrNo = errno;
	alifMem_dataFree(wpath);

#else
	AlifObject* bytes{};
	const char* pathBytes{};

	if (!alifUStr_fsConverter(_path, &bytes)) return nullptr;
	pathBytes = ALIFBYTES_AS_STRING(bytes);

	do {
		ALIF_BEGIN_ALLOW_THREADS
			f = fopen(pathBytes, _mode);
		ALIF_END_ALLOW_THREADS
	} while (f == nullptr and errno == EINTR /*and !(async_err = alifErr_checkSignals())*/);
	AlifIntT savedErrNo = errno;
	//ALIF_DECREF(bytes); // كائن بايت لا يملك dealloc - يجب مراجعة المشكلة
#endif
	if (asyncErr) return nullptr;

	if (f == nullptr) {
		errno = savedErrNo;
		//alifErr_setFromErrnoWithFilenameObject(_alifExcOSError_, _path);
		return nullptr;
	}

	//if (set_inheritable(fileno(f), 0, 1, nullptr) < 0) {
	//	fclose(f);
	//	return nullptr;
	//}
	return f;
}



AlifIntT alif_isAbs(const wchar_t* _path) { // 2147
#ifdef _WINDOWS
	const wchar_t* tail{};
	HRESULT hr = PathCchSkipRoot(_path, &tail); // تمت إضافة مكتبة pathcch.lib; الى Linker->Input
	if (FAILED(hr) or _path == tail) {
		return 0;
	}
	if (tail == &_path[1] and (_path[0] == SEP or _path[0] == ALTSEP)) {
		// Exclude paths with leading SEP
		return 0;
	}
	if (tail == &_path[2] and _path[1] == L':') {
		// Exclude drive-relative paths (e.g. C:filename.ext)
		return 0;
	}
	return 1;
#else
	return (_path[0] == SEP);
#endif
}


AlifIntT alif_absPath(const wchar_t* _path, wchar_t** _absPathP) { // 2176
	if (_path[0] == '\0' or !wcscmp(_path, L".")) {
		wchar_t cwd[MAXPATHLEN + 1]{};
		cwd[ALIF_ARRAY_LENGTH(cwd) - 1] = 0;
		if (!alif_wGetCWD(cwd, ALIF_ARRAY_LENGTH(cwd) - 1)) {
			/* unable to get the current directory */
			return -1;
		}
		*_absPathP = alifMem_wcsDup(cwd);
		return 0;
	}

	if (alif_isAbs(_path)) {
		*_absPathP = alifMem_wcsDup(_path);
		return 0;
	}

#ifdef _WINDOWS
	return alifOS_getFullPathName(_path, _absPathP);
#else
	wchar_t cwd[MAXPATHLEN + 1]{};
	cwd[ALIF_ARRAY_LENGTH(cwd) - 1] = 0;
	if (!alif_wGetCWD(cwd, ALIF_ARRAY_LENGTH(cwd) - 1)) {
		/* unable to get the current directory */
		return -1;
	}

	AlifUSizeT cwdLen = wcslen(cwd);
	AlifUSizeT pathLen = wcslen(_path);
	AlifUSizeT len = cwdLen + 1 + pathLen + 1;
	if (len <= (AlifUSizeT)ALIF_SIZET_MAX / sizeof(wchar_t)) {
		*_absPathP = (wchar_t*)alifMem_dataAlloc(len * sizeof(wchar_t));
	}
	else {
		*_absPathP = nullptr;
	}
	if (*_absPathP == nullptr) {
		return 0;
	}

	wchar_t* abspath = *_absPathP;
	memcpy(abspath, cwd, cwdLen * sizeof(wchar_t));
	abspath += cwdLen;

	*abspath = (wchar_t)SEP;
	abspath++;

	memcpy(abspath, _path, pathLen * sizeof(wchar_t));
	abspath += pathLen;

	*abspath = 0;
	return 0;
#endif
}



wchar_t* alif_wGetCWD(wchar_t* _buf, AlifUSizeT _bufLen) { // 2620
#ifdef _WINDOWS
	AlifIntT iBufLen = (AlifIntT)ALIF_MIN(_bufLen, INT_MAX);
	return _wgetcwd(_buf, iBufLen);
#else
	char fName[MAXPATHLEN]{};
	wchar_t* wName{};
	AlifUSizeT len_{};

	if (getcwd(fName, ALIF_ARRAY_LENGTH(fName)) == nullptr)
		return nullptr;
	wName = alif_decodeLocale(fName, &len_);
	if (wName == nullptr)
		return nullptr;
	if (_bufLen <= len_) {
		alifMem_dataFree(wName);
		return nullptr;
	}
	wcsncpy(_buf, wName, _bufLen);
	alifMem_dataFree(wName);
	return _buf;
#endif
}






void* _alifGet_osfHandleNoRaise(AlifIntT fd) { // 2836
	void* handle{};
	ALIF_BEGIN_SUPPRESS_IPH
	handle = (void*)_get_osfhandle(fd);
	ALIF_END_SUPPRESS_IPH
	return handle;
}
