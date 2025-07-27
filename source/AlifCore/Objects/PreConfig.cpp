#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Runtime.h"


AlifStatus alifArgv_asWStringList(AlifConfig* _config, AlifArgv* _args) { // 78


	AlifWStringList wArgv = { .length = 0, .items = nullptr };
	if (_args->useBytesArgv) {
		wArgv.items = (wchar_t**)alifMem_dataAlloc(_args->argc * sizeof(wchar_t*));
		if (wArgv.items == nullptr) {
			return ALIFSTATUS_NO_MEMORY();
		}

		for (AlifIntT i = 0; i < _args->argc; i++) {
			AlifUSizeT len{};
			wchar_t* arg = alif_decodeLocale(_args->bytesArgv[i], &len);
			if (arg == nullptr) {
				_alifWStringList_clear(&wArgv);
				DECODE_LOCALE_ERR("command line arguments", len);
			}
			wArgv.items[i] = arg;
			wArgv.length++;
		}

		_alifWStringList_clear(&_config->argv);
		_config->argv = wArgv;
	}
	else {
		wArgv.length = _args->argc;
		wArgv.items = (wchar_t**)_args->wcharArgv;
		if (alifWStringList_copy(&_config->argv, &wArgv)) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	return ALIFSTATUS_OK();
}







/* ---------------------------------- AlifPreConfig ---------------------------------- */


void _alifPreConfig_initCompatConfig(AlifPreConfig* _config) { // 280
	memset(_config, 0, sizeof(*_config));

	//_config->configInit = (AlifIntT)AlifConfig_Init_COMPAT;
	_config->parseArgv = 0;
	_config->isolated = -1;
	_config->useEnvironment = -1;
	_config->configureLocale = 1;

	_config->utf8Mode = 0;
	_config->coerceCLocale = 0;
	_config->coerceCLocaleWarn = 0;

	_config->devMode = -1;
#ifdef _WINDOWS
	_config->legacyWindowsFSEncoding = -1;
#endif
}


void alifPreConfig_initAlifConfig(AlifPreConfig* _config) { // 311
	_alifPreConfig_initCompatConfig(_config);

	//_config->configInit = (AlifIntT)AlifConfig_Init_ALIF;
	_config->isolated = 0;
	_config->parseArgv = 1;
	_config->useEnvironment = 1;
	_config->coerceCLocale = -1;
	_config->coerceCLocaleWarn = -1;
	_config->utf8Mode = -1;
#ifdef _WINDOWS
	_config->legacyWindowsFSEncoding = 0;
#endif
}



static void preconfig_copy(AlifPreConfig* config, const AlifPreConfig* config2) { // 379
#define COPY_ATTR(ATTR) config->ATTR = config2->ATTR

	COPY_ATTR(configInit);
	COPY_ATTR(parseArgv);
	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(configureLocale);
	COPY_ATTR(devMode);
	COPY_ATTR(coerceCLocale);
	COPY_ATTR(coerceCLocaleWarn);
	COPY_ATTR(utf8Mode);
	//COPY_ATTR(allocator);
#ifdef _WINDOWS
	COPY_ATTR(legacyWindowsFSEncoding);
#endif

#undef COPY_ATTR
}


AlifStatus _alifPreConfig_initFromPreConfig(AlifPreConfig* _config,
	const AlifPreConfig* _config2) { // 349
	alifPreConfig_initAlifConfig(_config);
	preconfig_copy(_config, _config2);
	return ALIFSTATUS_OK();
}



const char* _alif_getEnv(AlifIntT useEnvironment, const char* name) { // 526
	if (!useEnvironment) {
		return nullptr;
	}

	const char* var = getenv(name);
	if (var and var[0] != '\0') {
		return var;
	}
	else {
		return nullptr;
	}
}









//* here
AlifStatus _alifPreConfig_read(AlifPreConfig* _config, const AlifArgv* _args) { // 797
	AlifStatus status{};

	//	status = _alifRuntime_initialize();
	//	if (ALIFSTATUS_EXCEPTION(status)) {
	//		return status;
	//	}
	//
	//	preconfig_getGlobalVars(_config);
	//
	//	/* Copy LC_CTYPE locale, since it's modified later */
	//	const char* loc = setlocale(LC_CTYPE, nullptr);
	//	if (loc == nullptr) {
	//		return ALIFSTATUS_ERR("failed to LC_CTYPE locale");
	//	}
	//	char* initCtypeLocale = alifMem_StrDup(loc);
	//	if (initCtypeLocale == nullptr) {
	//		return ALIFSTATUS_NO_MEMORY();
	//	}
	//
	//	/* Save the config to be able to restore it if encodings change */
	//	AlifPreConfig saveConfig{};
	//
	//	status = _alifPreConfig_initFromPreConfig(&saveConfig, _config);
	//	if (ALIFSTATUS_EXCEPTION(status)) {
	//		return status;
	//	}
	//
	//	/* Set LC_CTYPE to the user preferred locale */
	//	if (_config->configureLocale) {
	//		_alif_setLocaleFromEnv(LC_CTYPE);
	//	}
	//
	//	AlifPreConfig saveRuntimeConfig{};
	//	preconfig_copy(&saveRuntimeConfig, &_alifRuntime_.preConfig);
	//
	//	AlifPreCmdline cmdline = ALIFPRECMDLINE_INIT;
	//	AlifIntT localeCoerced = 0;
	//	AlifIntT loops = 0;
	//
	//	while (1) {
	//		AlifIntT utf8Mode = _config->utf8Mode;
	//
	//		/* Watchdog to prevent an infinite loop */
	//		loops++;
	//		if (loops == 3) {
	//			status = ALIFSTATUS_ERR("Encoding changed twice while "
	//				"reading the configuration");
	//			goto done;
	//		}
	//
	//		preconfig_copy(&_alifRuntime_.preConfig, _config);
	//
	//		if (_args) {
	//			// Set command line arguments at each iteration. If they are bytes
	//			// strings, they are decoded from the new encoding.
	//			status = _alifPreCmdline_setArgv(&cmdline, _args);
	//			if (ALIFSTATUS_EXCEPTION(status)) {
	//				goto done;
	//			}
	//		}
	//
	//		status = preconfig_read(_config, &cmdline);
	//		if (ALIFSTATUS_EXCEPTION(status)) {
	//			goto done;
	//		}
	//
	//		/* The legacy C locale assumes ASCII as the default text encoding, which
	//		 * causes problems not only for the Alif runtime, but also other
	//		 * components like GNU readline.
	//		 *
	//		 * Accordingly, when the CLI detects it, it attempts to coerce it to a
	//		 * more capable UTF-8 based alternative.
	//		 */
	//		int encoding_changed = 0;
	//		if (_config->coerceCLocale and !localeCoerced) {
	//			localeCoerced = 1;
	//			_alif_coerceLegacyLocale(0);
	//			encoding_changed = 1;
	//		}
	//
	//		if (utf8Mode == -1) {
	//			if (_config->utf8Mode == 1) {
	//				/* UTF-8 Mode enabled */
	//				encoding_changed = 1;
	//			}
	//		}
	//		else {
	//			if (_config->utf8Mode != utf8Mode) {
	//				encoding_changed = 1;
	//			}
	//		}
	//
	//		if (!encoding_changed) {
	//			break;
	//		}
	//
	//		/* Reset the configuration before reading again the configuration,
	//		   just keep UTF-8 Mode and coerce C locale value. */
	//		AlifIntT newUtf8Mode = _config->utf8Mode;
	//		AlifIntT newCoerceCLocale = _config->coerceCLocale;
	//		preconfig_copy(_config, &saveConfig);
	//		_config->utf8Mode = newUtf8Mode;
	//		_config->coerceCLocale = newCoerceCLocale;
	//
	//		/* The encoding changed: read again the configuration
	//		   with the new encoding */
	//	}
	//	status = ALIFSTATUS_OK();
	//
	//done:
	//	// Revert side effects
	//	setlocale(LC_CTYPE, initCtypeLocale);
	//	alifMem_dataFree(initCtypeLocale);
	//	preconfig_copy(&_alifRuntime_.preConfig, &saveRuntimeConfig);
	//	_alifPreCmdline_clear(&cmdline);
	return status;
}
