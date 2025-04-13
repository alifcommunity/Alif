#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_PathConfig.h"
#include "AlifCore_Memory.h"
#include "AlifCore_State.h"

#include "osdefs.h"

#ifdef _WINDOWS
	#include <windows.h>
	#include <pathcch.h>
#endif

#ifdef __APPLE__
#  include <dlfcn.h>
#  include <mach-o/dyld.h>
#endif



#if !defined(EXE_SUFFIX)
#if defined(_WINDOWS) or defined(__CYGWIN__) or defined(__MINGW32__)
#define EXE_SUFFIX L".exe"
#else
#define EXE_SUFFIX nullptr
#endif
#endif




static AlifIntT alifConfig_initPathConfigAlif(AlifConfig*, AlifIntT); //* alif




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



//static AlifIntT progname_toDict(AlifObject* dict, const char* key) { // 770
//#ifdef _WINDOWS
//	return winModule_toDict(dict, key, nullptr);
//#elif defined(__APPLE__)
//	char* path;
//	uint32_t pathLen = 256;
//	while (pathLen) {
//		path = (char*)alifMem_dataAlloc((pathLen + 1) * sizeof(char));
//		if (!path) {
//			return 0;
//		}
//		if (_NSGetExecutablePath(path, &pathLen) != 0) {
//			alifMem_dataFree(path);
//			continue;
//		}
//		// Only keep if the path is absolute
//		if (path[0] == SEP) {
//			AlifintT r = decode_toDict(dict, key, path);
//			alifMem_dataFree(path);
//			return r;
//		}
//		// Fall back and store None
//		alifMem_dataFree(path);
//		break;
//	}
//#endif
//	return alifDict_setItemString(dict, key, ALIF_NONE) == 0;
//}




AlifIntT _alifConfig_initPathConfig(AlifConfig* _config, AlifIntT _computePathConfig) { // 863
	AlifIntT status = _alifPathConfig_readGlobal(_config);

	if (status != 1 or !_computePathConfig) {
		return status;
	}

	/* -------------- alif path config -------------- */
	alifConfig_initPathConfigAlif(_config, _computePathConfig);	//* alif
	/* ------------- !alif path config! ------------- */

//	if (!_alifThread_get()) {
//		//return alifStatus_error("cannot calculate path configuration");
//	}
//
//	AlifObject* configDict = _alifConfig_asDict(_config);
//	if (!configDict) {
//		//alifErr_clear();
//		//return alifStatus_noMemory();
//		return -1; //* alif
//	}
//
//	AlifObject* dict = alifDict_new();
//	if (!dict) {
//		alifErr_clear();
//		ALIF_DECREF(configDict);
//		//return alifStatus_noMemory();
//	}
//
//	if (alifDict_setItemString(dict, "config", configDict) < 0) {
//		alifErr_clear();
//		ALIF_DECREF(configDict);
//		ALIF_DECREF(dict);
//		return alifStatus_noMemory();
//	}
//	/* reference now held by dict */
//	ALIF_DECREF(configDict);
//
//	AlifObject* co = _alifGet_getPathCodeObject();
//	if (!co or !ALIFCODE_CHECK(co)) {
//		alifErr_clear();
//		ALIF_XDECREF(co);
//		ALIF_DECREF(dict);
//		return alifStatus_error("error reading frozen getpath.alif");
//	}
//
//#ifdef _WINDOWS
//	AlifObject* winreg = alifImport_importModule("winreg");
//	if (!winreg or alifDict_setItemString(dict, "winreg", winreg) < 0) {
//		alifErr_clear();
//		ALIF_XDECREF(winreg);
//		if (alifDict_setItemString(dict, "winreg", ALIF_NONE) < 0) {
//			alifErr_clear();
//			ALIF_DECREF(co);
//			ALIF_DECREF(dict);
//			return alifStatus_error("error importing winreg module");
//		}
//	}
//	else {
//		ALIF_DECREF(winreg);
//	}
//#endif
//
//	if (
//#ifdef _WINDOWS
//		!decode_toDict(dict, "os_name", "nt") or
//#elif defined(__APPLE__)
//		!decode_toDict(dict, "os_name", "darwin") or
//#else
//		!decode_toDict(dict, "os_name", "posix") or
//#endif
//#ifdef WITH_NEXT_FRAMEWORK
//		!int_toDict(dict, "WITH_NEXT_FRAMEWORK", 1) or
//#else
//		!int_toDict(dict, "WITH_NEXT_FRAMEWORK", 0) or
//#endif
//		!decode_toDict(dict, "PREFIX", PREFIX) or
//		!decode_toDict(dict, "EXEC_PREFIX", EXEC_PREFIX) or
//		!decode_toDict(dict, "ALIFPATH", ALIFPATH) or
//		!decode_toDict(dict, "VPATH", VPATH) or
//		!decode_toDict(dict, "PLATLIBDIR", PLATLIBDIR) or
//		!decode_toDict(dict, "ALIFDEBUGEXT", ALIFDEBUGEXT) or
//		!int_toDict(dict, "VERSION_MAJOR", ALIF_MAJOR_VERSION) or
//		!int_toDict(dict, "VERSION_MINOR", ALIF_MINOR_VERSION) or
//		!decode_toDict(dict, "ALIFWINVER", ALIFWINVER) or
//		!wchar_toDict(dict, "EXE_SUFFIX", EXE_SUFFIX) or
//		!env_toDict(dict, "ENV_PATH", 0) or
//		!env_toDict(dict, "ENV_ALIFHOME", 0) or
//		!env_toDict(dict, "ENV_ALIFEXECUTABLE", 0) or
//		!env_toDict(dict, "ENV___ALIFVENV_LAUNCHER__", 1) or
//		!progname_toDict(dict, "real_executable") or
//		!library_toDict(dict, "library") or
//		!wchar_toDict(dict, "executable_dir", nullptr) or
//		!wchar_toDict(dict, "alif_setpath", _alifPathConfig_getGlobalModuleSearchPath()) or
//		!funcs_toDict(dict, _config->pathconfig_warnings) or
//
//		!decode_toDict(dict, "ABI_THREAD", "t") or
//
//#ifndef _WINDOWS
//		alifDict_setItemString(dict, "winreg", ALIF_NONE) < 0 or
//#endif
//		alifDict_setItemString(dict, "__builtins__", alifEval_getBuiltins()) < 0
//		) {
//		ALIF_DECREF(co);
//		ALIF_DECREF(dict);
//		alifErr_formatUnraisable("Exception ignored in preparing getpath");
//		return alifStatus_error("error evaluating initial values");
//	}
//
//	AlifObject* r = alifEval_evalCode(co, dict, dict);
//	ALIF_DECREF(co);
//
//	if (!r) {
//		ALIF_DECREF(dict);
//		alifErr_formatUnraisable("Exception ignored in running getpath");
//		return alifStatus_error("error evaluating path");
//	}
//	ALIF_DECREF(r);
//
//	if (_alifConfig_fromDict(_config, configDict) < 0) {
//		alifErr_formatUnraisable("Exception ignored in reading getpath results");
//		ALIF_DECREF(dict);
//		return alifStatus_error("error getting getpath results");
//	}
//
//	ALIF_DECREF(dict);

	return 1;
}













/* ----------------------------------------------- alif path config ----------------------------------------------- */

#define DEFAULT_PROGRAM_NAME L"alif"




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




static AlifIntT alif_env(wchar_t** _buf, const char* _env, AlifIntT _andClear) { // 678
	AlifIntT r = 0;
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
		*_buf = alifMem_wcsDup(v);
		r = 1;
	}
#else
	const char* v = getenv(&_env[4]);
	if (v) {
		AlifUSizeT len{};
		const wchar_t* w = alif_decodeLocale(v, &len);
		if (w) {
			*_buf = alifMem_wcsDup(w);
			r = 1;
		}
	}
#endif
	if (r and _andClear) {
#ifdef _WINDOWS
		_wputenv_s(wkey, L"");
#else
		unsetenv(&_env[4]);
#endif
	}
	return r;
}


static AlifIntT prog_name(AlifConfig* _config) { // 770
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
	_config->executable = (wchar_t*)alifMem_dataAlloc(wcslen(buffer) * sizeof(wchar_t));
	wcscpy(_config->executable, buffer);
	alifMem_dataFree(buffer);
	return 1;
#elif defined(__APPLE__)
	char* path{};
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
			if (path and path[0]) {
				AlifUSizeT len{};
				const wchar_t* w = alif_decodeLocale(path, &len);
				if (w) {
					_config->executable = w;
				}
			}
			alifMem_dataFree(path);
			return 1;
		}
		// Fall back and store None
		alifMem_dataFree(path);
		break;
	}
#endif

	_config->executable = nullptr;
	return 1;
}


static wchar_t* add_string(const wchar_t* str1,const wchar_t* str2) {
	AlifUSizeT len1 = (str1 != nullptr) ? wcslen(str1) : 0;
	AlifUSizeT len2 = (str2 != nullptr) ? wcslen(str2) : 0;
	wchar_t* res = (wchar_t*)alifMem_dataAlloc((len1 + len2 + 1) * sizeof(wchar_t));
	if (str1 != nullptr) {
		wmemcpy(res, str1, len1);
	}
	if (str2 != nullptr) {
		wmemcpy(res + len1, str2, len2);
	}
	res[len1 + len2] = L'\0';

	return res;
}

static AlifIntT set_programName(AlifConfig* _config) {
	if (!_config->programName) {
		_config->programName = _config->origArgv.items[0];
	}
	if (!_config->programName) {
		_config->programName = (wchar_t*)alifMem_dataAlloc(sizeof(DEFAULT_PROGRAM_NAME));
		_config->programName = (wchar_t*)DEFAULT_PROGRAM_NAME;
	}

	if (EXE_SUFFIX and !has_suffix(_config->programName, EXE_SUFFIX)) {
		wchar_t* programNameSuffix = add_string(_config->programName, EXE_SUFFIX);
		if (is_xFile(programNameSuffix)) {
			_config->programName = programNameSuffix;
			return 1;
		}
		alifMem_dataFree(programNameSuffix);
	}

	if (!_config->programName) {
		return -1;
	}

	return 1;
}


static AlifIntT alifConfig_initPathConfigAlif(AlifConfig* _config, AlifIntT _computePathConfig) {
	AlifIntT status{};

	status = set_programName(_config);
	if (status < 1) {
		return status;
	}


	wchar_t* executableDir{};
	wchar_t* realExecutableDir{};
	if (_alifPathConfig_getGlobalModuleSearchPath()) {
		if (!_config->executable) {
			status = prog_name(_config);
		}
	}
	if (!_config->executable and wcschr(_config->programName, SEP)) {
		status = _alif_absPath(_config->programName, &_config->executable);
	}
	if (!_config->executable) {
		status = prog_name(_config);
	}
	wchar_t* env{};
	status = alif_env(&env, "ENV_PATH", 0);
	if (status < 1) {
		return status;
	}
	if (!_config->executable and _config->programName and env) {
		wchar_t* context{};
		for (wchar_t* p = wcstok(env, L";", &context);
			p != nullptr;
			p = wcstok(nullptr, L";", &context))
		{
			p = join_paths(2, p, _config->programName);
			if (is_xFile(p)) {
				_config->executable = p;
				break;
			}
			continue;
		}
	}
	if (!_config->executable) {
		_config->executable = (wchar_t*)alifMem_dataAlloc(sizeof(wchar_t));

		status = _alif_absPath(L".", &executableDir);
		realExecutableDir = alifMem_wcsDup(executableDir);
	}
	


	return 1;
}
