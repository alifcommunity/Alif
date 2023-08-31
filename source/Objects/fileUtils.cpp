#include "alif.h"
#include "alifCore_fileUtils.h"
#include "alifCore_runtime.h"
#include "osDefs.h"
#include <locale.h>
//#include <stdlib.h>

#ifdef MS_WINDOWS
#  include <malloc.h>
#  include <windows.h>
#  include <winioctl.h>
//#  include "alifCore_fileUtilsWindows.h
#  if defined(MS_WINDOWS_GAMES) && !defined(MS_WINDOWS_DESKTOP)
#    define PATHCCH_ALLOW_LONG_PATHS 0x01
#  else
#    include <pathcch.h>
#  endif
//extern int winerror_to_errno(int);
#endif

#ifdef HAVE_LANGINFO_H
#include <langinfo.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#ifdef HAVE_NON_UNICODE_WCHAR_T_REPRESENTATION
#include <iconv.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

















static const size_t DECODE_ERROR = ((size_t)-1);



static int get_surroGateEscape(AlifErrorHandler _errors, int* _surroGateEscape)
{
	switch (_errors)
	{
	case Alif_Error_Strict:
		*_surroGateEscape = 0;
		return 0;
	case Alif_Error_SurroGateEscape:
		*_surroGateEscape = 1;
		return 0;
	default:
		return -1;
	}
}












































static int isValid_wideChar(wchar_t _ch)
{
#ifdef HAVENON_UNICODEWCHART_REPRESENTATION
	return 1;
#endif
	//if (ALIF_UNICODE_ISSURROGATE(_ch)) {
	//	return 0;
	//}
	//if (ch > MAX_UNICODE) {
	//	return 0;
	//}
	return 1;
}











static size_t alif_mbstowcs(wchar_t* _dest, const char* _src, size_t _n)
{
	size_t count = mbstowcs(_dest, _src, _n);
	if (_dest != nullptr && count != DECODE_ERROR) {
		for (size_t i = 0; i < count; i++) {
			wchar_t ch = _dest[i];
			if (!isValid_wideChar(ch)) {
				return DECODE_ERROR;
			}
		}
	}
	return count;
}






















































































































































































































































#if !defined(HAVE_MBRTOWC) || defined(USE_FORCE_ASCII)
static int decode_ascii(const char* arg, wchar_t** wstr, size_t* wlen, const char** reason, AlifErrorHandler errors)
{
	wchar_t* res{};
	unsigned char* in{};
	wchar_t* out{};
	size_t argSize = strlen(arg) + 1;

	int surrogateescape;
	if (get_surroGateEscape(errors, &surrogateescape) < 0) {
		return -3;
	}

	if (argSize > ALIFSIZE_T_MAX / sizeof(wchar_t)) {
		return -1;
	}
	res = (wchar_t*)alifMem_rawMalloc(argSize * sizeof(wchar_t)); // تم تغيير النوع بسبب ظهور خطأ
	if (!res) {
		return -1;
	}

	out = res;
	for (in = (unsigned char*)arg; *in; in++) {
		unsigned char ch = *in;
		if (ch < 128) {
			*out++ = ch;
		}
		else {
			if (!surrogateescape) {
				alifMem_rawFree(res);
				if (wlen) {
					*wlen = in - (unsigned char*)arg;
				}
				if (reason) {
					*reason = "decoding error";
				}
				return -2;
			}
			*out++ = 0xdc00 + ch;
		}
	}
	*out = 0;

	if (wlen != nullptr) {
		*wlen = out - res;
	}
	*wstr = res;
	return 0;
}
#endif 



static int decodeCurrent_locale(const char* _arg, wchar_t** _wstr, size_t* _wLen, const char** _reason, AlifErrorHandler _errors)
{
	wchar_t* res{};
	size_t argSize{};
	size_t count{};
#ifdef HAVE_MBRTOWC
	unsigned char* in{};
	wchar_t* out{};
	mbstate_t mbs{};
#endif

	int surrogateescape;
	if (get_surroGateEscape(_errors, &surrogateescape) < 0) {
		return -3;
	}

#ifdef HAVE_BROKEN_MBSTOWCS
	argSize = strlen(arg);
#else
	argSize = alif_mbstowcs(nullptr, _arg, 0);
#endif
	if (argSize != DECODE_ERROR) {
		if (argSize > ALIFSIZE_T_MAX / sizeof(wchar_t) - 1) {
			return -1;
		}
		res = (wchar_t*)alifMem_rawMalloc((argSize + 1) * sizeof(wchar_t));
		if (!res) {
			return -1;
		}

		count = alif_mbstowcs(res, _arg, argSize + 1);
		if (count != DECODE_ERROR) {
			*_wstr = res;
			if (_wLen != nullptr) {
				*_wLen = count;
			}
			return 0;
		}
		alifMem_rawFree(res);
	}

#ifdef HAVE_MBRTOWC
	argSize = strlen(_arg) + 1;
	if (argSize > ALIFSIZE_T_MAX / sizeof(wchar_t)) {
		return -1;
	}
	res = (wchar_t*)alifMem_rawMalloc(argSize * sizeof(wchar_t));
	if (!res) {
		return -1;
	}

	in = (unsigned char*)_arg;
	out = res;
	memset(&mbs, 0, sizeof mbs);
	while (argSize) {
		size_t converted = alif_mbrtowc(out, (char*)in, argSize, &mbs);
		if (converted == 0) {
			break;
		}

		if (converted == INCOMPLETE_CHARACTER) {
			goto decode_error;
		}

		if (converted == DECODE_ERROR) {
			if (!surrogateescape) {
				goto decode_error;
			}

			*out++ = 0xdc00 + *in++;
			argSize--;
			memset(&mbs, 0, sizeof mbs);
			continue;
		}

		assert(!ALIF_UNICODE_IS_SURROGATE(*out));

		in += converted;
		argSize -= converted;
		out++;
	}
	if (_wLen != nullptr) {
		*_wLen = out - res;
	}
	*_wstr = res;
	return 0;

decode_error:
	alifMem_rawFree(res);
	if (_wLen) {
		*_wLen = in - (unsigned char*)_arg;
	}
	if (_reason) {
		*_reason = "decoding error";
	}
	return -2;
#else 
	return decode_ascii(_arg, _wstr, _wLen, _reason, _errors);
#endif
}














































int alif_decodeLocaleEx(const char* _arg, wchar_t** _wstr, size_t* _wLen, const char** reason, int current_locale, AlifErrorHandler errors)
{
	if (current_locale) {
#ifdef ALIF_FORCE_UTF8LOCALE
		return alif_decodeUTF8Ex(arg, strlen(arg), wstr, wlen, reason, errors);
#else
		return decodeCurrent_locale(_arg, _wstr, _wLen, reason, errors);
#endif
	}

#ifdef ALIF_FORCEUTF8_FSENCODING
	return alif_decodeUTF8Ex(_arg, strlen(_arg), _wstr, _wLen, reason, errors);
#else
	int use_utf8 = (alifRuntime.preConfig.utf8Mode >= 1);
#ifdef MS_WINDOWS
	use_utf8 |= (alifRuntime.preConfig.legacyWindowsFSEncoding == 0);
#endif
	if (use_utf8) {
		return alif_decodeUTF8Ex(_arg, strlen(_arg), _wstr, _wLen, reason,
			errors);
	}

#ifdef USE_FORCE_ASCII
	if (force_ascii == -1) {
		force_ascii = check_force_ascii();
	}

	if (force_ascii) {
		return decode_ascii(_arg, _wstr, _wLen, reason, errors);
	}
#endif

	return decodeCurrent_locale(_arg, _wstr, _wLen, reason, errors);
#endif 
}



























wchar_t* alif_decodeLocale(const char* _arg, size_t* _wLen)
{
	wchar_t* wstr{};
	int res = alif_decodeLocaleEx(_arg, &wstr, _wLen, nullptr, 0, Alif_Error_SurroGateEscape);
	if (res != 0) {
		//assert(res != -3);
		if (_wLen != nullptr) {
			*_wLen = (size_t)res;
		}
		return nullptr;
	}
	return wstr;
}























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































int alif_isAbs(const wchar_t* _path)
{
#ifdef MS_WINDOWS
	const wchar_t* tail{};
	//HRESULT hr = PathCchSkipRoot(_path, &tail); // ربما غير مدعوم في نسخ فيجول ستوديو 2019 فما فوق
	//if (FAILED(hr) || _path == tail) {
	//	return 0;
	//}
	//if (tail == &_path[1] && (_path[0] == SEP || _path[0] == ALTSEP)) {
	//	return 0;
	//}
	//if (tail == &_path[2] && _path[1] == L':') {
	//	return 0;
	//}
	return 1;
#else
	return (path[0] == SEP);
#endif
}









int alif_absPath(const wchar_t* _path, wchar_t** _absPathP)
{
	if (_path[0] == '\0' || !wcscmp(_path, L".")) {
		wchar_t cwd[MAXPATHLEN + 1];
		cwd[ALIF_ARRAY_LENGTH(cwd) - 1] = 0;
		if (!alif_wGetCwd(cwd, ALIF_ARRAY_LENGTH(cwd) - 1)) {
			/* unable to get the current directory */
			return -1;
		}
		*_absPathP = alifMem_rawWcsDup(cwd);
		return 0;
	}

	if (alif_isAbs(_path)) {
		*_absPathP = alifMem_rawWcsDup(_path);
		return 0;
	}

#ifdef MS_WINDOWS
	return alifOS_getFullPathName(_path, _absPathP);
#else
	wchar_t cwd[MAXPATHLEN + 1];
	cwd[ALIF_ARRAY_LENGTH(cwd) - 1] = 0;
	if (!alif_wGetCwd(cwd, ALIF_ARRAY_LENGTH(cwd) - 1)) {
		/* unable to get the current directory */
		return -1;
	}

	size_t cwdLen = wcslen(cwd);
	size_t pathLen = wcslen(path);
	size_t len = cwdLen + 1 + pathLen + 1;
	if (len <= (size_t)ALIFSIZE_T_MAX / sizeof(wchar_t)) {
		*_absPathP = (wchar_t*)alifMem_rawMalloc(len * sizeof(wchar_t)); // تم تغيير للنوع بسبب ظهور خطأ
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

	memcpy(abspath, path, pathLen * sizeof(wchar_t));
	abspath += pathLen;

	*abspath = 0;
	return 0;
#endif
}

















































































































































































































































































































wchar_t* alif_wGetCwd(wchar_t* _buf, size_t _bufLen)
{
#ifdef MS_WINDOWS
	int ibuflen = (int)ALIF_MIN(_bufLen, INT_MAX);
	return _wgetcwd(_buf, ibuflen);
#else
	char fName[MAXPATHLEN];
	wchar_t* wName;
	size_t len;

	if (getcwd(fName, ALIF_ARRAY_LENGTH(fName)) == nullptr)
		return nullptr;
	wName = alif_decodeLocale(fName, &len);
	if (wName == nullptr)
		return nullptr;
	if (_bufLen <= len) {
		alifMem_rawFree(wName);
		return nullptr;
	}
	wcsncpy(_buf, wName, _bufLen);
	alifMem_rawFree(wName);
	return _buf;
#endif
}


