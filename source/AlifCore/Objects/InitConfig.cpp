#include "alif.h"

//#include "AlifCore_GetConsoleLine.h"
#include "AlifCore_Memory.h"
#include "AlifCore_InitConfig.h"
//#include "AlifCore_Interpreter.h"
#include "AlifCore_LifeCycle.h"
//#include "AlifCore_AlifState.h"
#include "AlifCore_DureRun.h"

#include <locale.h>
#include <stdlib.h>
#ifdef _WINDOWS
#include <fcntl.h>
#endif


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

//static void config_usage(const wchar_t* program) {
//	FILE* f = stdout;
//
//	fprintf(f, usageLine, program);
//
//	fputs(usageHelp, f);
//}




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

AlifIntT alif_setStdioLocale(const AlifConfig* _config) { // alif

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

AlifIntT alif_preInitFromConfig(AlifConfig* _config) { // alif

	if (alif_mainMemoryInit() < 1) {
		return -1;
	}

	if (alif_setStdioLocale(_config) < 1) {
		return -1;
	}

	return 1;
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
		wchar_t* item = alifMem_rawWcsDup(_list2->items[i]);
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

void alifConfig_initAlifConfig(AlifConfig* _config) { // 897
	_config->configInit = ConfigInitEnum_::AlifConfig_Init_Alif;
	_config->tracemalloc = -1;
	_config->parseArgv = 1;
	_config->interactive = 0;
	_config->optimizationLevel = 0;
	_config->bufferedStdio = 1;
	_config->quite = 0;
	_config->cpuCount = -1;
	_config->initMain = 1;
}






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
	}
	else {
		wArgv.length = _args->argc;
		wArgv.items = (wchar_t**)_args->wcharArgv;
		if (alifWStringList_copy(&_config->argv, &wArgv)) {
			// memory error
			return -1;
		}
	}

	return 1;
}
