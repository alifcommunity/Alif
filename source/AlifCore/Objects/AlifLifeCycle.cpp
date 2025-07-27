#include "alif.h"

#include "AlifCore_Call.h"
#include "AlifCore_Exceptions.h"
#include "AlifCore_FileUtils.h"
#include "AlifCore_FloatObject.h"
#include "AlifCore_FreeList.h"
#include "AlifCore_Import.h"
#include "AlifCore_PathConfig.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_LifeCycle.h"
#include "AlifCore_Memory.h"
#include "AlifCore_State.h"
#include "AlifCore_Runtime.h"
#include "AlifCore_RuntimeInit.h"



#ifdef HAVE_SIGNAL_H
#  include <signal.h>             // SIG_IGN
#endif


//#define PUTS(_fd, _str) (void)_alifWrite_noRaise(_fd, _str, (AlifIntT)strlen(_str))


static AlifStatus init_setBuiltinsOpen(void); // 72
static AlifStatus init_sysStreams(AlifThread* _thread); // 73




AlifRuntime _alifRuntime_ = ALIF_DURERUNSTATE_INIT(_alifRuntime_); // 103

static AlifIntT runtimeInitialized = 0; // 110

AlifStatus _alifRuntime_initialize() { // 112
	if (runtimeInitialized) return ALIFSTATUS_OK();

	runtimeInitialized = 1;

	return _alifRuntimeState_init(&_alifRuntime_);
}


void _alifRuntime_finalize(void) { // 129
	_alifRuntimeState_fini(&_alifRuntime_);
	runtimeInitialized = 0;
}

AlifIntT _alif_legacyLocaleDetected(AlifIntT _warn) { // 185
#ifndef _WINDOWS
	if (!_warn) {
		const char* localeOverride = getenv("LC_ALL");
		if (localeOverride != nullptr and *localeOverride != '\0') {
			return 0;
		}
	}

	const char* ctypeLoc = setlocale(LC_CTYPE, nullptr);
	return ctypeLoc != nullptr and strcmp(ctypeLoc, "C") == 0;
#else
	/* Windows uses code pages instead of locales, so no locale is legacy */
	return 0;
#endif
}


class LocaleCoercionTarget { // 228
public:
	const char* localeName{};
};

static LocaleCoercionTarget _TargetLocales_[] = { // 232
	{"C.UTF-8"},
	{"C.utf8"},
	{"UTF-8"},
	{nullptr}
};

// 253
#ifdef ALIF_COERCE_C_LOCALE
static const char _cLocaleCoercionWarning_[] =
"Alif detected LC_CTYPE=C: LC_CTYPE coerced to %.20s (set another locale "
"or ALIFCOERCECLOCALE=0 to disable this locale coercion behavior).\n";

static AlifIntT _coerce_defaultLocaleSettings(AlifIntT _warn, const LocaleCoercionTarget* _target) {
	const char* newloc = _target->localeName;

	/* Reset locale back to currently configured defaults */
	_alif_setLocaleFromEnv(LC_ALL);

	/* Set the relevant locale environment variable */
	if (setenv("LC_CTYPE", newloc, 1)) {
		fprintf(stderr,
			"Error setting LC_CTYPE, skipping C locale coercion\n");
		return 0;
	}
	if (_warn) {
		fprintf(stderr, _cLocaleCoercionWarning_, newloc);
	}

	/* Reconfigure with the overridden environment variables */
	_alif_setLocaleFromEnv(LC_ALL);
	return 1;
}
#endif

AlifIntT _alif_coerceLegacyLocale(AlifIntT _warn) { // 282
	AlifIntT coerced = 0;
#ifdef ALIF_COERCE_C_LOCALE
	char* oldloc = nullptr;

	oldloc = alifMem_strDup(setlocale(LC_CTYPE, nullptr));
	if (oldloc == nullptr) {
		return coerced;
	}

	const char* localeOverride = getenv("LC_ALL");
	if (localeOverride == nullptr or *localeOverride == '\0') {
		/* LC_ALL is also not set (or is set to an empty string) */
		const LocaleCoercionTarget* target = nullptr;
		for (target = _TargetLocales_; target->localeName; target++) {
			const char* newLocale = setlocale(LC_CTYPE,
				target->localeName);
			if (newLocale != nullptr) {
			#if !defined(ALIF_FORCE_UTF8_LOCALE) and defined(HAVE_LANGINFO_H) and defined(CODESET)
				char* codeset = nl_langinfo(CODESET);
				if (!codeset or *codeset == '\0') {
					/* CODESET is not set or empty, so skip coercion */
					newLocale = nullptr;
					_alif_setLocaleFromEnv(LC_CTYPE);
					continue;
				}
			#endif
				/* Successfully configured locale, so make it the default */
				coerced = _coerce_defaultLocaleSettings(_warn, target);
				goto done;
			}
		}
	}
	/* No C locale warning here, as Py_Initialize will emit one later */

	setlocale(LC_CTYPE, oldloc);

done:
	alifMem_dataFree(oldloc);
#endif
	return coerced;
}

char* _alif_setLocaleFromEnv(AlifIntT category) {  // 332

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
		nullptr,
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
#else
	res = setlocale(category, "");
#endif

	return res;
}


static AlifIntT interpreter_updateConfig(AlifThread* tstate,
	AlifIntT _onlyUpdatePathConfig) { // 391
	const AlifConfig* config = &tstate->interpreter->config;

	if (!_onlyUpdatePathConfig) {
		AlifStatus status = alifConfig_write(config, tstate->interpreter->runtime);
		if (ALIFSTATUS_EXCEPTION(status)) {
			//_alifErr_setFromAlifStatus(status);
			return -1;
		}
	}

	if (alif_isMainInterpreter(tstate->interpreter)) {
		AlifStatus status = _alifPathConfig_updateGlobal(config);
		if (ALIFSTATUS_EXCEPTION(status)) {
			//_alifErr_setFromalifStatus(status);
			return -1;
		}
	}

	tstate->interpreter->longState.maxStrDigits = config->intMaxStrDigits;

	// Update the sys module for the new configuration
	if (_alifSys_updateConfig(tstate) < 0) {
		return -1;
	}
	return 0;
}


static AlifStatus alifCore_initRuntime(AlifRuntime* _runtime, const AlifConfig* _config) { // 507

	if (_runtime->initialized) {
		return ALIFSTATUS_ERR("main interpreter already initialized");
	}

	AlifStatus status = alifConfig_write(_config, _runtime);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = alifImport_init();
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = alifInterpreter_enable(_runtime);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	return ALIFSTATUS_OK();
}

static AlifStatus initInterpreter_settings(AlifInterpreter* _interp,
	const AlifInterpreterConfig* _config) { // 551

	if (_config->useMainAlifMem) {
		_interp->featureFlags |= ALIF_RTFLAGS_USE_ALIFMEM;
	}
	else if (!_config->checkMultiInterpExtensions) {
		return ALIFSTATUS_ERR("per-interpreter alifmem does not support "
			"single-phase init extension modules");
	}
	if (!alif_isMainInterpreter(_interp) and
		!_config->checkMultiInterpExtensions) {
		return ALIFSTATUS_ERR("The free-threaded build does not support "
			"single-phase init extension modules in "
			"subinterpreters");
	}

	if (_config->allowFork) {
		_interp->featureFlags |= ALIF_RTFLAGS_FORK;
	}
	if (_config->allowExec) {
		_interp->featureFlags |= ALIF_RTFLAGS_EXEC;
	}
	// Note that fork+exec is always allowed.

	if (_config->allowThreads) {
		_interp->featureFlags |= ALIF_RTFLAGS_THREADS;
	}
	if (_config->allowDaemonThreads) {
		_interp->featureFlags |= ALIF_RTFLAGS_DAEMON_THREADS;
	}

	if (_config->checkMultiInterpExtensions) {
		_interp->featureFlags |= ALIF_RTFLAGS_MULTI_INTERP_EXTENSIONS;
	}

	switch (_config->gil) {
	case ALIF_INTERPRETERCONFIG_DEFAULT_GIL: break;
	case ALIF_INTERPRETERCONFIG_SHARED_GIL: break;
	case ALIF_INTERPRETERCONFIG_OWN_GIL: break;
	default:
		return ALIFSTATUS_ERR("invalid interpreter config 'gil' value");
	}

	return ALIFSTATUS_OK();
}

static void initInterpreter_createGIL(AlifThread* tstate, int gil) { // 607
	alifEval_finiGIL(tstate->interpreter);

	AlifIntT ownGIL = (gil == ALIF_INTERPRETERCONFIG_OWN_GIL);

	alifEval_initGIL(tstate, ownGIL);
}

static AlifStatus alifCore_createInterpreter(AlifRuntime* _dureRun,
	const AlifConfig* _config, AlifThread** _threadP) { // 637

	AlifStatus status{};
	AlifInterpreter* interpreter{};

	status = alifInterpreter_new(nullptr, &interpreter);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	interpreter->ready = 1;

	status = alifConfig_copy(&interpreter->config, _config);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = alifGILState_init(interpreter);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	AlifInterpreterConfig config = ALIF_INTERPRETERCONFIG_LEGACY_INIT;
	config.gil = ALIF_INTERPRETERCONFIG_OWN_GIL;
	config.checkMultiInterpExtensions = 0;
	status = initInterpreter_settings(interpreter, &config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	if (alifInterpreterMem_init(interpreter) < 1) {
		return ALIFSTATUS_NO_MEMORY();
	}

	AlifThread* thread = alifThreadState_new(interpreter);

	if (thread == nullptr) {
		return ALIFSTATUS_ERR("can't make first thread");
	}

	_dureRun->mainThread = thread;
	alifThread_bind(thread);

	initInterpreter_createGIL(thread, config.gil);

	*_threadP = thread;
	return ALIFSTATUS_OK();
}


static AlifStatus alifCore_initGlobalObjects(AlifInterpreter* _interp) { // 696
	AlifStatus status{};

	//alifFloat_initState(_interp);

	status = alifUStr_initGlobalObjects(_interp);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//alifUStr_initState(_interp);

	if (alif_isMainInterpreter(_interp)) {
		_alif_getConstantInit();
	}

	return ALIFSTATUS_OK();
}


static AlifStatus alifCore_initTypes(AlifInterpreter* _interp) { // 718
	AlifStatus status{};

	status = alifTypes_initTypes(_interp);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//status = alifLong_initTypes(_interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = alifUstr_InitTypes(_interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = alifFloat_initTypes(_interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	if (_alifExc_initTypes(_interp) < 0) {
		return ALIFSTATUS_ERR("failed to initialize an exception type");
	}

	//status = alifExc_initGlobalObjects(_interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = alifExc_initState(_interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = alifErr_initTypes(_interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = alifContext_init(_interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = alifXI_initTypes(_interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	return ALIFSTATUS_OK();
}


static AlifStatus alifCore_builtinsInit(AlifThread* _thread) { // 775

	AlifObject* modules{};
	AlifObject* builtinsDict{};

	AlifInterpreter* interp = _thread->interpreter;

	AlifObject* biMod = alifBuiltin_init(interp);
	if (biMod == nullptr) goto error;

	modules = _alifImport_getModules(interp);
	if (_alifImport_fixupBuiltin(_thread, biMod, "builtins", modules) < 0) {
		goto error;
	}

	builtinsDict = alifModule_getDict(biMod);
	if (builtinsDict == nullptr) goto error;
	interp->builtins = ALIF_NEWREF(builtinsDict);


	interp->builtinsCopy = alifDict_copy(interp->builtins);
	if (interp->builtinsCopy == nullptr) {
		goto error;
	}
	ALIF_DECREF(biMod);

	if (_alifImport_initDefaultImportFunc(interp) < 0) {
		goto error;
	}

	return ALIFSTATUS_OK();

error:
	ALIF_XDECREF(biMod);
	return ALIFSTATUS_ERR("can't initialize builtins module");
}

static AlifStatus alifCore_interpreterInit(AlifThread* _thread) { // 843

	AlifInterpreter* interpreter = _thread->interpreter;
	AlifStatus status = ALIFSTATUS_OK();
	const AlifConfig* config{};
	AlifObject* sysMod = nullptr;

	status = alifCore_initGlobalObjects(interpreter);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = alifCode_init(interpreter);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = _alifDtoa_init(interpreter);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	//status = alifGC_init(interpreter);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = alifCore_initTypes(interpreter);
	if (ALIFSTATUS_EXCEPTION(status)) goto done;

	//status = alifAtExit_init(interpreter);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = alifSys_create(_thread, &sysMod);
	if (ALIFSTATUS_EXCEPTION(status)) goto done;

	status = alifCore_builtinsInit(_thread);
	if (ALIFSTATUS_EXCEPTION(status)) goto done;

	config = &interpreter->config;

	status = _alifImport_initCore(_thread, sysMod, config->installImportLib);
	if (ALIFSTATUS_EXCEPTION(status)) goto done;

done:
	//ALIF_XDECREF(sysmod);
	return status;

}

static AlifStatus alifInit_config(AlifRuntime* _runtime,
	AlifThread** _threadP, const AlifConfig* _config) { // 917

	AlifStatus status{};

	status = alifCore_initRuntime(_runtime, _config);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	AlifThread* thread_{};
	status = alifCore_createInterpreter(_runtime, _config, &thread_);
	if (ALIFSTATUS_EXCEPTION(status)) return status;
	*_threadP = thread_;

	status = alifCore_interpreterInit(thread_);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	_runtime->coreInitialized = 1;
	return ALIFSTATUS_OK();
}

AlifStatus _alif_preInitializeFromAlifArgv(const AlifPreConfig* _srcConfig,
	const AlifArgv* _args) { // 945

	AlifStatus status{};

	if (_srcConfig == nullptr) {
		return ALIFSTATUS_ERR("متغير التهيئة التمهيدية فارغ");
	}

	status = _alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	AlifRuntime* runtime = &_alifRuntime_;

	if (runtime->preinitialized) {
		return ALIFSTATUS_OK();
	}

	/* Note: preinitialized remains 1 on error, it is only set to 0
	   at exit on success. */
	runtime->preinitializing = 1;

	AlifPreConfig config{};

	status = _alifPreConfig_initFromPreConfig(&config, _srcConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = _alifPreConfig_read(&config, _args);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = _alifPreConfig_write(&config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	runtime->preinitializing = 0;
	runtime->preinitialized = 1;
	return ALIFSTATUS_OK();
}

AlifStatus alif_preInitialize(const AlifPreConfig* _srcConfig) { // 1008
	return _alif_preInitializeFromAlifArgv(_srcConfig, nullptr);
}

AlifStatus _alif_preInitializeFromConfig(const AlifConfig* _config,
	const AlifArgv* _args) { // 1016

	AlifStatus status = _alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	AlifRuntime* runtime = &_alifRuntime_;

	if (runtime->preinitialized) {
		/* Already initialized: do nothing */
		return ALIFSTATUS_OK();
	}

	AlifPreConfig preconfig{};

	_alifPreConfig_initFromConfig(&preconfig, _config);

	if (!_config->parseArgv) {
		return alif_preInitialize(&preconfig);
	}
	else if (_args == nullptr) {
		AlifArgv config_args = {
			.argc = _config->argv.length,
			.useBytesArgv = 0,
			.wcharArgv = _config->argv.items };
		return _alif_preInitializeFromAlifArgv(&preconfig, &config_args);
	}
	else {
		return _alif_preInitializeFromAlifArgv(&preconfig, _args);
	}
}

static AlifStatus alifInit_core(AlifRuntime* _runtime,
	const AlifConfig* _config, AlifThread** _threadPtr) { // 1069

	AlifStatus status{};

	AlifConfig config{};
	alifConfig_initAlifConfig(&config);

	status = alifConfig_copy(&config, _config);
	if (ALIFSTATUS_EXCEPTION(status)) goto done;

	status = alifConfig_read(&config);
	if (ALIFSTATUS_EXCEPTION(status)) goto done;

	if (!_runtime->coreInitialized) {
		status = alifInit_config(_runtime, _threadPtr, &config);
	}
	else {
		//status = alifInit_coreReconfig(_dureRun, _threadPtr, &config);
	}
	if (ALIFSTATUS_EXCEPTION(status)) goto done;

done:
	alifConfig_clear(&config);
	return status;
}

static AlifStatus initInterpreter_main(AlifThread* _thread) { // 1156

	AlifStatus status{};
	AlifIntT isMainInterpreter = alif_isMainInterpreter(_thread->interpreter);
	AlifInterpreter* interpreter = _thread->interpreter;
	const AlifConfig* config = alifInterpreter_getConfig(interpreter);

	if (!config->installImportLib) {
		if (isMainInterpreter) {
			interpreter->runtime->initialized = 1;
		}
		return ALIFSTATUS_OK();
	}

	status = _alifConfig_initImportConfig(&interpreter->config);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	if (interpreter_updateConfig(_thread, 1) < 0) {
		return ALIFSTATUS_ERR("failed to update the Alif config");
	}

	//status = alifImport_initExternal(_thread);
	//if (ALIFSTATUS_EXCEPTION(status)) return status;

	//status = alifUnicode_initEncoding(_thread);
	//if (ALIFSTATUS_EXCEPTION(status)) return status;

	if (isMainInterpreter) {
		if (_alifSignal_init(config->installSignalHandlers) < 0) {
			//return ALIFSTATUS_ERR("can't initialize signals");
		}

		//if (config_->tracemalloc) {
		//	if (alifTraceMalloc_start(config_->tracemalloc) < 0) {
		//		return ALIFSTATUS_ERR("can't start tracemalloc");
		//	}
		//}
	}

	status = init_sysStreams(_thread);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	status = init_setBuiltinsOpen();
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	//status = add_mainmodule(interpreter);
	//if (ALIFSTATUS_EXCEPTION(status)) return status;

	// need work

	return ALIFSTATUS_OK();

}

static AlifStatus alifInit_main(AlifThread* _thread) { // 1363

	AlifInterpreter* interpreter = _thread->interpreter;
	if (!interpreter->runtime->coreInitialized) {
		return ALIFSTATUS_ERR("runtime core not initialized");
	}

	if (interpreter->runtime->initialized) {
		//return alifInit_mainReconfigure(_thread);
	}

	AlifStatus status = initInterpreter_main(_thread);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	return ALIFSTATUS_OK();
}

AlifStatus alif_initFromConfig(const AlifConfig* _config) { // 1383

	if (_config == nullptr) {
		return ALIFSTATUS_ERR("متغير التهيئة فارغ");
	}

	AlifStatus status{};

	status = _alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	AlifRuntime* dureRun = &_alifRuntime_;
	AlifThread* thread_ = nullptr;

	status = alifInit_core(dureRun, _config, &thread_);
	if (ALIFSTATUS_EXCEPTION(status)) return status;

	_config = alifInterpreter_getConfig(thread_->interpreter);

	if (_config->initMain) {
		status = alifInit_main(thread_);
		if (ALIFSTATUS_EXCEPTION(status)) return status;
	}

	return ALIFSTATUS_OK();
}
















static AlifObject* create_stdio(const AlifConfig* config, AlifObject* io,
	AlifIntT fd, AlifIntT write_mode, const char* name,
	const wchar_t* encoding, const wchar_t* errors) { // 2568
	AlifObject* buf = nullptr, * stream = nullptr, * text = nullptr, * raw = nullptr, * res;
	const char* mode{};
	const char* newline{};
	AlifObject* line_buffering{}, * write_through{};
	AlifIntT buffering{}, isatty{};
	const AlifIntT bufferedStdio = config->bufferedStdio;

	if (!_alif_isValidFD(fd)) {
		return ALIF_NONE;
	}

	if (!bufferedStdio and write_mode)
		buffering = 0;
	else
		buffering = -1;
	if (write_mode)
		mode = "wb";
	else
		mode = "rb";
	buf = _alifObject_callMethod(io, &ALIF_STR(Open), "isiOOOO",
		fd, mode, buffering,
		ALIF_NONE, ALIF_NONE, /* encoding, errors */
		ALIF_NONE, ALIF_FALSE); /* newline, closefd */
	if (buf == nullptr)
		goto error;

	if (buffering) {
		raw = alifObject_getAttr(buf, &ALIF_STR(Raw));
		if (raw == nullptr)
			goto error;
	}
	else {
		raw = ALIF_NEWREF(buf);
	}

#ifdef HAVE_WINDOWS_CONSOLE_IO
	/* Windows console IO is always UTF-8 encoded */
	AlifTypeObject* winconsoleio_type;
	winconsoleio_type = (AlifTypeObject*)_alifImport_getModuleAttr(
		&ALIF_STR(_io), &ALIF_STR(_windowsConsoleIO));
	if (winconsoleio_type == nullptr) {
		goto error;
	}
	AlifIntT is_subclass;
	is_subclass = alifObject_typeCheck(raw, winconsoleio_type);
	ALIF_DECREF(winconsoleio_type);
	if (is_subclass) {
		encoding = L"utf-8";
	}
#endif

	text = alifUStr_fromString(name);
	if (text == nullptr or alifObject_setAttr(raw, &ALIF_STR(Name), text) < 0)
		goto error;
	res = alifObject_callMethodNoArgs(raw, &ALIF_STR(IsAtty));
	if (res == nullptr)
		goto error;
	isatty = alifObject_isTrue(res);
	ALIF_DECREF(res);
	if (isatty == -1)
		goto error;
	if (!bufferedStdio)
		write_through = ALIF_TRUE;
	else
		write_through = ALIF_FALSE;
	if (bufferedStdio and (isatty or fd == fileno(stderr)))
		line_buffering = ALIF_TRUE;
	else
		line_buffering = ALIF_FALSE;

	ALIF_CLEAR(raw);
	ALIF_CLEAR(text);

#ifdef _WINDOWS
	newline = nullptr;
#else
	newline = "\n";
#endif

	AlifObject* encoding_str;
	encoding_str = alifUStr_fromWideChar(encoding, -1);
	if (encoding_str == nullptr) {
		ALIF_CLEAR(buf);
		goto error;
	}

	//AlifObject* errors_str;
	//errors_str = alifUStr_fromWideChar(errors, -1);
	//if (errors_str == nullptr) {
	//	ALIF_CLEAR(buf);
	//	ALIF_CLEAR(encoding_str);
	//	goto error;
	//}

	stream = _alifObject_callMethod(io, &ALIF_STR(TextIOWrapper), "OOOsOO", buf,
		encoding_str, nullptr/*errors_str*/, newline, line_buffering, write_through);
	ALIF_CLEAR(buf);
	ALIF_CLEAR(encoding_str);
	//ALIF_CLEAR(errors_str);
	if (stream == nullptr)
		goto error;

	if (write_mode)
		mode = "w";
	else
		mode = "r";
	text = alifUStr_fromString(mode);
	if (!text or alifObject_setAttr(stream, &ALIF_STR(Mode), text) < 0)
		goto error;
	ALIF_CLEAR(text);
	return stream;

error:
	ALIF_XDECREF(buf);
	ALIF_XDECREF(stream);
	ALIF_XDECREF(text);
	ALIF_XDECREF(raw);

	//if (alifErr_exceptionMatches(_alifExcOSError_) and !_alif_isValidFD(fd)) {
	//	alifErr_clear();
	//	return ALIF_NONE;
	//}
	return nullptr;
}




static AlifStatus init_setBuiltinsOpen(void) { // 2709
	AlifObject* wrapper{};
	AlifObject* bimod = nullptr;
	AlifStatus res = ALIFSTATUS_OK();

	if (!(bimod = alifImport_importModule("builtins"))) {
		goto error;
	}

	if (!(wrapper = _alifImport_getModuleAttrString("التبادل", "افتح"))) {
		goto error;
	}

	/* Set builtins.open */
	if (alifObject_setAttrString(bimod, "افتح", wrapper) == -1) {
		ALIF_DECREF(wrapper);
		goto error;
	}
	ALIF_DECREF(wrapper);
	goto done;

error:
	res = ALIFSTATUS_ERR("can't initialize io.open");

done:
	ALIF_XDECREF(bimod);
	return res;
}



static AlifStatus init_sysStreams(AlifThread* _thread) { // 2742
	AlifObject* iomod = nullptr;
	AlifObject* std = nullptr;
	AlifIntT fd{};
	AlifObject* encoding_attr;
	AlifStatus res = ALIFSTATUS_OK();
	const AlifConfig* config = alifInterpreter_getConfig(_thread->interpreter);

#ifndef _WINDOWS
	class AlifStatStruct sb;
	if (_alifFStat_noraise(fileno(stdin), &sb) == 0 and
		S_ISDIR(sb.st_mode)) {
		return ALIFSTATUS_ERR("<stdin> is a directory, cannot continue");
	}
#endif

	if (!(iomod = alifImport_importModule("التبادل"))) {
		goto error;
	}

	/* Set sys.stdin */
	fd = fileno(stdin);
	//std = create_stdio(config, iomod, fd, 0, "<stdin>",
	//	config->stdioEncoding,
	//	config->stdioErrors);
	//if (std == nullptr)
	//	goto error;
	//alifSys_setObject("__stdin__", std);
	//_alifSys_setAttr(&ALIF_ID(Stdin), std);
	//ALIF_DECREF(std);

	///* Set sys.stdout */
	fd = fileno(stdout);
	//std = create_stdio(config, iomod, fd, 1, "<stdout>",
	//	config->stdioEncoding,
	//	config->stdioErrors);
	//if (std == nullptr)
	//	goto error;
	//alifSys_setObject("__stdout__", std);
	//_alifSys_setAttr(&ALIF_ID(stdout), std);
	//ALIF_DECREF(std);

	///* Set sys.stderr, replaces the preliminary stderr */
	//fd = fileno(stderr);
	//std = create_stdio(config, iomod, fd, 1, "<stderr>",
	//	config->stdioEncoding,
	//	L"backslashreplace");
	//if (std == nullptr)
	//	goto error;

	//encoding_attr = alifObject_getAttrString(std, "encoding");
	//if (encoding_attr != nullptr) {
	//	const char* std_encoding = alifUStr_asUTF8(encoding_attr);
	//	if (std_encoding != nullptr) {
	//		AlifObject* codec_info = _alifCodec_lookup(std_encoding);
	//		ALIF_XDECREF(codec_info);
	//	}
	//	ALIF_DECREF(encoding_attr);
	//}
	//_alifErr_clear(_thread);  /* Not a fatal error if codec isn't available */

	//if (alifSys_setObject("__stderr__", std) < 0) {
	//	ALIF_DECREF(std);
	//	goto error;
	//}
	//if (_alifSys_setAttr(&ALIF_ID(stderr), std) < 0) {
	//	ALIF_DECREF(std);
	//	goto error;
	//}
	//ALIF_DECREF(std);

	goto done;

error:
	// res = ALIFSTATUS_ERR("can't initialize sys standard streams");

done:
	ALIF_XDECREF(iomod);
	return res;
}


/* Print fatal error message and abort */
#ifdef _WINDOWS
static void fatalOutput_debug(const char* _msg) { // 2959
	WCHAR buffer[256 / sizeof(WCHAR)];
	size_t buflen = ALIF_ARRAY_LENGTH(buffer) - 1;
	size_t msglen;

	OutputDebugStringW(L"Fatal Alif error: ");

	msglen = strlen(_msg);
	while (msglen) {
		size_t i;

		if (buflen > msglen) {
			buflen = msglen;
		}

		for (i = 0; i < buflen; ++i) {
			buffer[i] = _msg[i];
		}
		buffer[i] = L'\0';
		OutputDebugStringW(buffer);

		_msg += buflen;
		msglen -= buflen;
	}
	OutputDebugStringW(L"\n");
}
#endif

static inline void ALIF_NO_RETURN fatalError_exit(AlifIntT _status) { // 3023
	if (_status < 0) {
	#if defined(_WINDOWS) and defined(_DEBUG)
		DebugBreak();
	#endif
		abort();
	}
	else {
		exit(_status);
	}
}

static void ALIF_NO_RETURN fatal_error(AlifIntT _fd, AlifIntT _header,
	const char* _prefix, const char* _msg, AlifIntT _status) { // 3167
	static AlifIntT reEntrant = 0;

	if (reEntrant) {
		fatalError_exit(_status);
	}
	reEntrant = 1;

	if (_header) {
		//PUTS(_fd, "Fatal Alif error: ");
		//if (_prefix) {
		//	PUTS(_fd, _prefix);
		//	PUTS(_fd, ": ");
		//}
		//if (_msg) {
		//	PUTS(_fd, _msg);
		//}
		//else {
		//	PUTS(_fd, "<message not set>");
		//}
		//PUTS(_fd, "\n");
	}

	//AlifRuntime* runtime = &_alifRuntime_;
	//fatalError_dumpRuntime(_fd, runtime);

	//AlifThread* tstate = _alifThread_get();
	//AlifInterpreter* interp = nullptr;
	//AlifThread* tssThread = alifGILState_getThisThreadState();
	//if (tstate != nullptr) {
	//	interp = tstate->interpreter;
	//}
	//else if (tssThread != nullptr) {
	//	interp = tssThread->interpreter;
	//}
	//int has_tstate_and_gil = (tssThread != nullptr && tssThread == tstate);

	//if (has_tstate_and_gil) {
	//	if (!_alifFatalError_printExc(tssThread)) {
	//		_alifFatalError_dumpTracebacks(_fd, interp, tssThread);
	//	}
	//}
	//else {
	//	_alifFatalError_dumpTracebacks(_fd, interp, tssThread);
	//}

	//_alif_dumpExtensionModules(_fd, interp);

	//_alifFaultHandler_Fini();

	//if (has_tstate_and_gil) {
	//	flush_std_files();
	//}

#ifdef _WINDOWS
	fatalOutput_debug(_msg);
#endif /* _WINDOWS */

	fatalError_exit(_status);
}

void ALIF_NO_RETURN alif_fatalError(const char* _msg) { // 3252
	fatal_error(fileno(stderr), 1, nullptr, _msg, -1);
}




void ALIF_NO_RETURN alif_exitStatusException(AlifStatus _status) { // 3306
	if (ALIFSTATUS_IS_EXIT(_status)) {
		exit(_status.exitcode);
	}
	else if (ALIFSTATUS_IS_ERROR(_status)) {
		fatal_error(fileno(stderr), 1, _status.func, _status.errMsg, 1);
	}
	else {
		alif_fatalError("alif_exitStatusException() يجب أن لا يتم إستدعاء هذه الدالة في حال انتهى التنفيذ بنجاح");
	}
}


void ALIF_NO_RETURN alif_exit(AlifIntT _sts) { // 3383
	AlifThread* thread = _alifThread_get();
	if (thread != nullptr and _alifThread_isRunningMain(thread)) {
		_alifInterpreter_setNotRunningMain(thread->interpreter);
	}
	//if (_alif_finalize(&_alifRuntime_) < 0) {
	//	_sts = 120;
	//}

	exit(_sts);
}



AlifOSSigHandlerT alifOS_setSig(AlifIntT sig, AlifOSSigHandlerT handler) { // 3475
#ifdef HAVE_SIGACTION
	sigaction context, ocontext;
	context.sa_handler = handler;
	sigemptyset(&context.sa_mask);
	context.sa_flags = SA_ONSTACK;
	if (sigaction(sig, &context, &ocontext) == -1)
		return SIG_ERR;
	return ocontext.sa_handler;
#else
	AlifOSSigHandlerT oldhandler{};
	oldhandler = signal(sig, handler);
#ifdef HAVE_SIGINTERRUPT
	siginterrupt(sig, 1);
#endif
	return oldhandler;
#endif
}
