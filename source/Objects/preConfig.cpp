#include "Alif.h"
#include "alifCore_getOpt.h"
#include "alifCore_initConfig.h"
#include "alifCore_alifCycle.h"
#include "alifCore_alifMem.h"


/* ___________ AlifArgv ___________ */

AlifStatus alifArgv_asWstrList(const AlifArgv* _args, AlifWideStringList* _list)
{
	AlifWideStringList wArgv = { .length = 0, .items = nullptr };
	if (_args->useCharArgv) {

	}
	else {
		wArgv.length = _args->argc;
		wArgv.items = (wchar_t**)_args->wcharArgv;

		*_list = wArgv;
	}


	return ALIFSTATUS_OK();
}


/* ___________ AlifPreCmdline ___________ */

AlifStatus alifPreCmdLine_setArgv(AlifPreCmdLine* _cmdline, const AlifArgv* _args)
{
	return alifArgv_asWstrList(_args, &_cmdline->argv);
}


static void preCmdLine_getPreConfig(AlifPreCmdLine* _cmdLine, const AlifPreConfig* _config)
{
#define COPY_ATTR(ATTR) \
    if (_config->ATTR != -1) { _cmdLine->ATTR = _config->ATTR;}

	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	//COPY_ATTR(devMode);

#undef COPY_ATTR
}


static void preCmdLine_setPreConfig(const AlifPreCmdLine* _cmdLine, AlifPreConfig* _config)
{
#define COPY_ATTR(ATTR) _config->ATTR = _cmdLine->ATTR

	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	//COPY_ATTR(devMode);

#undef COPY_ATTR
}


/* Parse the command line arguments */
static AlifStatus preCmdLine_parseCmdLine(AlifPreCmdLine* _cmdLine)
{
	const AlifWideStringList* argv = &_cmdLine->argv;

	alifOS_resetGetOpt();
	/* Don't log parsing errors into stderr here: alifConfig_read()
	   is responsible for that */
	alifOsOptErr = 0;
	do {
		int longIndex = -1;
		int c = alifOS_getOpt(argv->length, argv->items, &longIndex);

		if (c == EOF || c == 'c' || c == 'm') {
			break;
		}

		switch (c) {
		case 'E':
			_cmdLine->useEnvironment = 0;
			break;

		case 'I':
			_cmdLine->isolated = 1;
			break;

		case 'X':
		{
			AlifStatus status = alifWideStringList_append(&_cmdLine->xoptions, alifOsOptArg);
			if (ALIFSTATUS_EXCEPTION(status)) {
				return status;
			}
			break;
		}

		default:
			/* ignore other argument:
			   handled by alifConfig_read() */
			break;
		}
	} while (1);

	return ALIFSTATUS_OK();
}


AlifStatus alifPreCmdLine_read(AlifPreCmdLine* _cmdLine, const AlifPreConfig* _preConfig)
{
	preCmdLine_getPreConfig(_cmdLine, _preConfig);

	if (_preConfig->parseArgv) {
		AlifStatus status = preCmdLine_parseCmdLine(_cmdLine);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	/* isolated, use_environment */
	if (_cmdLine->isolated < 0) {
		_cmdLine->isolated = 0;
	}
	if (_cmdLine->isolated > 0) {
		_cmdLine->useEnvironment = 0;
	}
	if (_cmdLine->useEnvironment < 0) {
		_cmdLine->useEnvironment = 0;
	}

	/* dev_mode */
	//if ((_cmdline->devMode < 0) && (alif_getXoption(&_cmdline->xoptions, L"تطوير") || alif_getEnv(_cmdline->use_environment, "ALIFDEVMODE")))
	//{
	//	_cmdline->devMode = 1;
	//}
	//if (_cmdline->devMode < 0) {
	//	_cmdline->devMode = 0;
	//}

	// warn_default_encoding
	if (alifGet_xOption(&_cmdLine->xoptions, L"warnDefaultEncoding") || alif_getEnv(_cmdLine->useEnvironment, "ALIFWARNDEFAULTENCODING"))
	{
		_cmdLine->warnDefaultEncoding = 1;
	}

	//assert(cmdline->useEnvironment >= 0);
	//assert(cmdline->isolated >= 0);
	//assert(cmdline->warnDefaultEncoding >= 0);

	return ALIFSTATUS_OK();
}


































































































































/* ___________ AlifPreConfig ___________ */


void alifPreConfig_initCompatConfig(AlifPreConfig* _config)
{
	memset(_config, 0, sizeof(*_config));

	_config->configInit = (int)AlifConfig_Init_Compat;
	_config->parseArgv = 0;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->configureLocale = 1;

	_config->utf8Mode = 0;
	_config->coerceCppLocale = 0;
	_config->coerceCppLocaleWarn = 0;

	_config->devMode = -1;
	_config->allocator = AlifMem_Allocator_Not_Set;
#ifdef MS_WINDOWS
	_config->legacyWindowsFsEncoding = -1;
#endif
}








void alifPreConfig_initAlifConfig(AlifPreConfig* _config)
{
	alifPreConfig_initCompatConfig(_config);

	_config->configInit = (int)AlifConfig_Init_Alif;
	_config->isolated = 0;
	_config->parseArgv = 1;
	_config->useEnvironment = 1;
	_config->coerceCppLocale = -1;
	_config->coerceCppLocaleWarn = -1;
	_config->utf8Mode = -1;
#ifdef MS_WINDOWS
	_config->legacyWindowsFsEncoding = 0;
#endif
}


AlifStatus alifPreConfig_initFromPreConfig(AlifPreConfig* _config)
{
	alifPreConfig_initAlifConfig(_config);
	return ALIFSTATUS_OK();
}


static void preConfig_getGlobalVars(AlifPreConfig* _config)
{
	if (_config->configInit != 1) // AlifConfig_INIT_COMPAT = 1
	{
		/* Alif and Isolated configuration ignore global variables */
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

	ALIF_COMP_DIAG_PUSH
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS
	COPY_FLAG(isolated, alifIsolatedFlag);
	COPY_NOT_FLAG(useEnvironment, alifIgnoreEnvironmentFlag);
	if (alifUTF8Mode > 0) {
		_config->utf8Mode = alifUTF8Mode;
	}
#ifdef MS_WINDOWS
	//COPY_FLAG(legacyWindowsFsEncoding, alifLegacyWindowsFSEncodingFlag);
#endif
	ALIF_COMP_DIAG_POP

#undef COPY_FLAG
#undef COPY_NOT_FLAG
}

static void preConfig_setGlobalVars(const AlifPreConfig* _config)
{
#define COPY_FLAG(ATTR, VAR) \
    if (_config->ATTR >= 0) { \
        VAR = _config->ATTR; \
    }
#define COPY_NOT_FLAG(ATTR, VAR) \
    if (_config->ATTR >= 0) { \
        VAR = !_config->ATTR; \
    }

	ALIF_COMP_DIAG_PUSH
		ALIF_COMP_DIAG_IGNORE_DEPR_DECLS
		COPY_FLAG(isolated, alifIsolatedFlag);
	COPY_NOT_FLAG(useEnvironment, alifIgnoreEnvironmentFlag);
#ifdef MS_WINDOWS
	//COPY_FLAG(legacyWindowsFsEncoding, alifLegacyWindowsFSEncodingFlag);
#endif
	COPY_FLAG(utf8Mode, alifUTF8Mode);
	ALIF_COMP_DIAG_POP

#undef COPY_FLAG
#undef COPY_NOT_FLAG
}

const char* alif_getEnv(int _useEnvironment, const char* _name)
{
	//assert(_useEnvironment >= 0);

	if (!_useEnvironment) {
		return NULL;
	}

	const char* var = getenv(_name);
	if (var && var[0] != '\0') {
		return var;
	}
	else {
		return NULL;
	}
}


const wchar_t* alifGet_xOption(const AlifWideStringList* _xoptions, const wchar_t* _name)
{
	for (AlifSizeT i = 0; i < _xoptions->length; i++) {
		const wchar_t* option = _xoptions->items[i];
		size_t len;
		wchar_t* sep = (wchar_t*)wcschr(option, L'='); // تم عمل تغيير للنوع المرجع - يجب المراجعة قبل الإعتماد
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


static AlifStatus preConfig_initUtf8Mode(AlifPreConfig* _config, const AlifPreCmdLine* _cmdLine)
{
//#ifdef MS_WINDOWS
//	if (_config->legacyWindowsFsEncoding) {
//		_config->utf8Mode = 0;
//	}
//#endif

	if (_config->utf8Mode >= 0) {
		return ALIFSTATUS_OK();
	}

	const wchar_t* xopt;
	xopt = alifGet_xOption(&_cmdLine->xoptions, L"utf8");
	if (xopt) {
		wchar_t* sep = (wchar_t*)wcschr(xopt, L'='); // تم عمل تغيير لنوع القيمة المرجعة - يجب المراجعة قبل الإعتماد
		if (sep) {
			xopt = sep + 1;
			if (wcscmp(xopt, L"1") == 0) {
				_config->utf8Mode = 1;
			}
			else if (wcscmp(xopt, L"0") == 0) {
				_config->utf8Mode = 0;
			}
			else {
				return ALIFSTATUS_ERR("فشل -X utf8");
			}
		}
		else {
			_config->utf8Mode = 1;
		}
		return ALIFSTATUS_OK();
	}

	const char* opt = alif_getEnv(_config->useEnvironment, "ALIFUTF8");
	if (opt) {
		if (strcmp(opt, "1") == 0) {
			_config->utf8Mode = 1;
		}
		else if (strcmp(opt, "0") == 0) {
			_config->utf8Mode = 0;
		}
		else {
			return ALIFSTATUS_ERR("قيمة متغير البيئة ALIFUTF8 فشلت");
		}
		return ALIFSTATUS_OK();
	}


#ifndef MS_WINDOWS
	if (config->utf8Mode < 0) {
		/* The C locale and the POSIX locale enable the UTF-8 Mode */
		const char* ctypeLoc = setlocale(LC_CTYPE, nullptr);
		if (ctypeLoc != nullptr && (strcmp(ctypeLoc, "C") == 0 || strcmp(ctypeLoc, "POSIX") == 0)) {
			_config->utf8Mode = 1;
		}
	}
#endif

	if (_config->utf8Mode < 0) {
		_config->utf8Mode = 0;
	}
	return ALIFSTATUS_OK();
}


static AlifStatus preConfig_initAllocator(AlifPreConfig* _config)
{
	if (_config->allocator == ALIFMEM_ALLOCATOR_NOT_SET) {
		/* bpo-34247. The PYTHONMALLOC environment variable has the priority
		   over PYTHONDEV env var and "-X dev" command line option.
		   For example, PYTHONMALLOC=malloc PYTHONDEVMODE=1 sets the memory
		   allocators to "malloc" (and not to "debug"). */
		const char* envvar = alif_getEnv(_config->useEnvironment, "ALIFMALLOC");
		if (envvar) {
			AlifMemAllocatorName name{};
			//if (alifMem_getAllocatorName(envvar, &name) < 0) {
			//	return ALIFSTATUS_ERR("ALIFMALLOC: حجز ذاكرة غير محدد");
			//}
			_config->allocator = (int)name;
		}
	}

	//if (_config->devMode && _config->allocator == ALIFMEM_ALLOCATOR_NOT_SET) {
	//	_config->allocator = ALIFMEM_ALLOCATOR_DEBUG;
	//}
	return ALIFSTATUS_OK();
}


static AlifStatus preConfig_read(AlifPreConfig* _config, AlifPreCmdLine* _cmdLine)
{
	AlifStatus status{};

	status = alifPreCmdLine_read(_cmdLine, _config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	preCmdLine_setPreConfig(_cmdLine, _config);

	/* legacy_windows_fs_encoding, coerce_c_locale, utf8Mode */
#ifdef MS_WINDOWS
	//alif_getEnvFlag(_config->useEnvironment, &_config->legacy_windows_fs_encoding, "ALIFLEGACYWINDOWSFSENCODING");
#endif

	//preConfig_init_coerce_c_locale(config);

	status = preConfig_initUtf8Mode(_config, _cmdLine);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	/* allocator */
	status = preConfig_initAllocator(_config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//assert(_config->coerce_c_locale >= 0);
	//assert(_config->coerce_c_locale_warn >= 0);
//#ifdef MS_WINDOWS
//	assert(config->legacy_windows_fs_encoding >= 0);
//#endif
//	assert(_config->utf8Mode >= 0);
//	assert(_config->isolated >= 0);
//	assert(_config->useEnvironment >= 0);
//	assert(_config->devMode >= 0);

	return ALIFSTATUS_OK();
}


/* Read the configuration from:

   - command line arguments
   - environment variables
   - Alif_xxx global configuration variables
   - the LC_CTYPE locale
*/
AlifStatus alifPreConfig_read(AlifPreConfig* _config, const AlifArgv* _args)
{
	AlifStatus status{};

	//status = alifRuntime_initialize();
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	preConfig_getGlobalVars(_config);

	/* Copy LC_CTYPE locale, since it's modified later */
	const char* loc = std::setlocale(LC_CTYPE, nullptr);
	if (loc == nullptr) {
		return ALIFSTATUS_ERR("فشلت عملية تهيئة LC_CTYPE");
	}
	char* initCtypeLocale = alifMem_rawStrDup(loc); // يوجد مشكلة في الذاكرة ويجب التحقق ما إذا تم حلها
	if (initCtypeLocale == nullptr) {
		return ALIFSTATUS_NO_MEMORY();
	}

	/* Save the config to be able to restore it if encodings change */
	AlifPreConfig saveConfig{};

	status = alifPreConfig_initFromPreConfig(&saveConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	/* Set LC_CTYPE to the user preferred locale */
	if (_config->configureLocale) {
		alif_setLocaleFromEnv(LC_CTYPE);
	}

	AlifPreConfig saveRuntimeConfig{};
	//preConfig_copy(&saveRuntimeConfig, &alifRuntime.preconfig);

	AlifPreCmdLine cmdLine = ALIFPRECMDLINE_INIT;
	//int locale_coerced = 0;
	int loops = 0;

	while (1) {
		int utf8Mode = _config->utf8Mode;

		/* to prevent an infinite loop */
		loops++;
		if (loops == 3) {
			status = ALIFSTATUS_ERR("تم تغيير الترميز مرتين اثناء تهيئة الملف");
			goto done;
		}

		/* bpo-34207: alif_decodeLocale() and alif_encodeLocale() depend
		   on the utf8Mode and legacyWindowsFSEncoding members
		   of alifRuntime.preConfig. */
		//preConfig_copy(&alifRuntime.preConfig, _config);

		if (_args) {
			// Set command line arguments at each iteration. If they are bytes
			// strings, they are decoded from the new encoding.
			status = alifPreCmdLine_setArgv(&cmdLine, _args);
			if (ALIFSTATUS_EXCEPTION(status)) {
				goto done;
			}
		}

		status = preConfig_read(_config, &cmdLine);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}

		int encodingChanged = 0;
		//if (_config->coerceCLocale && !localeCoerced) {
		//	localeCoerced = 1;
		//	alif_coerceLegacyLocale(0);
		//	encoding_changed = 1;
		//}

		if (utf8Mode == -1) {
			if (_config->utf8Mode == 1) {
				/* UTF-8 Mode enabled */
				encodingChanged = 1;
			}
		}
		else {
			if (_config->utf8Mode != utf8Mode) {
				encodingChanged = 1;
			}
		}

		if (!encodingChanged) {
			break;
		}


		int newUtf8Mode = _config->utf8Mode;
		//int newCoerceCLocale = _config->coerceCLocale;
		//preConfig_copy(_config, &saveConfig);
		*_config = saveConfig;
		_config->utf8Mode = newUtf8Mode;
		//_config->coerceCLocale = newCoerceCLocale;

		/* The encoding changed: read again the configuration
		   with the new encoding */
	}
	status = ALIFSTATUS_OK();

done:
	// Revert side effects
	setlocale(LC_CTYPE, initCtypeLocale);
	//alifMem_rawFree(initCtypeLocale);
	//prConfig_copy(&alifRuntime.preconfig, &saveRuntimeConfig);
	//alifPreCmdLine_clear(&cmdLine);
	return status;
}


/* Write the pre-configuration:

   - set the memory allocators
   - set alif_xxx global configuration variables
   - set the LC_CTYPE locale (coerce C locale) and set the UTF-8 mode

   The applied configuration is written into alifRuntime.preConfig.
   If the C locale cannot be coerced, set coerce_c_locale to 0.

   Do nothing if called after alif_initialize(): ignore the new
   pre-configuration. */
AlifStatus alifPreConfig_write(const AlifPreConfig* _srcConfig)
{
	AlifPreConfig config{};

	//AlifStatus status = alifPreConfig_initFromPreConfig(&config, _srcConfig);
	AlifStatus status = alifPreConfig_initFromPreConfig(&config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//if (alifRuntime.core_initialized) {
	//	/* Calling this functions after alif_initialize() ignores
	//	   the new configuration. */
	//	return ALIFSTATUS_OK();
	//}

	AlifMemAllocatorName name = (AlifMemAllocatorName)config.allocator;
	if (name != ALIFMEM_ALLOCATOR_NOT_SET) {
		//if (alifMem_setupAllocators(name) < 0) {
		//	return ALIFSTATUS_ERR("غير معروف ALIFMALLOC حجز ذاكرة");
		//}
	}

	preConfig_setGlobalVars(&config);

	if (config.configureLocale) {
		//if (config.coerce_c_locale) {
		//	if (!_Py_CoerceLegacyLocale(config.coerce_c_locale_warn)) {
		//		/* C locale not coerced */
		//		config.coerce_c_locale = 0;
		//	}
		//}

		/* Set LC_CTYPE to the user preferred locale */
		alif_setLocaleFromEnv(LC_CTYPE);
	}

	/* Write the new pre-configuration into alifRuntime */
	//preConfig_copy(&alifRuntime.preConfig, &config);

	return ALIFSTATUS_OK();
}
