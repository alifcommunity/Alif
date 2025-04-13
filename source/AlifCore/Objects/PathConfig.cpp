#include "alif.h"

#include "AlifCore_InitConfig.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_Memory.h"
#include "AlifCore_PathConfig.h"


#include "OSDefs.h"



class AlifPathConfig { // 23
public:
	wchar_t* programFullPath{};
	wchar_t* prefix{};
	wchar_t* execPrefix{};
	wchar_t* stdLibDir{};
	wchar_t* moduleSearchPath{};
	wchar_t* calculatedModuleSearchPath{};
	wchar_t* programName{};
	wchar_t* home{};
	AlifIntT isAlifBuild{};
};

#  define ALIFPATHCONFIG_INIT \
      {.moduleSearchPath = nullptr, .isAlifBuild = 0}


AlifPathConfig _alifPathConfig_ = ALIFPATHCONFIG_INIT;

const wchar_t* _alifPathConfig_getGlobalModuleSearchPath(void) { // 47
	return _alifPathConfig_.moduleSearchPath;
}


AlifIntT _alifPathConfig_readGlobal(AlifConfig* _config) { // 81
	AlifIntT status = 1;

#define COPY(ATTR) \
    do { \
        if (_alifPathConfig_.ATTR and !_config->ATTR) { \
            status = alifConfig_setString(_config, &_config->ATTR, _alifPathConfig_.ATTR); \
            if (status < 1) goto done; \
        } \
    } while (0)

#define COPY2(ATTR, SRCATTR) \
    do { \
        if (_alifPathConfig_.SRCATTR and !_config->ATTR) { \
            status = alifConfig_setString(_config, &_config->ATTR, _alifPathConfig_.SRCATTR); \
            if (status < 1) goto done; \
        } \
    } while (0)

#define COPY_INT(ATTR) \
    do { \
        if ((_alifPathConfig_.ATTR >= 0) and (_config->ATTR <= 0)) { \
            _config->ATTR = _alifPathConfig_.ATTR; \
        } \
    } while (0)

	COPY(prefix);
	COPY(execPrefix);
	COPY(stdLibDir);
	COPY(programName);
	COPY(home);
	COPY2(executable, programFullPath);
	//COPY_INT(_isAlifBuild);
	// module_search_path must be initialised - not read
#undef COPY
#undef COPY2
#undef COPY_INT

	done :
	return status;
}



AlifIntT _alifPathConfig_updateGlobal(const AlifConfig* _config) { // 126

#define COPY(_attr) \
    do { \
        if (_config->_attr) { \
            alifMem_dataFree(_alifPathConfig_._attr); \
            _alifPathConfig_._attr = alifMem_wcsDup(_config->_attr); \
            if (!_alifPathConfig_._attr) goto error; \
        } \
    } while (0)

#define COPY2(_attr, _srcAttr) \
    do { \
        if (_config->_srcAttr) { \
            alifMem_dataFree(_alifPathConfig_._attr); \
            _alifPathConfig_._attr = alifMem_wcsDup(_config->_srcAttr); \
            if (!_alifPathConfig_._attr) goto error; \
        } \
    } while (0)

#define COPY_INT(_attr) \
    do { \
        if (_config->_attr > 0) { \
            _alifPathConfig_._attr = _config->_attr; \
        } \
    } while (0)

	COPY(prefix);
	COPY(execPrefix);
	COPY(stdLibDir);
	COPY(programName);
	COPY(home);
	COPY2(programFullPath, executable);
	//COPY_INT(isAlifBuild);
#undef COPY
#undef COPY2
#undef COPY_INT

	alifMem_dataFree(_alifPathConfig_.moduleSearchPath);
	_alifPathConfig_.moduleSearchPath = nullptr;
	alifMem_dataFree(_alifPathConfig_.calculatedModuleSearchPath);
	_alifPathConfig_.calculatedModuleSearchPath = nullptr;

	do {
		AlifUSizeT cch = 1;
		for (AlifSizeT i = 0; i < _config->moduleSearchPaths.length; ++i) {
			cch += 1 + wcslen(_config->moduleSearchPaths.items[i]);
		}

		wchar_t* path = (wchar_t*)alifMem_dataAlloc(sizeof(wchar_t) * cch);
		if (!path) {
			goto error;
		}
		wchar_t* p = path;
		for (AlifSizeT i = 0; i < _config->moduleSearchPaths.length; ++i) {
			wcscpy(p, _config->moduleSearchPaths.items[i]);
			p = wcschr(p, L'\0');
			*p++ = DELIM;
			*p = L'\0';
		}

		do {
			*p = L'\0';
		} while (p != path && *--p == DELIM);
		_alifPathConfig_.calculatedModuleSearchPath = path;
	} while (0);

	return 1;

error:
	return -1;
}









AlifIntT _alifPathConfig_computeSysPath0(const AlifWStringList* _argv,
	AlifObject** _path0P) { // 370
	if (_argv->length == 0) {
		return 0;
	}

	wchar_t* argv0 = _argv->items[0];
	AlifIntT haveModuleArg = (wcscmp(argv0, L"-m") == 0); //* todo
	AlifIntT haveScriptArg = (!haveModuleArg and (wcscmp(argv0, L"-c") != 0)); //* todo

	wchar_t* path0 = argv0;
	AlifSizeT n = 0;

#ifdef HAVE_REALPATH
	wchar_t fullpath[MAXPATHLEN];
#elif defined(_WINDOWS)
	wchar_t fullpath[MAX_PATH];
#endif

	if (haveModuleArg) {
#if defined(HAVE_REALPATH) or defined(_WINDOWS)
		if (!alif_wGetCWD(fullpath, ALIF_ARRAY_LENGTH(fullpath))) {
			return 0;
		}
		path0 = fullpath;
#else
		path0 = L".";
#endif
		n = wcslen(path0);
	}

#ifdef HAVE_READLINK
	wchar_t link[MAXPATHLEN + 1];
	int nr = 0;
	wchar_t path0copy[2 * MAXPATHLEN + 1];

	if (haveScriptArg) {
		nr = alif_wReadLink(path0, link, ALIF_ARRAY_LENGTH(link));
	}
	if (nr > 0) {
		/* It's a symlink */
		link[nr] = '\0';
		if (link[0] == SEP) {
			path0 = link; /* Link to absolute path */
		}
		else if (wcschr(link, SEP) == nullptr) {
			/* Link without path */
		}
		else {
			/* Must join(dirname(path0), link) */
			wchar_t* q = wcsrchr(path0, SEP);
			if (q == nullptr) {
				/* path0 without path */
				path0 = link;
			}
			else {
				/* Must make a copy, path0copy has room for 2 * MAXPATHLEN */
				wcsncpy(path0copy, path0, MAXPATHLEN);
				q = wcsrchr(path0copy, SEP);
				wcsncpy(q + 1, link, MAXPATHLEN);
				q[MAXPATHLEN + 1] = L'\0';
				path0 = path0copy;
			}
		}
	}
#endif /* HAVE_READLINK */

	wchar_t* p = nullptr;

#if SEP == '\\'
	/* Special case for Microsoft filename syntax */
	if (haveScriptArg) {
		wchar_t* q{};
#if defined(_WINDOWS)
		/* Replace the first element in argv with the full path. */
		wchar_t* ptemp;
		if (GetFullPathNameW(path0,
			ALIF_ARRAY_LENGTH(fullpath),
			fullpath,
			&ptemp)) {
			path0 = fullpath;
		}
#endif
		p = wcsrchr(path0, SEP);
		/* Test for alternate separator */
		q = wcsrchr(p ? p : path0, '/');
		if (q != nullptr)
			p = q;
		if (p != nullptr) {
			n = p + 1 - path0;
			if (n > 1 and p[-1] != ':')
				n--; /* Drop trailing separator */
		}
	}
#else
	/* All other filename syntaxes */
	if (haveScriptArg) {
#if defined(HAVE_REALPATH)
		if (alif_wRealPath(path0, fullpath, ALIF_ARRAY_LENGTH(fullpath))) {
			path0 = fullpath;
		}
#endif
		p = wcsrchr(path0, SEP);
	}
	if (p != nullptr) {
		n = p + 1 - path0;
#if SEP == '/' /* Special case for Unix filename syntax */
		if (n > 1) {
			/* Drop trailing separator */
			n--;
		}
#endif /* Unix */
	}
#endif /* All others */

	AlifObject* path0_obj = alifUStr_fromWideChar(path0, n);
	if (path0_obj == nullptr) {
		return -1;
	}

	*_path0P = path0_obj;
	return 1;
}
