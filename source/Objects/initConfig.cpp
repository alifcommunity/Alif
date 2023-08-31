#include "alif.h"
#include "alifCore_fileUtils.h"
#include "alifCore_getOpt.h"
#include "alifCore_initConfig.h"
#include "alifCore_interp.h"
//#include "alifCore_long.h"
//#include "alifCore_pathConfig.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifLifeCycle.h"
#include "alifCore_alifMem.h" 
#include "alifCore_alifState.h"

#include "osDefs.h"

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




static const char usageLine[] = "usage: %ls [option] ... [-c cmd | -m mod | file | -] [arg] ...\n";




static const char usageHelp[] = "\
Options (and corresponding environment variables):\n\
-b     : issue warnings about str(bytes_instance), str(bytearray_instance)\n\
         and comparing bytes/bytearray with str. (-bb: issue errors)\n\
-B     : don't write .alifc files on import; also ALIFDONTWRITEBYTECODE=x\n\
-c cmd : program passed in as string (terminates option list)\n\
-d     : turn on parser debugging output (for experts only, only works on\n\
         debug builds); also ALIFDEBUG=x\n\
-E     : ignore ALIF* environment variables (such as ALIFPATH)\n\
-h     : print this help message and exit (also -? or --help)\n\
-i     : inspect interactively after running script; forces a prompt even\n\
         if stdin does not appear to be a terminal; also ALIFINSPECT=x\n\
-I     : isolate Alif from the user's environment (implies -E and -s)\n\
-m mod : run library module as a script (terminates option list)\n\
-O     : remove assert and __debug__-dependent statements; add .opt-1 before\n\
         .alifc extension; also ALIFOPTIMIZE=x\n\
-OO    : do -O changes and also discard docstrings; add .opt-2 before\n\
         .alifc extension\n\
-P     : don't prepend a potentially unsafe path to sys.path; also ALIFSAFEPATH\n\
-q     : don't print version and coalifright messages on interactive startup\n\
-s     : don't add user site directory to sys.path; also ALIFNOUSERSITE\n\
-S     : don't imply 'import site' on initialization\n\
-u     : force the stdout and stderr streams to be unbuffered;\n\
         this option has no effect on stdin; also ALIFUNBUFFERED=x\n\
-v     : verbose (trace import statements); also ALIFVERBOSE=x\n\
         can be supplied multiple times to increase verbosity\n\
-V     : print the Alif version number and exit (also --version)\n\
         when given twice, print more information about the build\n\
-W arg : warning control; arg is action:message:category:module:lineno\n\
         also ALIFWARNINGS=arg\n\
-x     : skip first line of source, allowing use of non-Unix forms of #!cmd\n\
-X opt : set implementation-specific option\n\
--check-hash-based-alifcs always|default|never:\n\
         control how Alif invalidates hash-based .alifc files\n\
--help-env      : print help about Alif environment variables and exit\n\
--help-xoptions : print help about implementation-specific -X options and exit\n\
--help-all      : print complete help information and exit\n\
Arguments:\n\
file   : program read from script file\n\
-      : program read from stdin (default; interactive mode if a tty)\n\
arg ...: arguments passed to program in sys.argv[1:]\n\
";

static const char usageXOptions[] = "\
The following implementation-specific options are available:\n\
\n\
-X faulthandler: enable faulthandler\n\
\n\
-X showrefcount: output the total reference count and number of used\n\
    memory blocks when the program finishes or after each statement in the\n\
    interactive interpreter. This only works on debug builds\n\
\n\
-X tracemalloc: start tracing Alif memory allocations using the\n\
    tracemalloc module. By default, only the most recent frame is stored in a\n\
    traceback of a trace. Use -X tracemalloc=NFRAME to start tracing with a\n\
    traceback limit of NFRAME frames\n\
\n\
-X importtime: show how long each import takes. It shows module name,\n\
    cumulative time (including nested imports) and self time (excluding\n\
    nested imports). Note that its output may be broken in multi-threaded\n\
    application. Typical usage is alif -X importtime -c 'import asyncio'\n\
\n\
-X dev: enable CAlif's \"development mode\", introducing additional runtime\n\
    checks which are too expensive to be enabled by default. Effect of the\n\
    developer mode:\n\
       * Add default warning filter, as -W default\n\
       * Install debug hooks on memory allocators: see the PyMem_SetupDebugHooks()\n\
         C function\n\
       * Enable the faulthandler module to dump the Alif traceback on a crash\n\
       * Enable asyncio debug mode\n\
       * Set the dev_mode attribute of sys.flags to True\n\
       * io.IOBase destructor logs close() exceptions\n\
\n\
-X utf8: enable UTF-8 mode for operating system interfaces, overriding the default\n\
    locale-aware mode. -X utf8=0 explicitly disables UTF-8 mode (even when it would\n\
    otherwise activate automatically)\n\
\n\
-X alifcache_prefix=PATH: enable writing .alifc files to a parallel tree rooted at the\n\
    given directory instead of to the code tree\n\
\n\
-X warn_default_encoding: enable opt-in EncodingWarning for 'encoding=None'\n\
\n\
-X no_debug_ranges: disable the inclusion of the tables mapping extra location \n\
   information (end line, start column offset and end column offset) to every \n\
   instruction in code objects. This is useful when smaller code objects and alifc \n\
   files are desired as well as suppressing the extra visual location indicators \n\
   when the interpreter displays tracebacks.\n\
\n\
-X perf: activate support for the Linux \"perf\" profiler by activating the \"perf\"\n\
    trampoline. When this option is activated, the Linux \"perf\" profiler will be \n\
    able to report Alif calls. This option is only available on some platforms and will \n\
    do nothing if is not supported on the current system. The default value is \"off\".\n\
\n\
-X frozen_modules=[on|off]: whether or not frozen modules should be used.\n\
   The default is \"on\" (or \"off\" if you are running a local build).\n\
\n\
-X int_max_str_digits=number: limit the size of int<->str conversions.\n\
    This helps avoid denial of service attacks when parsing untrusted data.\n\
    The default is sys.int_info.default_max_str_digits.  0 disables."

#ifdef ALIF_STATS
	"\n\
\n\
-X alifstats: Enable alifstats collection at startup."
#endif
;


static const char usage_envvars[] =
"Environment variables that change behavior:\n"
"ALIFSTARTUP: file executed on interactive startup (no default)\n"
"ALIFPATH   : '%lc'-separated list of directories prefixed to the\n"
"               default module search path.  The result is sys.path.\n"
"ALIFHOME   : alternate <prefix> directory (or <prefix>%lc<exec_prefix>).\n"
"               The default module search path uses %s.\n"
"ALIFPLATLIBDIR : override sys.platlibdir.\n"
"ALIFCASEOK : ignore case in 'import' statements (Windows).\n"
"ALIFUTF8: if set to 1, enable the UTF-8 mode.\n"
"ALIFIOENCODING: Encoding[:errors] used for stdin/stdout/stderr.\n"
"ALIFFAULTHANDLER: dump the Alif traceback on fatal errors.\n"
"ALIFHASHSEED: if this variable is set to 'random', a random value is used\n"
"   to seed the hashes of str and bytes objects.  It can also be set to an\n"
"   integer in the range [0,4294967295] to get hash values with a\n"
"   predictable seed.\n"
"ALIFINTMAXSTRDIGITS: limits the maximum digit characters in an int value\n"
"   when converting from a string and when converting an int back to a str.\n"
"   A value of 0 disables the limit.  Conversions to or from bases 2, 4, 8,\n"
"   16, and 32 are never limited.\n"
"ALIFMALLOC: set the Alif memory allocators and/or install debug hooks\n"
"   on Alif memory allocators. Use ALIFMALLOC=debug to install debug\n"
"   hooks.\n"
"ALIFCOERCECLOCALE: if this variable is set to 0, it disables the locale\n"
"   coercion behavior. Use ALIFCOERCECLOCALE=warn to request display of\n"
"   locale coercion and locale compatibility warnings on stderr.\n"
"ALIFBREAKPOINT: if this variable is set to 0, it disables the default\n"
"   debugger. It can be set to the callable of your debugger of choice.\n"
"ALIFDEVMODE: enable the development mode.\n"
"ALIFPYCACHEPREFIX: root directory for bytecode cache (alifc) files.\n"
"ALIFWARNDEFAULTENCODING: enable opt-in EncodingWarning for 'encoding=None'.\n"
"ALIFNODEBUGRANGES: If this variable is set, it disables the inclusion of the \n"
"   tables mapping extra location information (end line, start column offset \n"
"   and end column offset) to every instruction in code objects. This is useful \n"
"   when smaller code objects and alifc files are desired as well as suppressing the \n"
"   extra visual location indicators when the interpreter displays tracebacks.\n"
"These variables have equivalent command-line parameters (see --help for details):\n"
"ALIFDEBUG             : enable parser debug mode (-d)\n"
"ALIFDONTWRITEBYTECODE : don't write .alifc files (-B)\n"
"ALIFINSPECT           : inspect interactively after running script (-i)\n"
"ALIFINTMAXSTRDIGITS   : limit max digit characters in an int value\n"
"                          (-X int_max_str_digits=number)\n"
"ALIFNOUSERSITE        : disable user site directory (-s)\n"
"ALIFOPTIMIZE          : enable level 1 optimizations (-O)\n"
"ALIFSAFEPATH          : don't prepend a potentially unsafe path to sys.path (-P)\n"
"ALIFUNBUFFERED        : disable stdout/stderr buffering (-u)\n"
"ALIFVERBOSE           : trace import statements (-v)\n"
"ALIFWARNINGS=arg      : warning control (-W arg)\n";

#if defined(MS_WINDOWS)
#  define ALIFHOMEHELP "<prefix>\\alif{major}{minor}"
#else
#  define ALIFHOMEHELP "<prefix>/lib/alifX.X"
#endif



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



























































































































































































































































































































































































































































































































































































































































































































































































































/* --- PyConfig command line parser -------------------------- */

static void config_usage(int _error, const wchar_t* _program)
{
	FILE* f = _error ? stderr : stdout;

	fprintf(f, usageLine, _program);
	if (_error)
		fprintf(f, "Try `alif -h' for more information.\n");
	else {
		fputs(usageHelp, f);
	}
}


static void config_envVarsUsage(void)
{
	printf(usage_envvars, (wint_t)DELIM, (wint_t)DELIM, ALIFHOMEHELP);
}


static void config_xOptionsUsage(void)
{
	puts(usageXOptions);
}


static void config_completeUsage(const wchar_t* _program)
{
	config_usage(0, _program);
	puts("\n");
	config_envVarsUsage();
	puts("\n");
	config_xOptionsUsage();
}




static AlifStatus config_parseCmdline(AlifConfig* _config, AlifWideStringList* _warnOptions, AlifSizeT* _optIndex)
{
	AlifStatus status{};
	const AlifWideStringList* argv = &_config->argv;
	int print_version = 0;
	const wchar_t* program = _config->programName;
	if (!program && argv->length >= 1) {
		program = argv->items[0];
	}

	alifOS_resetGetOpt();
	do {
		int longindex = -1;
		int c = alifOS_getOpt(argv->length, argv->items, &longindex);
		if (c == EOF) {
			break;
		}

		if (c == 'c') {
			if (_config->runCommand == NULL) {
				size_t len = wcslen(alifOSOptArg) + 1 + 1;
				wchar_t* command = (wchar_t*)alifMem_rawMalloc(sizeof(wchar_t) * len); // تم تغغير النوع المرجع بسبب ظهور خطأ
				if (command == NULL) {
					return ALIFSTATUS_NO_MEMORY();
				}
				memcpy(command, alifOSOptArg, (len - 2) * sizeof(wchar_t));
				command[len - 2] = '\n';
				command[len - 1] = 0;
				_config->runCommand = command;
			}
			break;
		}

		if (c == 'm') {
			if (_config->runModule == nullptr) {
				_config->runModule = alifMem_rawWcsDup(alifOSOptArg);
				if (_config->runModule == nullptr) {
					return ALIFSTATUS_NO_MEMORY();
				}
			}
			break;
		}

		switch (c) {
		case 0:
			if (wcscmp(alifOSOptArg, L"always") == 0
				|| wcscmp(alifOSOptArg, L"never") == 0
				|| wcscmp(alifOSOptArg, L"default") == 0)
			{
				status = alifConfig_setString(_config, &_config->checkHashAlifCSMode, alifOSOptArg);
				if (ALIFSTATUS_EXCEPTION(status)) {
					return status;
				}
			}
			else {
				fprintf(stderr, "--checkHashBasedAlifCS must be one of "
					"'default', 'always', or 'never'\n");
				config_usage(1, program);
				return ALIFSTATUS_EXIT(2);
			}
			break;

		case 1:
			config_completeUsage(program);
			return ALIFSTATUS_EXIT(0);

		case 2:
			config_envVarsUsage();
			return ALIFSTATUS_EXIT(0);

		case 3:
			config_xOptionsUsage();
			return ALIFSTATUS_EXIT(0);

		case 'b':
			_config->bytesWarning++;
			break;

		case 'd':
			_config->parserDebug++;
			break;

		case 'i':
			_config->inspect++;
			_config->interactive++;
			break;

		case 'E':
		case 'I':
		case 'X':
			break;


		case 'O':
			_config->optimizationLevel++;
			break;

		case 'P':
			_config->safePath = 1;
			break;

		case 'B':
			_config->writeBytecode = 0;
			break;

		case 's':
			_config->userSiteDirectory = 0;
			break;

		case 'S':
			_config->siteImport = 0;
			break;

		case 't':
			break;

		case 'u':
			_config->bufferedStdio = 0;
			break;

		case 'v':
			_config->verbose++;
			break;

		case 'x':
			_config->skipSourceFirstLine = 1;
			break;

		case 'h':
		case '?':
			config_usage(0, program);
			return ALIFSTATUS_EXIT(0);

		case 'V':
			print_version++;
			break;

		case 'W':
			status = alifWideStringList_append(_warnOptions, alifOSOptArg);
			if (ALIFSTATUS_EXCEPTION(status)) {
				return status;
			}
			break;

		case 'q':
			_config->quiet++;
			break;

		case 'R':
			_config->useHashSeed = 0;
			break;


		default:
			config_usage(1, program);
			return ALIFSTATUS_EXIT(2);
		}
	} while (1);

	if (print_version) {
		printf("Alif %s\n",
			(print_version >= 2) ? alif_getVersion() : ALIF_VERSION);
		return ALIFSTATUS_EXIT(0);
	}

	if (_config->runCommand == nullptr && _config->runModule == nullptr
		&& alifOSOptind < argv->length
		&& wcscmp(argv->items[alifOSOptind], L"-") != 0
		&& _config->runFilename == nullptr)
	{
		_config->runFilename = alifMem_rawWcsDup(argv->items[alifOSOptind]);
		if (_config->runFilename == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	if (_config->runCommand != nullptr || _config->runModule != nullptr) {
		alifOSOptind--;
	}

	*_optIndex = alifOSOptind;

	return ALIFSTATUS_OK();
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



static AlifStatus configRun_fileNameAbspath(AlifConfig* _config)
{
	if (!_config->runFilename) {
		return ALIFSTATUS_OK();
	}

#ifndef MS_WINDOWS
	if (alif_isAbs(config->runFilename)) {
		return ALIFSTATUS_OK();
	}
#endif

	wchar_t* absFilename{};
	if (alif_absPath(_config->runFilename, &absFilename) < 0) {
		return ALIFSTATUS_OK();
	}
	if (absFilename == nullptr) {
		return ALIFSTATUS_NO_MEMORY();
	}

	alifMem_rawFree(_config->runFilename);
	_config->runFilename = absFilename;
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

		status = configRun_fileNameAbspath(_config);
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





























































































































































































































































































































