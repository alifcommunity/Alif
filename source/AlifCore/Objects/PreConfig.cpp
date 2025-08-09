#include "alif.h"

#include "AlifCore_FileUtils.h"
#include "AlifCore_GetOption.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Runtime.h"


AlifStatus _alifArgv_asWStrList(const AlifArgv* _args, AlifWStringList* _list) { // 78


	AlifWStringList wArgv = ALIFWIDESTRINGLIST_INIT;
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

		_alifWStringList_clear(_list);
		*_list = wArgv;
	}
	else {
		wArgv.length = _args->argc;
		wArgv.items = (wchar_t**)_args->wcharArgv;
		if (_alifWStringList_copy(_list, &wArgv)) {
			return ALIFSTATUS_NO_MEMORY();
		}
	}

	return ALIFSTATUS_OK();
}


/* ---------------------------------- AlifPreCmdline ---------------------------------- */

void _alifPreCMDLine_clear(AlifPreCmdline* _cmdline) { // 116
	_alifWStringList_clear(&_cmdline->argv);
	_alifWStringList_clear(&_cmdline->xoptions);
}

AlifStatus _alifPreCMDLine_setArgv(AlifPreCmdline* _cmdline, const AlifArgv* _args) { // 124
	return _alifArgv_asWStrList(_args, &_cmdline->argv);
}

static void preCMDLine_getPreConfig(AlifPreCmdline* _cmdline, const AlifPreConfig* _config) { // 131
#define COPY_ATTR(_attr) \
    if (_config->_attr != -1) { \
        _cmdline->_attr = _config->_attr; \
    }

	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(devMode);

#undef COPY_ATTR
}

static void preCMDLine_setPreConfig(const AlifPreCmdline* _cmdline,
	AlifPreConfig* _config) { // 147
#define COPY_ATTR(_attr) \
    _config->_attr = _cmdline->_attr

	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(devMode);

#undef COPY_ATTR
}

AlifStatus _alifPreCMDLine_setConfig(const AlifPreCmdline* _cmdline, AlifConfig* _config) { // 154
#define COPY_ATTR(_attr) \
    _config->_attr = _cmdline->_attr

	AlifStatus status = _alifWStringList_extend(&_config->xoptions, &_cmdline->xoptions);
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



/* Parse the command line arguments */
static AlifStatus preCmdline_parseCmdline(AlifPreCmdline* _cmdline) { // 182
	const AlifWStringList* argv = &_cmdline->argv;

	_alifOS_resetGetOpt();
	_alifOSOptErr_ = 0;
	do {
		AlifIntT longindex = -1;
		AlifIntT c = _alifOS_getOpt(argv->length, argv->items, &longindex);

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
			AlifStatus status = alifWStringList_append(&_cmdline->xoptions,
				_alifOSOptArg_);
			if (ALIFSTATUS_EXCEPTION(status)) {
				return status;
			}
			break;
		}

		default:
			break;
		}
	}
	while (1);

	return ALIFSTATUS_OK();
}




AlifStatus _alifPreCMDLine_read(AlifPreCmdline* _cmdline,
	const AlifPreConfig* _preConfig) { // 230
	preCMDLine_getPreConfig(_cmdline, _preConfig);

	if (_preConfig->parseArgv) {
		AlifStatus status = preCmdline_parseCmdline(_cmdline);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	/* isolated, useEnvironment */
	if (_cmdline->isolated < 0) {
		_cmdline->isolated = 0;
	}
	if (_cmdline->isolated > 0) {
		_cmdline->useEnvironment = 0;
	}
	if (_cmdline->useEnvironment < 0) {
		_cmdline->useEnvironment = 0;
	}

	/* devMode */
	if ((_cmdline->devMode < 0)
		and (_alif_getXOption(&_cmdline->xoptions, L"dev")
			or _alif_getEnv(_cmdline->useEnvironment, "ALIFDEVMODE"))) {
		_cmdline->devMode = 1;
	}
	if (_cmdline->devMode < 0) {
		_cmdline->devMode = 0;
	}

	// warnDefaultEncoding
	if (_alif_getXOption(&_cmdline->xoptions, L"warnDefaultEncoding")
		or _alif_getEnv(_cmdline->useEnvironment, "ALIFWARNDEFAULTENCODING")) {
		_cmdline->warnDefaultEncoding = 1;
	}

	return ALIFSTATUS_OK();
}

/* ---------------------------------- AlifPreConfig ---------------------------------- */


void _alifPreConfig_initCompatConfig(AlifPreConfig* _config) { // 280
	memset(_config, 0, sizeof(*_config));

	_config->configInit = (AlifIntT)AlifConfig_Init_COMPAT;
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

	_config->configInit = (AlifIntT)AlifConfig_Init_ALIF;
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


void alifPreConfig_initIsolatedConfig(AlifPreConfig* config) { // 332
	_alifPreConfig_initCompatConfig(config);

	config->configInit = (AlifIntT)ConfigInitEnum_::AlifConfig_Init_ISOLATED;
	config->configureLocale = 0;
	config->isolated = 1;
	config->useEnvironment = 0;
	config->utf8Mode = 0;
	config->devMode = 0;
#ifdef _WINDOWS
	config->legacyWindowsFSEncoding = 0;
#endif
}


AlifStatus _alifPreConfig_initFromPreConfig(AlifPreConfig* _config,
	const AlifPreConfig* _config2) { // 349
	alifPreConfig_initAlifConfig(_config);
	preconfig_copy(_config, _config2);
	return ALIFSTATUS_OK();
}

void _alifPreConfig_initFromConfig(AlifPreConfig* _preConfig,
	const AlifConfig* _config) { // 359
	ConfigInitEnum_ config_init = (ConfigInitEnum_)_config->configInit;
	switch (config_init) {
	case ConfigInitEnum_::AlifConfig_Init_ALIF:
		alifPreConfig_initAlifConfig(_preConfig);
		break;
	case ConfigInitEnum_::AlifConfig_Init_ISOLATED:
		alifPreConfig_initIsolatedConfig(_preConfig);
		break;
	case ConfigInitEnum_::AlifConfig_Init_COMPAT:
	default:
		_alifPreConfig_initCompatConfig(_preConfig);
	}

	_alifPreConfig_getConfig(_preConfig, _config);
}

void _alifPreConfig_getConfig(AlifPreConfig* _preConfig,
	const AlifConfig* _config) { // 448
#define COPY_ATTR(_attr) \
    if (_config->_attr != -1) { \
        _preConfig->_attr = _config->_attr; \
    }

	COPY_ATTR(parseArgv);
	COPY_ATTR(isolated);
	COPY_ATTR(useEnvironment);
	COPY_ATTR(devMode);

#undef COPY_ATTR
}


static void preconfig_getGlobalVars(AlifPreConfig* _config) { // 465
	if (_config->configInit != ConfigInitEnum_::AlifConfig_Init_COMPAT) {
		/* Alif and Isolated configuration ignore global variables */
		return;
	}

#define COPY_FLAG(_attr, _value) \
    if (_config->_attr < 0) { \
        _config->_attr = _value; \
    }
#define COPY_NOT_FLAG(_attr, _value) \
    if (_config->_attr < 0) { \
        _config->_attr = !(_value); \
    }

	ALIF_COMP_DIAG_PUSH;
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS;
	COPY_FLAG(isolated, _alifIsolatedFlag_);
	COPY_NOT_FLAG(useEnvironment, _alifIgnoreEnvironmentFlag_);
	if (_alifUTF8Mode_ > 0) {
		_config->utf8Mode = _alifUTF8Mode_;
	}
#ifdef _WINDOWS
	COPY_FLAG(legacyWindowsFSEncoding, _alifLegacyWindowsFSEncodingFlag_);
#endif
	ALIF_COMP_DIAG_POP

	#undef COPY_FLAG
	#undef COPY_NOT_FLAG
}

static void preConfig_setGlobalVars(const AlifPreConfig* _config) { // 499
#define COPY_FLAG(_attr, _var) \
    if (_config->_attr >= 0) { \
        _var = _config->_attr; \
    }
#define COPY_NOT_FLAG(_attr, _var) \
    if (_config->_attr >= 0) { \
        _var = !_config->_attr; \
    }

	ALIF_COMP_DIAG_PUSH;
	ALIF_COMP_DIAG_IGNORE_DEPR_DECLS;
	COPY_FLAG(isolated, _alifIsolatedFlag_);
	COPY_NOT_FLAG(useEnvironment, _alifIgnoreEnvironmentFlag_);
#ifdef _WINDOWS
	COPY_FLAG(legacyWindowsFSEncoding, _alifLegacyWindowsFSEncodingFlag_);
#endif
	COPY_FLAG(utf8Mode, _alifUTF8Mode_);
	ALIF_COMP_DIAG_POP

	#undef COPY_FLAG
	#undef COPY_NOT_FLAG
}

const char* _alif_getEnv(AlifIntT _useEnvironment, const char* _name) { // 526
	if (!_useEnvironment) {
		return nullptr;
	}

	const char* var = getenv(_name);
	if (var and var[0] != '\0') {
		return var;
	}
	else {
		return nullptr;
	}
}


AlifIntT _alif_strToInt(const char* _str, AlifIntT* _result) { // 545
	const char* endptr = _str;
	errno = 0;
	long value = strtol(_str, (char**)&endptr, 10);
	if (*endptr != '\0' or errno == ERANGE) {
		return -1;
	}
	if (value < INT_MIN or value > INT_MAX) {
		return -1;
	}

	*_result = (AlifIntT)value;
	return 0;
}


void _alif_getEnvFlag(AlifIntT _useEnvironment, AlifIntT* _flag, const char* _name) { // 563
	const char* var = _alif_getEnv(_useEnvironment, _name);
	if (!var) {
		return;
	}
	AlifIntT value{};
	if (_alif_strToInt(var, &value) < 0 or value < 0) {
		value = 1;
	}
	if (*_flag < value) {
		*_flag = value;
	}
}


const wchar_t* _alif_getXOption(const AlifWStringList* _xoptions, const wchar_t* _name) { // 581
	for (AlifSizeT i = 0; i < _xoptions->length; i++) {
		const wchar_t* option = _xoptions->items[i];
		AlifUSizeT len{};
		const wchar_t* sep = wcschr(option, L'=');
		if (sep != nullptr) {
			len = (sep - option);
		}
		else {
			len = wcslen(option);
		}
		if (wcsncmp(option, _name, len) == 0 and _name[len] == L'\0') {
			return option;
		}
	}
	return nullptr;
}


static AlifStatus preConfig_initUTF8Mode(AlifPreConfig* _config, const AlifPreCmdline* _cmdline) { // 602
#ifdef _WINDOWS
	if (_config->legacyWindowsFSEncoding) {
		_config->utf8Mode = 0;
	}
#endif

	if (_config->utf8Mode >= 0) {
		return ALIFSTATUS_OK();
	}

	const wchar_t* xopt;
	xopt = _alif_getXOption(&_cmdline->xoptions, L"utf8");
	if (xopt) {
		const wchar_t* sep = wcschr(xopt, L'=');
		if (sep) {
			xopt = sep + 1;
			if (wcscmp(xopt, L"1") == 0) {
				_config->utf8Mode = 1;
			}
			else if (wcscmp(xopt, L"0") == 0) {
				_config->utf8Mode = 0;
			}
			else {
				return ALIFSTATUS_ERR("invalid -X utf8 option value");
			}
		}
		else {
			_config->utf8Mode = 1;
		}
		return ALIFSTATUS_OK();
	}

	const char* opt = _alif_getEnv(_config->useEnvironment, "ALIFUTF8");
	if (opt) {
		if (strcmp(opt, "1") == 0) {
			_config->utf8Mode = 1;
		}
		else if (strcmp(opt, "0") == 0) {
			_config->utf8Mode = 0;
		}
		else {
			return ALIFSTATUS_ERR("invalid ALIFUTF8 environment "
				"variable value");
		}
		return ALIFSTATUS_OK();
	}


#ifndef _WINDOWS
	if (_config->utf8Mode < 0) {
		const char* ctypeLoc = setlocale(LC_CTYPE, nullptr);
		if (ctypeLoc != nullptr
			and (strcmp(ctypeLoc, "C") == 0
				or strcmp(ctypeLoc, "POSIX") == 0)) {
			_config->utf8Mode = 1;
		}
	}
#endif

	if (_config->utf8Mode < 0) {
		_config->utf8Mode = 0;
	}
	return ALIFSTATUS_OK();
}


static void preConfig_initCoerceCLocale(AlifPreConfig* _config) { // 673
	if (!_config->configureLocale) {
		_config->coerceCLocale = 0;
		_config->coerceCLocaleWarn = 0;
		return;
	}

	const char* env = _alif_getEnv(_config->useEnvironment, "ALIFCOERCECLOCALE");
	if (env) {
		if (strcmp(env, "0") == 0) {
			if (_config->coerceCLocale < 0) {
				_config->coerceCLocale = 0;
			}
		}
		else if (strcmp(env, "warn") == 0) {
			if (_config->coerceCLocaleWarn < 0) {
				_config->coerceCLocaleWarn = 1;
			}
		}
		else {
			if (_config->coerceCLocale < 0) {
				_config->coerceCLocale = 1;
			}
		}
	}

	if (_config->coerceCLocale < 0 or _config->coerceCLocale == 1) {
		if (_alif_legacyLocaleDetected(0)) {
			_config->coerceCLocale = 2;
		}
		else {
			_config->coerceCLocale = 0;
		}
	}

	if (_config->coerceCLocaleWarn < 0) {
		_config->coerceCLocaleWarn = 0;
	}
}


static AlifStatus preConfig_read(AlifPreConfig* config, AlifPreCmdline* cmdline) { // 745
	AlifStatus status{};

	status = _alifPreCMDLine_read(cmdline, config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	preCMDLine_setPreConfig(cmdline, config);

	/* legacyWindowsFSEncoding, coerceCLocale, utf8Mode */
#ifdef _WINDOWS
	_alif_getEnvFlag(config->useEnvironment,
		&config->legacyWindowsFSEncoding,
		"ALIFLEGACYWINDOWSFSENCODING");
#endif

	preConfig_initCoerceCLocale(config);

	status = preConfig_initUTF8Mode(config, cmdline);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	/* allocator */
	//status = preConfig_initAllocator(config);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	return ALIFSTATUS_OK();
}




AlifStatus _alifPreConfig_read(AlifPreConfig* _config, const AlifArgv* _args) { // 797
	AlifStatus status{};

	status = _alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	preconfig_getGlobalVars(_config);

	/* Copy LC_CTYPE locale, since it's modified later */
	const char* loc = setlocale(LC_CTYPE, nullptr);
	if (loc == nullptr) {
		return ALIFSTATUS_ERR("failed to LC_CTYPE locale");
	}
	char* initCtypeLocale = alifMem_strDup(loc);
	if (initCtypeLocale == nullptr) {
		return ALIFSTATUS_NO_MEMORY();
	}

	/* Save the config to be able to restore it if encodings change */
	AlifPreConfig saveConfig{};

	status = _alifPreConfig_initFromPreConfig(&saveConfig, _config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	/* Set LC_CTYPE to the user preferred locale */
	if (_config->configureLocale) {
		_alif_setLocaleFromEnv(LC_CTYPE);
	}

	AlifPreConfig saveRuntimeConfig{};
	preconfig_copy(&saveRuntimeConfig, &_alifRuntime_.preConfig);

	AlifPreCmdline cmdline = ALIFPRECMDLINE_INIT;
	AlifIntT localeCoerced = 0;
	AlifIntT loops = 0;

	while (1) {
		AlifIntT utf8Mode = _config->utf8Mode;

		/* Watchdog to prevent an infinite loop */
		loops++;
		if (loops == 3) {
			status = ALIFSTATUS_ERR("Encoding changed twice while "
				"reading the configuration");
			goto done;
		}

		preconfig_copy(&_alifRuntime_.preConfig, _config);

		if (_args) {
			// Set command line arguments at each iteration. If they are bytes
			// strings, they are decoded from the new encoding.
			status = _alifPreCMDLine_setArgv(&cmdline, _args);
			if (ALIFSTATUS_EXCEPTION(status)) {
				goto done;
			}
		}

		status = preConfig_read(_config, &cmdline);
		if (ALIFSTATUS_EXCEPTION(status)) {
			goto done;
		}

		/* The legacy C locale assumes ASCII as the default text encoding, which
		 * causes problems not only for the Alif runtime, but also other
		 * components like GNU readline.
		 *
		 * Accordingly, when the CLI detects it, it attempts to coerce it to a
		 * more capable UTF-8 based alternative.
		 */
		int encoding_changed = 0;
		if (_config->coerceCLocale and !localeCoerced) {
			localeCoerced = 1;
			_alif_coerceLegacyLocale(0);
			encoding_changed = 1;
		}

		if (utf8Mode == -1) {
			if (_config->utf8Mode == 1) {
				/* UTF-8 Mode enabled */
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

		/* Reset the configuration before reading again the configuration,
		   just keep UTF-8 Mode and coerce C locale value. */
		AlifIntT newUtf8Mode = _config->utf8Mode;
		AlifIntT newCoerceCLocale = _config->coerceCLocale;
		preconfig_copy(_config, &saveConfig);
		_config->utf8Mode = newUtf8Mode;
		_config->coerceCLocale = newCoerceCLocale;

		/* The encoding changed: read again the configuration
		   with the new encoding */
	}
	status = ALIFSTATUS_OK();

done:
	// Revert side effects
	setlocale(LC_CTYPE, initCtypeLocale);
	alifMem_dataFree(initCtypeLocale);
	preconfig_copy(&_alifRuntime_.preConfig, &saveRuntimeConfig);
	_alifPreCMDLine_clear(&cmdline);
	return status;
}




AlifStatus _alifPreConfig_write(const AlifPreConfig* _srcConfig) { // 937
	AlifPreConfig config{};

	AlifStatus status = _alifPreConfig_initFromPreConfig(&config, _srcConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	if (_alifRuntime_.coreInitialized) {
		return ALIFSTATUS_OK();
	}

	//AlifMemAllocatorName name = (AlifMemAllocatorName)config.allocator;
	//if (name != AlifMem_Allocator_NotSet) {
	//	if (_alifMem_setupAllocators(name) < 0) {
	//		return ALIFSTATUS_ERR("Unknown ALIFMALLOC allocator");
	//	}
	//}

	preConfig_setGlobalVars(&config);

	if (config.configureLocale) {
		if (config.coerceCLocale) {
			if (!_alif_coerceLegacyLocale(config.coerceCLocaleWarn)) {
				config.coerceCLocale = 0;
			}
		}

		/* Set LC_CTYPE to the user preferred locale */
		_alif_setLocaleFromEnv(LC_CTYPE);
	}

	/* Write the new pre-configuration into _alifRuntime_ */
	preconfig_copy(&_alifRuntime_.preConfig, &config);

	return ALIFSTATUS_OK();
}
