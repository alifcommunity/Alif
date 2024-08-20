#include "alif.h"

#include "AlifCore_FileUtils.h"




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
		// bpo-35883: Reject characters outside [U+0000; U+10ffff] range.
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

	return alif_decodeUTF8Ex(_arg, strlen(_arg), _wstr, _wlen, _reason, _errors);
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





//FILE* alif_fOpenObj(AlifObject* _path, const char* _mode) { // 1764
//	FILE* f{};
//#ifdef _WINDOWS
//	wchar_t wmode[10];
//	AlifIntT uSize;
//
//	uSize = MultiByteToWideChar(CP_ACP, 0, _mode, -1,
//		wmode, ALIF_ARRAY_LENGTH(wmode));
//
//	f = _wfopen((wchar_t*)((AlifUStrObject*)_path)->UTF, wmode);
//
//#else
//	//AlifObject* bytes;
//	//const char* pathBytes;
//
//	//if (!alifUStr_fsConverter(_path, &bytes))
//	//	return nullptr;
//	//pathBytes = (const char*)_alifWBytes_asString(bytes); // need review
//
//	//if (alifSys_audit("open", "Osi", _path, _mode, 0) < 0) {
//	//	ALIF_DECREF(bytes);
//	//	return nullptr;
//	//}
//
//	// temp
//	mbstate_t mbState{};
//	char dist[128];
//	::memset((void*)&mbState, 0, sizeof(mbState));
//	const wchar_t* src = (const wchar_t*)((AlifUStrObject*)_path)->UTF;
//	AlifSizeT size = wcsrtombs(dist, &src, 128, &mbState);
//	//
//
//	do {
//		f = fopen(dist, _mode);
//	} while (f == nullptr and errno == EINTR);
//	//AlifIntT savedErrno = errno;
//	//ALIF_DECREF(bytes);
//#endif
//
//	return f;
//}
