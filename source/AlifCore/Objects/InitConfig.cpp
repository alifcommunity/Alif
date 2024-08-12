#include "alif.h"

#include "AlifCore_GetConsoleLine.h"
#include "AlifCore_Memory.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Interpreter.h"
#include "AlifCore_AlifCycle.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_DureRun.h"

#ifdef _WINDOWS
#include <windows.h>
#include <fcntl.h>
#else
#include <unistd.h>
typedef uint32_t DWORD;
#define MAX_PATH 260
#endif

#define MAXPATHLEN 256

#pragma warning(disable : 4996) // for disable unsafe functions error

#define ALIF_ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

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

static void config_usage(const wchar_t* program)
{
	FILE* f = stdout;

	fprintf(f, usageLine, program);

	fputs(usageHelp, f);
}


char* alif_setLocale(AlifIntT category) {

	char* res;
#ifdef __ANDROID__
	const char* locale;
	const char** pvar;
#ifdef ALIF_COERCE_C_LOCALE
	const char* coerce_c_locale;
#endif
	const char* utf8_locale = "C.UTF-8";
	const char* env_var_set[] = {
		"LC_ALL",
		"LC_CTYPE",
		"LANG",
		NULL,
	};

	for (pvar = env_var_set; *pvar; pvar++) {
		locale = getenv(*pvar);
		if (locale != nullptr and *locale != '\0') {
			if (strcmp(locale, utf8_locale) == 0 or
				strcmp(locale, "en_US.UTF-8") == 0) {
				return setlocale(category, utf8_locale);
			}
			return setlocale(category, "C");
		}
	}

#ifdef ALIF_COERCE_C_LOCALE
	coerce_c_locale = getenv("ALIFCOERCECLOCALE");
	if (coerce_c_locale == nullptr or strcmp(coerce_c_locale, "0") != 0) {
		if (setenv("LC_CTYPE", utf8_locale, 1)) {
			fprintf(stderr, "Warning: failed setting the LC_CTYPE "
				"environment variable to %s\n", utf8_locale);
		}
	}
#endif
	res = setlocale(category, utf8_locale);
#elif defined(_MAC)
	res = setlocale(category, "utf-8");
#else
    res = setlocale(category, "");
#endif

	return res;
}

static AlifIntT alif_setFileMode(const AlifConfig* _config) {

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

	bool buffIn{1}, buffOut{1}, buffErr{1};
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
		buffOut = setvbuf(stdout, (char*)NULL, _IONBF, BUFSIZ);
		if (buffOut) {
			std::cout << "لم يستطع تهيئة مخزن النصوص الإفتراضي في نظام ويندوز" << std::endl;
			return -1;
		}
#else /* !_WINDOWS */
#ifdef HAVE_SETVBUF
		buffIn  = setvbuf(stdin, (char*)NULL, _IOLBF, BUFSIZ);
		buffOut = setvbuf(stdout, (char*)NULL, _IOLBF, BUFSIZ);
		if (buffIn or buffOut) {
			std::cout << "لم يستطع تهيئة مخزن النصوص الإفتراضي في نظام ويندوز" << std::endl;
			return -1;
		}
#endif /* HAVE_SETVBUF */
#endif /* !_WINDOWS */
	/* Leave stderr alone - it should be unbuffered. */
	}

	return 1;
}

AlifIntT alif_setStdioLocale(const AlifConfig* _config) {

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

AlifIntT alifArgv_asStringList(AlifConfig* _config, AlifArgv* _args) {

	if (_args->useBytesArgv) {

		AlifWStringList wArgv = { .length = 0, .items = nullptr };
		wArgv.items = (wchar_t**)alifMem_dataAlloc(_args->argc * sizeof(wchar_t*));
		if (wArgv.items == nullptr) {
			// memory error
			return -1;
		}

		for (AlifIntT i = 0; i < _args->argc; i++) {
			AlifSizeT len = mbstowcs(nullptr, (const char*)_args->bytesArgv[i], 0);
			wchar_t* arg = (wchar_t*)alifMem_dataAlloc((len + 1) * sizeof(wchar_t));
			mbstowcs(arg, (const char*)_args->bytesArgv[i], len);
			wArgv.items[i] = arg;
			wArgv.length++;
		}
		_config->argv = wArgv;
		_config->origArgv = wArgv;

	}
	else {
		_config->argv.length = _args->argc;
		_config->argv.items = (wchar_t**)_args->wcharArgv;

		_config->origArgv.length = _args->argc;
		_config->origArgv.items = (wchar_t**)_args->wcharArgv;
	}

	return 1;
}


void alifConfig_initAlifConfig(AlifConfig* _config) {

	_config->configInit = ConfigInitEnum::AlifConfig_Init_Alif;
	_config->tracemalloc = -1;
	_config->parseArgv = 1;
	_config->interactive = 0;
	_config->optimizationLevel = 0;
	_config->bufferedStdio = 1;
	_config->quite = 0;
	_config->initMain = 1;
	_config->configStdio = 1;
}


static AlifIntT parse_consoleLine(AlifConfig* _config, AlifSizeT* _index) {

	AlifIntT status{};

	AlifIntT printVer{};
	const AlifWStringList* argv = &_config->argv;
	const wchar_t* program = _config->programName;
	if (!program and argv->length >= 1) {
		program = argv->items[0];
	}

	alif_resetConsoleLine();
	do {
		AlifIntT c = alif_getConsoleLine(argv->length, argv->items);

		if (c == EOF) {
			break;
		}

		if (c == 'c') {
			if (_config->runCommand == nullptr) {
				AlifUSizeT len = wcslen(optArg) + 1 + 1;
				wchar_t* command = (wchar_t*)alifMem_dataAlloc(len * sizeof(wchar_t));
				if (command == nullptr) {
					// memory error
					return -1;
				}
				memcpy(command, optArg, (len - 2) * sizeof(wchar_t));
				command[len - 2] = '\n';
				command[len - 1] = 0;
				_config->runCommand = command;
			}
			break;
		}

		if (c == 'm') {
			if (_config->runModule == nullptr) {
				AlifUSizeT len = (wcslen(optArg)) * sizeof(wchar_t);
				_config->runModule = (wchar_t*)alifMem_dataAlloc(len);
				if (_config->runModule == nullptr) {
					// memory error
					return -1;
				}
				memcpy(_config->runModule, optArg, len);
			}
			break;
		}

		if (c == 0) {
			//config_complete_usage(program);
			exit(1);
		}
		else if (c == 1) {
			//config_envvars_usage();
			exit(1);
		}
		else if (c == 'h') {
			config_usage(program);
			exit(1);
		}
		else if (c == 'v') {
			printVer++;
			break;
		}
		else {
			//config_usage(1, program);
			exit(1);
		}

	} while (true);



	if (printVer) {
		printf("alif %s\n", alif_getVersion());
		exit(1);
	}

	if (_config->runModule == nullptr and _config->runCommand == nullptr
		and optIdx < argv->length and wcscmp(argv->items[optIdx], L"-") != 0
		and _config->runFilename == nullptr) {
		AlifSizeT size = wcslen(argv->items[optIdx]) * sizeof(wchar_t);
		_config->runFilename = (wchar_t*)alifMem_dataAlloc(size);
		memcpy(_config->runFilename, argv->items[optIdx], size);
	}

	if (_config->runCommand != nullptr or
		_config->runModule != nullptr) {
		optIdx--;
	}

	_config->programName = (wchar_t*)program; // يحتاج مراجعة - لا يتم إسناده هنا

	*_index = optIdx;

	return 1;
}

static AlifIntT update_argv(AlifConfig* _config, AlifSizeT _index) {
	const AlifWStringList* cmdlineArgv = &_config->argv;
	AlifWStringList configArgv = { 0, nullptr };
	if (cmdlineArgv->length <= _index) {
		wchar_t* append = (wchar_t*)alifMem_dataAlloc(sizeof(L""));
		*append = L'\0';
		configArgv.items = &append;
		configArgv.length++;
	}
	else {
		AlifWStringList slice{};
		slice.length = cmdlineArgv->length - _index;
		slice.items = &cmdlineArgv->items[_index];
		configArgv = slice;
	}

	wchar_t* arg1{};
	if (_config->runCommand != nullptr) {
		arg1 = (wchar_t*)L"-c";
	}
	else if (_config->runModule != nullptr) {
		arg1 = (wchar_t*)L"-m";
	}

	if (arg1 != nullptr) {
		configArgv.items[0] = arg1;
	}

	_config->argv = configArgv;

	return 1;
}

static AlifIntT alif_extension(wchar_t* _filename) {
	const wchar_t* dotPos = wcschr(_filename, L'.');
	if (!dotPos) { return 0; }

	const wchar_t* suffix = wcsstr(_filename, dotPos + 1);
	if (suffix) { return wcscmp(suffix, L"alif") == 0; }
	else { return 0; }
}

static AlifIntT run_absPathFilename(AlifConfig* _config) {

	wchar_t* filename = _config->runFilename;
	// في حال عدم وجود ملف يجب تشغيله، لا تقم بجلب المسار
	if (!filename) { return -1; }

	// يتم التحقق من لاحقة الملف والتي يجب ان تكون .alif
	if (!alif_extension(filename)) {
		printf("%s", "تأكد من لاحقة الملف \n يجب ان ينتهي اسم الملف بـ .alif");
		exit(-1);
	}

	wchar_t* absFilename{};

#ifdef _WINDOWS
	wchar_t wOutBuf[MAX_PATH]{}, * wOutBufP = wOutBuf;
	DWORD result{};
	result = GetFullPathNameW(filename, ALIF_ARRAY_LENGTH(wOutBuf), wOutBuf, nullptr);
	if (!result) return -1;

	wOutBufP = (wchar_t*)alifMem_dataAlloc(result * sizeof(wchar_t));
	result = GetFullPathNameW(filename, result, wOutBufP, nullptr);

	absFilename = (wchar_t*)alifMem_dataAlloc(result * sizeof(wchar_t));
	memcpy(absFilename, wOutBuf, result * sizeof(wchar_t));
#else
	char buf[MAXPATHLEN]{};

	char* cwd = getcwd(buf, sizeof(buf));
	size_t len = mbstowcs(nullptr, cwd, 0);
	wchar_t* wCwd = (wchar_t*)alifMem_dataAlloc(len * sizeof(wchar_t));
	mbstowcs(wCwd, cwd, len);

	size_t cwdLen = wcslen(wCwd);
	size_t pathLen = wcslen(filename);
	size_t length = cwdLen + pathLen;

	absFilename = (wchar_t*)alifMem_dataAlloc(length * sizeof(wchar_t));

	wchar_t* absPath = absFilename;
	memcpy(absPath, wCwd, cwdLen * sizeof(wchar_t));
	absPath += cwdLen;

	*absPath = (wchar_t)L'/';
	absPath++;

	memcpy(absPath, filename, pathLen * sizeof(wchar_t));

	absPath += pathLen - 1;
	*absPath = 0;

#endif // _WINDOWS

	_config->runFilename = absFilename;

	return 1;
}

static AlifIntT config_readConsole(AlifConfig* _config) {

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






AlifIntT alifConfig_write(const AlifConfig* _config, AlifDureRun* _dureRun) {

	if (_config->configStdio) { // for set arabic char reading in console
		if (alif_setStdioLocale(_config) < 1) {
			return -1;
		}
	}

	memcpy(&_alifDureRun_.origArgv, &_config->argv, sizeof(AlifWStringList)); // يجب مراجعتها

	return 1;
}



AlifIntT alifConfig_read(AlifConfig* _config) {

	AlifIntT status{};

	if (_config->origArgv.length == 0 and !(_config->argv.length == 1
		and wcscmp(_config->argv.items[0], L"") == 0))
	{
		//if (alifWideStringList_copy(&_config->origArgv, &_config->argv) < 0) {
			//return ALIFSTATUS_NO_MEMORY();
		//}
		return -1; // temp
	}

	status = config_readConsole(_config);
	if (status < 1) return status;

	if (_config->tracemalloc < 0) _config->tracemalloc = 0;

	if (_config->argv.length < 1) {
		// error
	}


	if (_config->parseArgv == 1) _config->parseArgv = 2;

	return 1;
}
