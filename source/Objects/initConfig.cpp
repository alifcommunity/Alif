#include "alif.h"
#include "alifCore_fileUtils.h"
//#include "alifCore_getOpt.h"
#include "alifCore_initConfig.h"
#include "alifCore_interp.h"
//#include "alifCore_long.h"
//#include "alifCore_pathConfig.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifLifeCycle.h"
#include "alifCore_alifMem.h" 
#include "alifCore_alifState.h"

//#include "osdefs.h"

#include <locale.h>
//#include <stdlib.h>
#if defined(MS_WINDOWS) || defined(__CYGWIN__)
#  ifdef HAVE_IO_H
#    include <io.h>
#  endif
#  ifdef HAVE_FCNTL_H
#    include <fcntl.h>
#  endif
#endif








































































































































AlifStatus alifPreCmdline_setConfig(const AlifPreCmdline* cmdline, AlifConfig* config)
{
#define COPY_ATTR(ATTR) \
    config->ATTR = cmdline->ATTR

	AlifStatus status = alifWideStringList_extend(&config->xOptions, &cmdline->xOptions);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(devMode);
	COPY_ATTR(warnDefaultEncoding);
	return ALIFSTATUS_OK();

#undef COPY_ATTR
}




















/* --- Global configuration variables ----------------------------- */

int alifUTF8Mode = 0;
int alifDebugFlag = 0;
int alifVerboseFlag = 0;
int alifQuietFlag = 0;
int alifInteractiveFlag = 0;
int alifInspectFlag = 0;
int alifOptimizeFlag = 0;
int alifNoSiteFlag = 0;
int alifBytesWarningFlag = 0;
int alifFrozenFlag = 0;
int alifIgnoreEnvironmentFlag = 0;
int alifDontWriteBytecodeFlag = 0;
int alifNoUserSiteDirectory = 0;
int alifUnbufferedStdioFlag = 0;
int alifHashRandomizationFlag = 0;
int alifIsolatedFlag = 0;
#ifdef MS_WINDOWS
int alifLegacyWindowsFSEncodingFlag = 0;
int alifLegacyWindowsStdioFlag = 0;
#endif

























































































































































void alifWideStringList_clear(AlifWideStringList* _list)
{
	//assert(AlifWideStringList_checkConsistency(_list));
	for (AlifSizeT i = 0; i < _list->length; i++) {
		alifMem_rawFree(_list->items[i]);
	}
	alifMem_rawFree(_list->items);
	_list->length = 0;
	_list->items = nullptr;
}



int alifWideStringList_copy(AlifWideStringList* _list, const AlifWideStringList* _list2)
{
	//assert(alifWideStringList_checkConsistency(_list));
	//assert(alifWideStringList_checkConsistency(_list2));

	if (_list2->length == 0) {
		alifWideStringList_clear(_list);
		return 0;
	}

	AlifWideStringList copy = ALIFWIDESTRINGLIST_INIT;

	size_t size = _list2->length * sizeof(_list2->items[0]);
	copy.items = (wchar_t**)alifMem_rawMalloc(size); // تم عمل تغيير النوع بسبب وجود خطأ
	if (copy.items == nullptr) {
		return -1;
	}

	for (AlifSizeT i = 0; i < _list2->length; i++) {
		wchar_t* item = alifMem_rawWcsDup(_list2->items[i]);
		if (item == nullptr) {
			alifWideStringList_clear(&copy);
			return -1;
		}
		copy.items[i] = item;
		copy.length = i + 1;
	}

	alifWideStringList_clear(_list);
	*_list = copy;
	return 0;
}



AlifStatus alifWideStringList_insert(AlifWideStringList* _list, AlifSizeT _index, const wchar_t* _item)
{
	AlifSizeT len = _list->length;
	if (len == ALIFSIZE_T_MAX) {
		/* length+1 would overflow */
		return ALIFSTATUS_NO_MEMORY();
	}
	if (_index < 0) {
		return ALIFSTATUS_ERR("alifWideStringList_insert index must be >= 0");
	}
	if (_index > len) {
		_index = len;
	}

	wchar_t* item2 = alifMem_rawWcsDup(_item);
	if (item2 == nullptr) {
		return ALIFSTATUS_NO_MEMORY();
	}

	size_t size = (len + 1) * sizeof(_list->items[0]);
	wchar_t** items2 = (wchar_t**)alifMem_rawRealloc(_list->items, size);
	if (items2 == nullptr) {
		alifMem_rawFree(item2);
		return ALIFSTATUS_NO_MEMORY();
	}

	if (_index < len) {
		memmove(&items2[_index + 1],
			&items2[_index],
			(len - _index) * sizeof(items2[0]));
	}

	items2[_index] = item2;
	_list->items = items2;
	_list->length++;
	return ALIFSTATUS_OK();
}




AlifStatus alifWideStringList_append(AlifWideStringList* _list, const wchar_t* _item)
{
	return alifWideStringList_insert(_list, _list->length, _item);
}



AlifStatus alifWideStringList_extend(AlifWideStringList* _list, const AlifWideStringList* _list2)
{
	for (AlifSizeT i = 0; i < _list2->length; i++) {
		AlifStatus status = alifWideStringList_append(_list, _list2->items[i]);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}
	return ALIFSTATUS_OK();
}


















































































































































void alifConfig_clear(AlifConfig* _config)
{
#define CLEAR(ATTR) \
    do { \
        alifMem_rawFree(ATTR); \
        ATTR = nullptr; \
    } while (0)

	CLEAR(_config->alifCachePrefix);
	CLEAR(_config->alifPathEnv);
	CLEAR(_config->home);
	CLEAR(_config->programName);

	alifWideStringList_clear(&_config->argv);
	alifWideStringList_clear(&_config->warnOptions);
	alifWideStringList_clear(&_config->xOptions);
	alifWideStringList_clear(&_config->moduleSearchPaths);
	_config->moduleSearchPathsSet = 0;
	CLEAR(_config->stdlibDir);

	CLEAR(_config->executable);
	CLEAR(_config->baseExecutable);
	CLEAR(_config->prefix);
	CLEAR(_config->basePrefix);
	CLEAR(_config->execPrefix);
	CLEAR(_config->baseExecPrefix);
	CLEAR(_config->platLibDir);

	CLEAR(_config->fileSystemEncoding);
	CLEAR(_config->fileSystemErrors);
	CLEAR(_config->stdioEncoding);
	CLEAR(_config->stdioErrors);
	CLEAR(_config->runCommand);
	CLEAR(_config->runModule);
	CLEAR(_config->runFilename);
	CLEAR(_config->checkHashAlifCSMode);

	alifWideStringList_clear(&_config->origArgv);
#undef CLEAR
}



void alifConfig_initCompatConfig(AlifConfig* _config)
{
	memset(_config, 0, sizeof(*_config));

	_config->configInit = (int)AlifConfig_Init_Compat;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->devMode = -1;
	_config->installSignalHandlers = 1;
	_config->useHashSeed = -1;
	_config->faultHandler = -1;
	_config->traceMalloc = -1;
	_config->perfProfiling = -1;
	_config->moduleSearchPathsSet = 0;
	_config->parseArgv = 0;
	_config->siteImport = -1;
	_config->bytesWarning = -1;
	_config->warnDefaultEncoding = 0;
	_config->inspect = -1;
	_config->interactive = -1;
	_config->optimizationLevel = -1;
	_config->parserDebug = -1;
	_config->writeBytecode = -1;
	_config->verbose = -1;
	_config->quiet = -1;
	_config->userSiteDirectory = -1;
	_config->configureCStdio = 0;
	_config->bufferedStdio = -1;
	_config->installImportLib = 1;
	_config->checkHashAlifCSMode = NULL;
	_config->pathConfigWarnings = -1;
	_config->initMain = 1;
#ifdef MS_WINDOWS
	_config->legacyWindowsStdio = -1;
#endif
#ifdef ALIF_DEBUG
	_config->useFrozenModules = 0;
#else
	_config->useFrozenModules = 1;
#endif
	_config->safePath = 0;
	_config->intMaxStrDigits = -1;
	_config->isAlifBuild = 0;
	_config->codeDebugRanges = 1;
}



static void config_initDefaults(AlifConfig* _config)
{
	alifConfig_initCompatConfig(_config);

	_config->isolated = 0;
	_config->useEnvironment = 1;
	_config->siteImport = 1;
	_config->bytesWarning = 0;
	_config->inspect = 0;
	_config->interactive = 0;
	_config->optimizationLevel = 0;
	_config->parserDebug = 0;
	_config->writeBytecode = 1;
	_config->verbose = 0;
	_config->quiet = 0;
	_config->userSiteDirectory = 1;
	_config->bufferedStdio = 1;
	_config->pathConfigWarnings = 1;
#ifdef MS_WINDOWS
	_config->legacyWindowsStdio = 0;
#endif
}



void alifConfig_initAlifConfig(AlifConfig* _config)
{
	config_initDefaults(_config);

	_config->configInit = (int)AlifConfig_Init_Alif;
	_config->configureCStdio = 1;
	_config->parseArgv = 1;
}




























AlifStatus alifConfig_setString(AlifConfig* _config, wchar_t** _configStr, const wchar_t* _str)
{
	AlifStatus status = alif_preInitializeFromConfig(_config, nullptr);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	wchar_t* str2;
	if (_str != nullptr) {
		str2 = alifMem_rawWcsDup(_str);
		if (str2 == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}
	else {
		str2 = nullptr;
	}
	alifMem_rawFree(*_configStr);
	*_configStr = str2;
	return ALIFSTATUS_OK();
}













































AlifStatus alifConfig_copy(AlifConfig* _config, const AlifConfig* _config2)
{
	AlifStatus status{};

	alifConfig_clear(_config);

#define COPY_ATTR(ATTR) _config->ATTR = _config2->ATTR
#define COPY_WSTR_ATTR(ATTR) \
    do { \
        status = alifConfig_setString(_config, &_config->ATTR, _config2->ATTR); \
        if (ALIFSTATUS_EXCEPTION(status)) { \
            return status; \
        } \
    } while (0)
#define COPY_WSTRLIST(LIST) \
    do { \
        if (alifWideStringList_copy(&_config->LIST, &_config2->LIST) < 0) { \
            return ALIFSTATUS_NO_MEMORY(); \
        } \
    } while (0)

	COPY_ATTR(configInit);
	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(devMode);
	COPY_ATTR(installSignalHandlers);
	COPY_ATTR(useHashSeed);
	COPY_ATTR(hashSeed);
	COPY_ATTR(installImportLib);
	COPY_ATTR(faultHandler);
	COPY_ATTR(traceMalloc);
	COPY_ATTR(perfProfiling);
	COPY_ATTR(importTime);
	COPY_ATTR(codeDebugRanges);
	COPY_ATTR(showRefCount);
	COPY_ATTR(dumpRefs);
	COPY_ATTR(dumpRefsFile);
	COPY_ATTR(mallocStats);

	COPY_WSTR_ATTR(alifCachePrefix);
	COPY_WSTR_ATTR(alifPathEnv);
	COPY_WSTR_ATTR(home);
	COPY_WSTR_ATTR(programName);

	COPY_ATTR(parseArgv);
	COPY_WSTRLIST(argv);
	COPY_WSTRLIST(warnOptions);
	COPY_WSTRLIST(xOptions);
	COPY_WSTRLIST(moduleSearchPaths);
	COPY_ATTR(moduleSearchPathsSet);
	COPY_WSTR_ATTR(stdlibDir);

	COPY_WSTR_ATTR(executable);
	COPY_WSTR_ATTR(baseExecutable);
	COPY_WSTR_ATTR(prefix);
	COPY_WSTR_ATTR(basePrefix);
	COPY_WSTR_ATTR(execPrefix);
	COPY_WSTR_ATTR(baseExecPrefix);
	COPY_WSTR_ATTR(platLibDir);

	COPY_ATTR(siteImport);
	COPY_ATTR(bytesWarning);
	COPY_ATTR(warnDefaultEncoding);
	COPY_ATTR(inspect);
	COPY_ATTR(interactive);
	COPY_ATTR(optimizationLevel);
	COPY_ATTR(parserDebug);
	COPY_ATTR(writeBytecode);
	COPY_ATTR(verbose);
	COPY_ATTR(quiet);
	COPY_ATTR(userSiteDirectory);
	COPY_ATTR(configureCStdio);
	COPY_ATTR(bufferedStdio);
	COPY_WSTR_ATTR(fileSystemEncoding);
	COPY_WSTR_ATTR(fileSystemErrors);
	COPY_WSTR_ATTR(stdioEncoding);
	COPY_WSTR_ATTR(stdioErrors);
#ifdef MS_WINDOWS
	COPY_ATTR(legacyWindowsStdio);
#endif
	COPY_ATTR(skipSourceFirstLine);
	COPY_WSTR_ATTR(runCommand);
	COPY_WSTR_ATTR(runModule);
	COPY_WSTR_ATTR(runFilename);
	COPY_WSTR_ATTR(checkHashAlifCSMode);
	COPY_ATTR(pathConfigWarnings);
	COPY_ATTR(initMain);
	COPY_ATTR(useFrozenModules);
	COPY_ATTR(safePath);
	COPY_WSTRLIST(origArgv);
	COPY_ATTR(isAlifBuild);
	COPY_ATTR(intMaxStrDigits);

#undef COPY_ATTR
#undef COPY_WSTR_ATTR
#undef COPY_WSTRLIST
	return ALIFSTATUS_OK();
}









































































































































































































































































































































































































































































static void configGet_globalVars(AlifConfig* _config)
{
ALIFCOMP_DIAGPUSH
ALIFCOMP_DIAGIGNORE_DEPRDECLS
if (_config->configInit != AlifConfig_Init_Compat) {
	return;
}

#define COPY_FLAG(ATTR, VALUE) \
        if (_config->ATTR == -1) { \
            _config->ATTR = VALUE; \
        }
#define COPY_NOT_FLAG(ATTR, VALUE) \
        if (_config->ATTR == -1) { \
            _config->ATTR = !(VALUE); \
        }

	COPY_FLAG(isolated, alifIsolatedFlag);
	COPY_NOT_FLAG(useEnvironment, alifIgnoreEnvironmentFlag);
	COPY_FLAG(bytesWarning, alifBytesWarningFlag);
	COPY_FLAG(inspect, alifInspectFlag);
	COPY_FLAG(interactive, alifInteractiveFlag);
	COPY_FLAG(optimizationLevel, alifOptimizeFlag);
	COPY_FLAG(parserDebug, alifDebugFlag);
	COPY_FLAG(verbose, alifVerboseFlag);
	COPY_FLAG(quiet, alifQuietFlag);
#ifdef MS_WINDOWS
	COPY_FLAG(legacyWindowsStdio, alifLegacyWindowsStdioFlag);
#endif
	COPY_NOT_FLAG(pathConfigWarnings, alifFrozenFlag);

	COPY_NOT_FLAG(bufferedStdio, alifUnbufferedStdioFlag);
	COPY_NOT_FLAG(siteImport, alifNoSiteFlag);
	COPY_NOT_FLAG(writeBytecode, alifDontWriteBytecodeFlag);
	COPY_NOT_FLAG(userSiteDirectory, alifNoUserSiteDirectory);

#undef COPY_FLAG
#undef COPY_NOT_FLAG
	ALIFCOMP_DIAGPOP
}


































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































static AlifStatus core_readPreCmdline(AlifConfig* _config, AlifPreCmdline* _preCmdline)
{
	AlifStatus status{};

	if (_config->parseArgv == 1) {
		if (alifWideStringList_copy(&_preCmdline->argv, &_config->argv) < 0) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	AlifPreConfig preConfig{};

	status = alifPreConfig_initFromPreConfig(&preConfig, &alifRuntime.preConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	alifPreConfig_getConfig(&preConfig, _config);

	status = alifPreCmdline_read(_preCmdline, &preConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = alifPreCmdline_setConfig(_preCmdline, _config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	return ALIFSTATUS_OK();
}


































static AlifStatus config_readCmdline(AlifConfig* _config)
{
	AlifStatus status;
	AlifWideStringList cmdlineWarnOptions = ALIFWIDESTRINGLIST_INIT;
	AlifWideStringList envWarnOptions = ALIFWIDESTRINGLIST_INIT;
	AlifWideStringList sysWarnOptions = ALIFWIDESTRINGLIST_INIT;

	if (_config->parseArgv < 0) {
		_config->parseArgv = 1;
	}

	if (_config->parseArgv == 1) {
		AlifSizeT opt_index;
		status = config_parseCmdline(_config, &cmdlineWarnOptions, &opt_index);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}

		status = configRun_filenameAbspath(_config);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}

		status = config_updateArgv(_config, opt_index);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}
	}
	else {
		status = configRun_filenameAbspath(_config);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}
	}

	if (_config->useEnvironment) {
		status = configInit_envWarnOptions(_config, &envWarnOptions);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}
	}


	status = alifSys_readPreInitWarnOptions(&sysWarnOptions);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	status = config_initWarnOptions(_config,
		&cmdlineWarnOptions,
		&envWarnOptions,
		&sysWarnOptions);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	status = ALIFSTATUS_OK();

done:
	alifWideStringList_clear(&cmdlineWarnOptions);
	alifWideStringList_clear(&envWarnOptions);
	alifWideStringList_clear(&sysWarnOptions);
	return status;
}



AlifStatus alifConfig_setAlifArgv(AlifConfig* _config, const AlifArgv* _args)
{
	AlifStatus status = alif_preInitializeFromConfig(_config, _args);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	return alifArgv_asWstrList(_args, &_config->argv);
}






AlifStatus alifConfig_setBytesArgv(AlifConfig* _config, AlifSizeT _argc, char* const* _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 1,
		.bytesArgv = _argv,
		.wcharArgv = nullptr };
	return alifConfig_setAlifArgv(_config, &args);
}


AlifStatus alifConfig_setArgv(AlifConfig* config, AlifSizeT _argc, wchar_t* const* _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 0,
		.bytesArgv = nullptr,
		.wcharArgv = _argv };
	return alifConfig_setAlifArgv(config, &args);
}



























AlifStatus alifConfig_read(AlifConfig* _config, int _computePathConfig)
{
	AlifStatus status{};

	status = alif_preInitializeFromConfig(_config, nullptr);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	configGet_globalVars(_config);

	if (_config->origArgv.length == 0
		&& !(_config->argv.length == 1
			&& wcscmp(_config->argv.items[0], L"") == 0))
	{
		if (alifWideStringList_copy(&_config->origArgv, &_config->argv) < 0) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	AlifPreCmdline preCmdline = ALIFPRECMDLINE_INIT;
	status = core_readPreCmdline(_config, &preCmdline);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	//assert(_config->isolated >= 0);
	if (_config->isolated) {
		_config->safePath = 1;
		_config->useEnvironment = 0;
		_config->userSiteDirectory = 0;
	}

	status = config_readCmdline(_config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	status = alifSys_readPreInitXOptions(_config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	status = config_read(_config, _computePathConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	//assert(config_checkConsistency(_config));

	status = ALIFSTATUS_OK();

done:
	alifPreCmdline_clear(&preCmdline);
	return status;
}





























































































































































































































































































































