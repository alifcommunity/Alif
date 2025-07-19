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
extern int winerror_to_errno(int);
#endif

// 36
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>              // fcntl(F_GETFD)
#endif

#ifdef O_CLOEXEC
AlifIntT _alifOpenCloExecWorks_ = -1;
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


static AlifIntT decode_currentLocale(const char* _arg, wchar_t** _wstr, AlifUSizeT* _wlen,
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




static char* encode_locale(const wchar_t* _text, AlifUSizeT* _errorPos,
	AlifIntT _rawMalloc, AlifIntT _currentLocale) { // 836
	char* str{};
	AlifIntT res = encode_localeEX(_text, &str, _errorPos, nullptr,
		_rawMalloc, _currentLocale,
		AlifErrorHandler_::Alif_Error_SurrogateEscape);
	if (res != -2 and _errorPos) {
		*_errorPos = (AlifUSizeT)-1;
	}
	if (res != 0) {
		return nullptr;
	}
	return str;
}


char* _alif_encodeLocaleRaw(const wchar_t* text, AlifUSizeT* error_pos) { // 874
	return encode_locale(text, error_pos, 0, 0);
}


AlifIntT _alif_encodeLocaleEx(const wchar_t* _text, char** _str,
	AlifUSizeT* _error_pos, const char** _reason,
	AlifIntT _currentLocale, AlifErrorHandler_ _errors) { // 881
	return encode_localeEX(_text, _str, _error_pos, _reason, 0,
		_currentLocale, _errors);
}


// 1049
#ifdef _WINDOWS
static __int64 secs_between_epochs = 11644473600; /* Seconds between 1.1.1601 and 1.1.1970 */

static void FILE_TIME_to_time_t_nsec(FILETIME* in_ptr, time_t* time_out, int* nsec_out) { // 1052
	__int64 in;
	memcpy(&in, in_ptr, sizeof(in));
	*nsec_out = (int)(in % 10000000) * 100; /* FILETIME is in units of 100 nsec. */
	*time_out = ALIF_SAFE_DOWNCAST((in / 10000000) - secs_between_epochs, __int64, time_t);
}

static void LARGE_INTEGER_to_time_t_nsec(LARGE_INTEGER* in_ptr, time_t* time_out, int* nsec_out)
{ // 1065
	*nsec_out = (int)(in_ptr->QuadPart % 10000000) * 100; /* FILETIME is in units of 100 nsec. */
	*time_out = ALIF_SAFE_DOWNCAST((in_ptr->QuadPart / 10000000) - secs_between_epochs, __int64, time_t);
}

static int attributes_to_mode(DWORD attr) { // 1085
	int m = 0;
	if (attr & FILE_ATTRIBUTE_DIRECTORY)
		m |= _S_IFDIR | 0111; /* IFEXEC for user,group,other */
	else
		m |= _S_IFREG;
	if (attr & FILE_ATTRIBUTE_READONLY)
		m |= 0444;
	else
		m |= 0666;
	return m;
}

typedef union {
	FILE_ID_128 id;
	struct {
		uint64_t ino;
		uint64_t inoHigh;
	};
} id_128_to_ino;

void _alifAttribute_dataToStat(BY_HANDLE_FILE_INFORMATION* info, ULONG reparse_tag,
	FILE_BASIC_INFO* basic_info, FILE_ID_INFO* id_info,
	class AlifStatStruct* result) { // 1110
	memset(result, 0, sizeof(*result));
	result->mode = attributes_to_mode(info->dwFileAttributes);
	result->size = (((__int64)info->nFileSizeHigh) << 32) + info->nFileSizeLow;
	result->dev = id_info ? id_info->VolumeSerialNumber : info->dwVolumeSerialNumber;
	result->rdev = 0;
	/* st_ctime is deprecated, but we preserve the legacy value in our caller, not here */
	if (basic_info) {
		LARGE_INTEGER_to_time_t_nsec(&basic_info->CreationTime, &result->birthtime, &result->birthtimeNSec);
		LARGE_INTEGER_to_time_t_nsec(&basic_info->ChangeTime, &result->ctime, &result->ctimeNSec);
		LARGE_INTEGER_to_time_t_nsec(&basic_info->LastWriteTime, &result->mtime, &result->mtimeNSec);
		LARGE_INTEGER_to_time_t_nsec(&basic_info->LastAccessTime, &result->atime, &result->atimeNSec);
	}
	else {
		FILE_TIME_to_time_t_nsec(&info->ftCreationTime, &result->birthtime, &result->birthtimeNSec);
		FILE_TIME_to_time_t_nsec(&info->ftLastWriteTime, &result->mtime, &result->mtimeNSec);
		FILE_TIME_to_time_t_nsec(&info->ftLastAccessTime, &result->atime, &result->atimeNSec);
	}
	result->nlink = info->nNumberOfLinks;

	if (id_info) {
		id_128_to_ino file_id;
		file_id.id = id_info->FileId;
		result->ino = file_id.ino;
		result->inoHigh = file_id.inoHigh;
	}
	if (!result->ino and !result->inoHigh) {
		result->ino = (((uint64_t)info->nFileIndexHigh) << 32) + info->nFileIndexLow;
	}

	result->reparseTag = reparse_tag;
	if (info->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT &&
		reparse_tag == IO_REPARSE_TAG_SYMLINK) {
		/* set the bits that make this a symlink */
		result->mode = (result->mode & ~S_IFMT) | S_IFLNK;
	}
	result->fileAttributes = info->dwFileAttributes;
}



#endif // 1221


AlifIntT _alifFStat_noraise(AlifIntT fd, class AlifStatStruct* status) { // 1235
#ifdef _WINDOWS
	BY_HANDLE_FILE_INFORMATION info;
	FILE_BASIC_INFO basicInfo;
	FILE_ID_INFO idInfo;
	FILE_ID_INFO* pIdInfo = &idInfo;
	HANDLE h;
	AlifIntT type;

	h = _alifGet_osfHandleNoRaise(fd);

	if (h == INVALID_HANDLE_VALUE) {
		SetLastError(ERROR_INVALID_HANDLE);
		return -1;
	}
	memset(status, 0, sizeof(*status));

	type = GetFileType(h);
	if (type == FILE_TYPE_UNKNOWN) {
		DWORD error = GetLastError();
		if (error != 0) {
			errno = winerror_to_errno(error);
			return -1;
		}
		/* else: valid but unknown file */
	}

	if (type != FILE_TYPE_DISK) {
		if (type == FILE_TYPE_CHAR)
			status->mode = _S_IFCHR;
		else if (type == FILE_TYPE_PIPE)
			status->mode = _S_IFIFO;
		return 0;
	}

	if (!GetFileInformationByHandle(h, &info) ||
		!GetFileInformationByHandleEx(h, FileBasicInfo, &basicInfo, sizeof(basicInfo))) {
		/* The Win32 error is already set, but we also set errno for
		   callers who expect it */
		errno = winerror_to_errno(GetLastError());
		return -1;
	}

	if (!GetFileInformationByHandleEx(h, FileIdInfo, &idInfo, sizeof(idInfo))) {
		/* Failed to get FileIdInfo, so do not pass it along */
		pIdInfo = NULL;
	}

	_alifAttribute_dataToStat(&info, 0, &basicInfo, pIdInfo, status);
	return 0;
#else
	return fstat(fd, status);
#endif
}



AlifIntT _alif_wStat(const wchar_t* path, struct stat* buf) { // 1332
	AlifIntT err{};
#ifdef _WINDOWS
	struct _stat wstatbuf;
	err = _wstat(path, &wstatbuf);
	if (!err) {
		buf->st_mode = wstatbuf.st_mode;
	}
#else
	char* fname;
	fname = _alif_encodeLocaleRaw(path, nullptr);
	if (fname == nullptr) {
		errno = EINVAL;
		return -1;
	}
	err = stat(fname, buf);
	alifMem_dataFree(fname);
#endif
	return err;
}


//AlifIntT _alif_setInheritable(AlifIntT fd,
//	AlifIntT inheritable, AlifIntT* atomic_flag_works) { // 1604
//	return set_inheritable(fd, inheritable, 1, atomic_flag_works);
//}


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
	ALIF_DECREF(bytes);
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

AlifSizeT _alif_read(AlifIntT fd, void* buf, AlifUSizeT count) { // 1858
	AlifSizeT n{};
	AlifIntT err{};
	AlifIntT asyncErr = 0;

	if (count > ALIF_READ_MAX) {
		count = ALIF_READ_MAX;
	}

	ALIF_BEGIN_SUPPRESS_IPH
		do {
			ALIF_BEGIN_ALLOW_THREADS
				errno = 0;
#ifdef _WINDOWS
			_doserrno = 0;
			n = read(fd, buf, (AlifIntT)count);
			if (n < 0 and errno == EINVAL) {
				if (_doserrno == ERROR_NO_DATA) {
					errno = EAGAIN;
				}
			}
#else
			n = read(fd, buf, count);
#endif
			err = errno;
			ALIF_END_ALLOW_THREADS
		} while (n < 0 and err == EINTR /*and
			!(asyncErr = alifErr_checkSignals())*/);
		ALIF_END_SUPPRESS_IPH

		if (asyncErr) {
			errno = err;
			return -1;
		}
		if (n < 0) {
			//alifErr_setFromErrno(_alifExcOSError_);
			errno = err;
			return -1;
		}

		return n;
}

#ifdef HAVE_READLINK // 2054

AlifIntT alif_wReadLink(const wchar_t* path, wchar_t* buf, AlifUSizeT buflen) { // 2061
	char* cpath{};
	char cbuf[MAXPATHLEN]{};
	AlifUSizeT cbuf_len = ALIF_ARRAY_LENGTH(cbuf);
	wchar_t* wbuf{};
	AlifSizeT res{};
	AlifUSizeT r1{};

	cpath = _alif_encodeLocaleRaw(path, NULL);
	if (cpath == nullptr) {
		errno = EINVAL;
		return -1;
	}
	res = readlink(cpath, cbuf, cbuf_len);
	alifMem_dataFree(cpath);
	if (res == -1) {
		return -1;
	}
	if ((AlifUSizeT)res == cbuf_len) {
		errno = EINVAL;
		return -1;
	}
	cbuf[res] = '\0'; /* buf will be null terminated */
	wbuf = alif_decodeLocale(cbuf, &r1);
	if (wbuf == nullptr) {
		errno = EINVAL;
		return -1;
	}
	/* wbuf must have space to store the trailing NUL character */
	if (buflen <= r1) {
		alifMem_dataFree(wbuf);
		errno = EINVAL;
		return -1;
	}
	wcsncpy(buf, wbuf, buflen);
	alifMem_dataFree(wbuf);
	return (AlifIntT)r1;
}
#endif


#ifdef HAVE_REALPATH
wchar_t* alif_wRealPath(const wchar_t* _path,
	wchar_t* resolvedPath, AlifUSizeT _resolvedPathLen) { // 2110
	char* cpath{};
	char cresolved_path[MAXPATHLEN]{};
	wchar_t* wresolved_path{};
	char* res{};
	AlifUSizeT r{};
	cpath = _alif_encodeLocaleRaw(_path, nullptr);
	if (cpath == nullptr) {
		errno = EINVAL;
		return nullptr;
	}
	res = realpath(cpath, cresolved_path);
	alifMem_dataFree(cpath);
	if (res == nullptr)
		return nullptr;

	wresolved_path = alif_decodeLocale(cresolved_path, &r);
	if (wresolved_path == nullptr) {
		errno = EINVAL;
		return nullptr;
	}
	/* wresolved_path must have space to store the trailing NUL character */
	if (_resolvedPathLen <= r) {
		alifMem_dataFree(wresolved_path);
		errno = EINVAL;
		return nullptr;
	}
	wcsncpy(resolvedPath, wresolved_path, _resolvedPathLen);
	alifMem_dataFree(wresolved_path);
	return resolvedPath;
}
#endif




AlifIntT _alif_isAbs(const wchar_t* _path) { // 2147
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


AlifIntT _alif_absPath(const wchar_t* _path, wchar_t** _absPathP) { // 2176
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

	if (_alif_isAbs(_path)) {
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

void _alif_skipRoot(const wchar_t* path, AlifSizeT size,
	AlifSizeT* drvsize, AlifSizeT* rootsize) { // 2300
#ifndef _WINDOWS
#define IS_SEP(x) (*(x) == SEP)
	* drvsize = 0;
	if (!IS_SEP(&path[0])) {
		// Relative path, e.g.: 'foo'
		*rootsize = 0;
	}
	else if (!IS_SEP(&path[1]) || IS_SEP(&path[2])) {
		// Absolute path, e.g.: '/foo', '///foo', '////foo', etc.
		*rootsize = 1;
	}
	else {
		*rootsize = 2;
	}
#undef IS_SEP
#else
	const wchar_t* pEnd = size >= 0 ? &path[size] : NULL;
#define IS_END(x) (pEnd ? (x) == pEnd : !*(x))
#define IS_SEP(x) (*(x) == SEP || *(x) == ALTSEP)
#define SEP_OR_END(x) (IS_SEP(x) || IS_END(x))
	if (IS_SEP(&path[0])) {
		if (IS_SEP(&path[1])) {
			// Device drives, e.g. \\.\device or \\?\device
			// UNC drives, e.g. \\server\share or \\?\UNC\server\share
			AlifSizeT idx{};
			if (path[2] == L'?' and IS_SEP(&path[3]) and
				(path[4] == L'U' or path[4] == L'u') and
				(path[5] == L'N' or path[5] == L'n') and
				(path[6] == L'C' or path[6] == L'c') and
				IS_SEP(&path[7]))
			{
				idx = 8;
			}
			else {
				idx = 2;
			}
			while (!SEP_OR_END(&path[idx])) {
				idx++;
			}
			if (IS_END(&path[idx])) {
				*drvsize = idx;
				*rootsize = 0;
			}
			else {
				idx++;
				while (!SEP_OR_END(&path[idx])) {
					idx++;
				}
				*drvsize = idx;
				if (IS_END(&path[idx])) {
					*rootsize = 0;
				}
				else {
					*rootsize = 1;
				}
			}
		}
		else {
			// Relative path with root, e.g. \Windows
			*drvsize = 0;
			*rootsize = 1;
		}
	}
	else if (!IS_END(&path[0]) and path[1] == L':') {
		*drvsize = 2;
		if (IS_SEP(&path[2])) {
			// Absolute drive-letter path, e.g. X:\Windows
			*rootsize = 1;
		}
		else {
			// Relative path with drive, e.g. X:Windows
			*rootsize = 0;
		}
	}
	else {
		// Relative path, e.g. Windows
		*drvsize = 0;
		*rootsize = 0;
	}
#undef SEP_OR_END
#undef IS_SEP
#undef IS_END
#endif
}


static AlifIntT join_relfile(wchar_t* buffer, AlifUSizeT bufsize,
	const wchar_t* dirname, const wchar_t* relfile) { // 2394
#ifdef _WINDOWS
	if (FAILED(PathCchCombineEx(buffer, bufsize, dirname, relfile,
		PATHCCH_ALLOW_LONG_PATHS))) {
		return -1;
	}
#else
	AlifUSizeT dirlen = wcslen(dirname);
	AlifUSizeT rellen = wcslen(relfile);
	AlifUSizeT maxlen = bufsize - 1;
	if (maxlen > MAXPATHLEN || dirlen >= maxlen || rellen >= maxlen - dirlen) {
		return -1;
	}
	if (dirlen == 0) {
		// We do not add a leading separator.
		wcscpy(buffer, relfile);
	}
	else {
		if (dirname != buffer) {
			wcscpy(buffer, dirname);
		}
		AlifUSizeT relstart = dirlen;
		if (dirlen > 1 && dirname[dirlen - 1] != SEP) {
			buffer[dirlen] = SEP;
			relstart += 1;
		}
		wcscpy(&buffer[relstart], relfile);
	}
#endif
	return 0;
}

AlifIntT _alif_addRelfile(wchar_t* dirname,
	const wchar_t* relfile, AlifUSizeT bufsize) { // 2461
	return join_relfile(dirname, bufsize, dirname, relfile);
}


wchar_t* _alif_normPathAndSize(wchar_t* path,
	AlifSizeT size, AlifSizeT* normsize) { // 2487
	if ((size < 0 and !path[0]) or size == 0) {
		*normsize = 0;
		return path;
	}
	wchar_t* pEnd = size >= 0 ? &path[size] : NULL;
	wchar_t* p1 = path;     // sequentially scanned address in the path
	wchar_t* p2 = path;     // destination of a scanned character to be ljusted
	wchar_t* minP2 = path;  // the beginning of the destination range
	wchar_t lastC = L'\0';  // the last ljusted character, p2[-1] in most cases

#define IS_END(x) (pEnd ? (x) == pEnd : !*(x))
#ifdef ALTSEP
#define IS_SEP(x) (*(x) == SEP || *(x) == ALTSEP)
#else
#define IS_SEP(x) (*(x) == SEP)
#endif
#define SEP_OR_END(x) (IS_SEP(x) || IS_END(x))

	if (p1[0] == L'.' && IS_SEP(&p1[1])) {
		// Skip leading '.\'
		path = &path[2];
		while (IS_SEP(path)) {
			path++;
		}
		p1 = p2 = minP2 = path;
		lastC = SEP;
	}
	else {
		AlifSizeT drvsize, rootsize;
		_alif_skipRoot(path, size, &drvsize, &rootsize);
		if (drvsize || rootsize) {
			// Skip past root and update minP2
			p1 = &path[drvsize + rootsize];
#ifndef ALTSEP
			p2 = p1;
#else
			for (; p2 < p1; ++p2) {
				if (*p2 == ALTSEP) {
					*p2 = SEP;
				}
			}
#endif
			minP2 = p2 - 1;
			lastC = *minP2;
#ifdef MS_WINDOWS
			if (lastC != SEP) {
				minP2++;
			}
#endif
		}
	}

	/* if pEnd is specified, check that. Else, check for null terminator */
	for (; !IS_END(p1); ++p1) {
		wchar_t c = *p1;
#ifdef ALTSEP
		if (c == ALTSEP) {
			c = SEP;
		}
#endif
		if (lastC == SEP) {
			if (c == L'.') {
				int sep_at_1 = SEP_OR_END(&p1[1]);
				int sep_at_2 = !sep_at_1 && SEP_OR_END(&p1[2]);
				if (sep_at_2 && p1[1] == L'.') {
					wchar_t* p3 = p2;
					while (p3 != minP2 && *--p3 == SEP) {}
					while (p3 != minP2 && *(p3 - 1) != SEP) { --p3; }
					if (p2 == minP2
						|| (p3[0] == L'.' && p3[1] == L'.' && IS_SEP(&p3[2])))
					{
						// Previous segment is also ../, so append instead.
						// Relative path does not absorb ../ at minP2 as well.
						*p2++ = L'.';
						*p2++ = L'.';
						lastC = L'.';
					}
					else if (p3[0] == SEP) {
						// Absolute path, so absorb segment
						p2 = p3 + 1;
					}
					else {
						p2 = p3;
					}
					p1 += 1;
				}
				else if (sep_at_1) {
				}
				else {
					*p2++ = lastC = c;
				}
			}
			else if (c == SEP) {
			}
			else {
				*p2++ = lastC = c;
			}
		}
		else {
			*p2++ = lastC = c;
		}
	}
	*p2 = L'\0';
	if (p2 != minP2) {
		while (--p2 != minP2 && *p2 == SEP) {
			*p2 = L'\0';
		}
	}
	else {
		--p2;
	}
	*normsize = p2 - path + 1;
#undef SEP_OR_END
#undef IS_SEP
#undef IS_END
	return path;
}


wchar_t* _alif_normPath(wchar_t* path, AlifSizeT size) { // 2606
	AlifSizeT norm_length{};
	return _alif_normPathAndSize(path, size, &norm_length);
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


AlifIntT _alif_dup(AlifIntT fd) { // 2651
#ifdef _WINDOWS
	HANDLE handle;
#endif

#ifdef _WINDOWS
	handle = _alifGet_osfHandleNoRaise(fd);
	if (handle == INVALID_HANDLE_VALUE)
		return -1;

	ALIF_BEGIN_ALLOW_THREADS
	ALIF_BEGIN_SUPPRESS_IPH
		fd = dup(fd);
	ALIF_END_SUPPRESS_IPH
	ALIF_END_ALLOW_THREADS
		if (fd < 0) {
			//alifErr_setFromErrno(_alifExcOSError_);
			return -1;
		}

	//if (_alif_setInheritable(fd, 0, nullptr) < 0) {
	//	ALIF_BEGIN_SUPPRESS_IPH
	//		close(fd);
	//	ALIF_END_SUPPRESS_IPH
	//		return -1;
	//}
#elif defined(HAVE_FCNTL_H) and defined(F_DUPFD_CLOEXEC)
	ALIF_BEGIN_ALLOW_THREADS
	ALIF_BEGIN_SUPPRESS_IPH
	fd = fcntl(fd, F_DUPFD_CLOEXEC, 0);
	ALIF_END_SUPPRESS_IPH
	ALIF_END_ALLOW_THREADS
	if (fd < 0) {
		//alifErr_setFromErrno(_alifExcOSError_);
		return -1;
	}

#elif HAVE_DUP
	ALIF_BEGIN_ALLOW_THREADS
	ALIF_BEGIN_SUPPRESS_IPH
	fd = dup(fd);
	ALIF_END_SUPPRESS_IPH
	ALIF_END_ALLOW_THREADS
	if (fd < 0) {
		// alifErr_setFromErrno(_alifExcOSError_);
		return -1;
	}

	if (_alif_setInheritable(fd, 0, nullptr) < 0) {
		ALIF_BEGIN_SUPPRESS_IPH
			close(fd);
		ALIF_END_SUPPRESS_IPH
			return -1;
	}
#else
	errno = ENOTSUP;
	// alifErr_setFromErrno(_alifExcOSError_);
	return -1;
#endif
	return fd;
}


#ifndef _WINDOWS // 2717




#else

void* _alifGet_osfHandleNoRaise(AlifIntT fd) { // 2836
	void* handle{};
	ALIF_BEGIN_SUPPRESS_IPH
	handle = (void*)_get_osfhandle(fd);
	ALIF_END_SUPPRESS_IPH
	return handle;
}


AlifIntT _alifOpen_osfHandleNoRaise(void* handle, int flags) { // 2856
	AlifIntT fd{};
	ALIF_BEGIN_SUPPRESS_IPH
	fd = _open_osfhandle((intptr_t)handle, flags);
	ALIF_END_SUPPRESS_IPH
		return fd;
}



#endif // 2875 







AlifIntT _alif_isValidFD(AlifIntT fd) { // 3059
	if (fd < 0) {
		return 0;
	}
#if defined(F_GETFD) and ( \
        defined(__linux__) or \
        defined(__APPLE__) or \
        (defined(__wasm__) and !defined(__wasi__)))
	return fcntl(fd, F_GETFD) >= 0;
#elif defined(__linux__)
	int fd2 = dup(fd);
	if (fd2 >= 0) {
		close(fd2);
	}
	return (fd2 >= 0);
#elif defined(_WINDOWS)
	HANDLE hfile;
	ALIF_BEGIN_SUPPRESS_IPH
		hfile = (HANDLE)_get_osfhandle(fd);
	ALIF_END_SUPPRESS_IPH
		return (hfile != INVALID_HANDLE_VALUE
			and GetFileType(hfile) != FILE_TYPE_UNKNOWN);
#else
	struct stat st;
	return (fstat(fd, &st) == 0);
#endif
}
