#include "alif.h"

#include "AlifCore_GetOption.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Memory.h"
#include "AlifCore_Long.h"
#include "AlifCore_State.h"
#include "AlifCore_Runtime.h"

#include "OSDefs.h"

#include <locale.h>
#include <stdlib.h>
#ifdef _WINDOWS
#include <fcntl.h>
#else
#include <unistd.h>
#endif


#include "ConfigCommon.h"

/* --------------------------------------------- AlifConfig setters --------------------------------------------- */

typedef AlifObject* (*ConfigSysFlagSetter) (AlifIntT);

static AlifObject* config_sysFlagLong(AlifIntT _value) {
	return alifLong_fromLong(_value);
}

static AlifObject* config_sysFlagNot(AlifIntT _value) { // 39
	_value = not _value;
	return config_sysFlagLong(_value);
}



/* ---------------------------------------------- AlifConfig spec ---------------------------------------------- */

enum AlifConfigMemberType_ { // 49
	Alif_Config_Member_INT = 0,
	Alif_Config_Member_UINT = 1,
	Alif_Config_Member_ULONG = 2,
	Alif_Config_Member_BOOL = 3,
	Alif_Config_Member_WSTR = 10,
	Alif_Config_Member_WSTR_OPT = 11,
	Alif_Config_Member_WSTR_LIST = 12,
};

enum AlifConfigMemberVisibility { // 60
	Alif_Config_Member_INIT_ONLY = 0,
	Alif_Config_Member_READ_ONLY = 1,
	Alif_Config_Member_PUBLIC = 2,
};

class AlifConfigSysSpec { // 71
public:
	const char* attr{};
	AlifIntT flagIndex{};
	ConfigSysFlagSetter flagSetter{};
};

class AlifConfigSpec { // 43
public:
	const char* name{};
	AlifUSizeT offset{};
	AlifConfigMemberType_ type{};
	AlifConfigMemberVisibility visibility{};
	AlifConfigSysSpec sys{};
};

// 85
#define SPEC(_name, _member, _type, _visibility, _sys) \
    {#_name, offsetof(AlifConfig, _member), \
     Alif_Config_Member_##_type, Alif_Config_Member_##_visibility, _sys}

#define SYS_ATTR(name) {name, -1, nullptr}
#define SYS_FLAG_SETTER(index, setter) {nullptr, index, setter}
#define SYS_FLAG(index) SYS_FLAG_SETTER(index, nullptr)
#define NO_SYS SYS_ATTR(nullptr)

static const AlifConfigSpec _alifConfigSpec_[] = { // 95 //* todo //* important

	// --- Public options -----------
	SPEC(argv, argv, WSTR_LIST, PUBLIC, SYS_ATTR("argv")),
	SPEC(سابقة_اساسية_تنفيذي, baseExecPrefix, WSTR_OPT, PUBLIC, SYS_ATTR("baseExecPrefix")),
	SPEC(تنفيذي_اساسي, baseExecutable, WSTR_OPT, PUBLIC, SYS_ATTR("baseExecutable")),
	SPEC(سابقة_اساسية, basePrefix, WSTR_OPT, PUBLIC, SYS_ATTR("basePrefix")),
	SPEC(bytesWarning, bytesWarning, UINT, PUBLIC, SYS_FLAG(9)),
	SPEC(سابقة_التنفيذي, execPrefix, WSTR_OPT, PUBLIC, SYS_ATTR("execPrefix")),
	SPEC(تنفيذي, executable, WSTR_OPT, PUBLIC, SYS_ATTR("executable")),
	SPEC(inspect, inspect, BOOL, PUBLIC, SYS_FLAG(1)),
	SPEC(intMaxStrDigits, intMaxStrDigits, UINT, PUBLIC, NO_SYS),
	SPEC(interactive, interactive, BOOL, PUBLIC, SYS_FLAG(2)),
	SPEC(مسارات_البحث_الوحدة, moduleSearchPaths, WSTR_LIST, PUBLIC, SYS_ATTR("path")),
	SPEC(optimizationLevel, optimizationLevel, UINT, PUBLIC, SYS_FLAG(3)),
	SPEC(parserDebug, parserDebug, BOOL, PUBLIC, SYS_FLAG(0)),
	SPEC(مجلد_مكتبة_المنصة, platLibDir, WSTR, PUBLIC, SYS_ATTR("platLibDir")),
	SPEC(السابقة, prefix, WSTR_OPT, PUBLIC, SYS_ATTR("prefix")),
	SPEC(alifCachePrefix, alifCachePrefix, WSTR_OPT, PUBLIC, SYS_ATTR("alifCachePrefix")),
	SPEC(quiet, quiet, BOOL, PUBLIC, SYS_FLAG(10)),
	SPEC(مجلد_المكتبة_القياسية, stdLibDir, WSTR_OPT, PUBLIC, SYS_ATTR("stdLibDir")),
	SPEC(استخدم_البيئة, useEnvironment, BOOL, PUBLIC, SYS_FLAG_SETTER(7, config_sysFlagNot)),
	SPEC(verbose, verbose, UINT, PUBLIC, SYS_FLAG(8)),
	SPEC(warnoptions, warnoptions, WSTR_LIST, PUBLIC, SYS_ATTR("warnoptions")),
	SPEC(writeBytecode, writeBytecode, BOOL, PUBLIC, SYS_FLAG_SETTER(4, config_sysFlagNot)),
	SPEC(xoptions, xoptions, WSTR_LIST, PUBLIC, SYS_ATTR("xoptions")),

	// --- Read-only options -----------

#ifdef ALIF_STATS
	SPEC(_alifstats, BOOL, READ_ONLY, NO_SYS),
#endif
	SPEC(bufferedStdio, bufferedStdio, BOOL, READ_ONLY, NO_SYS),
	SPEC(checkHashAlifCSMode, checkHashAlifCSMode, WSTR, READ_ONLY, NO_SYS),
	SPEC(codeDebugRanges, codeDebugRanges, BOOL, READ_ONLY, NO_SYS),
	SPEC(configureCStdio, configureCStdio, BOOL, READ_ONLY, NO_SYS),
	SPEC(cpuCount, cpuCount, INT, READ_ONLY, NO_SYS),
	SPEC(devMode, devMode, BOOL, READ_ONLY, NO_SYS),  // sys.flags.devMode
	SPEC(dumpRefs, dumpRefs, BOOL, READ_ONLY, NO_SYS),
	SPEC(dumpRefsFile, dumpRefsFile, WSTR_OPT, READ_ONLY, NO_SYS),
	SPEC(faultHandler, faultHandler, BOOL, READ_ONLY, NO_SYS),
	SPEC(fileSystemEncoding, fileSystemEncoding, WSTR, READ_ONLY, NO_SYS),
	SPEC(fileSystemErrors, fileSystemErrors, WSTR, READ_ONLY, NO_SYS),
	SPEC(hashSeed, hashSeed, ULONG, READ_ONLY, NO_SYS),
	SPEC(الرئيسية, home, WSTR_OPT, READ_ONLY, NO_SYS),
	SPEC(importTime, importTime, BOOL, READ_ONLY, NO_SYS),
	SPEC(installSignalHandlers, installSignalHandlers, BOOL, READ_ONLY, NO_SYS),
	SPEC(isolated, isolated, BOOL, READ_ONLY, NO_SYS),  // sys.flags.isolated
#ifdef _WINDOWS
	SPEC(legacyWindowsStdio, legacyWindowsStdio, BOOL, READ_ONLY, NO_SYS),
#endif
	SPEC(mallocStats, mallocStats, BOOL, READ_ONLY, NO_SYS),
	SPEC(الوسيطات_الرئيسية, origArgv, WSTR_LIST, READ_ONLY, SYS_ATTR("origArgv")),
	SPEC(parseArgv, parseArgv, BOOL, READ_ONLY, NO_SYS),
	SPEC(pathConfigWarnings, pathConfigWarnings, BOOL, READ_ONLY, NO_SYS),
	SPEC(perfProfiling, perfProfiling, UINT, READ_ONLY, NO_SYS),
	SPEC(اسم_البرنامج, programName, WSTR, READ_ONLY, NO_SYS),
	SPEC(runCommand, runCommand, WSTR_OPT, READ_ONLY, NO_SYS),
	SPEC(runFilename, runFilename, WSTR_OPT, READ_ONLY, NO_SYS),
	SPEC(runModule, runModule, WSTR_OPT, READ_ONLY, NO_SYS),

	SPEC(safePath, safePath, BOOL, READ_ONLY, NO_SYS),
	SPEC(showRefCount, showRefCount, BOOL, READ_ONLY, NO_SYS),
	SPEC(siteImport, siteImport, BOOL, READ_ONLY, NO_SYS),
	SPEC(skipFirstLine, skipFirstLine, BOOL, READ_ONLY, NO_SYS),
	SPEC(stdioEncoding, stdioEncoding, WSTR, READ_ONLY, NO_SYS),
	SPEC(stdioErrors, stdioErrors, WSTR, READ_ONLY, NO_SYS),
	SPEC(tracemalloc, tracemalloc, UINT, READ_ONLY, NO_SYS),
	SPEC(useFrozenModules, useFrozenModules, BOOL, READ_ONLY, NO_SYS),
	SPEC(useHashSeed, useHashSeed, BOOL, READ_ONLY, NO_SYS),
	SPEC(userSiteDirectory, userSiteDirectory, BOOL, READ_ONLY, NO_SYS),
	SPEC(warnDefaultEncoding, warnDefaultEncoding, BOOL, READ_ONLY, NO_SYS),

	// --- Init-only options -----------

	SPEC(configInit, configInit, UINT, INIT_ONLY, NO_SYS),
	SPEC(initMain, initMain, BOOL, INIT_ONLY, NO_SYS),
	SPEC(installImportLib, installImportLib, BOOL, INIT_ONLY, NO_SYS),
	SPEC(هل_ألف_بناء, isAlifBuild, BOOL, INIT_ONLY, NO_SYS),
	SPEC(ضبط_مسارات_البحث_الوحدة, moduleSearchPathsSet, BOOL, INIT_ONLY, NO_SYS),
	SPEC(alifPathEnv, alifPathEnv, WSTR_OPT, INIT_ONLY, NO_SYS),
	SPEC(sysPath0, sysPath0, WSTR_OPT, INIT_ONLY, NO_SYS),

	// Array terminator
	{nullptr, 0, (AlifConfigMemberType_)0, (AlifConfigMemberVisibility)0, NO_SYS},
};

#undef SPEC
#define SPEC(_member, _type, _visibility) \
    {#_member, offsetof(AlifPreConfig, _member), AlifConfig_Member_##_type, \
     Alif_Config_Member_##_visibility, NO_SYS}


#undef SPEC
#undef SYS_ATTR
#undef SYS_FLAG_SETTER
#undef SYS_FLAG
#undef NO_SYS


// Forward declarations
static AlifObject*
config_get(const AlifConfig*, const AlifConfigSpec*, AlifIntT); // 225


/* --------------------------------------- Command line options --------------------------------------- */
// 140
static const char usageLine[] =
"طريقة_الاستخدام: %ls [خيار] ... [-ص نص | -ك مكتبة | ملف | - ] [وسيط] ...\n";

static const char usageHelp[] = "\
-ص : يتم تمرير البرنامج ك نص\n\
-ك : تشغيل مكتبة\n\
ملف : تشغيل البرنامج من ملف\n\
- : يتم تشغيل البرنامج من الطرفية\n\
وسيط ...: وسيطات يتم تمريرها الى sys.argv[1:]\n\
-م : طباعة رسالة المساعدة (--مساعدة)\n\
-ن : طباعة نسخة ألف الحالية (--نسخة)\n\
";

/* --- Global configuration variables ----------------------------- */

/* UTF-8 mode: if equals to 1, use the UTF-8 encoding, and change
   stdin and stdout error handler to "surrogateescape". */
AlifIntT _alifUTF8Mode_ = 0;
AlifIntT _alifDebugFlag_ = 0;
AlifIntT _alifVerboseFlag_ = 0;
AlifIntT _alifQuietFlag_ = 0;
AlifIntT _alifInteractiveFlag_ = 0;
AlifIntT _alifInspectFlag_ = 0;
AlifIntT _alifOptimizeFlag_ = 0;
AlifIntT _alifNoSiteFlag_ = 0;
AlifIntT _alifBytesWarningFlag_ = 0;
AlifIntT _alifFrozenFlag_ = 0;
AlifIntT _alifIgnoreEnvironmentFlag_ = 0;
AlifIntT _alifDontWriteBytecodeFlag_ = 0;
AlifIntT _alifNoUserSiteDirectory_ = 0;
AlifIntT _alifUnbufferedStdioFlag_ = 0;
AlifIntT _alifHashRandomizationFlag_ = 0;
AlifIntT _alifIsolatedFlag_ = 0;
#ifdef _WINDOWS
AlifIntT _alifLegacyWindowsFSEncodingFlag_ = 0;
AlifIntT _alifLegacyWindowsStdioFlag_ = 0;
#endif

/* -------------------------- AlifConfig command line parser -------------------------- */

static void config_usage(AlifIntT _error, const wchar_t* program) { // 2488
	FILE* f = _error ? stderr : stdout;

	fprintf(f, usageLine, program);

	if (_error) {
		fprintf(f, "لمزيد من المعلومات قم بكتابة `alif -م'.\n");
	}
	else {
		fputs(usageHelp, f);
	}
}

static void config_envVarsUsage(void) { // 2500
	//printf(_usageEnvVars_, (wint_t)DELIM, (wint_t)DELIM, ALIFHOMEHELP); //* todo
}

static void config_xoptionsUsage(void) { // 2505
	//puts(_usageXoptions_); //* todo
}

static void config_completeUsage(const wchar_t* _program) { // 2510
	config_usage(0, _program);
	putchar('\n');
	config_envVarsUsage();
	putchar('\n');
	config_xoptionsUsage();
}




/* ----------------------------------- AlifStatus ----------------------------------- */

AlifStatus alifStatus_ok(void) { // 533
	return ALIFSTATUS_OK();
}

AlifStatus alifStatus_error(const char* _errMsg) { // 536
	return {
		.type = AlifStatus::AlifStatus_Type_ERROR,
		.errMsg = _errMsg
	};
}

AlifStatus alifStatus_noMemory(void) { // 543
	return alifStatus_error("لم يتمكن من الحجز في الذاكرة");
}

AlifStatus alifStatus_exit(AlifIntT _exitcode) { // 546
	return ALIFSTATUS_EXIT(_exitcode);
}

AlifIntT alifStatus_exception(AlifStatus _status) { // 556
	return ALIFSTATUS_EXCEPTION(_status);
}




void _alifWStringList_clear(AlifWStringList* _list) { // 608
	for (AlifSizeT i = 0; i < _list->length; i++) {
		alifMem_dataFree(_list->items[i]);
	}

	alifMem_dataFree(_list->items);
	_list->length = 0;
	_list->items = nullptr;
}

AlifIntT _alifWStringList_copy(AlifWStringList* _list, const AlifWStringList* _list2) { // 529

	if (_list2->length == 0) {
		_alifWStringList_clear(_list);
		return 0;
	}

	AlifWStringList copy = ALIFWIDESTRINGLIST_INIT;

	AlifUSizeT size = _list2->length * sizeof(_list2->items[0]);
	copy.items = (wchar_t**)alifMem_dataAlloc(size);
	if (copy.items == nullptr) {
		return -1;
	}

	for (AlifSizeT i = 0; i < _list2->length; i++) {
		wchar_t* item = alifMem_wcsDup(_list2->items[i]);
		if (item == nullptr) {
			_alifWStringList_clear(&copy);
			return -1;
		}
		copy.items[i] = item;
		copy.length = i + 1;
	}

	_alifWStringList_clear(_list);
	*_list = copy;
	return 0;
}


AlifStatus alifWStringList_insert(AlifWStringList* _list,
	AlifSizeT _index, const wchar_t* _item) { // 564
	AlifSizeT len = _list->length;
	if (len == ALIF_SIZET_MAX) {
		/* length+1 would overflow */
		return ALIFSTATUS_NO_MEMORY();
	}
	if (_index < 0) {
		return ALIFSTATUS_ERR("AlifWStringList_Insert index must be >= 0");
	}
	if (_index > len) {
		_index = len;
	}

	wchar_t* item2 = alifMem_wcsDup(_item);
	if (item2 == nullptr) {
		return ALIFSTATUS_NO_MEMORY();
	}

	AlifUSizeT size = (len + 1) * sizeof(_list->items[0]);
	wchar_t** items2 = (wchar_t**)alifMem_dataRealloc(_list->items, size);
	if (items2 == nullptr) {
		alifMem_dataFree(item2);
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

AlifStatus alifWStringList_append(AlifWStringList* _list, const wchar_t* _item) { // 605
	return alifWStringList_insert(_list, _list->length, _item);
}

AlifStatus _alifWStringList_extend(AlifWStringList* _list, const AlifWStringList* _list2) { // 689
	for (AlifSizeT i = 0; i < _list2->length; i++) {
		AlifStatus status = alifWStringList_append(_list, _list2->items[i]);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}
	return ALIFSTATUS_OK();
}

static AlifIntT alif_setArgcArgv(AlifSizeT _argc, wchar_t* const* _argv) { // 673
	const AlifWStringList argv_list = { .length = _argc, .items = (wchar_t**)_argv };
	AlifIntT res{};

	// XXX _alifRuntime_.origArgv only gets cleared by alif_main(),
	// so it currently leaks for embedders.
	res = _alifWStringList_copy(&_alifRuntime_.origArgv, &argv_list);

	return res;
}


AlifObject* _alifWStringList_asList(const AlifWStringList* _list) { // 729
	AlifObject* alifList = alifList_new(_list->length);
	if (alifList == nullptr) {
		return nullptr;
	}

	for (AlifSizeT i = 0; i < _list->length; i++) {
		AlifObject* item = alifUStr_fromWideChar(_list->items[i], -1);
		if (item == nullptr) {
			ALIF_DECREF(alifList);
			return nullptr;
		}
		ALIFLIST_SET_ITEM(alifList, i, item);
	}
	return alifList;
}

static AlifObject* _alifWStringList_asTuple(const AlifWStringList* _list) { // 751
	AlifObject* tuple = alifTuple_new(_list->length);
	if (tuple == nullptr) {
		return nullptr;
	}

	for (AlifSizeT i = 0; i < _list->length; i++) {
		AlifObject* item = alifUStr_fromWideChar(_list->items[i], -1);
		if (item == nullptr) {
			ALIF_DECREF(tuple);
			return nullptr;
		}
		ALIFTUPLE_SET_ITEM(tuple, i, item);
	}
	return tuple;
}


/* ---------------------------------------- AlifConfig ---------------------------------------- */

#define MAX_HASH_SEED 4294967295UL


void alifConfig_clear(AlifConfig* _config) { // 773
#define CLEAR(_attr) \
    do { \
        alifMem_dataFree(_attr); \
        _attr = nullptr; \
    } while (0)

	CLEAR(_config->alifCachePrefix);
	CLEAR(_config->alifPathEnv);
	CLEAR(_config->home);
	CLEAR(_config->programName);

	_alifWStringList_clear(&_config->argv);
	_alifWStringList_clear(&_config->warnoptions);
	_alifWStringList_clear(&_config->xoptions);
	_alifWStringList_clear(&_config->moduleSearchPaths);
	_config->moduleSearchPathsSet = 0;
	CLEAR(_config->stdLibDir);

	CLEAR(_config->executable);
	CLEAR(_config->baseExecutable);
	CLEAR(_config->prefix);
	CLEAR(_config->basePrefix);
	CLEAR(_config->execPrefix);
	CLEAR(_config->baseExecPrefix);
	CLEAR(_config->platLibDir);
	CLEAR(_config->sysPath0);

	CLEAR(_config->fileSystemEncoding);
	CLEAR(_config->fileSystemErrors);
	CLEAR(_config->stdioEncoding);
	CLEAR(_config->stdioErrors);
	CLEAR(_config->runCommand);
	CLEAR(_config->runModule);
	CLEAR(_config->runFilename);
	CLEAR(_config->checkHashAlifCSMode);
#ifdef ALIF_DEBUG
	CLEAR(_config->runPreSite);
#endif

	_alifWStringList_clear(&_config->origArgv);
#undef CLEAR
}


void _alifConfig_initCompatConfig(AlifConfig* _config) { // 934
	memset(_config, 0, sizeof(*_config));

	_config->configInit = (AlifIntT)ConfigInitEnum_::AlifConfig_Init_COMPAT;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->devMode = -1;
	_config->installSignalHandlers = 1;
	_config->useHashSeed = -1;
	_config->faultHandler = -1;
	_config->tracemalloc = -1;
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
#ifdef _WINDOWS
	_config->legacyWindowsStdio = -1;
#endif
#ifdef ALIF_DEBUG
	config->useFrozenModules = 0;
#else
	_config->useFrozenModules = 1;
#endif
	_config->safePath = 0;
	_config->intMaxStrDigits = -1;
	_config->isAlifBuild = 0;
	_config->codeDebugRanges = 1;
	_config->cpuCount = -1;
}


static void config_initDefaults(AlifConfig* _config) { // 986
	_alifConfig_initCompatConfig(_config);

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
#ifdef _WINDOWS
	_config->legacyWindowsStdio = 0;
#endif
}


void alifConfig_initAlifConfig(AlifConfig* config) { // 1011
	config_initDefaults(config);

	config->configInit = (AlifIntT)ConfigInitEnum_::AlifConfig_Init_ALIF;
	config->configureCStdio = 1;
	config->parseArgv = 1;
}

void alifConfig_initIsolatedConfig(AlifConfig* _config) { // 1022
	config_initDefaults(_config);

	_config->configInit = (AlifIntT)ConfigInitEnum_::AlifConfig_Init_ISOLATED;
	_config->isolated = 1;
	_config->useEnvironment = 0;
	_config->userSiteDirectory = 0;
	_config->devMode = 0;
	_config->installSignalHandlers = 0;
	_config->useHashSeed = 0;
	_config->tracemalloc = 0;
	_config->perfProfiling = 0;
	_config->intMaxStrDigits = ALIF_LONG_DEFAULT_MAX_STR_DIGITS;
	_config->safePath = 1;
	_config->pathConfigWarnings = 0;
#ifdef _WINDOWS
	_config->legacyWindowsStdio = 0;
#endif
}

/* duplicate the string */
AlifStatus alifConfig_setString(AlifConfig* _config,
	wchar_t** _configStr, const wchar_t* _str) { // 933

	AlifStatus status = _alif_preInitializeFromConfig(_config, nullptr);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	wchar_t* str2{};
	if (_str != nullptr) {
		str2 = alifMem_wcsDup(_str);
		if (str2 == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}
	else {
		str2 = nullptr;
	}

	alifMem_dataFree(*_configStr);
	*_configStr = str2;
	return ALIFSTATUS_OK();
}


static AlifStatus config_setBytesString(AlifConfig* _config, wchar_t** _configStr,
	const char* _str, const char* _decodeErrMsg) { // 1042
	AlifStatus status = _alif_preInitializeFromConfig(_config, nullptr);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	wchar_t* str2{};
	if (_str != nullptr) {
		AlifUSizeT len{};
		str2 = alif_decodeLocale(_str, &len);
		if (str2 == nullptr) {
			if (len == (AlifUSizeT)-2) {
				return ALIFSTATUS_ERR(_decodeErrMsg);
			}
			else {
				return  ALIFSTATUS_NO_MEMORY();
			}
		}
	}
	else {
		str2 = nullptr;
	}
	alifMem_dataFree(*_configStr);
	*_configStr = str2;
	return ALIFSTATUS_OK();
}


#define CONFIG_SET_BYTES_STR(config, config_str, str, NAME) \
    config_setBytesString(config, config_str, str, "cannot decode " NAME)


static inline void* config_getSpecMember(const AlifConfig* _config,
	const AlifConfigSpec* _spec) { // 1117
	return (char*)_config + _spec->offset;
}


AlifStatus _alifConfig_copy(AlifConfig* _config, const AlifConfig* _config2) { // 1097
	alifConfig_clear(_config);

	AlifStatus status{};
	const AlifConfigSpec* spec = _alifConfigSpec_;
	for (; spec->name != nullptr; spec++) {
		void* member = config_getSpecMember(_config, spec);
		const void* member2 = config_getSpecMember((AlifConfig*)_config2, spec);
		switch (spec->type) {
		case Alif_Config_Member_INT:
		case Alif_Config_Member_UINT:
		case Alif_Config_Member_BOOL:
		{
			*(int*)member = *(int*)member2;
			break;
		}
		case Alif_Config_Member_ULONG:
		{
			*(unsigned long*)member = *(unsigned long*)member2;
			break;
		}
		case Alif_Config_Member_WSTR:
		case Alif_Config_Member_WSTR_OPT:
		{
			const wchar_t* str = *(const wchar_t**)member2;
			status = alifConfig_setString(_config, (wchar_t**)member, str);
			if (ALIFSTATUS_EXCEPTION(status)) return status;
			break;
		}
		case Alif_Config_Member_WSTR_LIST:
		{
			if (_alifWStringList_copy((AlifWStringList*)member,
				(const AlifWStringList*)member2) < 0) {
				return ALIFSTATUS_NO_MEMORY();
			}
			break;
		}
		default:
			ALIF_UNREACHABLE();
		}
	}
	return ALIFSTATUS_OK();
}


AlifObject* _alifConfig_asDict(const AlifConfig* _config) { // 1179
	AlifObject* dict = alifDict_new();
	if (dict == nullptr) {
		return nullptr;
	}

	const AlifConfigSpec* spec = _alifConfigSpec_;
	for (; spec->name != nullptr; spec++) {
		AlifObject* obj = config_get(_config, spec, 0);
		if (obj == nullptr) {
			ALIF_DECREF(dict);
			return nullptr;
		}

		AlifIntT res = alifDict_setItemString(dict, spec->name, obj);
		ALIF_DECREF(obj);
		if (res < 0) {
			ALIF_DECREF(dict);
			return nullptr;
		}
	}
	return dict;
}


static void config_dictInvalidValue(const char* _name) { // 1206
	alifErr_format(_alifExcValueError_, "invalid config value: %s", _name);
}

static AlifIntT config_dictGetInt(AlifObject* _dict,
	const char* _name, AlifIntT* _result) { // 1213
	AlifObject* item = config_dictGet(_dict, _name);
	if (item == nullptr) {
		return -1;
	}
	AlifIntT value = alifLong_asInt(item);
	ALIF_DECREF(item);
	if (value == -1 and alifErr_occurred()) {
		if (alifErr_exceptionMatches(_alifExcTypeError_)) {
			config_dictInvalidType(_name);
		}
		else if (alifErr_exceptionMatches(_alifExcOverflowError_)) {
			config_dictInvalidValue(_name);
		}
		return -1;
	}
	*_result = value;
	return 0;
}


static AlifIntT config_dictGetULong(AlifObject* dict,
	const char* name, unsigned long* result) { // 1236
	AlifObject* item = config_dictGet(dict, name);
	if (item == nullptr) {
		return -1;
	}
	unsigned long value = alifLong_asUnsignedLong(item);
	ALIF_DECREF(item);
	if (value == (unsigned long)-1 and alifErr_occurred()) {
		if (alifErr_exceptionMatches(_alifExcTypeError_)) {
			config_dictInvalidType(name);
		}
		else if (alifErr_exceptionMatches(_alifExcOverflowError_)) {
			config_dictInvalidValue(name);
		}
		return -1;
	}
	*result = value;
	return 0;
}

static AlifIntT config_dictGetWStr(AlifObject* _dict, const char* _name,
	AlifConfig* _config, wchar_t** _result) { // 1259
	AlifObject* item = config_dictGet(_dict, _name);
	if (item == nullptr) {
		return -1;
	}

	AlifStatus status{};
	if (item == ALIF_NONE) {
		status = alifConfig_setString(_config, _result, nullptr);
	}
	else if (!ALIFUSTR_CHECK(item)) {
		config_dictInvalidType(_name);
		goto error;
	}
	else {
		wchar_t* wstr = alifUStr_asWideCharString(item, nullptr);
		if (wstr == nullptr) {
			goto error;
		}
		status = alifConfig_setString(_config, _result, wstr);
		alifMem_dataFree(wstr);
	}
	if (ALIFSTATUS_EXCEPTION(status)) {
		//alifErr_noMemory();
		goto error;
	}
	ALIF_DECREF(item);
	return 0;

error:
	ALIF_DECREF(item);
	return -1;
}

static AlifIntT config_dictGetWStrList(AlifObject* _dict, const char* _name,
	AlifConfig* _config, AlifWStringList* _result) { // 1297
	AlifObject* list = config_dictGet(_dict, _name);
	if (list == nullptr) {
		return -1;
	}

	AlifIntT isList = ALIFLIST_CHECKEXACT(list);
	if (!isList and !ALIFTUPLE_CHECKEXACT(list)) {
		ALIF_DECREF(list);
		config_dictInvalidType(_name);
		return -1;
	}

	AlifWStringList wstrlist = ALIFWIDESTRINGLIST_INIT;
	AlifSizeT len = isList ? ALIFLIST_GET_SIZE(list) : ALIFTUPLE_GET_SIZE(list);
	for (AlifSizeT i = 0; i < len; i++) {
		AlifObject* item = isList ? ALIFLIST_GET_ITEM(list, i) : ALIFTUPLE_GET_ITEM(list, i);

		if (item == ALIF_NONE) {
			config_dictInvalidValue(_name);
			goto error;
		}
		else if (!ALIFUSTR_CHECK(item)) {
			config_dictInvalidType(_name);
			goto error;
		}
		wchar_t* wstr = alifUStr_asWideCharString(item, nullptr);
		if (wstr == nullptr) {
			goto error;
		}
		AlifStatus status = alifWStringList_append(&wstrlist, wstr);
		alifMem_dataFree(wstr);
		if (ALIFSTATUS_EXCEPTION(status)) {
			//alifErr_noMemory();
			goto error;
		}
	}

	if (_alifWStringList_copy(_result, &wstrlist) < 0) {
		//alifErr_noMemory();
		goto error;
	}
	_alifWStringList_clear(&wstrlist);
	ALIF_DECREF(list);
	return 0;

error:
	_alifWStringList_clear(&wstrlist);
	ALIF_DECREF(list);
	return -1;
}

AlifIntT _alifConfig_fromDict(AlifConfig* _config, AlifObject* _dict) { // 1413
	if (!ALIFDICT_CHECK(_dict)) {
		alifErr_setString(_alifExcTypeError_, "dict expected");
		return -1;
	}

	const AlifConfigSpec* spec = _alifConfigSpec_;
	for (; spec->name != nullptr; spec++) {
		char* member = (char*)_config + spec->offset;
		switch (spec->type) {
		case AlifConfigMemberType_::Alif_Config_Member_INT:
		case AlifConfigMemberType_::Alif_Config_Member_UINT:
		case AlifConfigMemberType_::Alif_Config_Member_BOOL:
		{
			AlifIntT value{};
			if (config_dictGetInt(_dict, spec->name, &value) < 0) {
				return -1;
			}
			if (spec->type == AlifConfigMemberType_::Alif_Config_Member_BOOL
				or spec->type == AlifConfigMemberType_::Alif_Config_Member_UINT) {
				if (value < 0) {
					config_dictInvalidValue(spec->name);
					return -1;
				}
			}
			*(AlifIntT*)member = value;
			break;
		}
		case AlifConfigMemberType_::Alif_Config_Member_ULONG:
		{
			if (config_dictGetULong(_dict, spec->name,
				(unsigned long*)member) < 0) {
				return -1;
			}
			break;
		}
		case AlifConfigMemberType_::Alif_Config_Member_WSTR:
		{
			wchar_t** wstr = (wchar_t**)member;
			if (config_dictGetWStr(_dict, spec->name, _config, wstr) < 0) {
				return -1;
			}
			if (*wstr == nullptr) {
				config_dictInvalidValue(spec->name);
				return -1;
			}
			break;
		}
		case AlifConfigMemberType_::Alif_Config_Member_WSTR_OPT:
		{
			wchar_t** wstr = (wchar_t**)member;
			if (config_dictGetWStr(_dict, spec->name, _config, wstr) < 0) {
				return -1;
			}
			break;
		}
		case AlifConfigMemberType_::Alif_Config_Member_WSTR_LIST:
		{
			if (strcmp(spec->name, "xoptions") == 0) {
				//if (config_dictGetXoptions(_dict, spec->name, _config,
				//	(AlifWStringList*)member) < 0) {
				//	return -1;
				//}
			}
			else {
				if (config_dictGetWStrList(_dict, spec->name, _config,
					(AlifWStringList*)member) < 0) {
					return -1;
				}
			}
			break;
		}
		default:
			ALIF_UNREACHABLE();
		}
	}

	if (!(_config->configInit == ConfigInitEnum_::AlifConfig_Init_COMPAT
		or _config->configInit == ConfigInitEnum_::AlifConfig_Init_ALIF
		or _config->configInit == ConfigInitEnum_::AlifConfig_Init_ISOLATED)) {
		config_dictInvalidValue("_config_init");
		return -1;
	}

	if (_config->hashSeed > MAX_HASH_SEED) {
		config_dictInvalidValue("hash_seed");
		return -1;
	}
	return 0;
}


static const char* config_getEnv(const AlifConfig* _config, const char* _name) { // 1465
	return _alif_getEnv(_config->useEnvironment, _name);
}


static AlifStatus config_getEnvDup(AlifConfig* _config,
	wchar_t** _dest, const wchar_t* _wname, const char* _name,
	const char* _decodeErrMsg) { // 1474

	if (!_config->useEnvironment) {
		*_dest = nullptr;
		return ALIFSTATUS_OK();
	}

#ifdef _WINDOWS
	const wchar_t* var = _wgetenv(_wname);
	if (!var or var[0] == '\0') {
		*_dest = nullptr;
		return ALIFSTATUS_OK();
	}

	return alifConfig_setString(_config, _dest, var);
#else
	const char* var = getenv(_name);
	if (!var or var[0] == '\0') {
		*_dest = nullptr;
		return ALIFSTATUS_OK();
	}

	return config_setBytesString(_config, _dest, var, _decodeErrMsg);
#endif
}


#define CONFIG_GET_ENV_DUP(_config, _dest, _wname, _name) \
    config_getEnvDup(_config, _dest, _wname, _name, "cannot decode " _name)


static void config_getGlobalVars(AlifConfig* _config) { // 1511
	ALIF_COMP_DIAG_PUSH;
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS;
	if (_config->configInit != AlifConfig_Init_COMPAT) {
		return;
	}

#define COPY_FLAG(_attr, _value) \
        if (_config->_attr == -1) { \
            _config->_attr = _value; \
        }
#define COPY_NOT_FLAG(_attr, _value) \
        if (_config->_attr == -1) { \
            _config->_attr = !(_value); \
        }

	COPY_FLAG(isolated, _alifIsolatedFlag_);
	COPY_NOT_FLAG(useEnvironment, _alifIgnoreEnvironmentFlag_);
	COPY_FLAG(bytesWarning, _alifBytesWarningFlag_);
	COPY_FLAG(inspect, _alifInspectFlag_);
	COPY_FLAG(interactive, _alifInteractiveFlag_);
	COPY_FLAG(optimizationLevel, _alifOptimizeFlag_);
	COPY_FLAG(parserDebug, _alifDebugFlag_);
	COPY_FLAG(verbose, _alifVerboseFlag_);
	COPY_FLAG(quiet, _alifQuietFlag_);
#ifdef _WINDOWS
	COPY_FLAG(legacyWindowsStdio, _alifLegacyWindowsStdioFlag_);
#endif
	COPY_NOT_FLAG(pathConfigWarnings, _alifFrozenFlag_);

	COPY_NOT_FLAG(bufferedStdio, _alifUnbufferedStdioFlag_);
	COPY_NOT_FLAG(siteImport, _alifNoSiteFlag_);
	COPY_NOT_FLAG(writeBytecode, _alifDontWriteBytecodeFlag_);
	COPY_NOT_FLAG(userSiteDirectory, _alifNoUserSiteDirectory_);

#undef COPY_FLAG
#undef COPY_NOT_FLAG
	ALIF_COMP_DIAG_POP
}

static void config_setGlobalVars(const AlifConfig* _config) { // 1555
	ALIF_COMP_DIAG_PUSH;
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS;
#define COPY_FLAG(_attr, _var) \
        if (_config->_attr != -1) { \
            _var = _config->_attr; \
        }
#define COPY_NOT_FLAG(_attr, _var) \
        if (_config->_attr != -1) { \
            _var = !_config->_attr; \
        }

	COPY_FLAG(isolated, _alifIsolatedFlag_);
	COPY_NOT_FLAG(useEnvironment, _alifIgnoreEnvironmentFlag_);
	COPY_FLAG(bytesWarning, _alifBytesWarningFlag_);
	COPY_FLAG(inspect, _alifInspectFlag_);
	COPY_FLAG(interactive, _alifInteractiveFlag_);
	COPY_FLAG(optimizationLevel, _alifOptimizeFlag_);
	COPY_FLAG(parserDebug, _alifDebugFlag_);
	COPY_FLAG(verbose, _alifVerboseFlag_);
	COPY_FLAG(quiet, _alifQuietFlag_);
#ifdef _WINDOWS
	COPY_FLAG(legacyWindowsStdio, _alifLegacyWindowsStdioFlag_);
#endif
	COPY_NOT_FLAG(pathConfigWarnings, _alifFrozenFlag_);

	COPY_NOT_FLAG(bufferedStdio, _alifUnbufferedStdioFlag_);
	COPY_NOT_FLAG(siteImport, _alifNoSiteFlag_);
	COPY_NOT_FLAG(writeBytecode, _alifDontWriteBytecodeFlag_);
	COPY_NOT_FLAG(userSiteDirectory, _alifNoUserSiteDirectory_);

	/* Random or non-zero hash seed */
	_alifHashRandomizationFlag_ = (_config->useHashSeed == 0 or
		_config->hashSeed != 0);

#undef COPY_FLAG
#undef COPY_NOT_FLAG
	ALIF_COMP_DIAG_POP
}

static const wchar_t* config_getXOption(const AlifConfig* _config, const wchar_t* _name) { // 1597
	return _alif_getXOption(&_config->xoptions, _name);
}

static const wchar_t* config_getXOptionValue(const AlifConfig* _config,
	const wchar_t* _name) { // 1602
	const wchar_t* xoption = config_getXOption(_config, _name);
	if (xoption == nullptr) {
		return nullptr;
	}
	const wchar_t* sep = wcschr(xoption, L'=');
	return sep ? sep + 1 : L"";
}


static AlifStatus config_initIntMaxStrDigits(AlifConfig* _config) { // 1944
	AlifIntT maxdigits{};

	//* todo


	if (_config->intMaxStrDigits < 0) {
		_config->intMaxStrDigits = ALIF_LONG_DEFAULT_MAX_STR_DIGITS;
	}
	return ALIFSTATUS_OK();
}

static AlifStatus config_readComplexOptions(AlifConfig* _config) { // 2049

	AlifStatus status{};

	if (_config->intMaxStrDigits < 0) {
		status = config_initIntMaxStrDigits(_config);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}


	return ALIFSTATUS_OK();
}

static const wchar_t* config_getStdioErrors(const AlifPreConfig* _preConfig) { // 2056
	if (_preConfig->utf8Mode) {
		/* UTF-8 Mode uses UTF-8/surrogateescape */
		return L"surrogateescape";
	}

#ifndef _WINDOWS
	const char* loc = setlocale(LC_CTYPE, nullptr);
	if (loc != nullptr) {
		/* surrogateescape is the default in the legacy C and POSIX locales */
		if (strcmp(loc, "C") == 0 or strcmp(loc, "POSIX") == 0) {
			return L"surrogateescape";
		}

	#ifdef ALIF_COERCE_C_LOCALE
		/* surrogateescape is the default in locale coercion target locales */
		if (_alif_isLocaleCoercionTarget(loc)) {
			return L"surrogateescape";
		}
	#endif
	}

	return L"strict";
#else
	/* On Windows, always use surrogateescape by default */
	return L"surrogateescape";
#endif
}

static AlifStatus config_getLocaleEncoding(AlifConfig* _config,
	const AlifPreConfig* _preConfig, wchar_t** _localeEncoding) { // 2088
	wchar_t* encoding{};
	if (_preConfig->utf8Mode) {
		encoding = alifMem_wcsDup(L"utf-8");
	}
	else {
		encoding = _alif_getLocaleEncoding();
	}
	if (encoding == nullptr) {
		return ALIFSTATUS_NO_MEMORY();
	}
	AlifStatus status = alifConfig_setString(_config, _localeEncoding, encoding);
	alifMem_dataFree(encoding);
	return status;
}

static AlifStatus config_initStdioEncoding(AlifConfig* _config,
	const AlifPreConfig* _preConfig) { // 2107
	AlifStatus status{};

	// Exit if encoding and errors are defined
	if (_config->stdioEncoding != nullptr and _config->stdioErrors != nullptr) {
		return ALIFSTATUS_OK();
	}

	/* ALIFIOENCODING environment variable */
	const char* opt = config_getEnv(_config, "ALIFIOENCODING");
	if (opt) {
		char* alifIOEncoding = alifMem_strDup(opt);
		if (alifIOEncoding == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}

		char* errors = strchr(alifIOEncoding, ':');
		if (errors) {
			*errors = '\0';
			errors++;
			if (!errors[0]) {
				errors = nullptr;
			}
		}

		/* Does ALIFIOENCODING contain an encoding? */
		if (alifIOEncoding[0]) {
			if (_config->stdioEncoding == nullptr) {
				status = CONFIG_SET_BYTES_STR(_config, &_config->stdioEncoding,
					alifIOEncoding,
					"ALIFIOENCODING environment variable");
				if (ALIFSTATUS_EXCEPTION(status)) {
					alifMem_dataFree(alifIOEncoding);
					return status;
				}
			}

			/* If the encoding is set but not the error handler,
			   use "strict" error handler by default.
			   ALIFIOENCODING=latin1 behaves as
			   ALIFIOENCODING=latin1:strict. */
			if (!errors) {
				errors = (char*)"strict";
			}
		}

		if (_config->stdioErrors == nullptr and errors != nullptr) {
			status = CONFIG_SET_BYTES_STR(_config, &_config->stdioErrors,
				errors,
				"ALIFIOENCODING environment variable");
			if (ALIFSTATUS_EXCEPTION(status)) {
				alifMem_dataFree(alifIOEncoding);
				return status;
			}
		}

		alifMem_dataFree(alifIOEncoding);
	}

	/* Choose the default error handler based on the current locale. */
	if (_config->stdioEncoding == nullptr) {
		status = config_getLocaleEncoding(_config, _preConfig,
			&_config->stdioEncoding);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}
	if (_config->stdioErrors == nullptr) {
		const wchar_t* errors = config_getStdioErrors(_preConfig);

		status = alifConfig_setString(_config, &_config->stdioErrors, errors);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	return ALIFSTATUS_OK();
}

static AlifStatus config_getFSEncoding(AlifConfig* _config,
	const AlifPreConfig* _preConfig, wchar_t** _fsEncoding) { // 2191
#ifdef ALIF_FORCE_UTF8_FS_ENCODING
	return alifConfig_setString(_config, _fsEncoding, L"utf-8");
#elif defined(_WINDOWS)
	const wchar_t* encoding;
	if (_preConfig->legacyWindowsFSEncoding) {
		// Legacy Windows filesystem encoding: mbcs/replace
		encoding = L"mbcs";
	}
	else {
		// Windows defaults to utf-8/surrogatepass
		encoding = L"utf-8";
	}
	return alifConfig_setString(_config, _fsEncoding, encoding);
#else  // !_WINDOWS
	if (_preConfig->utf8Mode) {
		return alifConfig_setString(_config, _fsEncoding, L"utf-8");
	}

	//if (_alif_getForceASCII()) {
	//	return alifConfig_setString(_config, _fsEncoding, L"ascii");
	//}

	return config_getLocaleEncoding(_config, _preConfig, _fsEncoding);
#endif  // !_WINDOWS
}

static AlifStatus config_initFSEncoding(AlifConfig* _config,
	const AlifPreConfig* _preConfig) { // 2221
	AlifStatus status{};

	if (_config->fileSystemEncoding == nullptr) {
		status = config_getFSEncoding(_config, _preConfig,
			&_config->fileSystemEncoding);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	if (_config->fileSystemErrors == nullptr) {
		const wchar_t* errors;
	#ifdef _WINDOWS
		if (_preConfig->legacyWindowsFSEncoding) {
			errors = L"replace";
		}
		else {
			errors = L"surrogatepass";
		}
	#else
		errors = L"surrogateescape";
	#endif
		status = alifConfig_setString(_config, &_config->fileSystemErrors, errors);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}
	return ALIFSTATUS_OK();
}

static AlifStatus config_initImport(AlifConfig* _config, AlifIntT _computePathConfig) { // 2321
	AlifStatus status{};

	status = _alifConfig_initPathConfig(_config, _computePathConfig); //* here
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	const char* env = config_getEnv(_config, "ALIF_FROZEN_MODULES");
	if (env == nullptr) {
	}
	else if (strcmp(env, "on") == 0) {
		_config->useFrozenModules = 1;
	}
	else if (strcmp(env, "off") == 0) {
		_config->useFrozenModules = 0;
	}
	else {
		return ALIFSTATUS_ERR("bad value for ALIF_FROZEN_MODULES "
			"(expected \"on\" or \"off\")");
	}

	const wchar_t* value = config_getXOptionValue(_config, L"frozen_modules");
	if (value == nullptr) {
	}
	else if (wcscmp(value, L"on") == 0) {
		_config->useFrozenModules = 1;
	}
	else if (wcscmp(value, L"off") == 0) {
		_config->useFrozenModules = 0;
	}
	else if (wcslen(value) == 0) {
		_config->useFrozenModules = 1;
	}
	else {
		return ALIFSTATUS_ERR("bad value for option -X frozen_modules "
			"(expected \"on\" or \"off\")");
	}

	return ALIFSTATUS_OK();
}

AlifStatus _alifConfig_initImportConfig(AlifConfig* _config) { // 2367
	return config_initImport(_config, 1);
}


static AlifStatus config_read(AlifConfig* _config, AlifIntT _computepathConfig) { // 2306
	AlifStatus status{};
	const AlifPreConfig* preconfig = &_alifRuntime_.preConfig;

	//if (_config->useEnvironment) {
	//	status = config_readEnvVars(_config);
	//	if (ALIFSTATUS_EXCEPTION(status)) return status;
	//}


	status = config_readComplexOptions(_config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	if (_config->installImportLib) {
		status = config_initImport(_config, _computepathConfig); //* here
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	/* default values */
	if (_config->devMode) {
		if (_config->faultHandler < 0) {
			_config->faultHandler = 1;
		}
	}
	if (_config->faultHandler < 0) {
		_config->faultHandler = 0;
	}
	if (_config->tracemalloc < 0) {
		_config->tracemalloc = 1;
	}
	if (_config->perfProfiling < 0) {
		_config->perfProfiling = 0;
	}
	if (_config->useHashSeed < 0) {
		_config->useHashSeed = 0;
		_config->hashSeed = 0;
	}

	if (_config->fileSystemEncoding == nullptr or _config->fileSystemErrors == nullptr) {
		status = config_initFSEncoding(_config, preconfig);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	status = config_initStdioEncoding(_config, preconfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	if (_config->argv.length < 1) {
		/* Ensure at least one (empty) argument is seen */
		status = alifWStringList_append(&_config->argv, L"");
		if (ALIFSTATUS_EXCEPTION(status)) return status;
	}

	if (_config->checkHashAlifCSMode == nullptr) {
		status = alifConfig_setString(_config, &_config->checkHashAlifCSMode,
			L"عادي");
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	if (_config->configureCStdio < 0) {
		_config->configureCStdio = 1;
	}

	// Only parse arguments once.
	if (_config->parseArgv == 1) {
		_config->parseArgv = 2;
	}

	return ALIFSTATUS_OK();
}

static void config_initStdio(const AlifConfig* _config) { // 2327

#if defined(_WINDOWS) or defined(__CYGWIN__)
	/* don't translate newlines (\r\n <=> \n) */
	_setmode(fileno(stdin), O_BINARY);
	_setmode(fileno(stdout), O_BINARY);
	_setmode(fileno(stderr), O_BINARY);
#endif

	if (!_config->bufferedStdio) {
	#ifdef HAVE_SETVBUF
		setvbuf(stdin, (char*)nullptr, _IONBF, BUFSIZ);
		setvbuf(stdout, (char*)nullptr, _IONBF, BUFSIZ);
		setvbuf(stderr, (char*)nullptr, _IONBF, BUFSIZ);
	#else /* !HAVE_SETVBUF */
		setbuf(stdin, (char*)nullptr);
		setbuf(stdout, (char*)nullptr);
		setbuf(stderr, (char*)nullptr);
	#endif /* !HAVE_SETVBUF */
	}
	else if (_config->interactive) {
	#ifdef _WINDOWS
		/* Doesn't have to have line-buffered -- use unbuffered */
		setvbuf(stdout, (char*)nullptr, _IONBF, BUFSIZ);
	#else /* !_WINDOWS */
	#ifdef HAVE_SETVBUF
		setvbuf(stdin, (char*)nullptr, _IOLBF, BUFSIZ);
		setvbuf(stdout, (char*)nullptr, _IOLBF, BUFSIZ);
	#endif /* HAVE_SETVBUF */
	#endif /* !_WINDOWS */
		/* Leave stderr alone - it should be unbuffered. */
	}
}

AlifStatus alifConfig_write(const AlifConfig* _config, AlifRuntime* _runtime) { // 2458
	config_setGlobalVars(_config);

	if (_config->configureCStdio) {
		config_initStdio(_config);
	}

	/* Write the new pre-configuration into _alifRuntime_ */
	AlifPreConfig* preconfig = &_runtime->preConfig;
	preconfig->isolated = _config->isolated;
	preconfig->useEnvironment = _config->useEnvironment;
	preconfig->devMode = _config->devMode;

	if (alif_setArgcArgv(_config->origArgv.length,
		_config->origArgv.items) < 0) {
		return ALIFSTATUS_NO_MEMORY();
	}

	return ALIFSTATUS_OK();
}




static AlifStatus config_parseCMDLine(AlifConfig* _config,
	AlifWStringList* _warnOptions, AlifSizeT* _index) { // 2521

	AlifStatus status{};
	const AlifWStringList* argv = &_config->argv;
	AlifIntT printVer{ 0 };
	const wchar_t* program = _config->programName;
	if (!program and argv->length >= 1) {
		program = argv->items[0];
	}

	_alifOS_resetGetOpt();
	do {
		AlifIntT longIndex = -1;
		AlifIntT c = _alifOS_getOpt(argv->length, argv->items, &longIndex);

		if (c == EOF) break;

		// تشغيل نص ك برنامج من موجه الأوامر
		// alif -ص "برنامج_هنا"
		if (c == L'ص') {
			if (_config->runCommand == nullptr) {
				AlifUSizeT len = wcslen(_alifOSOptArg_) + 1 + 1;
				wchar_t* command = (wchar_t*)alifMem_dataAlloc(len * sizeof(wchar_t));
				if (command == nullptr) {
					return ALIFSTATUS_NO_MEMORY();
				}
				memcpy(command, _alifOSOptArg_, (len - 2) * sizeof(wchar_t));
				command[len - 2] = '\n';
				command[len - 1] = 0;
				_config->runCommand = command;
			}
			break;
		}

		// تشغيل مكتبة ك برنامج من موجه الأوامر
		// alif -ك "اسم_المكتبة"
		if (c == L'ك') {
			if (_config->runModule == nullptr) {
				_config->runModule = alifMem_wcsDup(_alifOSOptArg_);
				if (_config->runModule == nullptr) {
					return ALIFSTATUS_NO_MEMORY();
				}
			}
			break;
		}

		if (c == 0) {
			if (wcscmp(_alifOSOptArg_, L"دائما") == 0
				or wcscmp(_alifOSOptArg_, L"ابدا") == 0
				or wcscmp(_alifOSOptArg_, L"عادي") == 0) {
				status = alifConfig_setString(_config, &_config->checkHashAlifCSMode,
					_alifOSOptArg_);
				if (ALIFSTATUS_EXCEPTION(status)) {
					return status;
				}
			}
			else {
				fprintf(stderr, "--checkHashAlifCSMode يجب ان يكون واحد من "
					"'عادي', 'دائما', او 'ابدا'\n");
				config_usage(1, program);
				return ALIFSTATUS_EXIT(2);
			}
			break;

		}
		else if (c == 1) {
			// help-all
			config_completeUsage(program);
			return ALIFSTATUS_EXIT(0);

		}
		else if (c == 2) {
			// help-env
			config_envVarsUsage();
			return ALIFSTATUS_EXIT(0);

		}
		else if (c == 3) {
			// help-xoptions
			config_xoptionsUsage();
			return ALIFSTATUS_EXIT(0);
		}
		//else if (c == L'ب') {
		//	_config->bytesWarning++;
		//	break;
		//}
		//else if (c == L'خ') {
		//	_config->parserDebug++;
		//	break;
		//}
		//else if (c == L'ت') {
		//	_config->inspect++;
		//	_config->interactive++;
		//	break;
		//}
		//else if (c == L'E' or c == L'I' or c == L'X') {
		//	break;
		//}
		else if (c == L'س') {
			_config->skipFirstLine = 1;
			break;
		}
		else if (c == L'م') {
			config_usage(0, program);
			return ALIFSTATUS_EXIT(0);
		}
		else if (c == L'ن') {
			printVer++;
			break;
		}
		else {
			// فشلت عملية تحليل سطر الاوامر
			config_usage(1, program);
			return ALIFSTATUS_EXIT(1);
		}

	}
	while (true);



	if (printVer) {
		printf("alif %s\n", alif_getVersion());
		return ALIFSTATUS_EXIT(0);
	}

	if (_config->runModule == nullptr and _config->runCommand == nullptr
		and _alifOSOptInd_ < argv->length and wcscmp(argv->items[_alifOSOptInd_], L"-") != 0
		and _config->runFilename == nullptr) {
		_config->runFilename = (wchar_t*)alifMem_wcsDup(argv->items[_alifOSOptInd_]);
		if (_config->runFilename == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	if (_config->runCommand != nullptr or
		_config->runModule != nullptr) {
		_alifOSOptInd_--;
	}

	*_index = _alifOSOptInd_;

	return ALIFSTATUS_OK();
}

#ifdef _WINDOWS
#  define WCSTOK wcstok_s
#else
#  define WCSTOK wcstok
#endif

static AlifStatus config_initEnvWarnOptions(AlifConfig* _config,
	AlifWStringList* _warnOptions) { // 2732

	AlifStatus status{};

	wchar_t* env = nullptr;
	status = CONFIG_GET_ENV_DUP(_config, &env,
		L"ALIFWARNINGS", "ALIFWARNINGS");
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	/* env var is not set or is empty */
	if (env == nullptr) {
		return ALIFSTATUS_OK();
	}


	wchar_t* warning, * context = nullptr;
	for (warning = WCSTOK(env, L",", &context);
		warning != nullptr;
		warning = WCSTOK(nullptr, L",", &context)) {
		status = alifWStringList_append(_warnOptions, warning);
		if (ALIFSTATUS_EXCEPTION(status)) {
			alifMem_dataFree(env);
			return status;
		}
	}
	alifMem_dataFree(env);
	return ALIFSTATUS_OK();
}


static AlifStatus config_updateArgv(AlifConfig* _config, AlifSizeT _index) { // 2803
	const AlifWStringList* cmdlineArgv = &_config->argv;
	AlifWStringList configArgv = ALIFWIDESTRINGLIST_INIT;
	if (cmdlineArgv->length <= _index) {
		AlifStatus status = alifWStringList_append(&configArgv, L"");
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}
	else {
		AlifWStringList slice{};
		slice.length = cmdlineArgv->length - _index;
		slice.items = &cmdlineArgv->items[_index];
		if (_alifWStringList_copy(&configArgv, &slice) < 0) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	wchar_t* arg1{};
	if (_config->runCommand != nullptr) {
		arg1 = (wchar_t*)L"-ص";
	}
	else if (_config->runModule != nullptr) {
		arg1 = (wchar_t*)L"-ك";
	}

	if (arg1 != nullptr) {
		arg1 = alifMem_wcsDup(arg1);
		if (arg1 == nullptr) {
			_alifWStringList_clear(&configArgv);
			return ALIFSTATUS_NO_MEMORY();
		}

		alifMem_dataFree(configArgv.items[0]);
		configArgv.items[0] = arg1;
	}

	_alifWStringList_clear(&_config->argv);
	_config->argv = configArgv;
	return ALIFSTATUS_OK();
}

static AlifStatus core_readPreCMDLine(AlifConfig* _config, AlifPreCmdline* _preCmdline) { // 2930
	AlifStatus status{};

	if (_config->parseArgv == 1) {
		if (_alifWStringList_copy(&_preCmdline->argv, &_config->argv) < 0) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	AlifPreConfig preconfig{};

	status = _alifPreConfig_initFromPreConfig(&preconfig, &_alifRuntime_.preConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	_alifPreConfig_getConfig(&preconfig, _config);

	status = _alifPreCMDLine_read(_preCmdline, &preconfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = _alifPreCMDLine_setConfig(_preCmdline, _config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	return ALIFSTATUS_OK();
}

static AlifIntT alif_extension(wchar_t* _filename) { //* alif
	const wchar_t* dotSuffix = wcsrchr(_filename, L'.');
	if (!dotSuffix) { return 0; }

	return wcscmp(dotSuffix, L".alif") == 0 or wcscmp(dotSuffix, L".الف") == 0;
}

static AlifStatus config_runFileNameAbsPath(AlifConfig* _config) { // 2963

	wchar_t* filename = _config->runFilename;
	if (!filename) {
		return ALIFSTATUS_OK();
	}

	// يتم التحقق من لاحقة الملف والتي يجب ان تكون .alif او .الف
	if (!alif_extension(filename)) {
		printf("%s \n\n", "تأكد من لاحقة الملف \n يجب ان ينتهي اسم الملف بـ .alif او .الف");
		return ALIFSTATUS_EXIT(0);
	}

#ifndef _WINDOWS
	if (_alif_isAbs(filename)) {
		/* path is already absolute */
		return ALIFSTATUS_OK();
	}
#endif

	wchar_t* absFilename{};
	if (_alif_absPath(filename, &absFilename) < 0) {
		/* failed to get the absolute path of the command line filename:
		   ignore the error, keep the relative path */
		return ALIFSTATUS_OK();
	}
	if (absFilename == nullptr) {
		return ALIFSTATUS_NO_MEMORY();
	}

	alifMem_dataFree(_config->runFilename);
	_config->runFilename = absFilename;
	return ALIFSTATUS_OK();
}


static AlifStatus config_readCMDLine(AlifConfig* _config) { // 2992

	AlifStatus status{};
	AlifWStringList cmdlineWarnOptions = ALIFWIDESTRINGLIST_INIT;
	AlifWStringList envWarnOptions = ALIFWIDESTRINGLIST_INIT;
	AlifWStringList sysWarnOptions = ALIFWIDESTRINGLIST_INIT;

	if (_config->parseArgv < 0) {
		_config->parseArgv = 1;
	}

	if (_config->parseArgv == 1) {

		AlifSizeT index{};

		// تقوم هذه الدالة بتحليل سطر الطرفية
		status = config_parseCMDLine(_config, &cmdlineWarnOptions, &index);
		if (ALIFSTATUS_EXCEPTION(status)) return status;

		// تقوم هذه الدالة بجلب المسار الخاص بتنفيذ الملف فقط
		status = config_runFileNameAbsPath(_config);
		if (ALIFSTATUS_EXCEPTION(status)) return status;

		// تقوم هذه الدالة بتحديث المعطيات الممررة عبر الطرفية
		status = config_updateArgv(_config, index);
		if (ALIFSTATUS_EXCEPTION(status)) return status;

	}
	else {
		// تقوم هذه الدالة بجلب المسار الخاص بتنفيذ الملف فقط
		status = config_runFileNameAbsPath(_config);
		if (ALIFSTATUS_EXCEPTION(status)) return status;
	}


	if (_config->useEnvironment) {
		status = config_initEnvWarnOptions(_config, &envWarnOptions);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}
	}

	//status = _alifSys_readPreinitWarnOptions(&sysWarnOptions);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	goto done;
	//}

	//status = config_initWarnOptions(_config, &cmdlineWarnOptions,
	//	&envWarnOptions, &sysWarnOptions);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	goto done;
	//}

	status = ALIFSTATUS_OK();

done:
	_alifWStringList_clear(&cmdlineWarnOptions);
	_alifWStringList_clear(&envWarnOptions);
	_alifWStringList_clear(&sysWarnOptions);
	return status;
}


AlifStatus _alifConfig_read(AlifConfig* _config, AlifIntT _computePathConfig) { // 3047

	AlifStatus status{};

	status = _alif_preInitializeFromConfig(_config, nullptr);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	config_getGlobalVars(_config);

	if (_config->origArgv.length == 0 and !(_config->argv.length == 1
		and wcscmp(_config->argv.items[0], L"") == 0)) {
		if (_alifWStringList_copy(&_config->origArgv, &_config->argv) < 0) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	AlifPreCmdline precmdline = ALIFPRECMDLINE_INIT;
	status = core_readPreCMDLine(_config, &precmdline);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	if (_config->isolated) {
		_config->safePath = 1;
		_config->useEnvironment = 0;
		_config->userSiteDirectory = 0;
	}

	status = config_readCMDLine(_config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	//status = _alifSys_readPreinitXOptions(_config);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	goto done;
	//}

	status = config_read(_config, _computePathConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	status = ALIFSTATUS_OK();

done:
	_alifPreCMDLine_clear(&precmdline);
	return status;
}



AlifStatus _alifConfig_setAlifArgv(AlifConfig* _config, const AlifArgv* _args) { // 3144
	AlifStatus status = _alif_preInitializeFromConfig(_config, _args);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	return _alifArgv_asWStrList(_args, &_config->argv);
}

AlifStatus alifConfig_setBytesArgv(AlifConfig* _config,
	AlifSizeT _argc, char* const* _argv) { // 3158
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 1,
		.bytesArgv = _argv,
		.wcharArgv = nullptr };
	return _alifConfig_setAlifArgv(_config, &args);
}

AlifStatus alifConfig_setArgv(AlifConfig* _config,
	AlifSizeT _argc, wchar_t* const* _argv) { // 3170
	AlifArgv args = {
		.argc = _argc,
		.useBytesArgv = 0,
		.bytesArgv = nullptr,
		.wcharArgv = _argv };
	return _alifConfig_setAlifArgv(_config, &args);
}






static AlifObject* config_getSys(const char* _name) { // 4029
	AlifObject* value = alifSys_getObject(_name);
	if (value == nullptr) {
		//alifErr_format(_alifExcRuntimeError_, "lost sys.%s", _name);
		return nullptr;
	}
	return ALIF_NEWREF(value);
}

static AlifIntT config_getSysWriteBytecode(const AlifConfig* _config, AlifIntT* _value) { // 4041
	AlifObject* attr = config_getSys("dont_write_bytecode");
	if (attr == nullptr) {
		return -1;
	}

	AlifIntT isTrue = alifObject_isTrue(attr);
	ALIF_DECREF(attr);
	if (isTrue < 0) {
		return -1;
	}
	*_value = (!isTrue);
	return 0;
}


static AlifObject* config_get(const AlifConfig* _config,
	const AlifConfigSpec* _spec, AlifIntT _useSys) { // 4059
	if (_useSys) {
		if (_spec->sys.attr != nullptr) {
			return config_getSys(_spec->sys.attr);
		}

		if (strcmp(_spec->name, "write_bytecode") == 0) {
			AlifIntT value;
			if (config_getSysWriteBytecode(_config, &value) < 0) {
				return nullptr;
			}
			return alifBool_fromLong(value);
		}

		if (strcmp(_spec->name, "int_max_str_digits") == 0) {
			AlifInterpreter* interp = _alifInterpreter_get();
			return alifLong_fromLong(interp->longState.maxStrDigits);
		}
	}

	void* member = config_getSpecMember(_config, _spec);
	switch (_spec->type) {
	case AlifConfigMemberType_::Alif_Config_Member_INT:
	case AlifConfigMemberType_::Alif_Config_Member_UINT:
	{
		AlifIntT value = *(AlifIntT*)member;
		return alifLong_fromLong(value);
	}

	case AlifConfigMemberType_::Alif_Config_Member_BOOL:
	{
		AlifIntT value = *(AlifIntT*)member;
		return alifBool_fromLong(value != 0);
	}

	case AlifConfigMemberType_::Alif_Config_Member_ULONG:
	{
		unsigned long value = *(unsigned long*)member;
		return alifLong_fromUnsignedLong(value);
	}

	case AlifConfigMemberType_::Alif_Config_Member_WSTR:
	case AlifConfigMemberType_::Alif_Config_Member_WSTR_OPT:
	{
		wchar_t* wstr = *(wchar_t**)member;
		if (wstr != nullptr) {
			return alifUStr_fromWideChar(wstr, -1);
		}
		else {
			return ALIF_NEWREF(ALIF_NONE);
		}
	}

	case AlifConfigMemberType_::Alif_Config_Member_WSTR_LIST:
	{
		//if (strcmp(_spec->name, "xoptions") == 0) {
		//	return _alifConfig_createXOptionsDict(_config);
		//}
		//else {
		const AlifWStringList* list = (const AlifWStringList*)member;
		return _alifWStringList_asTuple(list);
		//}
	}

	default:
		ALIF_UNREACHABLE();
	}
}
