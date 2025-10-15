#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_PathConfig.h"
#include "AlifCore_Memory.h"
#include "AlifCore_State.h"

#include "Marshal.h"
#include "OSDefs.h"

#ifdef _WINDOWS
#include <windows.h>
#include <pathcch.h>
#endif

#ifdef __APPLE__
#  include <dlfcn.h>
#  include <mach-o/dyld.h>
#endif

#include "FrozenModules/getpath.h"

//* alif
#define PREFIX nullptr
#define EXEC_PREFIX nullptr
#define VERSION nullptr
#define ALIFPATH nullptr
#define VPATH "..\\.."
#define PLATLIBDIR "DLLs"
#define ALIFDEBUGEXT ""
//* alif

#if (!defined(PREFIX) or !defined(EXEC_PREFIX) \
        or !defined(VERSION) or !defined(VPATH) \
        or !defined(PLATLIBDIR))
#error "PREFIX, EXEC_PREFIX, VERSION, VPATH and PLATLIBDIR macros must be defined"
#endif

#if !defined(ALIFDEBUGEXT)
#define ALIFDEBUGEXT nullptr
#endif

#if !defined(ALIFWINVER)
#ifdef MS_DLL_ID
#define ALIFWINVER MS_DLL_ID
#else
#define ALIFWINVER nullptr
#endif
#endif

#if !defined(EXE_SUFFIX)
#if defined(_WINDOWS) or defined(__CYGWIN__) or defined(__MINGW32__)
#define EXE_SUFFIX L".exe"
#else
#define EXE_SUFFIX nullptr
#endif
#endif




static AlifStatus alifConfig_initPathConfigAlif(AlifConfig*, AlifIntT); //* alif



/* HELPER FUNCTIONS for getpath.alif */

static AlifObject* getPath_absPath(AlifObject* ALIF_UNUSED(self), AlifObject* _args) { // 60
	AlifObject* r = nullptr;
	AlifObject* pathobj;
	wchar_t* path{};
	if (!alifArg_parseTuple(_args, "U", &pathobj)) {
		return nullptr;
	}
	AlifSizeT len{};
	path = alifUStr_asWideCharString(pathobj, &len);
	if (path) {
		wchar_t* abs{};
		if (_alif_absPath((const wchar_t*)_alif_normPath(path, -1), &abs) == 0 && abs) {
			r = alifUStr_fromWideChar(abs, -1);
			alifMem_dataFree((void*)abs);
		}
		else {
			alifErr_setString(_alifExcOSError_, "failed to make path absolute");
		}
		alifMem_dataFree((void*)path);
	}
	return r;
}

static AlifObject* getPath_baseName(AlifObject* ALIF_UNUSED(self), AlifObject* _args) { // 85
	AlifObject* path{};
	if (!alifArg_parseTuple(_args, "U", &path)) {
		return nullptr;
	}
	AlifSizeT end = ALIFUSTR_GET_LENGTH(path);
	AlifSizeT pos = alifUStr_findChar(path, SEP, 0, end, -1);
	if (pos < 0) {
		return ALIF_NEWREF(path);
	}
	return alifUStr_subString(path, pos + 1, end);
}

static AlifObject* getPath_dirName(AlifObject* ALIF_UNUSED(self), AlifObject* args) { // 101
	AlifObject* path{};
	if (!alifArg_parseTuple(args, "U", &path)) {
		return nullptr;
	}
	AlifSizeT end = ALIFUSTR_GET_LENGTH(path);
	AlifSizeT pos = alifUStr_findChar(path, SEP, 0, end, -1);
	if (pos < 0) {
		return alifUStr_fromStringAndSize(nullptr, 0);
	}
	return alifUStr_subString(path, 0, pos);
}

static AlifObject* getPath_isAbs(AlifObject* ALIF_UNUSED(self), AlifObject* _args) { // 117
	AlifObject* r = nullptr;
	AlifObject* pathObj{};
	const wchar_t* path{};
	if (!alifArg_parseTuple(_args, "U", &pathObj)) {
		return nullptr;
	}
	path = alifUStr_asWideCharString(pathObj, nullptr);
	if (path) {
		r = _alif_isAbs(path) ? ALIF_TRUE : ALIF_FALSE;
		alifMem_dataFree((void*)path);
	}
	return ALIF_XNEWREF(r);
}

static AlifObject* getPath_hasSuffix(AlifObject* ALIF_UNUSED(self), AlifObject* args) { // 135
	AlifObject* r = nullptr;
	AlifObject* pathobj{};
	AlifObject* suffixobj{};
	const wchar_t* path{};
	const wchar_t* suffix{};
	if (!alifArg_parseTuple(args, "UU", &pathobj, &suffixobj)) {
		return nullptr;
	}
	AlifSizeT len{}, suffixLen{};
	path = alifUStr_asWideCharString(pathobj, &len);
	if (path) {
		suffix = alifUStr_asWideCharString(suffixobj, &suffixLen);
		if (suffix) {
			if (suffixLen > len or
			#ifdef _WINDOWS
				wcsicmp(&path[len - suffixLen], suffix) != 0
			#else
				wcscmp(&path[len - suffixLen], suffix) != 0
			#endif
				) {
				r = ALIF_NEWREF(ALIF_FALSE);
			}
			else {
				r = ALIF_NEWREF(ALIF_TRUE);
			}
			alifMem_dataFree((void*)suffix);
		}
		alifMem_dataFree((void*)path);
	}
	return r;
}


static AlifObject* getPath_isDir(AlifObject* ALIF_UNUSED(self), AlifObject* _args) { // 170
	AlifObject* r = nullptr;
	AlifObject* pathObj{};
	const wchar_t* path{};
	if (!alifArg_parseTuple(_args, "U", &pathObj)) {
		return nullptr;
	}
	path = alifUStr_asWideCharString(pathObj, nullptr);
	if (path) {
	#ifdef _WINDOWS
		DWORD attr = GetFileAttributesW(path);
		r = (attr != INVALID_FILE_ATTRIBUTES) and
			(attr & FILE_ATTRIBUTE_DIRECTORY) ? ALIF_TRUE : ALIF_FALSE;
	#else
		struct stat st;
		r = (_alif_wStat(path, &st) == 0) && S_ISDIR(st.st_mode) ? ALIF_TRUE : ALIF_FALSE;
	#endif
		alifMem_dataFree((void*)path);
	}
	return ALIF_XNEWREF(r);
}


static AlifObject* getPath_isFile(AlifObject* ALIF_UNUSED(self), AlifObject* _args) { // 195
	AlifObject* r = nullptr;
	AlifObject* pathObj{};
	const wchar_t* path{};
	if (!alifArg_parseTuple(_args, "U", &pathObj)) {
		return nullptr;
	}
	path = alifUStr_asWideCharString(pathObj, nullptr);
	if (path) {
	#ifdef _WINDOWS
		DWORD attr = GetFileAttributesW(path);
		r = (attr != INVALID_FILE_ATTRIBUTES) and
			!(attr & FILE_ATTRIBUTE_DIRECTORY) ? ALIF_TRUE : ALIF_FALSE;
	#else
		struct stat st;
		r = (_alif_wStat(path, &st) == 0) and S_ISREG(st.st_mode) ? ALIF_TRUE : ALIF_FALSE;
	#endif
		alifMem_dataFree((void*)path);
	}
	return ALIF_XNEWREF(r);
}


static AlifObject* getPath_isXFile(AlifObject* ALIF_UNUSED(self), AlifObject* args) { // 220
	AlifObject* r = nullptr;
	AlifObject* pathobj{};
	const wchar_t* path{};
	AlifSizeT cchPath{};
	if (!alifArg_parseTuple(args, "U", &pathobj)) {
		return nullptr;
	}
	path = alifUStr_asWideCharString(pathobj, &cchPath);
	if (path) {
	#ifdef _WINDOWS
		DWORD attr = GetFileAttributesW(path);
		r = (attr != INVALID_FILE_ATTRIBUTES) &&
			!(attr & FILE_ATTRIBUTE_DIRECTORY) &&
			(cchPath >= 4) &&
			(CompareStringOrdinal(path + cchPath - 4, -1, L".exe", -1, 1 /* ignore case */) == CSTR_EQUAL)
			? ALIF_TRUE : ALIF_FALSE;
	#else
		struct stat st;
		r = (_alif_wStat(path, &st) == 0) and
			S_ISREG(st.st_mode) and
			(st.st_mode & 0111)
			? ALIF_TRUE : ALIF_FALSE;
	#endif
		alifMem_dataFree((void*)path);
	}
	return ALIF_XNEWREF(r);
}



static AlifObject* getPath_joinPath(AlifObject* ALIF_UNUSED(self), AlifObject* args) { // 252
	if (!ALIFTUPLE_CHECK(args)) {
		alifErr_setString(_alifExcTypeError_, "requires tuple of arguments");
		return nullptr;
	}
	AlifSizeT n = ALIFTUPLE_GET_SIZE(args);
	if (n == 0) {
		return alifUStr_fromStringAndSize(nullptr, 0);
	}
	/* Convert all parts to wchar and accumulate max final length */
	wchar_t** parts = (wchar_t**)alifMem_dataAlloc(n * sizeof(wchar_t*));
	if (parts == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	memset(parts, 0, n * sizeof(wchar_t*));
	AlifSizeT cchFinal = 0;
	AlifSizeT first = 0;

	for (AlifSizeT i = 0; i < n; ++i) {
		AlifObject* s = ALIFTUPLE_GET_ITEM(args, i);
		AlifSizeT cch;
		if (s == ALIF_NONE) {
			cch = 0;
		}
		else if (ALIFUSTR_CHECK(s)) {
			parts[i] = alifUStr_asWideCharString(s, &cch);
			if (!parts[i]) {
				cchFinal = -1;
				break;
			}
			if (_alif_isAbs(parts[i])) {
				first = i;
			}
		}
		else {
			alifErr_setString(_alifExcTypeError_, "all arguments to joinpath() must be str or None");
			cchFinal = -1;
			break;
		}
		cchFinal += cch + 1;
	}

	wchar_t* final = cchFinal > 0 ? (wchar_t*)alifMem_dataAlloc(cchFinal * sizeof(wchar_t)) : nullptr;
	if (!final) {
		for (AlifSizeT i = 0; i < n; ++i) {
			alifMem_dataFree(parts[i]);
		}
		alifMem_dataFree(parts);
		if (cchFinal) {
			//alifErr_noMemory();
			return nullptr;
		}
		return alifUStr_fromStringAndSize(nullptr, 0);
	}

	final[0] = '\0';
	/* Now join all the paths. The final result should be shorter than the buffer */
	for (AlifSizeT i = 0; i < n; ++i) {
		if (!parts[i]) {
			continue;
		}
		if (i >= first and final) {
			if (!final[0]) {
				/* final is definitely long enough to fit any individual part */
				wcscpy(final, parts[i]);
			}
			else if (_alif_addRelfile(final, parts[i], cchFinal) < 0) {
				/* if we fail, keep iterating to free memory, but stop adding parts */
				alifMem_dataFree(final);
				final = nullptr;
			}
		}
		alifMem_dataFree(parts[i]);
	}
	alifMem_dataFree(parts);
	if (!final) {
		//alifErr_setString(_alifExcSystemError_, "failed to join paths");
		return nullptr;
	}
	AlifObject* r = alifUStr_fromWideChar(_alif_normPath(final, -1), -1);
	alifMem_dataFree(final);
	return r;
}


static AlifObject* getPath_readLines(AlifObject* ALIF_UNUSED(self), AlifObject* _args) { // 337
	AlifObject* r = nullptr;
	AlifObject* pathObj{};
	const wchar_t* path{};
	if (!alifArg_parseTuple(_args, "U", &pathObj)) {
		return nullptr;
	}
	path = alifUStr_asWideCharString(pathObj, nullptr);
	if (!path) {
		return nullptr;
	}
	FILE* fp = _alif_wfOpen(path, L"rb");
	if (!fp) {
		//alifErr_setFromErrno(_alifExcOSError_);
		alifMem_dataFree((void*)path);
		return nullptr;
	}
	alifMem_dataFree((void*)path);

	r = alifList_new(0);
	if (!r) {
		fclose(fp);
		return nullptr;
	}
	const AlifUSizeT MAX_FILE = 32 * 1024;
	char* buffer = (char*)alifMem_dataAlloc(MAX_FILE);
	if (!buffer) {
		ALIF_DECREF(r);
		fclose(fp);
		return nullptr;
	}

	AlifUSizeT cb = fread(buffer, 1, MAX_FILE, fp);
	fclose(fp);
	if (!cb) {
		return r;
	}
	if (cb >= MAX_FILE) {
		ALIF_DECREF(r);
		//alifErr_setString(_alifExcMemoryError_,
		//	"cannot read file larger than 32KB during initialization");
		return nullptr;
	}
	buffer[cb] = '\0';

	AlifUSizeT len{};
	wchar_t* wbuffer = _alifDecodeUTF8_surrogateEscape(buffer, cb, &len);
	alifMem_dataFree((void*)buffer);
	if (!wbuffer) {
		ALIF_DECREF(r);
		//alifErr_noMemory();
		return nullptr;
	}

	wchar_t* p1 = wbuffer;
	wchar_t* p2 = p1;
	while ((p2 = wcschr(p1, L'\n')) != nullptr) {
		AlifSizeT cb = p2 - p1;
		while (cb >= 0 and (p1[cb] == L'\n' or p1[cb] == L'\r')) {
			--cb;
		}
		AlifObject* u = alifUStr_fromWideChar(p1, cb >= 0 ? cb + 1 : 0);
		if (!u or alifList_append(r, u) < 0) {
			ALIF_XDECREF(u);
			ALIF_CLEAR(r);
			break;
		}
		ALIF_DECREF(u);
		p1 = p2 + 1;
	}
	if (r and p1 and *p1) {
		AlifObject* u = alifUStr_fromWideChar(p1, -1);
		if (!u or alifList_append(r, u) < 0) {
			ALIF_CLEAR(r);
		}
		ALIF_XDECREF(u);
	}
	alifMem_dataFree(wbuffer);
	return r;
}

static AlifObject* getPath_realPath(AlifObject* ALIF_UNUSED(self), AlifObject* args) { // 421
	AlifObject* pathobj{};
	if (!alifArg_parseTuple(args, "U", &pathobj)) {
		return nullptr;
	}
#if defined(HAVE_READLINK)
	AlifObject* r = nullptr;
	AlifIntT nlink = 0;
	wchar_t* path = alifUStr_asWideCharString(pathobj, nullptr);
	if (!path) {
		goto done;
	}
	wchar_t* path2 = alifMem_wcsDup(path);
	alifMem_dataFree((void*)path);
	path = path2;
	while (path) {
		wchar_t resolved[MAXPATHLEN + 1];
		int linklen = _alif_wReadLink(path, resolved, ALIF_ARRAY_LENGTH(resolved));
		if (linklen == -1) {
			r = alifUStr_fromWideChar(path, -1);
			break;
		}
		if (_alif_isAbs(resolved)) {
			alifMem_dataFree((void*)path);
			path = alifMem_wcsDup(resolved);
		}
		else {
			wchar_t* s = wcsrchr(path, SEP);
			if (s) {
				*s = L'\0';
			}
			path2 = _alif_joinRelFile(path, resolved);
			if (path2) {
				path2 = _alif_normPath(path2, -1);
			}
			alifMem_dataFree((void*)path);
			path = path2;
		}
		nlink++;
		/* 40 is the Linux kernel 4.2 limit */
		if (nlink >= 40) {
			alifErr_setString(_alifExcOSError_, "maximum number of symbolic links reached");
			break;
		}
	}
	if (!path) {
		//alifErr_noMemory();
	}
done:
	alifMem_dataFree((void*)path);
	return r;

#elif defined(HAVE_REALPATH)
	AlifObject* r = nullptr;
	struct stat st {};
	const char* narrow = nullptr;
	wchar_t* path = PyUnicode_AsWideCharString(pathobj, nullptr);
	if (!path) {
		goto done;
	}
	narrow = alif_encodeLocale(path, nullptr);
	if (!narrow) {
		//alifErr_noMemory();
		goto done;
	}
	if (lstat(narrow, &st)) {
		//alifErr_setFromErrno(_alifExcOSError_);
		goto done;
	}
	if (!S_ISLNK(st.st_mode)) {
		r = ALIF_NEWREF(pathobj);
		goto done;
	}
	wchar_t resolved[MAXPATHLEN + 1];
	if (_alif_wRealPath(path, resolved, MAXPATHLEN) == nullptr) {
		//alifErr_setFromErrno(_alifExcOSError_);
	}
	else {
		r = alifUStr_fromWideChar(resolved, -1);
	}
done:
	alifMem_dataFree((void*)path);
	alifMem_dataFree((void*)narrow);
	return r;
#elif defined(_WINDOWS)
	HANDLE hFile;
	wchar_t resolved[MAXPATHLEN + 1];
	int len = 0, err;
	AlifSizeT pathlen{};
	AlifObject* result{};

	wchar_t* path = alifUStr_asWideCharString(pathobj, &pathlen);
	if (!path) {
		return nullptr;
	}
	if (wcslen(path) != pathlen) {
		alifErr_setString(_alifExcValueError_, "path contains embedded nulls");
		return nullptr;
	}

	ALIF_BEGIN_ALLOW_THREADS
		hFile = CreateFileW(path, 0, 0, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		len = GetFinalPathNameByHandleW(hFile, resolved, MAXPATHLEN, VOLUME_NAME_DOS);
		err = len ? 0 : GetLastError();
		CloseHandle(hFile);
	}
	else {
		err = GetLastError();
	}
	ALIF_END_ALLOW_THREADS

		if (err) {
			//alifErr_setFromWindowsErr(err);
			result = nullptr;
		}
		else if (len <= MAXPATHLEN) {
			const wchar_t* p = resolved;
			if (0 == wcsncmp(p, L"\\\\?\\", 4)) {
				if (GetFileAttributesW(&p[4]) != INVALID_FILE_ATTRIBUTES) {
					p += 4;
					len -= 4;
				}
			}
			if (CompareStringOrdinal(path, (int)pathlen, p, len, TRUE) == CSTR_EQUAL) {
				result = ALIF_NEWREF(pathobj);
			}
			else {
				result = alifUStr_fromWideChar(p, len);
			}
		}
		else {
			result = ALIF_NEWREF(pathobj);
		}
	alifMem_dataFree(path);
	return result;
#endif

	return ALIF_NEWREF(pathobj);
}

static AlifMethodDef _getPathMethods_[] = { // 563
	{"المسار_المطلق", getPath_absPath, METHOD_VARARGS/*, nullptr*/},
	{"اسم_الاساس", getPath_baseName, METHOD_VARARGS/*, nullptr*/},
	{"اسم_المجلد", getPath_dirName, METHOD_VARARGS/*, nullptr*/},
	{"يملك_بادئة", getPath_hasSuffix, METHOD_VARARGS/*, nullptr*/},
	{"هل_مطلق", getPath_isAbs, METHOD_VARARGS/*, nullptr*/},
	{"هل_مجلد", getPath_isDir, METHOD_VARARGS/*, nullptr*/},
	{"هل_ملف", getPath_isFile, METHOD_VARARGS/*, nullptr*/},
	{"هل_ملف_خاص", getPath_isXFile, METHOD_VARARGS/*, nullptr*/},
	{"ضم_المسار", getPath_joinPath, METHOD_VARARGS/*, nullptr*/},
	{"قراءة_الاسطر", getPath_readLines, METHOD_VARARGS/*, nullptr*/},
	{"مسار_حقيقي", getPath_realPath, METHOD_VARARGS/*, nullptr*/},
	{nullptr, nullptr, 0/*, nullptr*/}
};

static AlifObject* getPath_warn(AlifObject* ALIF_UNUSED(self), AlifObject* _args) { // 583
	AlifObject* msgobj{};
	if (!alifArg_parseTuple(_args, "U", &msgobj)) {
		return nullptr;
	}
	fprintf(stderr, "%s\n", alifUStr_asUTF8(msgobj));
	return ALIF_NONE;
}


static AlifObject* getPath_nowarn(AlifObject* ALIF_UNUSED(self), AlifObject* _args) {
	return ALIF_NONE;
}


static AlifMethodDef _getPathWarnMethod_ = { "warn", getPath_warn, METHOD_VARARGS/*, nullptr*/ };
static AlifMethodDef _getPathNowarnMethod_ = { "warn", getPath_nowarn, METHOD_VARARGS/*, nullptr*/ };

static AlifIntT funcs_toDict(AlifObject* dict, AlifIntT warnings) { // 605
	for (AlifMethodDef* m = _getPathMethods_; m->name; ++m) {
		AlifObject* f = ALIFCPPFUNCTION_NEWEX(m, nullptr, nullptr);
		if (!f) {
			return 0;
		}
		if (alifDict_setItemString(dict, m->name, f) < 0) {
			ALIF_DECREF(f);
			return 0;
		}
		ALIF_DECREF(f);
	}
	AlifMethodDef* m2 = warnings ? &_getPathWarnMethod_ : &_getPathNowarnMethod_;
	AlifObject* f = ALIFCPPFUNCTION_NEWEX(m2, nullptr, nullptr);
	if (!f) {
		return 0;
	}
	if (alifDict_setItemString(dict, m2->name, f) < 0) {
		ALIF_DECREF(f);
		return 0;
	}
	ALIF_DECREF(f);
	return 1;
}


static AlifIntT wchar_toDict(AlifObject* _dict, const char* _key, const wchar_t* _s) { // 634
	AlifObject* u{};
	AlifIntT r{};
	if (_s and _s[0]) {
		u = alifUStr_fromWideChar(_s, -1);
		if (!u) {
			return 0;
		}
	}
	else {
		u = ALIF_NEWREF(ALIF_NONE);
	}
	r = alifDict_setItemString(_dict, _key, u) == 0;
	ALIF_DECREF(u);
	return r;
}

static AlifIntT decode_toDict(AlifObject* _dict, const char* _key, const char* _s) { // 654
	AlifObject* u = nullptr;
	AlifIntT r{};
	if (_s and _s[0]) {
		AlifUSizeT len{};
		const wchar_t* w = alif_decodeLocale(_s, &len);
		if (w) {
			u = alifUStr_fromWideChar(w, len);
			alifMem_dataFree((void*)w);
		}
		if (!u) {
			return 0;
		}
	}
	else {
		u = ALIF_NEWREF(ALIF_NONE);
	}
	r = alifDict_setItemString(_dict, _key, u) == 0;
	ALIF_DECREF(u);
	return r;
}



static AlifIntT env_toDict(AlifObject* dict,
	const char* key, AlifIntT andClear) { // 678
	AlifObject* u = nullptr;
	AlifIntT r = 0;
#ifdef _WINDOWS
	wchar_t wkey[64];
	// Quick convert to wchar_t, since we know key is ASCII
	wchar_t* wp = wkey;
	for (const char* p = &key[4]; *p; ++p) {
		*wp++ = *p;
	}
	*wp = L'\0';
	const wchar_t* v = _wgetenv(wkey);
	if (v) {
		u = alifUStr_fromWideChar(v, -1);
		if (!u) {
			alifErr_clear();
		}
	}
#else
	const char* v = getenv(&key[4]);
	if (v) {
		AlifUSizeT len{};
		const wchar_t* w = alif_decodeLocale(v, &len);
		if (w) {
			u = alifUStr_fromWideChar(w, len);
			if (!u) {
				alifErr_clear();
			}
			alifMem_dataFree((void*)w);
		}
	}
#endif
	if (u) {
		r = alifDict_setItemString(dict, key, u) == 0;
		ALIF_DECREF(u);
	}
	else {
		r = alifDict_setItemString(dict, key, ALIF_NONE) == 0;
	}
	if (r and andClear) {
	#ifdef _WINDOWS
		_wputenv_s(wkey, L"");
	#else
		unsetenv(&key[4]);
	#endif
	}
	return r;
}


static AlifIntT int_toDict(AlifObject* _dict, const char* _key, AlifIntT _v) { // 733
	AlifObject* o{};
	AlifIntT r{};
	o = alifLong_fromLong(_v);
	if (!o) {
		return 0;
	}
	r = alifDict_setItemString(_dict, _key, o) == 0;
	ALIF_DECREF(o);
	return r;
}

#ifdef _WINDOWS
static AlifIntT winModule_toDict(AlifObject* _dict, const char* _key, HMODULE _mod) { // 748
	wchar_t* buffer = nullptr;
	for (DWORD cch = 256; buffer == nullptr and cch < (1024 * 1024); cch *= 2) {
		buffer = (wchar_t*)alifMem_dataAlloc(cch * sizeof(wchar_t));
		if (buffer) {
			if (GetModuleFileNameW(_mod, buffer, cch) == cch) {
				alifMem_dataFree(buffer);
				buffer = nullptr;
			}
		}
	}
	AlifIntT r = wchar_toDict(_dict, _key, buffer);
	alifMem_dataFree(buffer);
	return r;
}
#endif

static AlifIntT progname_toDict(AlifObject* _dict, const char* _key) { // 770
#ifdef _WINDOWS
	return winModule_toDict(_dict, _key, nullptr);
#elif defined(__APPLE__)
	char* path;
	uint32_t pathLen = 256;
	while (pathLen) {
		path = (char*)alifMem_dataAlloc((pathLen + 1) * sizeof(char));
		if (!path) {
			return 0;
		}
		if (_NSGetExecutablePath(path, &pathLen) != 0) {
			alifMem_dataFree(path);
			continue;
		}
		// Only keep if the path is absolute
		if (path[0] == SEP) {
			AlifIntT r = decode_toDict(_dict, _key, path);
			alifMem_dataFree(path);
			return r;
		}
		// Fall back and store None
		alifMem_dataFree(path);
		break;
	}
#endif
	return alifDict_setItemString(_dict, _key, ALIF_NONE) == 0;
}

static AlifIntT library_toDict(AlifObject* _dict, const char* _key) { // 803
#ifdef _WINDOWS
#ifdef ALIF_ENABLE_SHARED
	extern HMODULE _alifWinDLLhModule_;
	if (_alifWinDLLhModule_) {
		return winmodule_toDict(_dict, _key, _alifWinDLLhModule_);
	}
#endif
#elif defined(WITH_NEXT_FRAMEWORK)
	static char modPath[MAXPATHLEN + 1];
	static int modPathInitialized = -1;
	if (modPathInitialized < 0) {
		modPathInitialized = 0;

		Dl_info alifInfo;
		if (dladdr(&alif_initialize, &alifInfo)) {
			if (alifInfo.dli_fname) {
				strncpy(modPath, alifInfo.dli_fname, MAXPATHLEN);
				modPathInitialized = 1;
			}
		}
	}
	if (modPathInitialized > 0) {
		return decode_toDict(_dict, _key, modPath);
	}
#endif
	return alifDict_setItemString(_dict, _key, ALIF_NONE) == 0;
}

AlifObject* _alifGet_getPathCodeObject(void) { // 841
	return alifMarshal_readObjectFromString(
		(const char*)_ALIF_M__getpath, sizeof(_ALIF_M__getpath));
}

AlifStatus _alifConfig_initPathConfig(AlifConfig* _config, AlifIntT _computePathConfig) { // 863
	AlifStatus status = _alifPathConfig_readGlobal(_config);

	if (ALIFSTATUS_EXCEPTION(status) or !_computePathConfig) {
		return status;
	}

	/* -------------- alif path config -------------- */
	status = alifConfig_initPathConfigAlif(_config, _computePathConfig); //* alif
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	return ALIFSTATUS_OK();
	/* ------------- !alif path config! ------------- */

	if (!_alifThread_get()) {
		return alifStatus_error("لا يمكن إيجاد المسارات");
	}

	AlifObject* configDict = _alifConfig_asDict(_config);
	if (!configDict) {
		alifErr_clear();
		return alifStatus_noMemory();
	}

	AlifObject* dict = alifDict_new();
	if (!dict) {
		alifErr_clear();
		ALIF_DECREF(configDict);
		return alifStatus_noMemory();
	}

	if (alifDict_setItemString(dict, "التكوين", configDict) < 0) {
		alifErr_clear();
		ALIF_DECREF(configDict);
		ALIF_DECREF(dict);
		return alifStatus_noMemory();
	}
	/* reference now held by dict */
	ALIF_DECREF(configDict);

	AlifObject* co = _alifGet_getPathCodeObject();
	if (!co or !ALIFCODE_CHECK(co)) {
		alifErr_clear();
		ALIF_XDECREF(co);
		ALIF_DECREF(dict);
		return alifStatus_error("error reading frozen getpath.alif");
	}

#ifdef _WINDOWS
	//AlifObject* winreg = alifImport_importModule("winreg");
	//if (!winreg or alifDict_setItemString(dict, "winreg", winreg) < 0) {
	//	alifErr_clear();
	//	ALIF_XDECREF(winreg);
	//	if (alifDict_setItemString(dict, "winreg", ALIF_NONE) < 0) {
	//		alifErr_clear();
	//		ALIF_DECREF(co);
	//		ALIF_DECREF(dict);
	//		return alifStatus_error("error importing winreg module");
	//	}
	//}
	//else {
	//	ALIF_DECREF(winreg);
	//}
#endif

	if (
	#ifdef _WINDOWS
		!decode_toDict(dict, "os_name", "nt") or
	#elif defined(__APPLE__)
		!decode_toDict(dict, "os_name", "darwin") or
	#else
		!decode_toDict(dict, "os_name", "posix") or
	#endif
	#ifdef WITH_NEXT_FRAMEWORK
		!int_toDict(dict, "WITH_NEXT_FRAMEWORK", 1) or
	#else
		!int_toDict(dict, "WITH_NEXT_FRAMEWORK", 0) or
	#endif
		!decode_toDict(dict, "PREFIX", PREFIX) or
		!decode_toDict(dict, "EXEC_PREFIX", EXEC_PREFIX) or
		!decode_toDict(dict, "ALIFPATH", ALIFPATH) or
		!decode_toDict(dict, "VPATH", VPATH) or
		!decode_toDict(dict, "PLATLIBDIR", PLATLIBDIR) or
		!decode_toDict(dict, "ALIFDEBUGEXT", ALIFDEBUGEXT) or
		!int_toDict(dict, "VERSION_MAJOR", ALIF_MAJOR_VERSION) or
		!int_toDict(dict, "VERSION_MINOR", ALIF_MINOR_VERSION) or
		!decode_toDict(dict, "ALIFWINVER", ALIFWINVER) or
		!wchar_toDict(dict, "EXE_SUFFIX", EXE_SUFFIX) or
		!env_toDict(dict, "ENV_PATH", 0) or
		!env_toDict(dict, "ENV_ALIFHOME", 0) or
		!env_toDict(dict, "ENV_ALIFEXECUTABLE", 0) or
		!env_toDict(dict, "ENV___ALIFVENV_LAUNCHER__", 1) or
		!progname_toDict(dict, "real_executable") or
		!library_toDict(dict, "library") or
		!wchar_toDict(dict, "executable_dir", nullptr) or
		!wchar_toDict(dict, "alif_setpath", _alifPathConfig_getGlobalModuleSearchPath()) or
		!funcs_toDict(dict, _config->pathConfigWarnings) or

		!decode_toDict(dict, "ABI_THREAD", "t") or

	#ifndef _WINDOWS
		alifDict_setItemString(dict, "winreg", ALIF_NONE) < 0 or
	#endif
		alifDict_setItemString(dict, "__builtins__", alifEval_getBuiltins()) < 0
		) {
		ALIF_DECREF(co);
		ALIF_DECREF(dict);
		//alifErr_formatUnraisable("Exception ignored in preparing getpath");
		return alifStatus_error("error evaluating initial values");
	}

	AlifObject* r = alifEval_evalCode(co, dict, dict);
	ALIF_DECREF(co);

	if (!r) {
		ALIF_DECREF(dict);
		//alifErr_formatUnraisable("Exception ignored in running getpath");
		return alifStatus_error("error evaluating path");
	}
	ALIF_DECREF(r);

	if (_alifConfig_fromDict(_config, configDict) < 0) {
		//alifErr_formatUnraisable("Exception ignored in reading getpath results");
		ALIF_DECREF(dict);
		return alifStatus_error("error getting getpath results");
	}

	ALIF_DECREF(dict);

	return ALIFSTATUS_OK();
}













/* ----------------------------------------------- alif path config ----------------------------------------------- */

#define DEFAULT_PROGRAM_NAME L"alif"
#define STDLIB_SUBDIR L"library"
#define PREFIX nullptr
#define EXEC_PREFIX nullptr
#define VPATH L"..\\.."
#define VERSION_MAJOR ALIF_MAJOR_VERSION


#ifdef _WINDOWS
#define STDLIB_LANDMARKS L"library\\نظام_التشغيل.aliflib"
#define BUILDSTDLIB_LANDMARKS L"library\\نظام_التشغيل.aliflib"
#define PLATLIBDIR L"DLLs"
#define DELIM L";"
#else
#define STDLIB_LANDMARKS L"library/نظام_التشغيل.aliflib"
#define BUILDSTDLIB_LANDMARKS L"library/نظام_التشغيل.aliflib"
#define PLATLIBDIR L"lib"
#define DELIM L":"
#endif



static wchar_t* abs_path(wchar_t* _path) { // 60
	wchar_t* r{};

	AlifSizeT len{};
	len = wcslen(_path);
	if (_path) {
		wchar_t* abs{};
		if (_alif_absPath((const wchar_t*)_alif_normPath(_path, -1), &abs) == 0 and abs) {
			return abs;
		}
		else {
			//alifErr_setString(_alifExcOSError_, "failed to make path absolute");
			return nullptr;
		}
	}
	return nullptr;
}

static wchar_t* dir_name(wchar_t* _path) { // 101
	AlifSizeT end = wcslen(_path);
	wchar_t* charPos = wcsrchr(_path, SEP);
	AlifIntT pos = (AlifIntT)(charPos - _path);
	if (pos < 0) {
		return (wchar_t*)alifMem_dataAlloc(sizeof(wchar_t));
	}

	wchar_t* res = (wchar_t*)alifMem_dataAlloc((pos + 1) * sizeof(wchar_t));
	wcsncpy(res, _path, pos);
	return res;
}


static bool has_suffix(wchar_t* _path, const wchar_t* _suffix) { // 135
	bool r{};
	const wchar_t* path = _path;
	const wchar_t* suffix = _suffix;

	AlifSizeT len{}, suffixLen{};
	if (path) {
		len = wcslen(_path);
		if (suffix) {
			suffixLen = wcslen(_suffix);
			if (suffixLen > len or
			#ifdef _WINDOWS
				wcsicmp(&path[len - suffixLen], suffix) != 0
			#else
				wcscmp(&path[len - suffixLen], suffix) != 0
			#endif
				) {
				r = false;
			}
			else {
				r = true;
			}
		}
	}
	return r;
}

static bool is_file(wchar_t* _path) { // 195
	bool r{};
	const wchar_t* path = _path;
	if (path) {
	#ifdef _WINDOWS
		DWORD attr = GetFileAttributesW(path);
		r = (attr != INVALID_FILE_ATTRIBUTES) and
			!(attr & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
	#else
		struct stat st;
		r = (_alif_wStat(path, &st) == 0) and S_ISREG(st.st_mode) ? true : false;
	#endif
	}
	return r;
}

static bool is_xFile(wchar_t* _path) { // 220
	bool r{};
	const wchar_t* path = _path;
	AlifSizeT cchPath{};
	if (path) {
		cchPath = wcslen(_path);
	#ifdef _WINDOWS
		DWORD attr = GetFileAttributesW(path);
		r = (attr != INVALID_FILE_ATTRIBUTES) and
			!(attr & FILE_ATTRIBUTE_DIRECTORY) and
			(cchPath >= 4) and
			(CompareStringOrdinal(path + cchPath - 4, -1, L".exe", -1, 1 /* ignore case */) == CSTR_EQUAL)
			? true : false;
	#else
		struct stat st;
		r = (_alif_wStat(path, &st) == 0) and
			S_ISREG(st.st_mode) and
			(st.st_mode & 0111)
			? true : false;
	#endif
	}
	return r;
}


static wchar_t* join_paths(AlifIntT n, ...) { // 252
	/* Convert all parts to wchar and accumulate max final length */
	wchar_t** parts = (wchar_t**)alifMem_dataAlloc(n * sizeof(wchar_t*));
	if (parts == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	AlifSizeT cchFinal = 0;
	AlifSizeT first = 0;

	va_list args;
	va_start(args, n);

	for (AlifSizeT i = 0; i < n; ++i) {
		wchar_t* s = va_arg(args, wchar_t*);
		AlifSizeT cch{};
		if (s == nullptr) {
			cch = 0;
		}
		cch = wcslen(s);
		parts[i] = s;
		if (!parts[i]) {
			cchFinal = -1;
			break;
		}
		if (_alif_isAbs(parts[i])) {
			first = i;
		}
		cchFinal += cch + 1;
	}

	wchar_t* final = cchFinal > 0 ? (wchar_t*)alifMem_dataAlloc(cchFinal * sizeof(wchar_t)) : nullptr;
	if (!final) {
		//alifErr_noMemory();
		return nullptr;
	}

	final[0] = '\0';
	/* Now join all the paths. The final result should be shorter than the buffer */
	for (AlifSizeT i = 0; i < n; ++i) {
		if (!parts[i]) {
			continue;
		}
		if (i >= first and final) {
			if (!final[0]) {
				/* final is definitely long enough to fit any individual part */
				wcscpy(final, parts[i]);
			}
			else if (_alif_addRelfile(final, parts[i], cchFinal) < 0) {
				/* if we fail, keep iterating to free memory, but stop adding parts */
				alifMem_dataFree(final);
				final = nullptr;
			}
		}
	}
	alifMem_dataFree(parts);
	if (!final) {
		//alifErr_setString(_alifExcSystemError_, "failed to join paths");
		return nullptr;
	}
	final = alifMem_wcsDup(_alif_normPath(final, -1));
	return final;
}


static wchar_t* real_path(wchar_t* _path) { // 421
	wchar_t* path = _path;
#if defined(HAVE_READLINK)
	int nlink = 0;
	if (!path) {
		goto done;
	}
	wchar_t* path2 = alifMem_wcsDup(path);
	alifMem_dataFree((void*)path);
	path = path2;
	while (path) {
		wchar_t resolved[MAXPATHLEN + 1];
		AlifIntT linklen = _alif_wReadLink(path, resolved, ALIF_ARRAY_LENGTH(resolved));
		if (linklen == -1) {
			break;
		}
		if (_alif_isAbs(resolved)) {
			alifMem_dataFree((void*)path);
			path = alifMem_wcsDup(resolved);
		}
		else {
			wchar_t* s = wcsrchr(path, SEP);
			if (s) {
				*s = L'\0';
			}
			path2 = _alif_joinRelFile(path, resolved);
			if (path2) {
				path2 = _alif_normPath(path2, -1);
			}
			alifMem_dataFree((void*)path);
			path = path2;
		}
		nlink++;
		/* 40 is the Linux kernel 4.2 limit */
		if (nlink >= 40) {
			//alifErr_setString(_alifExcOSError_, "maximum number of symbolic links reached");
			break;
		}
	}
	if (!path) {
		//alifErr_noMemory();
		return nullptr;
	}
done:
	return path;

#elif defined(HAVE_REALPATH)
	struct stat st;
	const char* narrow = nullptr;
	if (!path) {
		goto done;
	}
	narrow = alif_encodeLocale(path, nullptr);
	if (!narrow) {
		//alifErr_noMemory();
		goto done;
	}
	if (lstat(narrow, &st)) {
		//alifErr_setFromErrno(_alifExcOSError_);
		goto done;
	}
	if (!S_ISLNK(st.st_mode)) {
		goto done;
	}
	wchar_t resolved[MAXPATHLEN + 1];
	if (_alif_wRealPath(path, resolved, MAXPATHLEN) == nullptr) {
		//alifErr_setFromErrno(_alifExcOSError_);
	}
	else {
		path = resolved;
	}
done:
	alifMem_dataFree((void*)narrow);
	return path;
#elif defined(_WINDOWS)
	HANDLE hFile;
	wchar_t resolved[MAXPATHLEN + 1];
	int len = 0, err;
	AlifSizeT pathlen{};

	pathlen = wcslen(path);
	if (!path) {
		return nullptr;
	}
	if (wcslen(path) != pathlen) {
		//alifErr_setString(_alifExcValueError_, "path contains embedded nulls");
		return nullptr;
	}

	ALIF_BEGIN_ALLOW_THREADS
		hFile = CreateFileW(path, 0, 0, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		len = GetFinalPathNameByHandleW(hFile, resolved, MAXPATHLEN, VOLUME_NAME_DOS);
		err = len ? 0 : GetLastError();
		CloseHandle(hFile);
	}
	else {
		err = GetLastError();
	}
	ALIF_END_ALLOW_THREADS

		if (err) {
			//alifErr_setFromWindowsErr(err);
			path = nullptr;
		}
		else if (len <= MAXPATHLEN) {
			const wchar_t* p = resolved;
			if (0 == wcsncmp(p, L"\\\\?\\", 4)) {
				if (GetFileAttributesW(&p[4]) != INVALID_FILE_ATTRIBUTES) {
					p += 4;
					len -= 4;
				}
			}
			if (CompareStringOrdinal(path, (int)pathlen, p, len, TRUE) == CSTR_EQUAL) {
				return path;
			}
			else {
				path = wcsdup(p);
			}
		}
#endif

	return path;
}


static wchar_t* alif_env(const char* _env, AlifIntT _andClear) { // 678
	wchar_t* buffer{};
#ifdef _WINDOWS
	wchar_t wkey[64]{};
	// Quick convert to wchar_t, since we know key is ASCII
	wchar_t* wp = wkey;
	for (const char* p = &_env[4]; *p; ++p) {
		*wp++ = *p;
	}
	*wp = L'\0';
	const wchar_t* v = _wgetenv(wkey);
	if (v) {
		buffer = alifMem_wcsDup(v);
	}
#else
	const char* v = getenv(&_env[4]);
	if (v) {
		AlifUSizeT len{};
		const wchar_t* w = alif_decodeLocale(v, &len);
		if (w) {
			buffer = alifMem_wcsDup(w);
		}
	}
#endif
	if (buffer and _andClear) {
	#ifdef _WINDOWS
		_wputenv_s(wkey, L"");
	#else
		unsetenv(&_env[4]);
	#endif
	}
	return buffer;
}


static wchar_t* prog_name() { // 770
#ifdef _WINDOWS
	wchar_t* buffer = nullptr;
	for (DWORD cch = 256; buffer == nullptr and cch < (1024 * 1024); cch *= 2) {
		buffer = (wchar_t*)alifMem_dataAlloc(cch * sizeof(wchar_t));
		if (buffer) {
			if (GetModuleFileNameW(nullptr, buffer, cch) == cch) {
				alifMem_dataFree(buffer);
				buffer = nullptr;
			}
		}
	}
	return buffer;

#elif defined(__APPLE__)
	char* path{};
	uint32_t pathLen = 256;
	while (pathLen) {
		path = (char*)alifMem_dataAlloc((pathLen + 1) * sizeof(char));
		if (!path) {
			return nullptr;
		}
		if (_NSGetExecutablePath(path, &pathLen) != 0) {
			alifMem_dataFree(path);
			continue;
		}
		// Only keep if the path is absolute
		if (path[0] == SEP) {
			if (path and path[0]) {
				AlifUSizeT len{};
				wchar_t* w = alif_decodeLocale(path, &len);
				if (w != nullptr) {
					alifMem_dataFree(path);
					return w;
				}
			}
		}
		// Fall back and store None
		alifMem_dataFree(path);
		break;
	}
#endif

	return nullptr;
}


static AlifIntT is_sep(wchar_t ch) {
#ifdef ALTSEP
	return ch == SEP || ch == ALTSEP;
#else
	return ch == SEP;
#endif
}
static void reduce(wchar_t* dir) {
	AlifUSizeT i = wcsnlen(dir, MAXPATHLEN + 1);
	if (i >= MAXPATHLEN + 1) {
		exit(-12);
	}

	while (i > 0 and !is_sep(dir[i]))
		--i;
	dir[i] = '\0';
}
static AlifIntT got_landmark(const wchar_t* prefix, const wchar_t* landmark) {
	wchar_t filename[MAXPATHLEN + 1]{};
	wcscpy(filename, prefix);
	wchar_t* fileName = join_paths(2, filename, landmark);
	return is_file(fileName);
}
static wchar_t* search_up(wchar_t* prefix, const wchar_t* landmark) {
	do {
		if (got_landmark(prefix, landmark)) {
			return prefix;
		}
		reduce(prefix);
	}
	while (prefix[0]);
	return nullptr;
}


static wchar_t* add_string(const wchar_t* str1, const wchar_t* str2) {
	AlifUSizeT len1 = (str1 != nullptr) ? wcslen(str1) : 0;
	AlifUSizeT len2 = (str2 != nullptr) ? wcslen(str2) : 0;

	if (len1 and len2) {
		wchar_t* res = (wchar_t*)alifMem_dataAlloc((len1 + len2 + 1) * sizeof(wchar_t));
		if (res == nullptr) {
			return nullptr;
		}

		wcscpy(res, str1);
		wcscpy(res + len1, str2);

		return res;
	}

	return nullptr;
}


static AlifStatus alifConfig_initPathConfigAlif(AlifConfig* _config, AlifIntT _computePathConfig) {
	//* todo

	wchar_t* programName = _config->programName;
	wchar_t* executable = _config->executable;
	wchar_t* home = _config->home;
	wchar_t* baseExecutable = _config->baseExecutable;
	wchar_t* stdLibDir = _config->stdLibDir;
	wchar_t* prefix = _config->prefix;
	wchar_t* execPrefix = _config->execPrefix;
	wchar_t* platLibDir = _config->platLibDir;
	wchar_t* library = nullptr;
	wchar_t* platStdLibDir = nullptr;

	bool strLibDirWasSetInConfig = (_config->stdLibDir != nullptr);

	const wchar_t* alifSetPath = _alifPathConfig_getGlobalModuleSearchPath();
	wchar_t* realExecutable = prog_name();
	wchar_t* envPath = alif_env("ENV_PATH", 0);


	// CALCULATE program name
	if (not programName) {
		programName = _config->origArgv.items[0];
	}
	if (not programName) {
		programName = (wchar_t*)alifMem_dataAlloc(sizeof(DEFAULT_PROGRAM_NAME));
		wcscpy(programName, DEFAULT_PROGRAM_NAME);
	}

	if (not programName) {
		programName = nullptr;
	}

	if (EXE_SUFFIX and !has_suffix(programName, EXE_SUFFIX)) {
		wchar_t* programNameSuffix = add_string(programName, EXE_SUFFIX);
		if (is_xFile(programNameSuffix)) {
			programName = programNameSuffix;
		}
		alifMem_dataFree(programNameSuffix);
	}


	// CALCULATE executable
	wchar_t* realExecutableDir{};
	wchar_t* executableDir{};
	if (alifSetPath) {
		if (not executable) {
			executable = realExecutable;
		}
	}
	if (not executable and wcschr(programName, SEP)) {
		executable = abs_path(programName);
	}
	if (not executable) {
		executable = realExecutable;
	}

#if defined(__APPLE__)
	realExecutable = executable;
#endif

	if (not executable and programName and envPath) {
		wchar_t* context{};
		for (wchar_t* p = wcstok(envPath, DELIM, &context);
			p != nullptr;
			p = wcstok(nullptr, DELIM, &context)) {
			p = join_paths(2, p, programName);
			if (is_xFile(p)) {
				executable = p;
				break;
			}
			continue;
		}
	}
	if (not executable) {
		executable = (wchar_t*)alifMem_dataAlloc(sizeof(wchar_t));

		wchar_t* dot = wcsdup(L".");
		executableDir = abs_path(dot);
		realExecutableDir = alifMem_wcsDup(executableDir);
	}


	// CALCULATE baseExecutable, realExecutable AND executableDir
	if (not baseExecutable) {
		if (executable) {
			baseExecutable = executable;
		}
		else if (realExecutable) {
			baseExecutable = realExecutable;
		}
		else {
			baseExecutable = (wchar_t*)alifMem_dataAlloc(sizeof(wchar_t));
		}
	}

	if (not realExecutable) {
		realExecutable = baseExecutable;
	}

	if (realExecutable) {
		try {
			realExecutable = real_path(realExecutable);
		}
		catch (const std::exception&) {
			if (is_file(realExecutable)) {
				printf("فشلت عملية إيجاد مسار حقيقي لـ %s \n", baseExecutable);
			}
		}
	}

#if defined(__APPLE__)
	if (not executableDir and library) {
		wchar_t* libraryDir = dir_name(library);
		wchar_t* path = join_paths(2, libraryDir, STDLIB_LANDMARKS);
		if (is_file(path)) {
			executableDir = real_path(libraryDir);
			realExecutableDir = executableDir;
		}
	}
#endif


	// CHECK FOR BUILD DIRECTORY
	if (not executableDir and realExecutable) {
		executableDir = realExecutableDir = dir_name(realExecutable);
	}

	if (not realExecutableDir and realExecutable) {
		realExecutableDir = dir_name(realExecutable);
	}

	wchar_t* buildPrefix{};
	if ((realExecutableDir and not alifSetPath) or _config->isAlifBuild == 0) {
		platStdLibDir = realExecutableDir;
		//buildPrefix = join_paths(2, realExecutableDir, VPATH); // هنا يتم تحديد المسار الأساسي بالرجوع مجلدين الى الخلف وهذا يسبب عدم إكتشاف لمجلد libraray في حال كان مضمن في نفس مجلد ملف اللغة
		buildPrefix = realExecutableDir;

		wchar_t* buildStdlibPrefix{};
		if (buildPrefix) {
			//#ifdef _WINDOWS
				//buildStdlibPrefix = buildPrefix;
			//#else
			buildStdlibPrefix = search_up(buildPrefix, BUILDSTDLIB_LANDMARKS);
			//#endif

			if (not strLibDirWasSetInConfig) {
				if (buildStdlibPrefix) {
					stdLibDir = join_paths(2, buildStdlibPrefix, L"library");
				}
				else {
					stdLibDir = join_paths(2, buildPrefix, L"library");
				}
			}

			if (not prefix) {
				prefix = buildStdlibPrefix;
			}

			if (not execPrefix) {
				execPrefix = buildPrefix;
			}

			_config->isAlifBuild = 1;
		}
	}


	// CALCULATE prefix AND execPrefix
	if (alifSetPath) {
		prefix = execPrefix = (wchar_t*)alifMem_dataAlloc(sizeof(wchar_t));
	}
	else {
		if (home) {
			const wchar_t* delimPtr = wcsstr(home, DELIM);
			if (delimPtr == nullptr) {
				// Delimiter not found
				prefix = (wchar_t*)alifMem_dataAlloc((wcslen(home) + 1) * sizeof(wchar_t));
				prefix = home;
				execPrefix = prefix;
			}
			else {
				// Delimiter found
				AlifUSizeT prefixLen = delimPtr - home;
				AlifUSizeT delimLen = 1;
				AlifUSizeT suffixLen = wcslen(delimPtr + delimLen);

				prefix = (wchar_t*)alifMem_dataAlloc(prefixLen + 1);
				if (prefix == nullptr) {
					perror("Memory allocation failed");
					exit(EXIT_FAILURE);
				}
				wcsncpy(prefix, home, prefixLen);

				execPrefix = (wchar_t*)alifMem_dataAlloc(suffixLen + 1);
				if (execPrefix == nullptr) {
					perror("Memory allocation failed");
					alifMem_dataFree(prefix);
					exit(EXIT_FAILURE);
				}
				wcscpy(execPrefix, delimPtr + delimLen);
			}
		}



		if (STDLIB_SUBDIR and STDLIB_LANDMARKS and executableDir and not prefix) {
			prefix = search_up(wcsdup(executableDir), STDLIB_LANDMARKS);
			if (prefix and not stdLibDir) {
				stdLibDir = join_paths(2, prefix, STDLIB_SUBDIR);
			}
		}


		if (not prefix) {
			prefix = abs_path((wchar_t*)L"");
		}
	}


	// UPDATE sys.path
	_config->moduleSearchPaths.items = (wchar_t**)alifMem_dataAlloc(sizeof(wchar_t*) * 2);
	_config->moduleSearchPaths.items[0] = stdLibDir;
	_config->moduleSearchPaths.items[1] = executableDir;
	_config->moduleSearchPaths.length = 2;
	_config->moduleSearchPathsSet = 1;


	// UPDATE config FROM CALCULATED VALUES
	_config->programName = programName;
	_config->home = home;
	_config->executable = executable;
	_config->baseExecutable = baseExecutable;
	_config->prefix = prefix;
	_config->execPrefix = execPrefix;
	_config->basePrefix = prefix;
	_config->baseExecPrefix = execPrefix;

	_config->platLibDir = platLibDir;
	_config->stdLibDir = stdLibDir;

	return ALIFSTATUS_OK();
}
