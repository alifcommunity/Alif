

#include "alif.h"

//#include "alifCore_call.h"
//#include "alifCore_cEval.h"
//#include "alifCore_codeCS.h"
//#include "alifCore_context.h"
//#include "alifCore_dict.h"
//#include "alifCore_exceptions.h"
#include "alifCore_fileUtils.h"
//#include "alifCore_floatObject.h"
//#include "alifCore_genObject.h"
//#include "alifCore_globalObjectsFiniGenerated.h"
#include "alifCore_import.h"
#include "alifCore_initConfig.h"
//#include "alifCore_list.h"
//#include "alifCore_long.h"
//#include "alifCore_object.h"
//#include "alifCore_pathcConfig.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifLifeCycle.h"
#include "alifCore_alifMem.h"
#include "alifCore_alifState.h"
#include "alifCore_runtime.h"
#include "alifCore_runtimeInit.h"
//#include "alifCore_setObject.h"
//#include "alifCore_sliceObject.h"
//#include "alifCore_sysModule.h"
//#include "alifCore_traceback.h"
//#include "alifCore_typeObject.h"
//#include "alifCore_typeVarObject.h"
//#include "alifCore_unicodeObject.h"
//#include "alifCore_weakRef.h"
//#include "opCode.h"

#include <locale.h>               // setlocale()
//#include <stdlib.h>

#if defined(__APPLE__)
#include <mach-o/loader.h>
#endif

#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif

#ifdef HAVE_LANGINFO_H
#  include <langinfo.h> 
#endif

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#ifdef MS_WINDOWS
#  undef BYTE
#endif







































AlifRuntimeState alifRuntime
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((section(".AlifRuntime")))
#endif
= ALIFRUNTIMESTATE_INIT(alifRuntime);
ALIFCOMP_DIAGPOP

static int runtimeInitialized = 0;

AlifStatus alifRuntime_initialize()
{







	if (runtimeInitialized) {
		return ALIFSTATUS_OK();
	}
	runtimeInitialized = 1;

	return alifRuntimeState_init(&alifRuntime);
}


























































































































































int alif_coerceLegacyLocale(int warn)
{
	int coerced = 0;
#ifdef ALIFCOERCE_C_LOCALE
	char* oldloc = nullptr;

	oldloc = alifMem_rawStrdup(setlocale(LC_CTYPE, nullptr));
	if (oldloc == nullptr) {
		return coerced;
	}

	const char* localeOverride = getenv("LC_ALL");
	if (localeOverride == nullptr || *localeOverride == '\0') {
		const LocaleCoercionTarget* target = nullptr;
		for (target = TARGET_LOCALES; target->localeName; target++) {
			const char* newLocale = setlocale(LC_CTYPE,
				target->localeName);
			if (newLocale != nullptr) {
#if !defined(ALIFFORCE_UTF8_LOCALE) && defined(HAVE_LANGINFO_H) && defined(CODESET)
				char* codeset = nL_langInfo(CODESET);
				if (!codeset || *codeset == '\0') {
					newLocale = nullptr;
					alif_setLocaleFromEnv(LC_CTYPE);
					continue;
				}
#endif
				coerced = coerceDefault_localeSettings(warn, target);
				goto done;
			}
		}
	}

	setlocale(LC_CTYPE, oldloc);

done:
	alifMem_rawFree(oldloc);
#endif
	return coerced;
}











char* alif_setLocaleFromEnv(int category)
{
	char* res;
#ifdef __ANDROID__
	const char* locale;
	const char** pVar;
#ifdef ALIFCOERCE_C_LOCALE
	const char* coerceCLocale;
#endif
	const char* utf8Locale = "C.UTF-8";
	const char* envVarSet[] = {
		"LC_ALL",
		"LC_CTYPE",
		"LANG",
		nullptr,
	};

	for (pVar = envVarSet; *pVar; pVar++) {
		locale = getenv(*pVar);
		if (locale != nullptr && *locale != '\0') {
			if (strcmp(locale, utf8Locale) == 0 ||
				strcmp(locale, "en_US.UTF-8") == 0) {
				return setlocale(category, utf8Locale);
			}
			return setlocale(category, "C");
		}
	}

#ifdef ALIFCOERCE_C_LOCALE
	coerceCLocale = getenv("ALIFCOERCECLOCALE");
	if (coerceCLocale == nullptr || strcmp(coerceCLocale, "0") != 0) {

		if (setenv("LC_CTYPE", utf8Locale, 1)) {
			fprintf(stderr, "Warning: failed setting the LC_CTYPE "
				"environment variable to %s\n", utf8Locale);
		}
	}
#endif
	res = setlocale(category, utf8Locale);
#else /* !defined(__ANDROID__) */
	res = setlocale(category, "");
#endif
	//alif_resetForceASCII();
	return res;
}














static int interpreter_updateConfig(AlifThreadState* _tState, int _onlyUpdatePathConfig)
{
	const AlifConfig* config = &_tState->interp->config;

	if (!_onlyUpdatePathConfig) {
		AlifStatus status = alifConfig_write(config, _tState->interp->runtime);
		if (ALIFSTATUS_EXCEPTION(status)) {
			//alifErr_setFromAlifStatus(status);
			return -1;
		}
	}

	//if (alif_isMainInterpreter(_tState->interp)) {
	//	AlifStatus status = alifPathConfig_updateGlobal(config);
	//	if (ALIFSTATUS_EXCEPTION(status)) {
	//		alifErr_setFromAlifStatus(status);
	//		return -1;
	//	}
	//}

	//_tState->interp->longState.maxStrDigits = config->intMaxStrDigits;

	//if (alifSys_updateConfig(_tState) < 0) {
	//	return -1;
	//}
	return 0;
}


















































static AlifStatus alifInit_coreReconfigure(AlifRuntimeState* _runtime, AlifThreadState** _tStateP, const AlifConfig* _config)
{
	AlifStatus status{};
	AlifThreadState* tState = alifThreadState_get();
	if (!tState) {
		return ALIFSTATUS_ERR("failed to read thread state");
	}
	*_tStateP = tState;

	AlifInterpreterState* interp = tState->interp;
	if (interp == nullptr) {
		return ALIFSTATUS_ERR("can't make main interpreter");
	}

	status = alifConfig_write(_config, _runtime);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = alifConfig_copy(&interp->config, _config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	_config = alifInterpreterState_getConfig(interp);

	if (_config->installImportLib) {
		//status = alifPathConfig_updateGlobal(_config);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}
	return ALIFSTATUS_OK();
}





static AlifStatus alifCore_initRuntime(AlifRuntimeState* _runtime, const AlifConfig* _config)
{
	if (_runtime->initialized) {
		return ALIFSTATUS_ERR("main interpreter already initialized");
	}

	AlifStatus status = alifConfig_write(_config, _runtime);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	alifRuntimeState_setFinalizing(_runtime, nullptr);

	alif_initVersion();

	//status = alif_hashRandomizationInit(_config);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = alifTime_init();
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = alifImport_init();
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	status = alifInterpreterState_enable(_runtime);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	return ALIFSTATUS_OK();
}













static AlifStatus init_interpSettings(AlifInterpreterState* _interp, const AlifInterpreterConfig* _config)
{
	//assert(_interp->featureFlags == 0);

	//if (_config->useMainObmalloc) {
	//	_interp->featureFlags |= ALIF_RTFLAGS_USE_MAIN_OBMALLOC;
	//}
	//else if (!_config->checkMultiInterpExtensions) {
	//	return _PyStatus_ERR("per-interpreter obmalloc does not support "
	//		"single-phase init extension modules");
	//}

	//if (_config->allowFork) {
	//	_interp->featureFlags |= ALIF_RTFLAGS_FORK;
	//}
	//if (_config->allowExec) {
	//	_interp->featureFlags |= ALIF_RTFLAGS_EXEC;
	//}

	//if (_config->allowThreads) {
	//	_interp->featureFlags |= ALIF_RTFLAGS_THREADS;
	//}
	//if (_config->allowDaemonThreads) {
	//	_interp->featureFlags |= ALIF_RTFLAGS_DAEMON_THREADS;
	//}

	//if (_config->checkMultiInterpExtensions) {
	//	_interp->featureFlags |= ALIF_RTFLAGS_MULTI_INTERP_EXTENSIONS;
	//}

	return ALIFSTATUS_OK();
}









static AlifStatus initInterp_createGIL(AlifThreadState* _tState, int _gil)
{
	AlifStatus status{};

	//alifEval_finiGIL(_tState->interp);

	//status = alifGILState_setTstate(_tState);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	int own_gil;
	switch (_gil) {
	case ALIFINTERPRETERCONFIG_DEFAULT_GIL: own_gil = 0; break;
	case ALIFINTERPRETERCONFIG_SHARED_GIL: own_gil = 0; break;
	case ALIFINTERPRETERCONFIG_OWN_GIL: own_gil = 1; break;
	default:
		return ALIFSTATUS_ERR("invalid interpreter config 'gil' value");
	}

	//status = alifEval_initGIL(_tState, own_gil);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	return ALIFSTATUS_OK();
}







static AlifStatus alifCore_createInterpreter(AlifRuntimeState* _runtime, const AlifConfig* _srcConfig, AlifThreadState** _tStateP)
{
	AlifStatus status{};
	AlifInterpreterState* interp = alifInterpreterState_new();
	if (interp == nullptr) {
		return ALIFSTATUS_ERR("can't make main interpreter");
	}
	//assert(alif_isMainInterpreter(interp));

	status = alifConfig_copy(&interp->config, _srcConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = alifGILState_init(interp);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	AlifInterpreterConfig config = ALIFINTERPRETEECONFIG_LEGACY_INIT;
	config.gil = ALIFINTERPRETERCONFIG_OWN_GIL;
	status = init_interpSettings(interp, &config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	AlifThreadState* tState = alifThreadState_new(interp);
	if (tState == nullptr) {
		return ALIFSTATUS_ERR("can't make first thread");
	}
	//alifThreadState_bind(tState);
	//(void)alifThreadState_swapNoGIL(tState);

	status = initInterp_createGIL(tState, config.gil);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	*_tStateP = tState;
	return ALIFSTATUS_OK();
}






































































































































static AlifStatus alifCore_interpInit(AlifThreadState* _tState)
{
	AlifInterpreterState* interp = _tState->interp;
	AlifStatus status{};
	//AlifObject* sysMod = nullptr;

	//status = alifCore_initGlobalObjects(interp);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//status = alifGC_init(interp);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	//if (alif_deepFreezeInit() < 0) {
	//	return ALIFSTATUS_ERR("failed to initialize deep-frozen modules");
	//}

	//status = alifCore_initTypes(interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	goto done;
	//}

	//if (alifWarnings_initState(interp) < 0) {
	//	return ALIFSTATUS_ERR("can't initialize warnings");
	//}

	//status = alifAtExit_init(interp);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//status = alifSys_create(_tState, &sysMod);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	goto done;
	//}

	//status = alifCore_initBuiltins(_tState);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	goto done;
	//}

	const AlifConfig* config = alifInterpreterState_getConfig(interp);

	//status = alifImport_initCore(_tState, sysMod, config->installImportLib);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

done:
	//ALIF_XDECREF(sysMod);
	return status;
}









static AlifStatus alifInit_config(AlifRuntimeState* _runtime, AlifThreadState** _tStateP, const AlifConfig* _config)
{
	AlifStatus status = alifCore_initRuntime(_runtime, _config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	AlifThreadState* tState{};
	status = alifCore_createInterpreter(_runtime, _config, &tState);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	*_tStateP = tState;

	status = alifCore_interpInit(tState);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	_runtime->coreInitialized = 1;
	return ALIFSTATUS_OK();
}






AlifStatus alif_preInitializeFromAlifArgv(const AlifPreConfig* _srcConfig, const AlifArgv* _args)
{
	AlifStatus status{};

	if (_srcConfig == nullptr) {
		return ALIFSTATUS_ERR("preinitialization config is null");
	}

	//status = alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	//AlifRuntimeState* runtime = &alifRuntime;

	//if (runtime->preinitialized) {

	//	return ALIFSTATUS_OK();
	//}

	//runtime->preinitializing = 1;

	AlifPreConfig config{};

	status = alifPreConfig_initFromPreConfig(&config, _srcConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = alifPreConfig_read(&config, _args);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = alifPreConfig_write(&config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//runtime->preinitializing = 0;
	//runtime->preinitialized = 1;
	return ALIFSTATUS_OK();
}





















AlifStatus alif_preInitialize(const AlifPreConfig* _srcConfig)
{
	return alif_preInitializeFromAlifArgv(_srcConfig, nullptr);
}



AlifStatus alif_preInitializeFromConfig(const AlifConfig* _config, const AlifArgv* _args)
{
	//assert(_config != nullptr);

	AlifStatus status = alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	AlifRuntimeState* runtime = &alifRuntime;

	if (runtime->preinitialized) {

		return ALIFSTATUS_OK();
	}

	AlifPreConfig preConfig{};

	alifPreConfig_initFromConfig(&preConfig, _config);

	if (!_config->parseArgv) {
		return alif_preInitialize(&preConfig);
	}
	else if (_args == nullptr) {
		AlifArgv configArgs = {
			.argc = _config->argv.length,
			.useBytesArgv = 0,
			.wcharArgv = _config->argv.items };
		return alif_preInitializeFromAlifArgv(&preConfig, &configArgs);
	}
	else {
		return alif_preInitializeFromAlifArgv(&preConfig, _args);
	}
}





















static AlifStatus alifInit_core(AlifRuntimeState* _runtime, const AlifConfig* _srcConfig, AlifThreadState** _tstateP)
{
	AlifStatus status{};

	status = alif_preInitializeFromConfig(_srcConfig, nullptr);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	AlifConfig config{};
	alifConfig_initAlifConfig(&config);

	status = alifConfig_copy(&config, _srcConfig);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	status = alifConfig_read(&config, 0);
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

	if (!_runtime->coreInitialized) {
		status = alifInit_config(_runtime, _tstateP, &config);
	}
	else {
		status = alifInit_coreReconfigure(_runtime, _tstateP, &config);
	}
	if (ALIFSTATUS_EXCEPTION(status)) {
		goto done;
	}

done:
	alifConfig_clear(&config);
	return status;
}











static AlifStatus alifInit_mainReconfigure(AlifThreadState* _tState)
{
	if (interpreter_updateConfig(_tState, 0) < 0) {
		return ALIFSTATUS_ERR("fail to reconfigure ALif");
	}
	return ALIFSTATUS_OK();
}



static AlifStatus init_interpMain(AlifThreadState* _tState)
{
	//assert(!alifErr_occurred(_tState));

	AlifStatus status{};
	//int isMainInterp = alif_isMainInterpreter(_tState->interp);
	AlifInterpreterState* interp = _tState->interp;
	const AlifConfig* config = alifInterpreterState_getConfig(interp);

	if (!config->installImportLib) {
		//if (isMainInterp) {
		//	interp->runtime->initialized = 1;
		//}
		return ALIFSTATUS_OK();
	}

	//status = alifConfig_initImportConfig(&interp->config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	if (interpreter_updateConfig(_tState, 1) < 0) {
		return ALIFSTATUS_ERR("failed to update the Alif config");
	}

	//status = alifImport_initExternal(_tState);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//if (isMainInterp) {
	//	status = alifFaultHandler_init(config->faultHandler);
	//	if (ALIFSTATUS_EXCEPTION(status)) {
	//		return status;
	//	}
	//}

	//status = alifUnicode_initEncodings(_tState);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

//	if (isMainInterp) {
//		if (alifSignal_init(config->installSignalHandlers) < 0) {
//			return ALIFSTATUS_ERR("can't initialize signals");
//		}
//
//		if (config->traceMalloc) {
//			if (alifTraceMalloc_start(config->traceMalloc) < 0) {
//				return ALIFSTATUS_ERR("can't start tracemalloc");
//			}
//		}
//
//#ifdef ALIF_HAVEPERF_TRAMPOLINE
//		if (config->perfProfiling) {
//			if (alifPerfTrampoline_setCallbacks(&alifPerfmapCallbacks) < 0 ||
//				alifPerfTrampoline_init(config->perfProfiling) < 0) {
//				return ALIFSTATUS_ERR("can't initialize the perf trampoline");
//			}
//		}
//#endif
//	}

	//status = init_sysStreams(_tState);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = initSet_builtinsOpen();
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//status = add_mainModule(interp);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//if (isMainInterp) {
	//	AlifObject* warnOptions = alifSys_getObject("warnoptions");
	//	if (warnOptions != nullptr && alifList_size(warnOptions) > 0)
	//	{
	//		AlifObject* warningsModule = alifImport_importModule("warnings");
	//		if (warningsModule == nullptr) {
	//			fprintf(stderr, "'import warnings' failed; traceback:\n");
	//			alifErr_print(_tState);
	//		}
	//		ALIF_XDECREF(warningsModule);
	//	}

	//	interp->runtime->initialized = 1;
	//}

	//if (config->siteImport) {
	//	status = init_importSite();
	//	if (ALIFSTATUS_EXCEPTION(status)) {
	//		return status;
	//	}
	//}

//	if (isMainInterp) {
//#ifndef MS_WINDOWS
//		emitStderrWarning_forLegacyLocale(interp->runtime);
//#endif
//	}

	//if (isMainInterp) {
	//	char* envVar = ALIF_GETENV("ALIFUOPS");
	//	int enabled = envVar != nullptr && *envVar > '0';
	//	if (alif_getXOption(&config->xOptions, L"uops") != nullptr) {
	//		enabled = 1;
	//	}
	//	if (enabled) {
	//		AlifObject* opt = alifUnstableOptimizer_newUOpOptimizer();
	//		if (opt == nullptr) {
	//			return ALIFSTATUS_ERR("can't initialize optimizer");
	//		}
	//		alifUnstable_setOptimizer((AlifOptimizerObject*)opt);
	//		ALIF_DECREF(opt);
	//	}
	//}

	//assert(!alifErr_occurred(_tState));

	return ALIFSTATUS_OK();
}























static AlifStatus alifInit_main(AlifThreadState* _tState)
{
	AlifInterpreterState* interp = _tState->interp;
	if (!interp->runtime->coreInitialized) {
		return ALIFSTATUS_ERR("runtime core not initialized");
	}

	if (interp->runtime->initialized) {
		return alifInit_mainReconfigure(_tState);
	}

	AlifStatus status = init_interpMain(_tState);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	return ALIFSTATUS_OK();
}


AlifStatus alif_initializeFromConfig(const AlifConfig* _config)
{
	if (_config == nullptr) {
		return ALIFSTATUS_ERR("initialization config is nullptr");
	}

	AlifStatus status;

	status = alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	AlifRuntimeState* runtime = &alifRuntime;

	AlifThreadState* tstate = nullptr;
	status = alifInit_core(runtime, _config, &tstate);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	_config = alifInterpreterState_getConfig(tstate->interp);

	if (_config->initMain) {
		status = alifInit_main(tstate);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	return ALIFSTATUS_OK();
}
