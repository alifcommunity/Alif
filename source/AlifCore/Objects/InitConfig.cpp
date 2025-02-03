#include "alif.h"

#include "AlifCore_GetOption.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Memory.h"
#include "AlifCore_Long.h"
#include "AlifCore_State.h"
#include "AlifCore_DureRun.h"

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
    //SPEC(executable, WSTR_OPT, Public, SYS_ATTR("executable")),
    SPEC(interactive, BOOL, Public, SYS_FLAG(2)),
    SPEC(moduleSearchPaths, WSTR_LIST, Public, SYS_ATTR("path")),
    SPEC(optimizationLevel, UINT, Public, SYS_FLAG(3)),


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
	//SPEC(installImportLib, BOOL, Init_Only, NO_SYS),
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


void alifWStringList_clear(AlifWStringList* _list) { // 516
	for (AlifSizeT i = 0; i < _list->length; i++) {
		alifMem_dataFree(_list->items[i]);
	}

	//free(_list->items);
	_list->length = 0;
	_list->items = nullptr;
}

AlifIntT alifWStringList_copy(AlifWStringList* _list, const AlifWStringList* _list2) { // 529

	if (_list2->length == 0) {
		alifWStringList_clear(_list);
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
			alifWStringList_clear(&copy);
			return -1;
		}
		copy.items[i] = item;
		copy.length = i + 1;
	}

	alifWStringList_clear(_list);
	*_list = copy;
	return 0;
}


AlifIntT alifWStringList_insert(AlifWStringList* _list,
	AlifSizeT _index, const wchar_t* _item) { // 564
	AlifSizeT len = _list->length;
	if (len == ALIF_SIZET_MAX) {
		/* length+1 would overflow */
		// memory error
		return -1;
	}
	if (_index < 0) {
		// error
		return -1;
	}
	if (_index > len) {
		_index = len;
	}

	wchar_t* item2 = alifMem_wcsDup(_item);
	if (item2 == nullptr) {
		// memory error
		return -1;
	}

	AlifUSizeT size = (len + 1) * sizeof(_list->items[0]);
	wchar_t** items2 = (wchar_t**)alifMem_dataRealloc(_list->items, size);
	if (items2 == nullptr) {
		alifMem_dataFree(item2);
		// memory error
		return -1;
	}

	if (_index < len) {
		memmove(&items2[_index + 1],
			&items2[_index],
			(len - _index) * sizeof(items2[0]));
	}

	items2[_index] = item2;
	_list->items = items2;
	_list->length++;
	return 1;
}

AlifIntT alifWStringList_append(AlifWStringList* _list, const wchar_t* _item)
{ // 605
	return alifWStringList_insert(_list, _list->length, _item);
}

static AlifIntT alif_setArgcArgv(AlifSizeT _argc, wchar_t* const* _argv) { // 673
	const AlifWStringList argv_list = { .length = _argc, .items = (wchar_t**)_argv };
	AlifIntT res{};

	// XXX _alifDureRun_.origArgv only gets cleared by alif_main(),
	// so it currently leaks for embedders.
	res = alifWStringList_copy(&_alifDureRun_.origArgv, &argv_list);

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

	//CLEAR(_config->programName);

	alifWStringList_clear(&_config->argv);
	alifWStringList_clear(&_config->moduleSearchPaths);
	_config->moduleSearchPathsSet = 0;

	CLEAR(_config->runCommand);
	CLEAR(_config->runModule);
	CLEAR(_config->runFilename);

	alifWStringList_clear(&_config->origArgv);
#undef CLEAR
}


void alifConfig_initAlifConfig(AlifConfig* _config) { // 897
	_config->configInit = ConfigInitEnum_::AlifConfig_Init_Alif;
	_config->tracemalloc = -1;
	_config->parseArgv = 1;
	_config->interactive = 0;
	_config->optimizationLevel = 0;
	_config->bufferedStdio = 1;
	_config->installImportLib = 1;
	_config->initMain = 1;
	_config->intMaxStrDigits = ALIF_LONG_DEFAULT_MAX_STR_DIGITS; // need review
}

/* duplicate the string */
AlifIntT alifConfig_setString(AlifConfig* _config, wchar_t** _configStr, const wchar_t* _str) { // 933

	wchar_t* str2;
	if (_str != nullptr) {
		str2 = alifMem_wcsDup(_str);
		if (str2 == nullptr) {
			// memory error
			return -1;
		}
	}
	else {
		str2 = nullptr;
	}

	free(*_configStr);
	*_configStr = str2;
	return 1;
}


static inline void* config_getSpecMember(const AlifConfig* _config,
	const AlifConfigSpec* _spec) { // 1117
	return (char*)_config + _spec->offset;
}


AlifIntT alifConfig_copy(AlifConfig* _config, const AlifConfig* _config2) { // 1003
	alifConfig_clear(_config);

	AlifIntT status{};
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
			if (status < 1) return status;
			break;
		}
		case Alif_Config_Member_WSTR_LIST:
		{
			if (alifWStringList_copy((AlifWStringList*)member,
				(const AlifWStringList*)member2) < 0) {
				// memory error
				return -1;
			}
			break;
		}
		default:
			ALIF_UNREACHABLE();
		}
	}
	return 1;
}

static AlifIntT config_read(AlifConfig* _config) { // 2215
	AlifIntT status{};

	//if (_config->useEnvironment) {
	//	status = config_readEnvVars(_config);
	//	if (status < 1) return status;
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
		if (status < 1) return status;
	}

	//if (_config->configureStdio < 0) {
	//	_config->configureStdio = 1;
	//}

	// Only parse arguments once.
	if (_config->parseArgv == 1) {
		_config->parseArgv = 2;
	}

	return 1;
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

AlifIntT alifConfig_write(const AlifConfig* _config, AlifDureRun* _dureRun) { // 2368

	if (alif_setArgcArgv(_config->origArgv.length, _config->origArgv.items) < 0) {
		// memory error
		return -1;
	}

	return 1;
}

AlifIntT alif_setStdioLocale(const AlifConfig* _config) { //* alif

	AlifIntT mode = alif_setFileMode(_config);
	if (!mode) return -1;

	const char* locale = alif_setLocale(LC_CTYPE);
	if (locale == nullptr) {
		std::cout <<
			"can't init locale for reading Arabic characters" // يجب أن تكون باللغة الاجنبية في حال لم يستطع تهيئة طباعة الاحرف العربية
			<< std::endl;
		return -1;
	}

	return 1;
}

AlifIntT alif_preInitFromConfig(AlifConfig* _config) { //* alif

	if (alif_mainMemoryInit() < 1) {
		return -1;
	}

	if (alif_setStdioLocale(_config) < 1) {
		return -1;
	}

	return 1;
}




static AlifIntT parse_consoleLine(AlifConfig* _config, AlifSizeT* _index) { // 2438

	AlifIntT status{};

	AlifIntT printVer{};
	const AlifWStringList* argv = &_config->argv;
	const wchar_t* program = _config->programName;
	if (!program and argv->length >= 1) {
		program = argv->items[0];
	}

	alif_resetGetOption();
	do {
		AlifIntT longIndex = -1;
		AlifIntT c = alif_getOption(argv->length, argv->items, &longIndex);

		if (c == EOF) break;

		if (c == 'c') {
			if (_config->runCommand == nullptr) {
				AlifUSizeT len = wcslen(_optArg_) + 1 + 1;
				wchar_t* command = (wchar_t*)alifMem_dataAlloc(len * sizeof(wchar_t));
				if (command == nullptr) {
					// memory error
					return -1;
				}
				memcpy(command, _optArg_, (len - 2) * sizeof(wchar_t));
				command[len - 2] = '\n';
				command[len - 1] = 0;
				_config->runCommand = command;
			}
			break;
		}

		if (c == 'm') {
			if (_config->runModule == nullptr) {
				_config->runModule = alifMem_wcsDup(_optArg_);
				if (_config->runModule == nullptr) {
					// memory error
					return -1;
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
			// فشلت عملية تحلل سطر الاوامر
			config_usage(1, program);
			exit(1);
		}

	} while (true);



	if (printVer) {
		printf("alif %s\n", alif_getVersion());
		exit(1);
	}

	if (_config->runModule == nullptr and _config->runCommand == nullptr
		and _optIdx_ < argv->length and wcscmp(argv->items[_optIdx_], L"-") != 0
		and _config->runFilename == nullptr) {
		_config->runFilename = (wchar_t*)alifMem_wcsDup(argv->items[_optIdx_]);
		if (_config->runFilename == nullptr) {
			// memory error
			return -1;
		}
	}

	if (_config->runCommand != nullptr or
		_config->runModule != nullptr) {
		_optIdx_--;
	}

	//_config->programName = (wchar_t*)program; // يحتاج مراجعة - لا يتم إسناده هنا

	*_index = _optIdx_;

	return 1;
}


static AlifIntT update_argv(AlifConfig* _config, AlifSizeT _index) { // 2803
	const AlifWStringList* cmdlineArgv = &_config->argv;
	AlifWStringList configArgv = { .length = 0, .items = nullptr };
	if (cmdlineArgv->length <= _index) {
		if (alifWStringList_append(&configArgv, L"") < 1) {
			// error
			return -1;
		}
	}
	else {
		AlifWStringList slice{};
		slice.length = cmdlineArgv->length - _index;
		slice.items = &cmdlineArgv->items[_index];
		if (alifWStringList_copy(&configArgv, &slice) < 0) {
			// memory error
			return -1;
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
			alifWStringList_clear(&configArgv);
			// memory error
			return -1;
		}

		if (configArgv.items == nullptr) {
			// error
			return -1;
		}
		alifMem_dataFree(configArgv.items[0]);
		configArgv.items[0] = arg1;
	}

	alifWStringList_clear(&_config->argv);
	_config->argv = configArgv;
	return 1;
}


static AlifIntT alif_extension(wchar_t* _filename) { //* alif
	const wchar_t* dotPos = wcschr(_filename, L'.');
	if (!dotPos) { return 0; }

	const wchar_t* suffix = wcsstr(_filename, dotPos + 1);
	if (suffix) { return wcscmp(suffix, L"alif") == 0; }
	else { return 0; }
}

static AlifIntT run_absPathFilename(AlifConfig* _config) { // 2888

	wchar_t* filename = _config->runFilename;
	if (!filename) {
		return 1;
	}

	// يتم التحقق من لاحقة الملف والتي يجب ان تكون .alif
	if (!alif_extension(filename)) {
		printf("%s", "تأكد من لاحقة الملف \n يجب ان ينتهي اسم الملف بـ .alif");
		exit(-1);
	}

#ifndef _WINDOWS
	if (alif_isAbs(filename)) {
		/* path is already absolute */
		return 1;
	}
#endif

	wchar_t* absFilename{};
	if (alif_absPath(filename, &absFilename) < 0) {
		/* failed to get the absolute path of the command line filename:
		   ignore the error, keep the relative path */
		return 1;
	}
	if (absFilename == nullptr) {
		return -1;
	}

	alifMem_dataFree(_config->runFilename);
	_config->runFilename = absFilename;
	return 1;
}


static AlifIntT config_readConsoleLine(AlifConfig* _config) { // 2918

	AlifIntT status{};

	if (_config->parseArgv < 0) _config->parseArgv = 1;

	if (_config->parseArgv == 1) {

		AlifSizeT index{};

		// تقوم هذه الدالة بتحليل سطر الطرفية
		status = parse_consoleLine(_config, &index);
		if (status < 1) return status;

		// تقوم هذه الدالة بجلب المسار الخاص بتنفيذ الملف فقط
		status = run_absPathFilename(_config);
		if (status < 1) return status;

		// تقوم هذه الدالة بتحديث المعطيات الممررة عبر الطرفية
		status = update_argv(_config, index);
		if (status < 1) return status;

	}
	else {
		// تقوم هذه الدالة بجلب المسار الخاص بتنفيذ الملف فقط
		status = run_absPathFilename(_config);
		if (status < 1) return status;
	}

	return 1;
}


AlifIntT alifConfig_read(AlifConfig* _config) { // 3047

	AlifIntT status{};

	if (_config->origArgv.length == 0 and !(_config->argv.length == 1
		and wcscmp(_config->argv.items[0], L"") == 0))
	{
		if (alifWStringList_copy(&_config->origArgv, &_config->argv) < 0) {
			// memory error
			return -1;
		}
	}

	status = config_readConsoleLine(_config);
	if (status < 1) return status;

	status = config_read(_config);

	return 1;
}





/* ------------------------------------------------------------------------------------------- */


AlifIntT alifArgv_asWStringList(AlifConfig* _config, AlifArgv* _args) { // 78 in preconfig file


	AlifWStringList wArgv = { .length = 0, .items = nullptr };
	if (_args->useBytesArgv) {
		wArgv.items = (wchar_t**)alifMem_dataAlloc(_args->argc * sizeof(wchar_t*));
		if (wArgv.items == nullptr) {
			// memory error
			return -1;
		}

		for (AlifIntT i = 0; i < _args->argc; i++) {
			AlifUSizeT len{};
			wchar_t* arg = alif_decodeLocale(_args->bytesArgv[i], &len);
			if (arg == nullptr) {
				alifWStringList_clear(&wArgv);
				return -1;
			}
			wArgv.items[i] = arg;
			wArgv.length++;
		}

		alifWStringList_clear(&_config->argv);
		_config->argv = wArgv;

		/*
			تم إسناد اسم البرنامج هنا مؤقتاُ
			موقع الإسناد يتم خلال الدوال -
			alifInit_main -> init_interpMain -> _alifConfig_initImportConfig -> config_initImport ->
			_alifConfig_initPathConfig -> alifEval_evalCode ...
		*/
		_config->programName = wArgv.items[0]; // مؤقت
	}
	else {
		wArgv.length = _args->argc;
		wArgv.items = (wchar_t**)_args->wcharArgv;
		if (alifWStringList_copy(&_config->argv, &wArgv)) {
			// memory error
			return -1;
		}
		/*
			تم إسناد اسم البرنامج هنا مؤقتاُ
			موقع الإسناد يتم خلال الدوال -
			alifInit_main -> init_interpMain -> _alifConfig_initImportConfig -> config_initImport ->
			_alifConfig_initPathConfig -> alifEval_evalCode ...

		*/
		_config->programName = wArgv.items[0]; // مؤقت
	}

	return 1;
}
