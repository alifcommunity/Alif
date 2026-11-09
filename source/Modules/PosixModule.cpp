#include "alif.h"

#include "AlifCore_Abstract.h"
#include "AlifCore_Call.h"
#include "AlifCore_Eval.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Long.h"
#include "AlifCore_ModuleObject.h"
#include "AlifCore_Object.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_State.h"
#include "AlifCore_Signal.h"
#include "AlifCore_Time.h"
#include "AlifCore_TypeObject.h"
#include "AlifCore_Memory.h"


// 28
#ifdef HAVE_UNISTD_H
#  include <unistd.h>             // symlink()
#endif

// 33
#ifdef _WINDOWS
#  include <windows.h>
#  if !defined(MS_WINDOWS_GAMES) or defined(MS_WINDOWS_DESKTOP)
#    include <pathcch.h>
#  endif
#  include <winioctl.h>
#  include <lmcons.h>
#  include "osdefs.h"
#  include <aclapi.h>
#  include <sddl.h>
#  if defined(MS_WINDOWS_DESKTOP) or defined(MS_WINDOWS_SYSTEM)
#    define HAVE_SYMLINK
#  endif /* MS_WINDOWS_DESKTOP | MS_WINDOWS_SYSTEM */
#endif

// 48
#ifndef _WINDOWS
#  include "PosixModule.h"
#else
#  include "AlifCore_FileUtilsWindows.h"
//#  include "winreparse.h"
#endif

















// 340
#if defined(__DragonFly__) || \
    defined(__OpenBSD__)   || \
    defined(__FreeBSD__)   || \
    defined(__NetBSD__)    || \
    defined(__APPLE__)
#  include <sys/sysctl.h>
#endif










// 447
#ifdef HAVE_DIRENT_H
#  include <dirent.h>
#  define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#  if defined(__WATCOMC__) && !defined(__QNX__)
#    include <direct.h>
#    define NAMLEN(dirent) strlen((dirent)->d_name)
#  else
#    define dirent direct
#    define NAMLEN(dirent) (dirent)->d_namlen
#  endif
#  ifdef HAVE_SYS_NDIR_H
#    include <sys/ndir.h>
#  endif
#  ifdef HAVE_SYS_DIR_H
#    include <sys/dir.h>
#  endif
#  ifdef HAVE_NDIR_H
#    include <ndir.h>
#  endif
#endif











// 517
#undef STAT
#undef FSTAT
#undef STRUCT_STAT
#ifdef _WINDOWS
#  define STAT win32_stat
#  define LSTAT win32_lstat
#  define FSTAT _alifFStat_noraise
#  define STRUCT_STAT class AlifStatStruct
#else
#  define STAT stat
#  define LSTAT lstat
#  define FSTAT fstat
#  define STRUCT_STAT struct stat
#endif





// 543
#ifdef _WINDOWS
#  define INITFUNC alifInit_nt
#  define MODNAME "واجهة_نظام_ويندوز"
#  define MODNAME_OBJ &ALIF_STR(nt)
#else
#  define INITFUNC alifInit_posix
#  define MODNAME "واجهة_نظام_يونكس"
#  define MODNAME_OBJ &ALIF_STR(posix)
#endif


// 729
#ifdef _WINDOWS
/* defined in FileUtils.cpp */
void _alifAttribute_dataToStat(BY_HANDLE_FILE_INFORMATION*, ULONG,
	FILE_BASIC_INFO*, FILE_ID_INFO*, class AlifStatStruct*);
void _alifStat_basicInfoToStat(FILE_STAT_BASIC_INFORMATION*,
		class AlifStatStruct*);
#endif

// 738
#ifndef _WINDOWS
AlifObject* _alifLong_fromUid(uid_t uid) { // 739
    if (uid == (uid_t)-1)
        return alifLong_fromLong(-1);
    return alifLong_fromUnsignedLong(uid);
}


AlifObject* _alifLong_fromGid(gid_t gid) { // 747
    if (gid == (gid_t)-1)
        return alifLong_fromLong(-1);
    return alifLong_fromUnsignedLong(gid);
}




#endif /* _WINDOWS */ // 967


static AlifObject* _alifLong_fromDev(dev_t dev) { // 970
#ifdef NODEV
    if (dev == NODEV) {
        return alifLong_fromLongLong((long long)dev);
    }
#endif
    return alifLong_fromUnsignedLongLong((unsigned long long)dev);
}




// 1017
#ifdef AT_FDCWD
/*
* Why the (int) cast?  Solaris 10 defines AT_FDCWD as 0xffd19553 (-3041965);
* without the int cast, the value gets interpreted as uint (4291925331),
* which doesn't play nicely with all the initializer lines in this file that
* look like this:
*      int dir_fd = DEFAULT_DIR_FD;
*/
#define DEFAULT_DIR_FD (int)AT_FDCWD
#else
#define DEFAULT_DIR_FD (-100)
#endif


static AlifIntT _fd_converter(AlifObject* o, AlifIntT* p) { // 1030
	AlifIntT overflow{};
	long longValue{};

	//if (ALIFBOOL_CHECK(o)) {
	//	if (alifErr_warnEx(_alifExcRuntimeWarning_,
	//		"يتم استخدام نوع منطق ك واصف ملف", 1)) {
	//		return 0;
	//	}
	//}
	AlifObject* index = _alifNumber_index(o);
	if (index == nullptr) {
		return 0;
	}

	longValue = alifLong_asLongAndOverflow(index, &overflow);
	ALIF_DECREF(index);
	if (overflow > 0 or longValue > INT_MAX) {
		alifErr_setString(_alifExcOverflowError_,
			"واصف_الملف اكبر من أقصى قيمة");
		return 0;
	}
	if (overflow < 0 or longValue < INT_MIN) {
		alifErr_setString(_alifExcOverflowError_,
			"واصف_الملف اصغر من أدنى قيمة");
		return 0;
	}

	*p = (AlifIntT)longValue;
	return 1;
}


static AlifIntT dirFD_converter(AlifObject* _o, void* _p) { // 1067
	if (_o == ALIF_NONE) {
		*(int*)_p = DEFAULT_DIR_FD;
		return 1;
	}
	else if (alifIndex_check(_o)) {
		return _fd_converter(_o, (int*)_p);
	}
	else {
		alifErr_format(_alifExcTypeError_,
			"المعامل يجب أن يكون عدد صحيح او عدم, وليس %.200s",
			_alifType_name(ALIF_TYPE(_o)));
		return 0;
	}
}




class  PosixState { // 1085
public:
	AlifObject* billion{};
	AlifObject* DirEntryType{};
	AlifObject* ScandirIteratorType{};
#if defined(HAVE_SCHED_SETPARAM) or defined(HAVE_SCHED_SETSCHEDULER)
	or defined(POSIX_SPAWN_SETSCHEDULER) or defined(POSIX_SPAWN_SETSCHEDPARAM)
		AlifObject* SchedParamType{};
#endif
	NewFunc statresult_new_orig{};
	AlifObject* StatResultType{};
	AlifObject* StatVFSResultType{};
	AlifObject* TerminalSizeType{};
	AlifObject* TimesResultType{};
	AlifObject* UnameResultType{};
#if defined(HAVE_WAITID)
	AlifObject* WaitidResultType{};
#endif
#if defined(HAVE_WAIT3) or defined(HAVE_WAIT4)
	AlifObject* struct_rusage{};
#endif
	AlifObject* st_mode;
#ifndef _WINDOWS
	// times() clock frequency in hertz; used by os.times()
	long ticks_per_second{};
#endif
};

static inline PosixState* get_posixState(AlifObject* _module) { // 1112
	void* state = _alifModule_getState(_module);
	return (PosixState*)state;
}


class PathT { // 1215
public:
	// Input fields
	const char* functionName{};
	const char* argumentName{};
	AlifIntT nullable{};
	AlifIntT nonstrict{};
	AlifIntT makeWide{};
	AlifIntT suppressValueError{};
	AlifIntT allowFD{};
	// Output fields
	const wchar_t* wide{};
	const char* narrow{};
	AlifIntT fd{};
	AlifIntT valueError{};
	AlifSizeT length{};
	AlifObject* object{};
	AlifObject* cleanup{};
};

// 1234
#define PATH_T_INITIALIZE(functionName, argumentName, nullable, nonstrict, \
                          makeWide, suppressValueError, allowFD) \
    {functionName, argumentName, nullable, nonstrict, makeWide, \
     suppressValueError, allowFD, nullptr, nullptr, -1, 0, 0, nullptr, nullptr}
#ifdef _WINDOWS
#define PATH_T_INITIALIZE_P(functionName, argumentName, nullable, \
                            nonstrict, suppressValueError, allowFD) \
    PATH_T_INITIALIZE(functionName, argumentName, nullable, nonstrict, 1, \
                      suppressValueError, allowFD)
#else
#define PATH_T_INITIALIZE_P(functionName, argumentName, nullable, \
                            nonstrict, suppressValueError, allowFD) \
    PATH_T_INITIALIZE(functionName, argumentName, nullable, nonstrict, 0, \
                      suppressValueError, allowFD)
#endif

static void path_cleanup(PathT* path) { // 1250
	wchar_t* wide = (wchar_t*)path->wide;
	path->wide = nullptr;
	alifMem_dataFree(wide);
	ALIF_CLEAR(path->object);
	ALIF_CLEAR(path->cleanup);
}

static AlifIntT path_converter(AlifObject* o, void* p) { // 1260
	PathT* path = (PathT*)p;
	AlifObject* bytes = nullptr;
	AlifSizeT length = 0;
	AlifIntT is_index{}, is_bytes{}, is_unicode{};
	const char* narrow{};
	AlifObject* wo = nullptr;
	wchar_t* wide = nullptr;

#define FORMAT_EXCEPTION(exc, fmt) \
    alifErr_format(exc, "%s%s" fmt, \
        path->functionName ? path->functionName : "", \
        path->functionName ? ": "                : "", \
        path->argumentName ? path->argumentName : "مسار")

	/* Py_CLEANUP_SUPPORTED support */
	if (o == nullptr) {
		path_cleanup(path);
		return 1;
	}

	/* Ensure it's always safe to call path_cleanup(). */
	path->object = path->cleanup = nullptr;
	/* path->object owns a reference to the original object */
	ALIF_INCREF(o);

	if ((o == ALIF_NONE) and path->nullable) {
		path->wide = nullptr;
		path->narrow = nullptr;
		path->fd = -1;
		goto success_exit;
	}

	/* Only call this here so that we don't treat the return value of
	os.fspath() as an fd or buffer. */
	is_index = path->allowFD and alifIndex_check(o);
	is_bytes = ALIFBYTES_CHECK(o);
	is_unicode = ALIFUSTR_CHECK(o);

	if (!is_index and !is_unicode and !is_bytes) {
		/* Inline alifOS_fsPath() for better error messages. */
		AlifObject* func{}, * res{};

		func = _alifObject_lookupSpecial(o, &ALIF_STR(__fspath__));
		if ((nullptr == func) or (func == ALIF_NONE)) {
			goto error_format;
		}
		res = _alifObject_callNoArgs(func);
		ALIF_DECREF(func);
		if (nullptr == res) {
			goto error_exit;
		}
		else if (ALIFUSTR_CHECK(res)) {
			is_unicode = 1;
		}
		else if (ALIFBYTES_CHECK(res)) {
			is_bytes = 1;
		}
		else {
			alifErr_format(_alifExcTypeError_,
				"يتوقع %.200s.__fspath__() ليرجع نص او ثمانيات, "
				"وليس %.200s", _alifType_name(ALIF_TYPE(o)),
				_alifType_name(ALIF_TYPE(res)));
			ALIF_DECREF(res);
			goto error_exit;
		}

		/* still owns a reference to the original object */
		ALIF_SETREF(o, res);
	}

	if (is_unicode) {
		if (path->makeWide) {
			wide = alifUStr_asWideCharString(o, &length);
			if (!wide) {
				goto error_exit;
			}
		#ifdef _WINDOWS
			if (!path->nonstrict and length > 32767) {
				FORMAT_EXCEPTION(_alifExcValueError_, "%s طويل جداً بالنسبة لنظام ويندوز");
				goto error_exit;
			}
		#endif
			if (!path->nonstrict and wcslen(wide) != (size_t)length) {
				FORMAT_EXCEPTION(_alifExcValueError_,
					"يحتوي حرف فارغ في %s");
				goto error_exit;
			}

			path->wide = wide;
			path->narrow = nullptr;
			path->fd = -1;
			wide = nullptr;
			goto success_exit;
		}
		bytes = alifUStr_encodeFSDefault(o);
		if (!bytes) {
			goto error_exit;
		}
	}
	else if (is_bytes) {
		bytes = ALIF_NEWREF(o);
	}
	else if (is_index) {
		if (!_fd_converter(o, &path->fd)) {
			goto error_exit;
		}
		path->wide = nullptr;
		path->narrow = nullptr;
		goto success_exit;
	}
	else {
error_format:
		alifErr_format(_alifExcTypeError_, "%s%s%s يجب أن يكون %s, وليس %.200s",
			path->functionName ? path->functionName : "",
			path->functionName ? ": " : "",
			path->argumentName ? path->argumentName : "مسار",
			path->allowFD and path->nullable ? "نص, ثمانيات, نظام_التشغيل.كمسار, "
			"عدد صحيح او عدم" :
			path->allowFD ? "نص, ثمانيات, نظام_التشغيل.كمسار او عدد صحيح" :
			path->nullable ? "نص, ثمانيات, نظام_التشغيل.كمسار او عدم" :
			"نص, ثمانيات او نظام_التشغيل.كمسار",
			_alifType_name(ALIF_TYPE(o)));
		goto error_exit;
	}

	length = ALIFBYTES_GET_SIZE(bytes);
	narrow = ALIFBYTES_AS_STRING(bytes);
	if (!path->nonstrict and strlen(narrow) != (size_t)length) {
		FORMAT_EXCEPTION(_alifExcValueError_, "يحتوي حرف فارغ في %s");
		goto error_exit;
	}

	if (path->makeWide) {
		wo = alifUStr_decodeFSDefaultAndSize(narrow, length);
		if (!wo) {
			goto error_exit;
		}

		wide = alifUStr_asWideCharString(wo, &length);
		ALIF_DECREF(wo);
		if (!wide) {
			goto error_exit;
		}
	#ifdef _WINDOWS
		if (!path->nonstrict and length > 32767) {
			FORMAT_EXCEPTION(_alifExcValueError_, "%s طويل جداً بالنسبة لنظام ويندوز");
			goto error_exit;
		}
	#endif
		if (!path->nonstrict and wcslen(wide) != (size_t)length) {
			FORMAT_EXCEPTION(_alifExcValueError_,
				"يحتوي حرف فارغ في %s");
			goto error_exit;
		}
		path->wide = wide;
		path->narrow = nullptr;
		ALIF_DECREF(bytes);
		wide = nullptr;
	}
	else {
		path->wide = nullptr;
		path->narrow = narrow;
		if (bytes == o) {
			/* Still a reference owned by path->object, don't have to
			worry about path->narrow is used after free. */
			ALIF_DECREF(bytes);
		}
		else {
			path->cleanup = bytes;
		}
	}
	path->fd = -1;

success_exit:
	path->valueError = 0;
	path->length = length;
	path->object = o;
	return ALIF_CLEANUP_SUPPORTED;

error_exit:
	ALIF_XDECREF(o);
	ALIF_XDECREF(bytes);
	alifMem_dataFree(wide);
	if (!path->suppressValueError or
		!alifErr_exceptionMatches(_alifExcValueError_)) {
		return 0;
	}
	alifErr_clear();
	path->wide = nullptr;
	path->narrow = nullptr;
	path->fd = -1;
	path->valueError = 1;
	path->length = 0;
	path->object = nullptr;
	return ALIF_CLEANUP_SUPPORTED;
}




static void argument_unavailableError(const char* _functionName,
	const char* _argumentName) { // 1461
	//alifErr_format(_alifExcNotImplementedError_,
	//	"%s%s%s غير متوفر على هذه المنصة",
	//	(_functionName != nullptr) ? _functionName : "",
	//	(_functionName != nullptr) ? ": " : "",
	//	_argumentName);
}

static AlifIntT dirFD_unavailable(AlifObject* _o, void* _p) { // 1471
	AlifIntT dirFD{};
	if (!dirFD_converter(_o, &dirFD))
		return 0;
	if (dirFD != DEFAULT_DIR_FD) {
		argument_unavailableError(nullptr, "dirFD");
		return 0;
	}
	*(int*)_p = dirFD;
	return 1;
}

static AlifIntT fd_specified(const char* _functionName,
	AlifIntT _fd) { // 1485
	if (_fd == -1)
		return 0;

	argument_unavailableError(_functionName, "fd");
	return 1;
}

static AlifIntT followSymlinks_specified(const char* _functionName,
	AlifIntT _followSymlinks) { // 1495
	if (_followSymlinks)
		return 0;

	argument_unavailableError(_functionName, "follow_symlinks");
	return 1;
}




static AlifIntT pathAndDirFD_invalid(const char* _functionName,
	PathT* _path, AlifIntT _dirFD) { // 1505
	if (!_path->wide and (_dirFD != DEFAULT_DIR_FD) and !_path->narrow) {
		alifErr_format(_alifExcValueError_,
			"%s: لا يمكن تحديد dirFD بدون مطابقة المسار",
			_functionName);
		return 1;
	}
	return 0;
}

static AlifIntT dirFDAndFD_invalid(const char* _functionName,
	AlifIntT _dirFD, AlifIntT _fd) { // 1517
	if ((_dirFD != DEFAULT_DIR_FD) and (_fd != -1)) {
		alifErr_format(_alifExcValueError_,
			"%s: لا يمكن تحديد لك من dirFD و fd",
			_functionName);
		return 1;
	}
	return 0;
}

static AlifIntT fdAndFollowSymlinks_invalid(const char* _functionName, AlifIntT _fd,
	AlifIntT _followSymlinks) { // 1529
	if ((_fd > 0) and (!_followSymlinks)) {
		alifErr_format(_alifExcValueError_,
			"%s: لا يمكن إستخدام fd و followSymlinks معاً",
			_functionName);
		return 1;
	}
	return 0;
}

static AlifIntT dirFDAndFollowSymlinks_invalid(const char* _functionName,
	AlifIntT _dirFD, AlifIntT _followSymlinks) { // 1542
	if ((_dirFD != DEFAULT_DIR_FD) and (!_followSymlinks)) {
		alifErr_format(_alifExcValueError_,
			"%s: لا يمكن إستخدام dirFD و followSymlinks معاً",
			_functionName);
		return 1;
	}
	return 0;
}






// 1658
/* Return a dictionary corresponding to the POSIX environment table */
#if defined(WITH_NEXT_FRAMEWORK) or (defined(__APPLE__) and defined(ALIF_ENABLE_SHARED))
#include <crt_externs.h>
#define USE_DARWIN_NS_GET_ENVIRON 1
#elif !defined(_MSC_VER) and (!defined(__WATCOMC__) or defined(__QNX__) or defined(__VXWORKS__))
extern char** environ;
#endif /* !_MSC_VER */

static AlifObject*
convertenviron(void) { // 1670
	AlifObject* d{};
#ifdef _WINDOWS
	wchar_t** e{};
#else
	char** e{};
#endif

	d = alifDict_new();
	if (d == nullptr)
		return nullptr;
#ifdef _WINDOWS
	(void)_wgetenv(L"");
	e = _wenviron;
#elif defined(USE_DARWIN_NS_GET_ENVIRON)
	e = *_NSGetEnviron();
#else
	e = environ;
#endif
	if (e == nullptr)
		return d;
	for (; *e != nullptr; e++) {
		AlifObject* k{};
		AlifObject* v{};
	#ifdef _WINDOWS
		const wchar_t* p = wcschr(*e, L'=');
	#else
		const char* p = strchr(*e, '=');
	#endif
		if (p == nullptr)
			continue;
	#ifdef _WINDOWS
		k = alifUStr_fromWideChar(*e, (AlifSizeT)(p - *e));
	#else
		k = alifBytes_fromStringAndSize(*e, (int)(p - *e));
	#endif
		if (k == nullptr) {
			ALIF_DECREF(d);
			return nullptr;
		}
	#ifdef _WINDOWS
		v = alifUStr_fromWideChar(p + 1, wcslen(p + 1));
	#else
		v = alifBytes_fromStringAndSize(p + 1, strlen(p + 1));
	#endif
		if (v == nullptr) {
			ALIF_DECREF(k);
			ALIF_DECREF(d);
			return nullptr;
		}
		if (alifDict_setDefaultRef(d, k, v, nullptr) < 0) {
			ALIF_DECREF(v);
			ALIF_DECREF(k);
			ALIF_DECREF(d);
			return nullptr;
		}
		ALIF_DECREF(k);
		ALIF_DECREF(v);
	}
	return d;
}









static AlifObject* posix_pathObjectError(AlifObject* _path) { // 1785
	return alifErr_setFromErrnoWithFilenameObject(_alifExcOSError_, _path);
}

static AlifObject* path_objectError(AlifObject* _path) {
#ifdef _WINDOWS
	return alifErr_setExcFromWindowsErrWithFilenameObject(
		_alifExcOSError_, 0, _path);
#else
	return posix_pathObjectError(_path);
#endif
}

static AlifObject* path_error(PathT* _path) { // 1813
	return path_objectError(_path->object);
}







#ifdef _WINDOWS // 1895

#define HAVE_STAT_NSEC 1
#define HAVE_STRUCT_STAT_ST_FILE_ATTRIBUTES 1
#define HAVE_STRUCT_STAT_ST_REPARSE_TAG 1

static void find_dataToFileInfo(WIN32_FIND_DATAW* pFileData,
	BY_HANDLE_FILE_INFORMATION* info,
	ULONG* reparse_tag) { // 1906
	memset(info, 0, sizeof(*info));
	info->dwFileAttributes = pFileData->dwFileAttributes;
	info->ftCreationTime = pFileData->ftCreationTime;
	info->ftLastAccessTime = pFileData->ftLastAccessTime;
	info->ftLastWriteTime = pFileData->ftLastWriteTime;
	info->nFileSizeHigh = pFileData->nFileSizeHigh;
	info->nFileSizeLow = pFileData->nFileSizeLow;
	/*  info->nNumberOfLinks   = 1; */
	if (pFileData->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		*reparse_tag = pFileData->dwReserved0;
	else
		*reparse_tag = 0;
}

static BOOL attributes_fromDir(LPCWSTR pszFile,
	BY_HANDLE_FILE_INFORMATION* info, ULONG* reparse_tag) { // 1925
	HANDLE hFindFile;
	WIN32_FIND_DATAW FileData;
	LPCWSTR filename = pszFile;
	size_t n = wcslen(pszFile);
	if (n && (pszFile[n - 1] == L'\\' || pszFile[n - 1] == L'/')) {
		// cannot use PyMem_Malloc here because we do not hold the GIL
		filename = (LPCWSTR)malloc((n + 1) * sizeof(filename[0]));
		if (!filename) {
			SetLastError(ERROR_NOT_ENOUGH_MEMORY);
			return FALSE;
		}
		wcsncpy_s((LPWSTR)filename, n + 1, pszFile, n);
		while (--n > 0 && (filename[n] == L'\\' || filename[n] == L'/')) {
			((LPWSTR)filename)[n] = L'\0';
		}
		if (!n || (n == 1 && filename[1] == L':')) {
			// Nothing left to query
			free((void*)filename);
			return FALSE;
		}
	}
	hFindFile = FindFirstFileW(filename, &FileData);
	if (pszFile != filename) {
		free((void*)filename);
	}
	if (hFindFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FindClose(hFindFile);
	find_dataToFileInfo(&FileData, info, reparse_tag);
	return TRUE;
}

static void updateSTMode_fromPath(const wchar_t* _path, DWORD _attr,
	class AlifStatStruct* _result) { // 1962
	if (!(_attr & FILE_ATTRIBUTE_DIRECTORY)) {
		const wchar_t* fileExtension = wcsrchr(_path, '.');
		if (fileExtension) {
			if (_wcsicmp(fileExtension, L".exe") == 0 ||
				_wcsicmp(fileExtension, L".bat") == 0 ||
				_wcsicmp(fileExtension, L".cmd") == 0 ||
				_wcsicmp(fileExtension, L".com") == 0) {
				_result->st_mode |= 0111;
			}
		}
	}
}

static AlifIntT win32_xstatSlowImpl(const wchar_t* _path,
	class AlifStatStruct* _result, BOOL _traverse) { // 1986
	HANDLE hFile;
	BY_HANDLE_FILE_INFORMATION fileInfo;
	FILE_BASIC_INFO basicInfo;
	FILE_BASIC_INFO* pBasicInfo = nullptr;
	FILE_ID_INFO idInfo;
	FILE_ID_INFO* pIdInfo = nullptr;
	FILE_ATTRIBUTE_TAG_INFO tagInfo = { 0 };
	DWORD fileType, error;
	BOOL isUnhandledTag = FALSE;
	int retval = 0;

	DWORD access = FILE_READ_ATTRIBUTES;
	DWORD flags = FILE_FLAG_BACKUP_SEMANTICS; /* Allow opening directories. */
	if (!_traverse) {
		flags |= FILE_FLAG_OPEN_REPARSE_POINT;
	}

	hFile = CreateFileW(_path, access, 0, nullptr, OPEN_EXISTING, flags, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		/* Either the path doesn't exist, or the caller lacks access. */
		error = GetLastError();
		switch (error) {
		case ERROR_ACCESS_DENIED:     /* Cannot sync or read attributes. */
		case ERROR_SHARING_VIOLATION: /* It's a paging file. */
			/* Try reading the parent directory. */
			if (!attributes_fromDir(_path, &fileInfo, &tagInfo.ReparseTag)) {
				/* Cannot read the parent directory. */
				switch (GetLastError()) {
				case ERROR_FILE_NOT_FOUND: /* File cannot be found */
				case ERROR_PATH_NOT_FOUND: /* File parent directory cannot be found */
				case ERROR_NOT_READY: /* Drive exists but unavailable */
				case ERROR_BAD_NET_NAME: /* Remote drive unavailable */
					break;
					/* Restore the error from CreateFileW(). */
				default:
					SetLastError(error);
				}

				return -1;
			}
			if (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
				if (_traverse ||
					!IsReparseTagNameSurrogate(tagInfo.ReparseTag)) {
					/* The stat call has to traverse but cannot, so fail. */
					SetLastError(error);
					return -1;
				}
			}
			break;

		case ERROR_INVALID_PARAMETER:
			/* \\.\con requires read or write access. */
			hFile = CreateFileW(_path, access | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
				OPEN_EXISTING, flags, nullptr);
			if (hFile == INVALID_HANDLE_VALUE) {
				SetLastError(error);
				return -1;
			}
			break;

		case ERROR_CANT_ACCESS_FILE:
			if (_traverse) {
				_traverse = FALSE;
				isUnhandledTag = TRUE;
				hFile = CreateFileW(_path, access, 0, nullptr, OPEN_EXISTING,
					flags | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
			}
			if (hFile == INVALID_HANDLE_VALUE) {
				SetLastError(error);
				return -1;
			}
			break;

		default:
			return -1;
		}
	}

	if (hFile != INVALID_HANDLE_VALUE) {
		/* Handle types other than files on disk. */
		fileType = GetFileType(hFile);
		if (fileType != FILE_TYPE_DISK) {
			if (fileType == FILE_TYPE_UNKNOWN && GetLastError() != 0) {
				retval = -1;
				goto cleanup;
			}
			DWORD fileAttributes = GetFileAttributesW(_path);
			memset(_result, 0, sizeof(*_result));
			if (fileAttributes != INVALID_FILE_ATTRIBUTES &&
				fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				/* \\.\pipe\ or \\.\mailslot\ */
				_result->st_mode = _S_IFDIR;
			}
			else if (fileType == FILE_TYPE_CHAR) {
				/* \\.\nul */
				_result->st_mode = _S_IFCHR;
			}
			else if (fileType == FILE_TYPE_PIPE) {
				/* \\.\pipe\spam */
				_result->st_mode = _S_IFIFO;
			}
			/* FILE_TYPE_UNKNOWN, e.g. \\.\mailslot\waitfor.exe\spam */
			goto cleanup;
		}

		/* Query the reparse tag, and traverse a non-link. */
		if (!_traverse) {
			if (!GetFileInformationByHandleEx(hFile, FileAttributeTagInfo,
				&tagInfo, sizeof(tagInfo))) {
				/* Allow devices that do not support FileAttributeTagInfo. */
				switch (GetLastError()) {
				case ERROR_INVALID_PARAMETER:
				case ERROR_INVALID_FUNCTION:
				case ERROR_NOT_SUPPORTED:
					tagInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
					tagInfo.ReparseTag = 0;
					break;
				default:
					retval = -1;
					goto cleanup;
				}
			}
			else if (tagInfo.FileAttributes &
				FILE_ATTRIBUTE_REPARSE_POINT) {
				if (IsReparseTagNameSurrogate(tagInfo.ReparseTag)) {
					if (isUnhandledTag) {
						/* Traversing previously failed for either this link
						or its target. */
						SetLastError(ERROR_CANT_ACCESS_FILE);
						retval = -1;
						goto cleanup;
					}
					/* Traverse a non-link, but not if traversing already failed
					for an unhandled tag. */
				}
				else if (!isUnhandledTag) {
					CloseHandle(hFile);
					return win32_xstatSlowImpl(_path, _result, TRUE);
				}
			}
		}

		if (!GetFileInformationByHandle(hFile, &fileInfo) ||
			!GetFileInformationByHandleEx(hFile, FileBasicInfo,
				&basicInfo, sizeof(basicInfo))) {
			switch (GetLastError()) {
			case ERROR_INVALID_PARAMETER:
			case ERROR_INVALID_FUNCTION:
			case ERROR_NOT_SUPPORTED:
				/* Volumes and physical disks are block devices, e.g.
				\\.\C: and \\.\PhysicalDrive0. */
				memset(_result, 0, sizeof(*_result));
				_result->st_mode = 0x6000; /* S_IFBLK */
				goto cleanup;
			}
			retval = -1;
			goto cleanup;
		}

		/* Successfully got FileBasicInfo, so we'll pass it along */
		pBasicInfo = &basicInfo;

		if (GetFileInformationByHandleEx(hFile, FileIdInfo, &idInfo, sizeof(idInfo))) {
			/* Successfully got FileIdInfo, so pass it along */
			pIdInfo = &idInfo;
		}
	}

	_alifAttribute_dataToStat(&fileInfo, tagInfo.ReparseTag, pBasicInfo, pIdInfo, _result);
	updateSTMode_fromPath(_path, fileInfo.dwFileAttributes, _result);

cleanup:
	if (hFile != INVALID_HANDLE_VALUE) {
		/* Preserve last error if we are failing */
		error = retval ? GetLastError() : 0;
		if (!CloseHandle(hFile)) {
			retval = -1;
		}
		else if (retval) {
			/* Restore last error */
			SetLastError(error);
		}
	}

	return retval;
}

static AlifIntT win32_xstatImpl(const wchar_t* _path,
	class AlifStatStruct* _result, BOOL _traverse) { // 2174
	FILE_STAT_BASIC_INFORMATION statInfo{};
	if (_alif_getFileInformationByName(_path, FileStatBasicByNameInfo,
		&statInfo, sizeof(statInfo))) {
		if (// Cannot use fast path for reparse points ...
			!(statInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			// ... unless it's a name surrogate (symlink) and we're not following
			or (!_traverse && IsReparseTagNameSurrogate(statInfo.ReparseTag))
			) {
			_alifStat_basicInfoToStat(&statInfo, _result);
			updateSTMode_fromPath(_path, statInfo.FileAttributes, _result);
			return 0;
		}
	}
	else {
		switch (GetLastError()) {
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_NOT_READY:
		case ERROR_BAD_NET_NAME:
			/* These errors aren't worth retrying with the slow path */
			return -1;
		case ERROR_NOT_SUPPORTED:
			/* indicates the API couldn't be loaded */
			break;
		}
	}

	return win32_xstatSlowImpl(_path, _result, _traverse);
}

static AlifIntT win32_xstat(const wchar_t* _path,
	class AlifStatStruct* _result, BOOL _traverse) { // 2207
	/* Protocol violation: we explicitly clear errno, instead of
	setting it to a POSIX error. Callers should use GetLastError. */
	AlifIntT code = win32_xstatImpl(_path, _result, _traverse);
	errno = 0;

	_result->st_ctime = _result->st_birthtime;
	_result->st_ctime_nsec = _result->st_birthtime_nsec;
	return code;
}

static AlifIntT win32_lstat(const wchar_t* _path,
	class AlifStatStruct* _result) { // 2230
	return win32_xstat(_path, _result, FALSE);
}

static AlifIntT win32_stat(const wchar_t* _path,
	class AlifStatStruct* _result) { // 2236
	return win32_xstat(_path, _result, TRUE);
}

#endif // 2242

//* alif //* todo
static AlifStructSequenceField _statResultFields_[] = { // 2255
	{"الوضع",    "ثمانيات الحماية"},
	{"st_ino",     "inode"},
	{"st_dev",     "جهاز"},
	{"st_nlink",   "عدد الروابط المتينة"},
	{"st_uid",     "هوية المالك"},
	{"st_gid",     "هوية المجموعة المالكة"},
	{"الحجم",    "الحجم الكلي, بالثمانية"},
	/* The NULL is replaced with alifStructSequence_unnamedField later. */
	{nullptr,   "integer time of last access"},
	{nullptr,   "integer time of last modification"},
	{nullptr,   "integer time of last change"},
	{"st_atime",   "time of last access"},
	{"زمن_التعديل",   "time of last modification"},
	{"st_ctime",   "time of last change"},
	{"st_atime_ns",   "time of last access in nanoseconds"},
	{"st_mtime_ns",   "time of last modification in nanoseconds"},
	{"st_ctime_ns",   "time of last change in nanoseconds"},
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	{"st_blksize", "blocksize for filesystem I/O"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
	{"st_blocks",  "number of blocks allocated"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_RDEV
	{"st_rdev",    "device type (if inode device)"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
	{"st_flags",   "user defined flags for file"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_GEN
	{"st_gen",    "generation number"},
#endif
#if defined(HAVE_STRUCT_STAT_ST_BIRTHTIME) || defined(_WINDOWS)
	{"st_birthtime",   "time of creation"},
#endif
#ifdef _WINDOWS
	{"st_birthtime_ns", "time of creation in nanoseconds"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_FILE_ATTRIBUTES
	{"st_file_attributes", "Windows file attribute bits"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_FSTYPE
	{"st_fstype",  "Type of filesystem"},
#endif
#ifdef HAVE_STRUCT_STAT_ST_REPARSE_TAG
	{"st_reparse_tag", "Windows reparse tag"},
#endif
	{0}
};


// 2306
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
#define ST_BLKSIZE_IDX 16
#else
#define ST_BLKSIZE_IDX 15
#endif

#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
#define ST_BLOCKS_IDX (ST_BLKSIZE_IDX+1)
#else
#define ST_BLOCKS_IDX ST_BLKSIZE_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_RDEV
#define ST_RDEV_IDX (ST_BLOCKS_IDX+1)
#else
#define ST_RDEV_IDX ST_BLOCKS_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_FLAGS
#define ST_FLAGS_IDX (ST_RDEV_IDX+1)
#else
#define ST_FLAGS_IDX ST_RDEV_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_GEN
#define ST_GEN_IDX (ST_FLAGS_IDX+1)
#else
#define ST_GEN_IDX ST_FLAGS_IDX
#endif

#if defined(HAVE_STRUCT_STAT_ST_BIRTHTIME) || defined(_WINDOWS)
#define ST_BIRTHTIME_IDX (ST_GEN_IDX+1)
#else
#define ST_BIRTHTIME_IDX ST_GEN_IDX
#endif

#ifdef _WINDOWS
#define ST_BIRTHTIME_NS_IDX (ST_BIRTHTIME_IDX+1)
#else
#define ST_BIRTHTIME_NS_IDX ST_BIRTHTIME_IDX
#endif

#if defined(HAVE_STRUCT_STAT_ST_FILE_ATTRIBUTES) || defined(_WINDOWS)
#define ST_FILE_ATTRIBUTES_IDX (ST_BIRTHTIME_NS_IDX+1)
#else
#define ST_FILE_ATTRIBUTES_IDX ST_BIRTHTIME_NS_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_FSTYPE
#define ST_FSTYPE_IDX (ST_FILE_ATTRIBUTES_IDX+1)
#else
#define ST_FSTYPE_IDX ST_FILE_ATTRIBUTES_IDX
#endif

#ifdef HAVE_STRUCT_STAT_ST_REPARSE_TAG
#define ST_REPARSE_TAG_IDX (ST_FSTYPE_IDX+1)
#else
#define ST_REPARSE_TAG_IDX ST_FSTYPE_IDX
#endif
// 2364

static AlifStructSequenceDesc _statResultDesc_ = { // 2366
	"نتيجة_الحالة", // stat_result /* name */
	nullptr, /* doc */
	_statResultFields_,
	10
};

static AlifStructSequenceField _statvfsResultFields_[] = { // 2381
	//* todo
	{"f_bsize",  },
	{"f_frsize", },
	{"f_blocks", },
	{"f_bfree",  },
	{"f_bavail", },
	{"f_files",  },
	{"f_ffree",  },
	{"f_favail", },
	{"f_flag",   },
	{"f_namemax",},
	{"f_fsid",   },
	{0}
};

static AlifStructSequenceDesc _statvfsResultDesc_ = { // 2396
	"نتيجة_حالة_ملف_النظام_الافتراضي", // statvfs_result /* name */
	nullptr, /* doc */
	_statvfsResultFields_,
	10
};


static AlifObject* statResult_new(AlifTypeObject* _type,
	AlifObject* _args, AlifObject* _kwds) { // 2429
	AlifStructSequence* result{};
	AlifIntT i{};

	AlifObject* mod = alifImport_getModule(MODNAME_OBJ);
	if (mod == nullptr) {
		return nullptr;
	}
	PosixState* state = get_posixState(mod);
	ALIF_DECREF(mod);
	if (state == nullptr) {
		return nullptr;
	}
#define STRUCTSEQ_NEW state->statresult_new_orig

	result = (AlifStructSequence*)STRUCTSEQ_NEW(_type, _args, _kwds);
	if (!result)
		return nullptr;
	for (i = 7; i <= 9; i++) {
		if (result->item[i + 3] == ALIF_NONE) {
			ALIF_DECREF(ALIF_NONE);
			result->item[i + 3] = ALIF_NEWREF(result->item[i]);
		}
	}
	return (AlifObject*)result;
}





static AlifIntT fill_time(AlifObject* module,
	AlifObject* v, AlifIntT s_index, AlifIntT f_index,
	AlifIntT ns_index, time_t sec, unsigned long nsec) { // 2519
	AlifIntT res = -1;
	AlifObject* s_in_ns = nullptr;
	AlifObject* ns_total = nullptr;
	AlifObject* float_s = nullptr;

	AlifObject* s = _alifLong_fromTimeT(sec);
	AlifObject* ns_fractional = alifLong_fromUnsignedLong(nsec);
	if (!(s && ns_fractional)) {
		goto exit;
	}

	s_in_ns = alifNumber_multiply(s, get_posixState(module)->billion);
	if (!s_in_ns) {
		goto exit;
	}

	ns_total = alifNumber_add(s_in_ns, ns_fractional);
	if (!ns_total)
		goto exit;

	float_s = alifFloat_fromDouble(sec + 1e-9 * nsec);
	if (!float_s) {
		goto exit;
	}

	if (s_index >= 0) {
		ALIFSTRUCTSEQUENCE_SET_ITEM(v, s_index, s);
		s = nullptr;
	}
	if (f_index >= 0) {
		ALIFSTRUCTSEQUENCE_SET_ITEM(v, f_index, float_s);
		float_s = nullptr;
	}
	if (ns_index >= 0) {
		ALIFSTRUCTSEQUENCE_SET_ITEM(v, ns_index, ns_total);
		ns_total = nullptr;
	}

	res = 0;

exit:
	ALIF_XDECREF(s);
	ALIF_XDECREF(ns_fractional);
	ALIF_XDECREF(s_in_ns);
	ALIF_XDECREF(ns_total);
	ALIF_XDECREF(float_s);
	return res;
}

#ifdef _WINDOWS
static AlifObject* _alifStat_l128FromL64L64(uint64_t _low, uint64_t _high) { // 2574
	AlifObject* oLow = alifLong_fromUnsignedLongLong(_low);
	if (!oLow or !_high) {
		return oLow;
	}
	AlifObject* oHigh = alifLong_fromUnsignedLongLong(_high);
	AlifObject* l64 = oHigh ? alifLong_fromLong(64) : nullptr;
	if (!l64) {
		ALIF_XDECREF(oHigh);
		ALIF_DECREF(oLow);
		return nullptr;
	}
	ALIF_SETREF(oHigh, alifNumber_lshift(oHigh, l64));
	ALIF_DECREF(l64);
	if (!oHigh) {
		ALIF_DECREF(oLow);
		return nullptr;
	}
	ALIF_SETREF(oLow, alifNumber_add(oLow, oHigh));
	ALIF_DECREF(oHigh);
	return oLow;
}
#endif

static AlifObject* _alifStat_fromStructStat(AlifObject* _module,
	STRUCT_STAT* st) { // 2603
	AlifObject* StatResultType = get_posixState(_module)->StatResultType;
	AlifObject* v = alifStructSequence_new((AlifTypeObject*)StatResultType);
	if (v == nullptr) {
		return nullptr;
	}

#define SET_ITEM(pos, expr) \
    do { \
        AlifObject *obj = (expr); \
        if (obj == nullptr) { \
            goto error; \
        } \
        ALIFSTRUCTSEQUENCE_SET_ITEM(v, (pos), obj); \
    } while (0)

	SET_ITEM(0, alifLong_fromLong((long)st->st_mode));
#ifdef _WINDOWS
	SET_ITEM(1, _alifStat_l128FromL64L64(st->st_ino, st->st_ino_high));
	SET_ITEM(2, alifLong_fromUnsignedLongLong(st->st_dev));
#else
	SET_ITEM(1, alifLong_fromUnsignedLongLong(st->st_ino));
	SET_ITEM(2, _alifLong_fromDev(st->st_dev));
#endif
	SET_ITEM(3, alifLong_fromLong((long)st->st_nlink));
#if defined(_WINDOWS)
	SET_ITEM(4, alifLong_fromLong(0));
	SET_ITEM(5, alifLong_fromLong(0));
#else
	SET_ITEM(4, _alifLong_fromUid(st->st_uid));
	SET_ITEM(5, _alifLong_fromGid(st->st_gid));
#endif
	SET_ITEM(6, alifLong_fromLongLong(st->st_size));

	// Set st_atime, st_mtime and st_ctime
	unsigned long ansec, mnsec, cnsec;
#if defined(HAVE_STAT_TV_NSEC)
	ansec = st->st_atim.tv_nsec;
	mnsec = st->st_mtim.tv_nsec;
	cnsec = st->st_ctim.tv_nsec;
#elif defined(HAVE_STAT_TV_NSEC2)
	ansec = st->st_atimespec.tv_nsec;
	mnsec = st->st_mtimespec.tv_nsec;
	cnsec = st->st_ctimespec.tv_nsec;
#elif defined(HAVE_STAT_NSEC)
	ansec = st->st_atime_nsec;
	mnsec = st->st_mtime_nsec;
	cnsec = st->st_ctime_nsec;
#else
	ansec = mnsec = cnsec = 0;
#endif
	if (fill_time(_module, v, 7, 10, 13, st->st_atime, ansec) < 0) {
		goto error;
	}
	if (fill_time(_module, v, 8, 11, 14, st->st_mtime, mnsec) < 0) {
		goto error;
	}
	if (fill_time(_module, v, 9, 12, 15, st->st_ctime, cnsec) < 0) {
		goto error;
	}

#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	SET_ITEM(ST_BLKSIZE_IDX, alifLong_fromLong((long)st->st_blksize));
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
	SET_ITEM(ST_BLOCKS_IDX, alifLong_fromLong((long)st->st_blocks));
#endif
#ifdef HAVE_STRUCT_STAT_ST_RDEV
	SET_ITEM(ST_RDEV_IDX, alifLong_fromLong((long)st->st_rdev));
#endif
#ifdef HAVE_STRUCT_STAT_ST_GEN
	SET_ITEM(ST_GEN_IDX, alifLong_fromLong((long)st->st_gen));
#endif
#if defined(HAVE_STRUCT_STAT_ST_BIRTHTIME)
	{
		unsigned long bsec, bnsec;
		bsec = (long)st->st_birthtime;
	#ifdef HAVE_STAT_TV_NSEC2
		bnsec = st->st_birthtimespec.tv_nsec;
	#else
		bnsec = 0;
	#endif
		SET_ITEM(ST_BIRTHTIME_IDX, alifFloat_fromDouble(bsec + bnsec * 1e-9));
	}
#elif defined(_WINDOWS)
	if (fill_time(_module, v, -1, ST_BIRTHTIME_IDX, ST_BIRTHTIME_NS_IDX,
		st->st_birthtime, st->st_birthtime_nsec) < 0) {
		goto error;
	}
#endif
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
	SET_ITEM(ST_FLAGS_IDX, alifLong_fromLong((long)st->st_flags));
#endif
#ifdef HAVE_STRUCT_STAT_ST_FILE_ATTRIBUTES
	SET_ITEM(ST_FILE_ATTRIBUTES_IDX,
		alifLong_fromUnsignedLong(st->st_file_attributes));
#endif
#ifdef HAVE_STRUCT_STAT_ST_FSTYPE
	SET_ITEM(ST_FSTYPE_IDX, alifUStr_fromString(st->st_fstype));
#endif
#ifdef HAVE_STRUCT_STAT_ST_REPARSE_TAG
	SET_ITEM(ST_REPARSE_TAG_IDX, alifLong_fromUnsignedLong(st->st_reparse_tag));
#endif

	return v;

error:
	ALIF_DECREF(v);
	return nullptr;

#undef SET_ITEM
}

/* POSIX methods */

static AlifObject* posix_doStat(AlifObject* _module,
	const char* _functionName, PathT* _path,
	AlifIntT _dirFD, AlifIntT _followSymlinks) { // 2728
	STRUCT_STAT st{};
	AlifIntT result{};

#ifdef HAVE_FSTATAT
	AlifIntT fstatat_unavailable = 0;
#endif

#if !defined(_WINDOWS) and !defined(HAVE_FSTATAT) and !defined(HAVE_LSTAT)
	if (followSymlinks_specified(_functionName, _followSymlinks))
		return nullptr;
#endif

	if (pathAndDirFD_invalid("حالة", _path, _dirFD) ||
		dirFDAndFD_invalid("حالة", _dirFD, _path->fd) ||
		fdAndFollowSymlinks_invalid("حالة", _path->fd, _followSymlinks))
		return nullptr;

	ALIF_BEGIN_ALLOW_THREADS
		if (_path->fd != -1)
			result = FSTAT(_path->fd, &st);
#ifdef _WINDOWS
		else if (_followSymlinks)
			result = win32_stat(_path->wide, &st);
		else
			result = win32_lstat(_path->wide, &st);
#else
		else
		#if defined(HAVE_LSTAT)
			if ((!_followSymlinks) and (_dirFD == DEFAULT_DIR_FD))
				result = LSTAT(_path->narrow, &st);
			else
			#endif /* HAVE_LSTAT */
			#ifdef HAVE_FSTATAT
				if ((_dirFD != DEFAULT_DIR_FD) or !_followSymlinks) {
					if (HAVE_FSTATAT_RUNTIME) {
						result = fstatat(_dirFD, _path->narrow, &st,
							_followSymlinks ? 0 : AT_SYMLINK_NOFOLLOW);

					}
					else {
						fstatat_unavailable = 1;
					}
				}
				else
				#endif /* HAVE_FSTATAT */
					result = STAT(_path->narrow, &st);
#endif /* _WINDOWS */
	ALIF_END_ALLOW_THREADS

	#ifdef HAVE_FSTATAT
		if (fstatat_unavailable) {
			argument_unavailableError("stat", "dirFD");
			return nullptr;
		}
#endif

	if (result != 0) {
		return path_error(_path);
	}

	return _alifStat_fromStructStat(_module, &st);
}









// 2861
#ifdef HAVE_FSTATAT
#define FSTATAT_DIR_FD_CONVERTER dirFD_converter
#else
#define FSTATAT_DIR_FD_CONVERTER dirFD_unavailable
#endif

// 2873
#ifdef HAVE_MKDIRAT
#define MKDIRAT_DIR_FD_CONVERTER dirFD_converter
#else
#define MKDIRAT_DIR_FD_CONVERTER dirFD_unavailable
#endif

// 2933
#ifdef HAVE_FDOPENDIR
#define PATH_HAVE_FDOPENDIR 1
#else
#define PATH_HAVE_FDOPENDIR 0
#endif















static AlifObject* os_statImpl(AlifObject* _module,
	PathT* path, AlifIntT _dirFD, AlifIntT _followSymlinks) { // 3160
	return posix_doStat(_module, "حالة", path, _dirFD, _followSymlinks);
}


// 3322
#ifndef F_OK
#define F_OK 0
#endif
#ifndef R_OK
#define R_OK 4
#endif
#ifndef W_OK
#define W_OK 2
#endif
#ifndef X_OK
#define X_OK 1
#endif



static AlifObject* posix_getcwd(AlifIntT _useBytes) { // 4125
#ifdef _WINDOWS
	wchar_t wbuf[MAXPATHLEN]{};
	wchar_t* wbuf2 = wbuf;
	DWORD len{};

	ALIF_BEGIN_ALLOW_THREADS
		len = GetCurrentDirectoryW(ALIF_ARRAY_LENGTH(wbuf), wbuf);
	/* If the buffer is large enough, len does not include the
	   terminating \0. If the buffer is too small, len includes
	   the space needed for the terminator. */
	if (len >= ALIF_ARRAY_LENGTH(wbuf)) {
		if (len <= ALIF_SIZET_MAX / sizeof(wchar_t)) {
			wbuf2 = (wchar_t*)alifMem_dataAlloc(len * sizeof(wchar_t));
		}
		else {
			wbuf2 = nullptr;
		}
		if (wbuf2) {
			len = GetCurrentDirectoryW(len, wbuf2);
		}
	}
	ALIF_END_ALLOW_THREADS

		if (!wbuf2) {
			//alifErr_noMemory();
			return nullptr;
		}
	if (!len) {
		//alifErr_setFromWindowsErr(0);
		if (wbuf2 != wbuf)
			alifMem_dataFree(wbuf2);
		return nullptr;
	}

	AlifObject* resobj = alifUStr_fromWideChar(wbuf2, len);
	if (wbuf2 != wbuf) {
		alifMem_dataFree(wbuf2);
	}

	if (_useBytes) {
		if (resobj == nullptr) {
			return nullptr;
		}
		ALIF_SETREF(resobj, alifUStr_encodeFSDefault(resobj));
	}

	return resobj;
#else
	const size_t chunk = 1024;

	char* buf = nullptr;
	char* cwd = nullptr;
	size_t buflen = 0;

	ALIF_BEGIN_ALLOW_THREADS
		do {
			char* newbuf;
			if (buflen <= ALIF_SIZET_MAX - chunk) {
				buflen += chunk;
				newbuf = (char*)alifMem_dataRealloc(buf, buflen);
			}
			else {
				newbuf = nullptr;
			}
			if (newbuf == nullptr) {
				alifMem_dataFree(buf);
				buf = nullptr;
				break;
			}
			buf = newbuf;

			cwd = getcwd(buf, buflen);
		} while (cwd == nullptr && errno == ERANGE);
	ALIF_END_ALLOW_THREADS

		if (buf == nullptr) {
			//return alifErr_noMemory();
		}
	if (cwd == nullptr) {
		// posix_error();
		alifMem_dataFree(buf);
		return nullptr;
	}

	AlifObject* obj{};
	if (_useBytes) {
		obj = alifBytes_fromStringAndSize(buf, strlen(buf));
	}
	else {
		obj = alifUStr_decodeFSDefault(buf);
	}
#ifdef __linux__
	if (buf[0] != '/') {
		/*
		 * On Linux >= 2.6.36 with glibc < 2.27, getcwd() can return a
		 * relative pathname starting with '(unreachable)'. We detect this
		 * and fail with ENOENT, matching newer glibc behaviour.
		 */
		errno = ENOENT;
		path_objectError(obj);
		alifMem_dataFree(buf);
		return nullptr;
	}
#endif
	alifMem_dataFree(buf);

	return obj;
#endif   /* !_WINDOWS */
}

static AlifObject* os_getcwdImpl(AlifObject* _module) { // 4246
	return posix_getcwd(0);
}




#if defined(_WINDOWS) and !defined(HAVE_OPENDIR)
static AlifObject* _listdirWindows_noOpenDir(PathT* _path,
	AlifObject* _list) { // 4391
	AlifObject* v{};
	HANDLE hFindFile = INVALID_HANDLE_VALUE;
	BOOL result, return_bytes;
	wchar_t namebuf[MAX_PATH + 4]{}; /* Overallocate for "\*.*" */
	/* only claim to have space for MAX_PATH */
	AlifSizeT len = ALIF_ARRAY_LENGTH(namebuf) - 4;
	wchar_t* wnamebuf = nullptr;

	WIN32_FIND_DATAW wFileData;
	const wchar_t* po_wchars{};

	if (!_path->wide) { /* Default arg: "." */
		po_wchars = L".";
		len = 1;
		return_bytes = 0;
	}
	else {
		po_wchars = _path->wide;
		len = wcslen(_path->wide);
		return_bytes = ALIFBYTES_CHECK(_path->object);
	}
	/* The +5 is so we can append "\\*.*\0" */
	wnamebuf = ALIFMEM_NEW(wchar_t, len + 5);
	if (!wnamebuf) {
		//alifErr_noMemory();
		goto exit;
	}
	wcscpy(wnamebuf, po_wchars);
	if (len > 0) {
		wchar_t wch = wnamebuf[len - 1];
		if (wch != SEP && wch != ALTSEP && wch != L':')
			wnamebuf[len++] = SEP;
		wcscpy(wnamebuf + len, L"*.*");
	}
	if ((_list = alifList_new(0)) == nullptr) {
		goto exit;
	}
	ALIF_BEGIN_ALLOW_THREADS
		hFindFile = FindFirstFileW(wnamebuf, &wFileData);
	ALIF_END_ALLOW_THREADS
		if (hFindFile == INVALID_HANDLE_VALUE) {
			int error = GetLastError();
			if (error == ERROR_FILE_NOT_FOUND)
				goto exit;
			path_error(_path);
			ALIF_CLEAR(_list);
			goto exit;
		}
	do {
		/* Skip over . and .. */
		if (wcscmp(wFileData.cFileName, L".") != 0 &&
			wcscmp(wFileData.cFileName, L"..") != 0) {
			v = alifUStr_fromWideChar(wFileData.cFileName,
				wcslen(wFileData.cFileName));
			if (return_bytes && v) {
				ALIF_SETREF(v, alifUStr_encodeFSDefault(v));
			}
			if (v == nullptr) {
				ALIF_CLEAR(_list);
				break;
			}
			if (alifList_append(_list, v) != 0) {
				ALIF_DECREF(v);
				ALIF_CLEAR(_list);
				break;
			}
			ALIF_DECREF(v);
		}
		ALIF_BEGIN_ALLOW_THREADS
			result = FindNextFileW(hFindFile, &wFileData);
		ALIF_END_ALLOW_THREADS
			/* FindNextFile sets error to ERROR_NO_MORE_FILES if
			it got to the end of the directory. */
			if (!result and GetLastError() != ERROR_NO_MORE_FILES) {
				path_error(_path);
				ALIF_CLEAR(_list);
				goto exit;
			}
	}
	while (result == TRUE);

exit:
	if (hFindFile != INVALID_HANDLE_VALUE) {
		if (FindClose(hFindFile) == FALSE) {
			if (_list != nullptr) {
				path_error(_path);
				ALIF_CLEAR(_list);
			}
		}
	}
	alifMem_dataFree(wnamebuf);

	return _list;
}  /* end of _listdir_windows_no_opendir */

#else  /* thus POSIX, ie: not (_WINDOWS and not HAVE_OPENDIR) */ // 4487
static AlifObject* _posix_listdir(PathT *path, AlifObject *list) {
    AlifObject* v{};
    DIR *dirp = nullptr;
    struct dirent *ep;
    AlifIntT return_str; /* if false, return bytes */
#ifdef HAVE_FDOPENDIR
    AlifIntT fd = -1;
#endif

    errno = 0;
#ifdef HAVE_FDOPENDIR
    if (path->fd != -1) {
      if (HAVE_FDOPENDIR_RUNTIME) {
        /* closedir() closes the FD, so we duplicate it */
        fd = _alif_dup(path->fd);
        if (fd == -1)
            return nullptr;

        return_str = 1;

        ALIF_BEGIN_ALLOW_THREADS
        dirp = fdopendir(fd);
        ALIF_END_ALLOW_THREADS
      } else {
        alifErr_setString(_alifExcTypeError_,
            "listdir: path should be string, bytes, os.PathLike or None, not int");
        return nullptr;
      }
    }
    else
#endif
    {
        const char *name;
        if (path->narrow) {
            name = path->narrow;
            /* only return bytes if they specified a bytes object */
            return_str = !ALIFBYTES_CHECK(path->object);
        }
        else {
            name = ".";
            return_str = 1;
        }

        ALIF_BEGIN_ALLOW_THREADS
        dirp = opendir(name);
        ALIF_END_ALLOW_THREADS
    }

    if (dirp == nullptr) {
         path_error(path);
        list = nullptr;
#ifdef HAVE_FDOPENDIR
        if (fd != -1) {
            ALIF_BEGIN_ALLOW_THREADS
            close(fd);
            ALIF_END_ALLOW_THREADS
        }
#endif
        goto exit;
    }
    if ((list = alifList_new(0)) == nullptr) {
        goto exit;
    }
    for (;;) {
        errno = 0;
        ALIF_BEGIN_ALLOW_THREADS
        ep = readdir(dirp);
        ALIF_END_ALLOW_THREADS
        if (ep == nullptr) {
            if (errno == 0) {
                break;
            } else {
                 path_error(path);
                ALIF_CLEAR(list);
                goto exit;
            }
        }
        if (ep->d_name[0] == '.' &&
            (NAMLEN(ep) == 1 ||
             (ep->d_name[1] == '.' && NAMLEN(ep) == 2)))
            continue;
        if (return_str)
            v = alifUStr_decodeFSDefaultAndSize(ep->d_name, NAMLEN(ep));
        else
            v = alifBytes_fromStringAndSize(ep->d_name, NAMLEN(ep));
        if (v == nullptr) {
            ALIF_CLEAR(list);
            break;
        }
        if (alifList_append(list, v) != 0) {
            ALIF_DECREF(v);
            ALIF_CLEAR(list);
            break;
        }
        ALIF_DECREF(v);
    }

exit:
    if (dirp != nullptr) {
        ALIF_BEGIN_ALLOW_THREADS
#ifdef HAVE_FDOPENDIR
        if (fd > -1)
            rewinddir(dirp);
#endif
        closedir(dirp);
        ALIF_END_ALLOW_THREADS
    }

    return list;
}  /* end of _posix_listdir */
#endif  /* which OS */ // 4601



static AlifObject* os_listdirImpl(AlifObject* _module, PathT* _path) { // 4625
	//if (alifSys_audit("نظام_التشغيل.محتويات_المجلد", "O",
	//	_path->object ? _path->object : ALIF_NONE) < 0) {
	//	return nullptr;
	//}
#if defined(_WINDOWS) and !defined(HAVE_OPENDIR)
	return _listdirWindows_noOpenDir(_path, nullptr);
#else
	return _posix_listdir(_path, nullptr);
#endif
}

#ifdef _WINDOWS // 4641





AlifIntT _alifOS_getFullPathName(const wchar_t* _path, wchar_t** _absPathP) { // 4921
	wchar_t woutbuf[MAX_PATH], * woutbufp = woutbuf;
	DWORD result{};

	result = GetFullPathNameW(_path,
		ALIF_ARRAY_LENGTH(woutbuf), woutbuf, nullptr);
	if (!result) {
		return -1;
	}

	if (result >= ALIF_ARRAY_LENGTH(woutbuf)) {
		if ((AlifUSizeT)result <= (AlifUSizeT)ALIF_SIZET_MAX / sizeof(wchar_t)) {
			woutbufp = (wchar_t*)alifMem_dataAlloc((AlifUSizeT)result * sizeof(wchar_t));
		}
		else {
			woutbufp = nullptr;
		}
		if (!woutbufp) {
			*_absPathP = nullptr;
			return 0;
		}

		result = GetFullPathNameW(_path, result, woutbufp, nullptr);
		if (!result) {
			alifMem_dataFree(woutbufp);
			return -1;
		}
	}

	if (woutbufp != woutbuf) {
		*_absPathP = woutbufp;
		return 0;
	}

	*_absPathP = alifMem_wcsDup(woutbufp);
	return 0;
}





static AlifObject* os__pathSplitRootImpl(AlifObject* _module,
	PathT* _path) { // 5163
	wchar_t* buffer{};
	wchar_t* end{};
	AlifObject* result{};
	HRESULT ret{};

	buffer = (wchar_t*)alifMem_dataAlloc(sizeof(wchar_t) * (wcslen(_path->wide) + 1));
	if (!buffer) {
		return nullptr;
	}
	wcscpy(buffer, _path->wide);
	for (wchar_t* p = wcschr(buffer, L'/'); p; p = wcschr(p, L'/')) {
		*p = L'\\';
	}

	ALIF_BEGIN_ALLOW_THREADS
	ret = PathCchSkipRoot(buffer, &end);
	ALIF_END_ALLOW_THREADS
		if (FAILED(ret)) {
			result = alif_buildValue("sO", "", _path->object);
		}
		else if (end != buffer) {
			size_t rootLen = (size_t)(end - buffer);
			result = alif_buildValue("NN",
				alifUStr_fromWideChar(_path->wide, rootLen),
				alifUStr_fromWideChar(_path->wide + rootLen, -1)
			);
		}
		else {
			result = alif_buildValue("Os", _path->object, "");
		}
	alifMem_dataFree(buffer);

	return result;
}



#endif // 5494






static AlifObject* os_mkdirImpl(AlifObject* _module,
	PathT* _path, AlifIntT _mode, AlifIntT _dirFD) { // 5648
	AlifIntT result{};
#ifdef _WINDOWS
	AlifIntT error = 0;
	AlifIntT pathError = 0;
	SECURITY_ATTRIBUTES secAttr = { sizeof(secAttr) };
	SECURITY_ATTRIBUTES* pSecAttr{};
#endif
#ifdef HAVE_MKDIRAT
	AlifIntT mkdirat_unavailable = 0;
#endif

	//if (alifSys_audit("نظام_التشغيل.اوجد_مجلد", "Oii", _path->object, _mode,
	//	_dirFD == DEFAULT_DIR_FD ? -1 : _dirFD) < 0) {
	//	return nullptr;
	//}

#ifdef _WINDOWS
	ALIF_BEGIN_ALLOW_THREADS
		if (_mode == 0700 /* 0o700 */) {
			ULONG sdSize;
			pSecAttr = &secAttr;
			// Set a discretionary ACL (D) that is protected (P) and includes
			// inheritable (OICI) entries that allow (A) full control (FA) to
			// SYSTEM (SY), Administrators (BA), and the owner (OW).
			if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(
				L"D:P(A;OICI;FA;;;SY)(A;OICI;FA;;;BA)(A;OICI;FA;;;OW)",
				SDDL_REVISION_1,
				&secAttr.lpSecurityDescriptor,
				&sdSize
			)) {
				error = GetLastError();
			}
		}
	if (!error) {
		result = CreateDirectoryW(_path->wide, pSecAttr);
		if (secAttr.lpSecurityDescriptor &&
			// uncommonly, LocalFree returns non-zero on error, but still uses
			// GetLastError() to see what the error code is
			LocalFree(secAttr.lpSecurityDescriptor)) {
			error = GetLastError();
		}
	}
	ALIF_END_ALLOW_THREADS

		if (error) {
			//return alifErr_setFromWindowsErr(error);
			printf("خطأ اثناء إنشاء مجلد جديد");
			return nullptr;
		}
	if (!result) {
		return path_error(_path);
	}
#else
	ALIF_BEGIN_ALLOW_THREADS
	#if HAVE_MKDIRAT
		if (_dirFD != DEFAULT_DIR_FD) {
			if (HAVE_MKDIRAT_RUNTIME) {
				result = mkdirat(_dirFD, _path->narrow, _mode);

			}
			else {
				mkdirat_unavailable = 1;
			}
		}
		else
		#endif
		#if defined(__WATCOMC__) && !defined(__QNX__)
			result = mkdir(path->narrow);
#else
			result = mkdir(_path->narrow, _mode);
#endif
	ALIF_END_ALLOW_THREADS

	#if HAVE_MKDIRAT
		if (mkdirat_unavailable) {
			argument_unavailableError(nullptr, "مجلد_واصف_ملف");
			return nullptr;
		}
#endif

	if (result < 0) {
		return path_error(_path);
	}
#endif /* _WINDOWS */
	return ALIF_NONE;
}






static AlifStructSequenceField _unameResultFields_[] = { // 6217
	//* todo
	{"sysname",    "operating system name"},
	{"nodename",   "name of machine on network (implementation-defined)"},
	{"release",    "operating system release"},
	{"version",    "operating system version"},
	{"machine",    "hardware identifier"},
	{nullptr}
};

static AlifStructSequenceDesc _unameResultDesc_ = { // 6234
	MODNAME ".نتيجة_بدون_اسم", // uname_result /* name */
	nullptr, /* doc */
	_unameResultFields_,
	5
};















static AlifStructSequenceField _timesResultFields_[] = { // 10594
	//* todo
	{"user",    "user time"},
	{"system",   "system time"},
	{"children_user",    "user time of children"},
	{"children_system",    "system time of children"},
	{"elapsed",    "elapsed time since an arbitrary point in the past"},
	{nullptr}
};

static AlifStructSequenceDesc _timesResultDesc_ = { // 10612
	"نتيجة_الوقت", // times_result /* name */
	nullptr, /* doc */
	_timesResultFields_,
	5
};






/* Return -1 on failure, 0 on success. */
static AlifIntT setup_confNameTables(AlifObject* module) { // 14547
#if defined(HAVE_FPATHCONF) or defined(HAVE_PATHCONF)
	if (setup_confNameTable(posix_constants_pathconf,
		sizeof(posix_constants_pathconf)
		/ sizeof(struct constdef),
		"pathconf_names", module))
		return -1;
#endif
#ifdef HAVE_CONFSTR
	if (setup_confNameTable(posix_constants_confstr,
		sizeof(posix_constants_confstr)
		/ sizeof(struct constdef),
		"confstr_names", module))
		return -1;
#endif
#ifdef HAVE_SYSCONF
	if (setup_confNameTable(posix_constants_sysconf,
		sizeof(posix_constants_sysconf)
		/ sizeof(struct constdef),
		"sysconf_names", module))
		return -1;
#endif
	return 0;
}



static AlifStructSequenceField _terminalSizeFields_[] = { // 15253
	//* todo
	{"columns", "width of the terminal window in characters"},
	{"lines", "height of the terminal window in characters"},
	{nullptr, nullptr}
};

static AlifStructSequenceDesc _terminalSizeDesc_ = { // 15259
	"نظام_التشغيل.حجم_الطرفية", // os.terminal_size
	nullptr,
	_terminalSizeFields_,
	2,
};



static AlifObject* os_cpuCountImpl(AlifObject* _module) { // 15362
	const AlifConfig* config = _alif_getConfig();
	if (config->cpuCount > 0) {
		return alifLong_fromLong(config->cpuCount);
	}

	AlifIntT ncpu = 0;
#ifdef _WINDOWS
# ifdef MS_WINDOWS_DESKTOP
	ncpu = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
# else
	ncpu = 0;
# endif

#elif defined(__hpux)
	ncpu = mpctl(MPC_GETNUMSPUS, nullptr, nullptr);

#elif defined(HAVE_SYSCONF) && defined(_SC_NPROCESSORS_ONLN)
	ncpu = sysconf(_SC_NPROCESSORS_ONLN);

#elif defined(__VXWORKS__)
	ncpu = _alif_popcount32(vxCpuEnabledGet());

#elif defined(__DragonFly__) or \
      defined(__OpenBSD__)   or \
      defined(__FreeBSD__)   or \
      defined(__NetBSD__)    or \
      defined(__APPLE__)
	ncpu = 0;
	size_t len = sizeof(ncpu);
	int mib[2] = { CTL_HW, HW_NCPU };
	if (sysctl(mib, 2, &ncpu, &len, nullptr, 0) != 0) {
		ncpu = 0;
	}
#endif

	if (ncpu < 1) {
		return ALIF_NONE;
	}
	return alifLong_fromLong(ncpu);
}




class DirEntry { // 15555
public:
	ALIFOBJECT_HEAD;
	AlifObject* name{};
	AlifObject* path{};
	AlifObject* stat{};
	AlifObject* lstat{};
#ifdef _WINDOWS
	class AlifStatStruct win32_lstat{};
	uint64_t win32_file_index{};
	uint64_t win32_file_index_high{};
	AlifIntT got_file_index{};
#else /* POSIX */
#ifdef HAVE_DIRENT_D_TYPE
	unsigned char d_type{};
#endif
	ino_t d_ino{};
	AlifIntT dir_fd{};
#endif
};



static AlifMemberDef _dirEntryMembers_[] = {
	{"اسم", ALIF_T_OBJECT_EX, offsetof(DirEntry, name), ALIF_READONLY},
	{"مسار", ALIF_T_OBJECT_EX, offsetof(DirEntry, path), ALIF_READONLY},
	{nullptr}
};

#include "clinic/PosixModule.cpp.h" // 15935

static AlifMethodDef _dirEntryMethods_[] = { // 15937
	{nullptr}
};

static AlifTypeSlot _dirEntryTypeSlots_[] = { // 15950
	{ALIF_TP_METHODS, _dirEntryMethods_},
	{ALIF_TP_MEMBERS, _dirEntryMembers_},
	{0, 0},
};

static AlifTypeSpec _dirEntryTypeSpec_ = { // 15958
	MODNAME ".مدخل_مجلد", // .DirEntry
	sizeof(DirEntry),
	0,
	ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_DISALLOW_INSTANTIATION,
	_dirEntryTypeSlots_
};



class ScanDirIterator { // 16158
public:
	ALIFOBJECT_HEAD;
	PathT path{};
#ifdef _WINDOWS
	HANDLE handle;
	WIN32_FIND_DATAW file_data;
	AlifIntT first_time{};
#else /* POSIX */
	DIR* dirp;
#endif
#ifdef HAVE_FDOPENDIR
	AlifIntT fd;
#endif
};


static AlifMethodDef _scanDirIteratorMethods_[] = { // 16376
	//{"__enter__", (AlifCPPFunction)ScandirIterator_enter, METHOD_NOARGS},
	//{"__exit__", (AlifCPPFunction)ScandirIterator_exit, METHOD_VARARGS},
	//{"close", (AlifCPPFunction)ScandirIterator_close, METHOD_NOARGS},
	{nullptr}
};

static AlifTypeSlot _scanDirIteratorTypeSlots_[] = { // 16383
	//{ALIF_TP_DEALLOC, ScandirIterator_dealloc},
	//{ALIF_TP_FINALIZE, ScandirIterator_finalize},
	//{ALIF_TP_ITER, alifObject_selfIter},
	//{ALIF_TP_ITERNEXT, ScandirIterator_iternext},
	{ALIF_TP_METHODS, _scanDirIteratorMethods_},
	{0, 0},
};

static AlifTypeSpec _scanDirIteratorTypeSpec_ = { // 16392
	MODNAME ".مسح_المجلد_بالتكرار", // ScandirIterator
	sizeof(ScanDirIterator),
	0,
	(ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_FINALIZE
		| ALIF_TPFLAGS_DISALLOW_INSTANTIATION),
	_scanDirIteratorTypeSlots_
};










AlifObject* alifOS_fsPath(AlifObject* path) { // 16477
	AlifObject* func = nullptr;
	AlifObject* pathRepr = nullptr;

	if (ALIFUSTR_CHECK(path) or ALIFBYTES_CHECK(path)) {
		return ALIF_NEWREF(path);
	}

	//func = _alifObject_lookupSpecial(path, &ALIF_ID(__fspath__));
	//if ((func == nullptr) or (func == ALIF_NONE)) {
	//	return alifErr_format(_alifExcTypeError_,
	//		"expected str, bytes or os.PathLike object, "
	//		"not %.200s",
	//		_alifType_name(ALIF_TYPE(path)));
	//}

	//pathRepr = _alifObject_callNoArgs(func);
	//ALIF_DECREF(func);
	//if (nullptr == pathRepr) {
	//	return nullptr;
	//}

	if (!(ALIFUSTR_CHECK(pathRepr) or ALIFBYTES_CHECK(pathRepr))) {
		alifErr_format(_alifExcTypeError_,
			"يتوقع %.200s.__fspath__() ان يرجع نص او ثمانيات, "
			"وليس %.200s", _alifType_name(ALIF_TYPE(path)),
			_alifType_name(ALIF_TYPE(pathRepr)));
		ALIF_DECREF(pathRepr);
		return nullptr;
	}

	return pathRepr;
}













static AlifMethodDef _posixMethods_[] = { // 16898
	OS_STAT_METHODDEF

	OS_GETCWD_METHODDEF

	OS_LISTDIR_METHODDEF

	OS__PATH_SPLITROOT_METHODDEF

	OS_MKDIR_METHODDEF

	OS_CPU_COUNT_METHODDEF
	{nullptr, nullptr}            /* Sentinel */
};

static AlifIntT all_ins(AlifObject* m) { // 17116
#ifdef F_OK
	if (ALIFMODULE_ADDINTMACRO(m, F_OK, F_OK)) return -1;
#endif
	//* alif //* todo


	return 0;
};


static const class HaveFunction {
public:
	const char * const label;
	AlifIntT (*probe)(void);
} _haveFunctions_[] = {

	//* todo

#ifdef _WINDOWS
	{ "MS_WINDOWS", nullptr },
#endif

	{ nullptr, nullptr }
};



static AlifIntT posixModule_exec(AlifObject* m) { // 17979
	PosixState* state = get_posixState(m);

#if defined(HAVE_PWRITEV)
	if (HAVE_PWRITEV_RUNTIME) {}
	else {
		AlifObject* dct = alifModule_getDict(m);

		if (dct == nullptr) {
			return -1;
		}

		if (alifDict_popString(dct, "pwritev", nullptr) < 0) {
			return -1;
		}
		if (alifDict_popString(dct, "preadv", nullptr) < 0) {
			return -1;
		}
	}
#endif

	/* Initialize environ dictionary */
	if (alifModule_add(m, "environ", convertenviron()) != 0) {
		return -1;
	}

	if (all_ins(m))
		return -1;

	if (setup_confNameTables(m))
		return -1;

	if (alifModule_addObjectRef(m, "error", _alifExcOSError_) < 0) {
		return -1;
	}

#if defined(HAVE_WAITID)
	waitid_result_desc.name = MODNAME ".waitid_result";
	state->WaitidResultType = (AlifObject*)alifStructSequence_newType(&waitid_result_desc);
	if (alifModule_addObjectRef(m, "waitid_result", state->WaitidResultType) < 0) {
		return -1;
	}
#endif

	_statResultDesc_.name = "نظام_التشغيل.نتيجة_الحالة"; 
	_statResultDesc_.fields[7].name = _alifStructSequenceUnnamedField_;
	_statResultDesc_.fields[8].name = _alifStructSequenceUnnamedField_;
	_statResultDesc_.fields[9].name = _alifStructSequenceUnnamedField_;
	state->StatResultType = (AlifObject*)alifStructSequence_newType(&_statResultDesc_);
	if (alifModule_addObjectRef(m, "نتيجة_الحالة", state->StatResultType) < 0) {
		return -1;
	}
	state->statresult_new_orig = ((AlifTypeObject*)state->StatResultType)->new_;
	((AlifTypeObject*)state->StatResultType)->new_ = statResult_new;

	_statvfsResultDesc_.name = "نظام_التشغيل.نتيجة_حالة_ملف_النظام_الافتراضي";
	state->StatVFSResultType = (AlifObject*)alifStructSequence_newType(&_statvfsResultDesc_);
	if (alifModule_addObjectRef(m, "نتيجة_حالة_ملف_النظام_الافتراضي", state->StatVFSResultType) < 0) {
		return -1;
	}

#if defined(HAVE_SCHED_SETPARAM) or defined(HAVE_SCHED_SETSCHEDULER) or defined(POSIX_SPAWN_SETSCHEDULER) || defined(POSIX_SPAWN_SETSCHEDPARAM)
	_schedParamDesc_.name = MODNAME ".sched_param";
	state->SchedParamType = (AlifObject*)alifStructSequence_newType(&_schedParamDesc_);
	if (alifModule_addObjectRef(m, "sched_param", state->SchedParamType) < 0) {
		return -1;
	}
	((AlifTypeObject*)state->SchedParamType)->new_ = os_sched_param;
	if (_alifType_addMethod((AlifTypeObject*)state->SchedParamType,
		&os_sched_param_reduce_method) < 0) {
		return -1;
	}
	alifType_modified((AlifTypeObject*)state->SchedParamType);
#endif

	/* initialize TerminalSize_info */
	state->TerminalSizeType = (AlifObject*)alifStructSequence_newType(&_terminalSizeDesc_);
	if (alifModule_addObjectRef(m, "حجم_الطرفية", state->TerminalSizeType) < 0) {
		return -1;
	}

	/* initialize scandir types */
	AlifObject* ScandirIteratorType = alifType_fromModuleAndSpec(m, &_scanDirIteratorTypeSpec_, nullptr);
	if (ScandirIteratorType == nullptr) {
		return -1;
	}
	state->ScandirIteratorType = ScandirIteratorType;

	state->DirEntryType = alifType_fromModuleAndSpec(m, &_dirEntryTypeSpec_, nullptr);
	if (alifModule_addObjectRef(m, "مدخل_مجلد", state->DirEntryType) < 0) {
		return -1;
	}

	_timesResultDesc_.name = MODNAME ".نتيجة_الوقت";
	state->TimesResultType = (AlifObject*)alifStructSequence_newType(&_timesResultDesc_);
	if (alifModule_addObjectRef(m, "نتيجة_الوقت", state->TimesResultType) < 0) {
		return -1;
	}

	state->UnameResultType = (AlifObject*)alifStructSequence_newType(&_unameResultDesc_);
	if (alifModule_addObjectRef(m, "نتيجة_بدون_اسم", state->UnameResultType) < 0) {
		return -1;
	}

	if ((state->billion = alifLong_fromLong(1000000000)) == nullptr)
		return -1;
#if defined(HAVE_WAIT3) || defined(HAVE_WAIT4)
	state->struct_rusage = alifUStr_internFromString("struct_rusage");
	if (state->struct_rusage == nullptr)
		return -1;
#endif
	state->st_mode = alifUStr_internFromString("الوضع");
	if (state->st_mode == nullptr)
		return -1;
	/* suppress "function not used" warnings */
	{
		int ignored;
		fd_specified("", -1);
		followSymlinks_specified("", 1);
		dirFDAndFollowSymlinks_invalid("chmod", DEFAULT_DIR_FD, 1);
		dirFD_converter(ALIF_NONE, &ignored);
		dirFD_unavailable(ALIF_NONE, &ignored);
	}

	AlifObject* list = alifList_new(0);
	if (!list) {
		return -1;
	}
	for (const HaveFunction* trace = _haveFunctions_; trace->label; trace++) {
		AlifObject* unicode{};
		if (trace->probe and !trace->probe()) continue;
		unicode = alifUStr_decodeASCII(trace->label, strlen(trace->label), nullptr);
		if (!unicode)
			return -1;
		if (alifList_append(list, unicode))
			return -1;
		ALIF_DECREF(unicode);
	}

#ifndef _WINDOWS
	if (_alif_getTicksPerSecond(&state->ticks_per_second) < 0) {
		alifErr_setString(_alifExcRuntimeError_,
			"لم يتمكن من قراءة ticks_per_second");
		return -1;
	}
#endif

	return alifModule_add(m, "_have_functions", list);
}


static AlifModuleDefSlot _posixModuleSlots_[] = { // 18138
	{ALIF_MOD_EXEC, (void*)posixModule_exec},
	{ALIF_MOD_MULTIPLE_INTERPRETERS, ALIF_MOD_PER_INTERPRETER_GIL_SUPPORTED},
	{ALIF_MOD_GIL, ALIF_MOD_GIL_NOT_USED},
	{0, nullptr}
};

static AlifModuleDef _posixModule_ = { // 18145
	.base = ALIFMODULEDEF_HEAD_INIT,
	.name = MODNAME,
	.size = sizeof(PosixState),
	.methods = _posixMethods_,
	.slots = _posixModuleSlots_,
	//.traverse = _posix_traverse,
	//.clear = _posix_clear,
	//.free = _posix_free,
};



ALIFMODINIT_FUNC
INITFUNC(void) { // 18157
	return alifModuleDef_init(&_posixModule_);
}


