

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

void alifRuntime_initialize()
{







	if (runtimeInitialized) {
		return;
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
		alifConfig_write(config, _tState->interp->runtime);



	}

	//if (alif_isMainInterpreter(_tState->interp)) {
	//	alifPathConfig_updateGlobal(config);




	//}

	//_tState->interp->longState.maxStrDigits = config->intMaxStrDigits;

	//if (alifSys_updateConfig(_tState) < 0) {
	//	return -1;
	//}
	return 0;
}


















































static void alifInit_coreReconfigure(AlifRuntimeState* _runtime, AlifThreadState** _tStateP, const AlifConfig* _config)
{

	AlifThreadState* tState = alifThreadState_get();



	*_tStateP = tState;

	AlifInterpreterState* interp = tState->interp;
	if (interp == nullptr) {
		std::cout << ("can't make main interpreter") << std::endl;
		exit(-1);
	}

	alifConfig_write(_config, _runtime);




	alifConfig_copy(&interp->config, _config);



	_config = alifInterpreterState_getConfig(interp);

	if (_config->installImportLib) {
		//alifPathConfig_updateGlobal(_config);



	}

}




static void alifCore_initRuntime(AlifRuntimeState* _runtime, const AlifConfig* _config)
{
	if (_runtime->initialized2) {
		std::cout << ("main interpreter already initialized") << std::endl;
		exit(-1);
	}

	alifConfig_write(_config, _runtime);




	alifRuntimeState_setFinalizing(_runtime, nullptr);

	alif_initVersion();

	//alif_hashRandomizationInit(_config);




	//alifTime_init();




	//alifImport_init();




	alifInterpreterState_enable(_runtime);




}












static void init_interpSettings(AlifInterpreterState* _interp, const AlifInterpreterConfig* _config)
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


}









static void initInterp_createGIL(AlifThreadState* _tState, int _gil)
{


	//alifEval_finiGIL(_tState->interp);

	//alifGILState_setTstate(_tState);




	int own_gil;
	switch (_gil) {
	case ALIFINTERPRETERCONFIG_DEFAULT_GIL: own_gil = 0; break;
	case ALIFINTERPRETERCONFIG_SHARED_GIL: own_gil = 0; break;
	case ALIFINTERPRETERCONFIG_OWN_GIL: own_gil = 1; break;
	default:
		std::cout << ("invalid interpreter config 'gil' value") << std::endl;
		exit(-1);
	}

	//alifEval_initGIL(_tState, own_gil);





}









static void alifCore_createInterpreter(AlifRuntimeState* _runtime,
	const AlifConfig* _srcConfig,
	AlifThreadState** _tStateP)
{

	AlifInterpreterState* interp{};
	alifInterpreterState_new(nullptr, &interp);




	//assert(alif_isMainInterpreter(interp));

	alifConfig_copy(&interp->config, _srcConfig);




	alifGILState_init(interp);




	AlifInterpreterConfig config = ALIFINTERPRETEECONFIG_LEGACY_INIT;

	config.gil = ALIFINTERPRETERCONFIG_OWN_GIL;
	init_interpSettings(interp, &config);




	AlifThreadState* tState = alifThreadState_new(interp);
	if (tState == nullptr) {
		std::cout << ("can't make first thread") << std::endl;
		exit(-1);
	}


	//alifThreadState_bind(tState);
	//(void)alifThreadState_swapNoGIL(tState);

	initInterp_createGIL(tState, config.gil);




	*_tStateP = tState;

}











































































































































static void alifCore_interpInit(AlifThreadState* _tState)
{
	AlifInterpreterState* interp = _tState->interp;

	//AlifObject* sysMod = nullptr;

	//alifCore_initGlobalObjects(interp);




	//alifGC_init(interp);



	//if (alif_deepFreezeInit() < 0) {
	//	std::cout << ("failed to initialize deep-frozen modules") << std::endl;
	//  exit(-1);
	//}

	//alifCore_initTypes(interp);




	//if (alifWarnings_initState(interp) < 0) {
	//	std::cout << ("can't initialize warnings") << std::endl;
	//  exit(-1);
	//}

	//alifAtExit_init(interp);




	//alifSys_create(_tState, &sysMod);




	//alifCore_initBuiltins(_tState);




	const AlifConfig* config = alifInterpreterState_getConfig(interp);

	//alifImport_initCore(_tState, sysMod, config->installImportLib);







}



static void alifInit_config(AlifRuntimeState* _runtime, AlifThreadState** _tStateP, const AlifConfig* _config)
{
	alifCore_initRuntime(_runtime, _config);




	AlifThreadState* tState{};
	alifCore_createInterpreter(_runtime, _config, &tState);



	*_tStateP = tState;

	alifCore_interpInit(tState);




	_runtime->coreInitialized = 1;

}






void alif_preInitializeFromAlifArgv(const AlifPreConfig* _srcConfig, const AlifArgv* _args)
{


	if (_srcConfig == nullptr) {
		std::cout << ("preinitialization config is null") << std::endl;
		exit(-1);
	}

	alifRuntime_initialize();



	AlifRuntimeState* runtime = &alifRuntime;

	if (runtime->preinitialized) {

		return;
	}

	runtime->preinitializing = 1;

	AlifPreConfig config{};

	alifPreConfig_initFromPreConfig(&config, _srcConfig);




	alifPreConfig_read(&config, _args);




	alifPreConfig_write(&config);




	runtime->preinitializing = 0;
	runtime->preinitialized = 1;

}





















void alif_preInitialize(const AlifPreConfig* _srcConfig)
{
	alif_preInitializeFromAlifArgv(_srcConfig, nullptr);
}



void alif_preInitializeFromConfig(const AlifConfig* _config, const AlifArgv* _args)
{
	//assert(_config != nullptr);

	alifRuntime_initialize();



	AlifRuntimeState* runtime = &alifRuntime;

	if (runtime->preinitialized) {

		return;
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





















static void alifInit_core(AlifRuntimeState* _runtime, const AlifConfig* _srcConfig, AlifThreadState** _tstateP)
{


	alif_preInitializeFromConfig(_srcConfig, nullptr);




	AlifConfig config{};
	alifConfig_initAlifConfig(&config);

	alifConfig_copy(&config, _srcConfig);




	alifConfig_read(&config, 0);




	if (!_runtime->coreInitialized) {
		alifInit_config(_runtime, _tstateP, &config);
	}
	else {
		alifInit_coreReconfigure(_runtime, _tstateP, &config);
	}







}











static void alifInit_mainReconfigure(AlifThreadState* _tState)
{
	if (interpreter_updateConfig(_tState, 0) < 0) {
		std::cout << ("fail to reconfigure ALif") << std::endl;
		exit(-1);
	}

}


static void init_interpMain(AlifThreadState* _tState)
{
	//assert(!alifErr_occurred(_tState));


	int isMainInterp = alif_isMainInterpreter(_tState->interp);
	AlifInterpreterState* interp = _tState->interp;
	const AlifConfig* config = alifInterpreterState_getConfig(interp);

	if (!config->installImportLib) {
		if (isMainInterp) {
			interp->runtime->initialized = 1;
		}
		return;
	}

	//alifConfig_initImportConfig(&interp->config);




	if (interpreter_updateConfig(_tState, 1) < 0) {
		std::cout << ("failed to update the Alif config") << std::endl;
		exit(-1);
	}

	//alifImport_initExternal(_tState);




	//if (isMainInterp) {
	//	alifFaultHandler_init(config->faultHandler);



	//}

	//alifUnicode_initEncodings(_tState);




//	if (isMainInterp) {
//		if (alifSignal_init(config->installSignalHandlers) < 0) {
//			std::cout << ("can't initialize signals") << std::endl;
//			exit(-1);
//		}
//
//		if (config->traceMalloc) {
//			if (alifTraceMalloc_start(config->traceMalloc) < 0) {
//				std::cout << ("can't start tracemalloc") << std::endl;
//				exit(-1);
//			}
//		}
//
//#ifdef ALIF_HAVEPERF_TRAMPOLINE
//		if (config->perfProfiling) {
//			if (alifPerfTrampoline_setCallbacks(&alifPerfmapCallbacks) < 0 ||
//				alifPerfTrampoline_init(config->perfProfiling) < 0) {
//				std::cout << ("can't initialize the perf trampoline") << std::endl;
//				exit(-1);
//			}
//		}
//#endif
//	}

	//init_sysStreams(_tState);




	//initSet_builtinsOpen();




	//add_mainModule(interp);




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
	//	init_importSite();



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
	//			std::cout << ("can't initialize optimizer") << std::endl;
	//			exit(-1);
	//		}
	//		alifUnstable_setOptimizer((AlifOptimizerObject*)opt);
	//		ALIF_DECREF(opt);
	//	}
	//}

	//assert(!alifErr_occurred(_tState));


}


















static void alifInit_main(AlifThreadState* _tState)
{
	AlifInterpreterState* interp = _tState->interp;
	if (!interp->runtime->coreInitialized) {
		std::cout << ("runtime core not initialized") << std::endl;
		exit(-1);
	}

	if (interp->runtime->initialized) {
		return alifInit_mainReconfigure(_tState);
	}

	init_interpMain(_tState);




}

void alif_initializeFromConfig(const AlifConfig* _config)
{
	if (_config == nullptr) {
		std::cout << ("initialization config is nullptr") << std::endl;
		exit(-1);
	}



	alifRuntime_initialize();



	AlifRuntimeState* runtime = &alifRuntime;

	AlifThreadState* tstate = nullptr;
	alifInit_core(runtime, _config, &tstate);



	_config = alifInterpreterState_getConfig(tstate->interp);

	if (_config->initMain) {
		alifInit_main(tstate);



	}

}
