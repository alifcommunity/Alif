#include "Alif.h"
#include "alifcore_initConfig.h" // ALIFSTATUS_OK()
#include "alifCore_alifCycle.h"

#include <filesystem>


/* ___________ Global Configuration Variables ___________ */

int alifUTF8Mode = 0;
int alifDebugFlag = 0; /* Needed by parser.cpp */
int alifVerboseFlag = 0; /* Needed by import.cpp */
int alifQuietFlag = 0; /* Needed by sysmodule.cpp */
int alifInteractiveFlag = 0; /* Previously, was used by Py_FdIsInteractive() */
int alifInspectFlag = 0; /* Needed to determine whether to exit at SystemExit */
int alifOptimizeFlag = 0; /* Needed by compile.cpp */
int alifNoSiteFlag = 0; /* Suppress 'import site' */
int alifBytesWarningFlag = 0; /* Warn on str(bytes) and str(buffer) */
int alifFrozenFlag = 0; /* Needed by getpath.cpp */
int alifIgnoreEnvironmentFlag = 0; /* e.g. ALIFPATH, ALIFHOME */
int alifDontWriteBytecodeFlag = 0; /* Suppress writing bytecode files (*.alc) */
int alifNoUserSiteDirectory = 0; /* for -s and site.alif */
int alifUnbufferedStdioFlag = 0; /* Unbuffered binary std{in,out,err} */
int alifHashRandomizationFlag = 0; /* for -R and ALIFHASHSEED */
int alifIsolatedFlag = 0; /* for -I, isolate from user's env */
#ifdef MS_WINDOWS
int alifLegacyWindowsFsEncodingFlag = 0; /* Uses mbcs instead of utf-8 */
int alifLegacyWindowsStdioFlag = 0; /* Uses FileIO instead of WindowsConsoleIO */
#endif


/* ___________ AlifWideStringList ___________ */

AlifStatus alifWideStringList_insert(AlifWideStringList* _list, AlifSizeT _index, const wchar_t* _item)
{
	AlifSizeT len = _list->length;
	if (len == ALIFSIZET_MAX) {
		/* length+1 would overflow */
		return ALIFSTATUS_NO_MEMORY();
	}
	if (_index < 0) {
		return ALIFSTATUS_ERR("alifWideStringList_insert مؤشر يجب ان يكون >= 0");
	}
	if (_index > len) {
		_index = len;
	}

	//wchar_t* item2 = alifMem_rawWcsdup(_item);
	//if (item2 == NULL) {
	//	return ALIFSTATUS_NO_MEMORY();
	//}

	size_t size = (len + 1) * sizeof(_list->items[0]);
	//wchar_t** items2 = (wchar_t**)alifMem_rawRealloc(_list->items, size);
	//if (items2 == NULL) {
	//	alifMem_rawFree(item2);
	//	return ALIFSTATUS_NO_MEMORY();
	//}

	//if (_index < len) {
	//	memmove(&items2[_index + 1],
	//		&items2[_index],
	//		(len - _index) * sizeof(items2[0]));
	//}

	//items2[_index] = item2;
	//_list->items = items2;
	//_list->length++;
	return ALIFSTATUS_OK();
}


AlifStatus alifWideStringList_append(AlifWideStringList* _list, const wchar_t* _item)
{
	return alifWideStringList_insert(_list, _list->length, _item);
}



void alifConfig_initCompatConfig(AlifConfig* _config)
{
	_config->configInit = 1;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	//_config->dev_mode = -1;
	//_config->install_signal_handlers = 1;
	//_config->use_hash_seed = -1;
	//_config->faulthandler = -1;
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
	_config->writeByteCode = -1;
	_config->verbose = -1;
	_config->quiet = -1;
	_config->userSiteDirectory = -1;
	//_config->configure_c_stdio = 0;
	//_config->buffered_stdio = -1;
	//_config->_install_importlib = 1;
	//_config->checkHashAlifcsMode = nullptr;
	_config->pathConfigWarnings = -1;
	_config->initMain = 1;
//#ifdef MS_WINDOWS
//	_config->legacy_windows_stdio = -1;
//#endif
#ifdef Py_DEBUG
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
	_config->writeByteCode = 1;
	_config->verbose = 0;
	_config->quiet = 0;
	_config->userSiteDirectory = 1;
	//_config->bufferedStdio = 1;
	_config->pathConfigWarnings = 1;
#ifdef MS_WINDOWS
	//_config->legacyWindowsStdio = 0;
#endif
}

void alifConfig_initAlifConfig(AlifConfig* _config)
{
	config_initDefaults(_config);

	_config->configInit = 2; // CCONFIG_INIT_COMPAT = 1, CCONFIG_INIT_ALIF = 2, CCONFIG_INIT_ISOLATED = 3
	//_config->configure_c_stdio = 1;
	_config->parseArgv = 1;
}


/* Get run_filename absolute path */
static AlifStatus configRun_filenameAbspath(AlifConfig* _config)
{
	std::error_code error;
	std::filesystem::path absPath = std::filesystem::absolute(_config->runFilename, error);
	if(error) {
		return ALIFSTATUS_OK();
	}

	return ALIFSTATUS_OK();
}

static AlifStatus configRead_cmdLine(AlifConfig* _config)
{
	AlifStatus status;

	//status = configParse_cmdLine(config, &cmdline_warnoptions, &opt_index);
	_config->runFilename = _config->argv.items[1]; // temp

	status = configRun_filenameAbspath(_config);

	status = ALIFSTATUS_OK();

	return status;
}

AlifStatus alifConfig_read(AlifConfig* _config)
{
	AlifStatus status;

	status = configRead_cmdLine(_config);

	status = ALIFSTATUS_OK();


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


AlifStatus alifConfig_setCharArgv(AlifConfig* _config, AlifSizeT _argc, char* const* _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useCharArgv = 1,
		.charArgv = _argv,
		.wcharArgv = nullptr
	};
	return alifConfig_setAlifArgv(_config, &args);
}


AlifStatus alifConfig_setArgv(AlifConfig* _config, AlifSizeT _argc, wchar_t* const* _argv)
{
	AlifArgv args = {
		.argc = _argc,
		.useCharArgv = 0,
		.charArgv = nullptr,
		.wcharArgv = _argv
	};

	return alifConfig_setAlifArgv(_config, &args);
}


