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



typedef AlifObject* (*ConfigSysFlagSetter) (AlifIntT);





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
	Alif_Config_Member_Init_Only = 0,
	Alif_Config_Member_Read_Only = 1,
	Alif_Config_Member_Public = 2,
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
#define SPEC(_member, _type, _visibility, _sys) \
    {#_member, offsetof(AlifConfig, _member), \
     Alif_Config_Member_##_type, Alif_Config_Member_##_visibility, _sys}

#define SYS_ATTR(name) {name, -1, nullptr}
#define SYS_FLAG_SETTER(index, setter) {nullptr, index, setter}
#define SYS_FLAG(index) SYS_FLAG_SETTER(index, nullptr)
#define NO_SYS SYS_ATTR(nullptr)

static const AlifConfigSpec _alifConfigSpec_[] = { // 95

	// --- Public options -----------
	SPEC(argv, WSTR_LIST, Public, SYS_ATTR("argv")),
	SPEC(baseExecPrefix, WSTR_OPT, Public, SYS_ATTR("baseExecPrefix")),
	SPEC(baseExecutable, WSTR_OPT, Public, SYS_ATTR("_baseExecutable")),
	SPEC(basePrefix, WSTR_OPT, Public, SYS_ATTR("basePrefix")),
	SPEC(execPrefix, WSTR_OPT, Public, SYS_ATTR("execPrefix")),
	SPEC(executable, WSTR_OPT, Public, SYS_ATTR("executable")),
	SPEC(intMaxStrDigits, UINT, Public, NO_SYS),
	SPEC(interactive, BOOL, Public, SYS_FLAG(2)),
	SPEC(moduleSearchPaths, WSTR_LIST, Public, SYS_ATTR("Path")),
	SPEC(optimizationLevel, UINT, Public, SYS_FLAG(3)),
	SPEC(platLibDir, WSTR, Public, SYS_ATTR("platLibDir")),
	SPEC(prefix, WSTR_OPT, Public, SYS_ATTR("prefix")),


	// --- Read-only options -----------

	SPEC(bufferedStdio, BOOL, Read_Only, NO_SYS),


	//SPEC(isolated, BOOL, Read_Only, NO_SYS),  // sys.flags.isolated
#ifdef _WINDOWS
	SPEC(legacyWindowsStdio, BOOL, Read_Only, NO_SYS),
#endif
	SPEC(origArgv, WSTR_LIST, Read_Only, SYS_ATTR("origArgv")),
	SPEC(parseArgv, BOOL, Read_Only, NO_SYS),
	SPEC(programName, WSTR, Read_Only, NO_SYS),
	SPEC(runCommand, WSTR_OPT, Read_Only, NO_SYS),
	SPEC(runFilename, WSTR_OPT, Read_Only, NO_SYS),
	SPEC(runModule, WSTR_OPT, Read_Only, NO_SYS),
	{nullptr, 0, AlifConfigMemberType_::Alif_Config_Member_INT},

	SPEC(safePath, BOOL, Read_Only, NO_SYS),
	//SPEC(skipSourceFirstLine, BOOL, Read_Only, NO_SYS),
	SPEC(tracemalloc, UINT, Read_Only, NO_SYS),

	// --- Init-only options -----------

	SPEC(configInit, UINT, Init_Only, NO_SYS),
	SPEC(initMain, BOOL, Init_Only, NO_SYS),
	SPEC(installImportLib, BOOL, Init_Only, NO_SYS),
	SPEC(moduleSearchPathsSet, BOOL, Init_Only, NO_SYS),
	SPEC(sysPath0, WSTR_OPT, Init_Only, NO_SYS),

	// Array terminator
	{nullptr, 0, AlifConfigMemberType_::Alif_Config_Member_INT,
	AlifConfigMemberVisibility::Alif_Config_Member_Init_Only, NO_SYS},
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



/* --------------------------------------- Command line options --------------------------------------- */
// 140
static const char usageLine[] =
"usage: %ls [option] ... [-c cmd | -m mod | file | - ] [arg] ...\n";

static const char usageHelp[] = "\
-c cmd : program passed in as string (terminates option list)\n\
-m mod : run library module as a script (terminates option list)\n\
file   : program read from script file\n\
-      : program read from stdin (default; interactive mode if a tty)\n\
arg ...: arguments passed to program in sys.argv[1:]\n\
-h     : print this help message and exit (--help)\n\
-v     : print Alif version number and exit (also --version)\n\
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

static void config_usage(AlifIntT _error, const wchar_t* program) { // 2401
	FILE* f = _error ? stderr : stdout;

	fprintf(f, usageLine, program);

	if (_error) {
		fprintf(f, "لمزيد من المعلومات قم بكتابة `alif -h'.\n");
	}
	else {
		fputs(usageHelp, f);
	}
}


void _alifWStringList_clear(AlifWStringList* _list) { // 608
	for (AlifSizeT i = 0; i < _list->length; i++) {
		alifMem_dataFree(_list->items[i]);
	}

	alifMem_dataFree(_list->items);
	_list->length = 0;
	_list->items = nullptr;
}

AlifIntT alifWStringList_copy(AlifWStringList* _list, const AlifWStringList* _list2) { // 529

	if (_list2->length == 0) {
		_alifWStringList_clear(_list);
		return 0;
	}

	AlifWStringList copy = { .length = 0, .items = nullptr };

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

static AlifIntT alif_setArgcArgv(AlifSizeT _argc, wchar_t* const* _argv) { // 673
	const AlifWStringList argv_list = { .length = _argc, .items = (wchar_t**)_argv };
	AlifIntT res{};

	// XXX _alifRuntime_.origArgv only gets cleared by alif_main(),
	// so it currently leaks for embedders.
	res = alifWStringList_copy(&_alifRuntime_.origArgv, &argv_list);

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

void alifConfig_clear(AlifConfig* _config) { // 773
#define CLEAR(_ATTR)							\
    do {										\
		alifMem_dataFree(_ATTR);				\
		_ATTR = nullptr;						\
    } while (0)

	CLEAR(_config->programName);

	_alifWStringList_clear(&_config->argv);
	_alifWStringList_clear(&_config->moduleSearchPaths);
	_config->moduleSearchPathsSet = 0;

	CLEAR(_config->runCommand);
	CLEAR(_config->runModule);
	CLEAR(_config->runFilename);

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

/* duplicate the string */
AlifStatus alifConfig_setString(AlifConfig* _config, wchar_t** _configStr, const wchar_t* _str) { // 933

	//AlifStatus status = _alif_preInitializeFromConfig(_config, nullptr);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	wchar_t* str2;
	if (_str != nullptr) {
		str2 = alifMem_wcsDup(_str);
		if (str2 == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}
	else {
		str2 = nullptr;
	}

	free(*_configStr);
	*_configStr = str2;
	return ALIFSTATUS_OK();
}


static inline void* config_getSpecMember(const AlifConfig* _config,
	const AlifConfigSpec* _spec) { // 1117
	return (char*)_config + _spec->offset;
}


AlifStatus alifConfig_copy(AlifConfig* _config, const AlifConfig* _config2) { // 1003
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
			if (alifWStringList_copy((AlifWStringList*)member,
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



static AlifStatus config_initImport(AlifConfig* _config, AlifIntT _computePathConfig) { // 2321
	AlifStatus status{};

	status = _alifConfig_initPathConfig(_config, _computePathConfig); //* todo
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//const char* env = config_getEnv(_config, "ALIF_FROZEN_MODULES");
	//if (env == nullptr) {
	//}
	//else if (strcmp(env, "on") == 0) {
	//	_config->useFrozenModules = 1;
	//}
	//else if (strcmp(env, "off") == 0) {
	//	_config->useFrozenModules = 0;
	//}
	//else {
	//	return alifStatus_error("bad value for ALIF_FROZEN_MODULES "
	//		"(expected \"on\" or \"off\")");
	//}

	//const wchar_t* value = config_getXoptionValue(_config, L"frozen_modules");
	//if (value == nullptr) {
	//}
	//else if (wcscmp(value, L"on") == 0) {
	//	_config->useFrozenModules = 1;
	//}
	//else if (wcscmp(value, L"off") == 0) {
	//	_config->useFrozenModules = 0;
	//}
	//else if (wcslen(value) == 0) {
	//	_config->useFrozenModules = 1;
	//}
	//else {
	//	return alifStatus_Error("bad value for option -X frozen_modules "
	//		"(expected \"on\" or \"off\")");
	//}

	return ALIFSTATUS_OK();
}

AlifStatus _alifConfig_initImportConfig(AlifConfig* _config) { // 2367
	return config_initImport(_config, 1);
}


static AlifStatus config_read(AlifConfig* _config) { // 2215
	AlifStatus status{};

	//if (_config->useEnvironment) {
	//	status = config_readEnvVars(_config);
	//	if (ALIFSTATUS_EXCEPTION(status)) return status;
	//}

	if (_config->tracemalloc < 0) { // from config_read_complex_options()
		_config->tracemalloc = 1;
	}

	/* default values */
	//if (_config->tracemalloc < 0) {
	//	_config->tracemalloc = 0;
	//}

	if (_config->argv.length < 1) {
		/* Ensure at least one (empty) argument is seen */
		status = alifWStringList_append(&_config->argv, L"");
		if (ALIFSTATUS_EXCEPTION(status)) return status;
	}

	//if (_config->configureStdio < 0) {
	//	_config->configureStdio = 1;
	//}

	// Only parse arguments once.
	if (_config->parseArgv == 1) {
		_config->parseArgv = 2;
	}

	return ALIFSTATUS_OK();
}

static AlifIntT alif_setFileMode(const AlifConfig* _config) { // 2327

#if defined(_WINDOWS) or defined(__CYGWIN__)
	/* don't translate newlines (\r\n <=> \n) */
	bool modeIn = _setmode(fileno(stdin), O_BINARY);
	bool modeOut = _setmode(fileno(stdout), O_BINARY);
	bool modeErr = _setmode(fileno(stderr), O_BINARY);

	if (!modeIn or !modeOut or !modeErr) {
		std::cout <<
			"can't init _setmode in windows for reading Arabic characters" // يجب أن تكون باللغة الاجنبية في حال لم يستطع تهيئة طباعة الاحرف العربية
			<< std::endl;
		return -1;
	}
#endif

	bool buffIn{ 1 }, buffOut{ 1 }, buffErr{ 1 };
	if (!_config->bufferedStdio) {
	#ifdef HAVE_SETVBUF
		buffIn = setvbuf(stdin, (char*)nullptr, _IONBF, BUFSIZ);
		buffOut = setvbuf(stdout, (char*)nullptr, _IONBF, BUFSIZ);
		buffErr = setvbuf(stderr, (char*)nullptr, _IONBF, BUFSIZ);
	#else /* !HAVE_SETVBUF */
		buffIn = setbuf(stdin, (char*)nullptr);
		buffOut = setbuf(stdout, (char*)nullptr);
		buffErr = setbuf(stderr, (char*)nullptr);
	#endif /* !HAVE_SETVBUF */

		if (buffIn or buffOut or buffErr) {
			std::cout << "لم يستطع تهيئة مخزن النصوص الإفتراضي في نظام ويندوز" << std::endl;
			return -1;
		}
	}
	else if (_config->interactive) {
	#ifdef _WINDOWS
		/* Doesn't have to have line-buffered -- use unbuffered */
		buffOut = setvbuf(stdout, (char*)nullptr, _IONBF, BUFSIZ);
		if (buffOut) {
			std::cout << "لم يستطع تهيئة مخزن النصوص الإفتراضي في نظام ويندوز" << std::endl;
			return -1;
		}
	#else /* !_WINDOWS */
	#ifdef HAVE_SETVBUF
		buffIn = setvbuf(stdin, (char*)nullptr, _IOLBF, BUFSIZ);
		buffOut = setvbuf(stdout, (char*)nullptr, _IOLBF, BUFSIZ);
		if (buffIn or buffOut) {
			std::cout << "لم يستطع تهيئة مخزن النصوص الإفتراضي" << std::endl;
			return -1;
		}
	#endif /* HAVE_SETVBUF */
	#endif /* !_WINDOWS */
		/* Leave stderr alone - it should be unbuffered. */
	}

	return 1;
}

AlifStatus alifConfig_write(const AlifConfig* _config, AlifRuntime* _dureRun) { // 2368

	if (alif_setArgcArgv(_config->origArgv.length, _config->origArgv.items) < 0) {
		return ALIFSTATUS_NO_MEMORY();
	}

	return ALIFSTATUS_OK();
}




static AlifStatus parse_consoleLine(AlifConfig* _config, AlifSizeT* _index) { // 2438

	AlifStatus status{};

	AlifIntT printVer{};
	const AlifWStringList* argv = &_config->argv;
	const wchar_t* program = _config->programName;
	if (!program and argv->length >= 1) {
		program = argv->items[0];
	}

	_alifOS_resetGetOpt();
	do {
		AlifIntT longIndex = -1;
		AlifIntT c = _alifOS_getOpt(argv->length, argv->items, &longIndex);

		if (c == EOF) break;

		if (c == 'c') {
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

		if (c == 'm') {
			if (_config->runModule == nullptr) {
				_config->runModule = alifMem_wcsDup(_alifOSOptArg_);
				if (_config->runModule == nullptr) {
					return ALIFSTATUS_NO_MEMORY();
				}
			}
			break;
		}

		if (c == 0) {
			//config_completeUsage(program);
			exit(1);
		}
		else if (c == 1) {
			//config_envVarsUsage();
			exit(1);
		}
		else if (c == 'h') {
			config_usage(0, program);
			exit(1);
		}
		else if (c == 'v') {
			printVer++;
			break;
		}
		else {
			// فشلت عملية تحليل سطر الاوامر
			config_usage(1, program);
			exit(1);
		}

	}
	while (true);



	if (printVer) {
		printf("alif %s\n", alif_getVersion());
		exit(1);
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


static AlifStatus update_argv(AlifConfig* _config, AlifSizeT _index) { // 2803
	const AlifWStringList* cmdlineArgv = &_config->argv;
	AlifWStringList configArgv = { .length = 0, .items = nullptr };
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
		if (alifWStringList_copy(&configArgv, &slice) < 0) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	wchar_t* arg1{};
	if (_config->runCommand != nullptr) {
		arg1 = (wchar_t*)L"-c";
	}
	else if (_config->runModule != nullptr) {
		arg1 = (wchar_t*)L"-m";
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


static AlifIntT alif_extension(wchar_t* _filename) { //* alif
	const wchar_t* dotSuffix = wcsrchr(_filename, L'.');
	if (!dotSuffix) { return 0; }

	return wcscmp(dotSuffix, L".alif") == 0 or wcscmp(dotSuffix, L".الف") == 0;
}

static AlifStatus run_absPathFilename(AlifConfig* _config) { // 2888

	wchar_t* filename = _config->runFilename;
	if (!filename) {
		return ALIFSTATUS_OK();
	}

	// يتم التحقق من لاحقة الملف والتي يجب ان تكون .alif او .الف
	if (!alif_extension(filename)) {
		printf("%s \n\n", "تأكد من لاحقة الملف \n يجب ان ينتهي اسم الملف بـ .alif او .الف");
		exit(-1);
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


static AlifStatus config_readConsoleLine(AlifConfig* _config) { // 2918

	AlifStatus status{};

	if (_config->parseArgv < 0) _config->parseArgv = 1;

	if (_config->parseArgv == 1) {

		AlifSizeT index{};

		// تقوم هذه الدالة بتحليل سطر الطرفية
		status = parse_consoleLine(_config, &index);
		if (ALIFSTATUS_EXCEPTION(status)) return status;

		// تقوم هذه الدالة بجلب المسار الخاص بتنفيذ الملف فقط
		status = run_absPathFilename(_config);
		if (ALIFSTATUS_EXCEPTION(status)) return status;

		// تقوم هذه الدالة بتحديث المعطيات الممررة عبر الطرفية
		status = update_argv(_config, index);
		if (ALIFSTATUS_EXCEPTION(status)) return status;

	}
	else {
		// تقوم هذه الدالة بجلب المسار الخاص بتنفيذ الملف فقط
		status = run_absPathFilename(_config);
		if (ALIFSTATUS_EXCEPTION(status)) return status;
	}

	return ALIFSTATUS_OK();
}


AlifStatus alifConfig_read(AlifConfig* _config) { // 3047

	AlifStatus status{};

	if (_config->origArgv.length == 0 and !(_config->argv.length == 1
		and wcscmp(_config->argv.items[0], L"") == 0)) {
		if (alifWStringList_copy(&_config->origArgv, &_config->argv) < 0) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	status = config_readConsoleLine(_config);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = config_read(_config);

	return ALIFSTATUS_OK();
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
