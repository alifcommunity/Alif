#include "alif.h"
#include "alifCore_fileUtils.h"
#include "alifCore_getOpt.h"
#include "alifCore_initConfig.h"
#include "alifCore_alifLifeCycle.h"
#include "alifCore_alifMem.h"
#include "alifCore_runtime.h"

#include <locale.h>
//#include <stdlib.h>


/* Forward declarations */
static void preConfig_copy(AlifPreConfig*, const AlifPreConfig*);































































AlifStatus alifArgv_asWstrList(const AlifArgv* _args, AlifWideStringList* _list)
{
	AlifWideStringList wargv = ALIFWIDESTRINGLIST_INIT;
	if (_args->useBytesArgv) {
		size_t size = sizeof(wchar_t*)* _args->argc;
		wargv.items = (wchar_t**)alifMem_rawMalloc(size);
		if (wargv.items == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}

		for (AlifSizeT i = 0; i < _args->argc; i++) {
			size_t len{};
			wchar_t* arg = alif_decodeLocale(_args->bytesArgv[i], &len);
			if (arg == nullptr) {
				alifWideStringList_clear(&wargv);
				//return DECODE_LOCALE_ERR("command line arguments", len);
				if (len == (size_t)-2) return  ALIFSTATUS_ERR("cannot decode command line arguments"); else return ALIFSTATUS_NO_MEMORY();
			}
			wargv.items[i] = arg;
			wargv.length++;
		}

		alifWideStringList_clear(_list);
		*_list = wargv;
	}
	else {
		wargv.length = _args->argc;
		wargv.items = (wchar_t**)_args->wcharArgv;
		if (alifWideStringList_copy(_list, &wargv) < 0) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}
	return ALIFSTATUS_OK();
}



















static void preCmdline_getPreConfig(AlifPreCmdline* _cmdline, const AlifPreConfig* _config)
{
#define COPY_ATTR(ATTR) \
    if (_config->ATTR != -1) { \
        _cmdline->ATTR = _config->ATTR; \
    }

	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(devMode);

#undef COPY_ATTR
}

















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




static AlifStatus preCmdline_parseCmdline(AlifPreCmdline* _cmdline)
{
	const AlifWideStringList* argv = &_cmdline->argv;

	alifOS_resetGetOpt();

	alifOSOptErr = 0;
	do {
		int longindex = -1;
		int c = alifOS_getOpt(argv->length, argv->items, &longindex);

		if (c == EOF || c == 'c' || c == 'm') {
			break;
		}

		switch (c) {
		case 'E':
			_cmdline->useEnvironment = 0;
			break;

		case 'I':
			_cmdline->isolated = 1;
			break;

		case 'X':
		{
			AlifStatus status = alifWideStringList_append(&_cmdline->xOptions, alifOSOptArg);
			if (ALIFSTATUS_EXCEPTION(status)) {
				return status;
			}
			break;
		}

		default:
			break;
		}
	} while (1);

	return ALIFSTATUS_OK();
}







AlifStatus alifPreCmdline_read(AlifPreCmdline* _cmdline, const AlifPreConfig* _preConfig)
{
	preCmdline_getPreConfig(_cmdline, _preConfig);

	if (_preConfig->parseArgv) {
		AlifStatus status = preCmdline_parseCmdline(_cmdline);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	if (_cmdline->isolated < 0) {
		_cmdline->isolated = 0;
	}
	if (_cmdline->isolated > 0) {
		_cmdline->useEnvironment = 0;
	}
	if (_cmdline->useEnvironment < 0) {
		_cmdline->useEnvironment = 0;
	}

	if ((_cmdline->devMode < 0)
		&& (alif_getXOption(&_cmdline->xOptions, L"dev")
			|| alif_getEnv(_cmdline->useEnvironment, "ALIFDEVMODE")))
	{
		_cmdline->devMode = 1;
	}
	if (_cmdline->devMode < 0) {
		_cmdline->devMode = 0;
	}


	if (alif_getXOption(&_cmdline->xOptions, L"warnDefaultEncoding")
		|| alif_getEnv(_cmdline->useEnvironment, "ALIFWARNDEFAULTENCODING"))
	{
		_cmdline->warnDefaultEncoding = 1;
	}

	//assert(_cmdline->useEnvironment >= 0);
	//assert(_cmdline->isolated >= 0);
	//assert(_cmdline->devMode >= 0);
	//assert(_cmdline->warnDefaultEncoding >= 0);

	return ALIFSTATUS_OK();
}





/* ----- AlifPreConfig ------------------------------------------- */


void alifPreConfig_initCompatConfig(AlifPreConfig* _config)
{
	memset(_config, 0, sizeof(*_config));

	_config->configInit = (int)AlifConfig_Init_Compat;
	_config->parseArgv = 0;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->configureLocale = 1;
	_config->utf8Mode = 0;
	_config->coerceCLocale = 0;
	_config->coerceCLocaleWarn = 0;
	_config->devMode = -1;
	_config->allocator = AlifMem_Allocator_NotSet;
#ifdef MS_WINDOWS
	_config->legacyWindowsFSEncoding = -1;
#endif
}










void alifPreConfig_initAlifConfig(AlifPreConfig* _config)
{
	alifPreConfig_initCompatConfig(_config);

	_config->configInit = (int)AlifConfig_Init_Alif;
	_config->isolated = 0;
	_config->parseArgv = 1;
	_config->useEnvironment = 1;
	_config->coerceCLocale = -1;
	_config->coerceCLocaleWarn = -1;
	_config->utf8Mode = -1;
#ifdef MS_WINDOWS
	_config->legacyWindowsFSEncoding = 0;
#endif
}






void alifPreConfig_initIsolatedConfig(AlifPreConfig* _config)
{
	alifPreConfig_initCompatConfig(_config);

	_config->configInit = (int)AlifConfig_Init_Isolated;
	_config->configureLocale = 0;
	_config->isolated = 1;
	_config->useEnvironment = 0;
	_config->utf8Mode = 0;
	_config->devMode = 0;
#ifdef MS_WINDOWS
	_config->legacyWindowsFSEncoding = 0;
#endif
}



AlifStatus alifPreConfig_initFromPreConfig(AlifPreConfig* _config, const AlifPreConfig* _config2)
{
	alifPreConfig_initAlifConfig(_config);
	preConfig_copy(_config, _config2);
	return ALIFSTATUS_OK();
}




void alifPreConfig_initFromConfig(AlifPreConfig* _preConfig, const AlifConfig* _config)
{
	AlifConfigInitEnum configInit = (AlifConfigInitEnum)_config->configInit;
	switch (configInit) {
	case AlifConfig_Init_Alif:
		alifPreConfig_initAlifConfig(_preConfig);
		break;
	case AlifConfig_Init_Isolated:
		alifPreConfig_initIsolatedConfig(_preConfig);
		break;
	case AlifConfig_Init_Compat:
	default:
		alifPreConfig_initCompatConfig(_preConfig);
	}

	alifPreConfig_getConfig(_preConfig, _config);
}



static void preConfig_copy(AlifPreConfig* _config, const AlifPreConfig* _config2)
{
#define COPY_ATTR(ATTR) _config->ATTR = _config2->ATTR

	COPY_ATTR(configInit);
	COPY_ATTR(parseArgv);
	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(configureLocale);
	COPY_ATTR(devMode);
	COPY_ATTR(coerceCLocale);
	COPY_ATTR(coerceCLocaleWarn);
	COPY_ATTR(utf8Mode);
	COPY_ATTR(allocator);
#ifdef MS_WINDOWS
	COPY_ATTR(legacyWindowsFSEncoding);
#endif

#undef COPY_ATTR
}

















































void alifPreConfig_getConfig(AlifPreConfig* _preConfig, const AlifConfig* _config)
{
#define COPY_ATTR(ATTR) \
    if (_config->ATTR != -1) { \
        _preConfig->ATTR = _config->ATTR; \
    }

	COPY_ATTR(parseArgv);
	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(devMode);

#undef COPY_ATTR
}



static void preConfig_getGlobalVars(AlifPreConfig* _config)
{
	if (_config->configInit != AlifConfig_Init_Compat) {
		return;
	}

#define COPY_FLAG(ATTR, VALUE) \
    if (_config->ATTR < 0) { \
        _config->ATTR = VALUE; \
    }
#define COPY_NOT_FLAG(ATTR, VALUE) \
    if (_config->ATTR < 0) { \
        _config->ATTR = !(VALUE); \
    }

	ALIFCOMP_DIAGPUSH
		ALIFCOMP_DIAGIGNORE_DEPRDECLS
		COPY_FLAG(isolated, alifIsolatedFlag);
	COPY_NOT_FLAG(useEnvironment, alifIgnoreEnvironmentFlag);
	if (alifUTF8Mode > 0) {
		_config->utf8Mode = alifUTF8Mode;
	}
#ifdef MS_WINDOWS
	COPY_FLAG(legacyWindowsFSEncoding, alifLegacyWindowsFSEncodingFlag);
#endif
	ALIFCOMP_DIAGPOP

#undef COPY_FLAG
#undef COPY_NOT_FLAG
}




static void preConfig_setGlobalVars(const AlifPreConfig* config)
{
#define COPY_FLAG(ATTR, VAR) \
    if (config->ATTR >= 0) { \
        VAR = config->ATTR; \
    }
#define COPY_NOT_FLAG(ATTR, VAR) \
    if (config->ATTR >= 0) { \
        VAR = !config->ATTR; \
    }

	ALIFCOMP_DIAGPUSH
		ALIFCOMP_DIAGIGNORE_DEPRDECLS
		COPY_FLAG(isolated, alifIsolatedFlag);
	COPY_NOT_FLAG(useEnvironment, alifIgnoreEnvironmentFlag);
#ifdef MS_WINDOWS
	COPY_FLAG(legacyWindowsFSEncoding, alifLegacyWindowsFSEncodingFlag);
#endif
	COPY_FLAG(utf8Mode, alifUTF8Mode);
	ALIFCOMP_DIAGPOP

#undef COPY_FLAG
#undef COPY_NOT_FLAG
}



const char* alif_getEnv(int _useEnvironment, const char* name)
{
	//assert(_useEnvironment >= 0);

	if (!_useEnvironment) {
		return nullptr;
	}

	const char* var = getenv(name);
	if (var && var[0] != '\0') {
		return var;
	}
	else {
		return nullptr;
	}
}







































const wchar_t* alif_getXOption(const AlifWideStringList* _xOptions, const wchar_t* _name)
{
	for (AlifSizeT i = 0; i < _xOptions->length; i++) {
		const wchar_t* option = _xOptions->items[i];
		size_t len;
		wchar_t* sep = (wchar_t*)wcschr(option, L'='); // تم تعديل المرجع بسبب ظهور خطأ
		if (sep != NULL) {
			len = (sep - option);
		}
		else {
			len = wcslen(option);
		}
		if (wcsncmp(option, _name, len) == 0 && _name[len] == L'\0') {
			return option;
		}
	}
	return NULL;
}






































































































































































































AlifStatus alifPreConfig_read(AlifPreConfig* _config, const AlifArgv* _args)
{
	AlifStatus status{};

	//status = alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	preConfig_getGlobalVars(_config);

	const char* loc = setlocale(LC_CTYPE, nullptr);
	if (loc == nullptr) {
		return ALIFSTATUS_ERR("failed to LC_CTYPE locale");
	}
	//char* initCTypeLocale = alifMem_rawStrdup(loc);
	//if (initCTypeLocale == nullptr) {
	//	return ALIFSTATUS_NO_MEMORY();
	//}

	AlifPreConfig saveConfig{};

	status = alifPreConfig_initFromPreConfig(&saveConfig, _config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//if (_config->configureLocale) {
	//	alif_setLocaleFromEnv(LC_CTYPE);
	//}

	AlifPreConfig saveRuntimeConfig{};
	//preConfig_copy(&saveRuntimeConfig, &alifRuntime.preConfig);

	//AlifPreCmdline cmdline = ALIFPRECMDLINE_INIT;
	int localeCoerced = 0;
	int loops = 0;

	while (1) {
		int utf8Mode = _config->utf8Mode;

		loops++;
		if (loops == 3) {
			status = ALIFSTATUS_ERR("Encoding changed twice while "
				"reading the configuration");
			goto done;
		}

		//preConfig_copy(&alifRuntime.preConfig, _config);

		if (_args) {
			//status = alifPreCmdline_setArgv(&cmdline, _args);
			if (ALIFSTATUS_EXCEPTION(status)) {
				goto done;
			}
		}

		//status = preConfig_read(_config, &cmdline);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}

		int encoding_changed = 0;
		if (_config->coerceCLocale && !localeCoerced) {
			localeCoerced = 1;
			//alif_coerceLegacyLocale(0);
			encoding_changed = 1;
		}

		if (utf8Mode == -1) {
			if (_config->utf8Mode == 1) {
				encoding_changed = 1;
			}
		}
		else {
			if (_config->utf8Mode != utf8Mode) {
				encoding_changed = 1;
			}
		}

		if (!encoding_changed) {
			break;
		}

		int newUtf8Mode = _config->utf8Mode;
		int newCoerceCLocale = _config->coerceCLocale;
		preConfig_copy(_config, &saveConfig);
		_config->utf8Mode = newUtf8Mode;
		_config->coerceCLocale = newCoerceCLocale;
	}
	status = ALIFSTATUS_OK();

done:
	//setlocale(LC_CTYPE, initCTypeLocale);
	//alifMem_rawFree(initCTypeLocale);
	//preConfig_copy(&alifRuntime.preconfig, &saveRuntimeConfig);
	//alifPreCmdline_clear(&cmdline);
	return status;
}









































AlifStatus alifPreConfig_write(const AlifPreConfig* _srcConfig)
{
	AlifPreConfig config{};

	AlifStatus status = alifPreConfig_initFromPreConfig(&config, _srcConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//if (alifRuntime.coreInitialized) {
	//	return ALIFSTATUS_OK();
	//}

	AlifMemAllocatorName name = (AlifMemAllocatorName)config.allocator;
	if (name != AlifMem_Allocator_NotSet) {
		//if (alifMem_setupAllocators(name) < 0) {
		//	return ALIFSTATUS_ERR("Unknown ALIFMALLOC allocator");
		//}
	}

	preConfig_setGlobalVars(&config);

	if (config.configureLocale) {
		if (config.coerceCLocale) {
			if (!alif_coerceLegacyLocale(config.coerceCLocaleWarn)) {
				config.coerceCLocale = 0;
			}
		}

		alif_setLocaleFromEnv(LC_CTYPE);
	}

	//preConfig_copy(&alifRuntime.preConfig, &config);

	return ALIFSTATUS_OK();
}








































