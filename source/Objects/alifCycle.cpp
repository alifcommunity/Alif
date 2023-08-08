#include "alif.h"
#include "alifCore_initConfig.h"
#include "alifCore_alifCycle.h"
#include "alifCore_runtime.h"
#include "alifCore_runtime_init.h"
#include "alifCore_alifMem_init.h"
#include "alifCore_alifMemory_init.h"

// هنا تم عمل متغير يتم القراءة منه والكتابة يسمى alifRuntime 
// ومن ثم التحقق من النظام لوضع بيانات كائن AlifRuntimeState 
// ثم يتم استخراجه في ملف compiler لاحقا مع بيانات النظام كاملة

#if defined(MS_WINDOWS)

#pragma section("alifRuntime", read, write)
__declspec(allocate("alifRuntime"))

#elif defined(__APPLE__)

__attribute__((
	section(SEG_DATA ",alifRuntime")
	))

#endif

AlifRuntimeState runtime
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__ ((section(".alifRuntime")))
#endif
= { .allocators = {
	.standard = {
		{ &runtime.allocators.debug.raw, alifMem_debug_raw_malloc, alifMem_debug_raw_calloc, alifMem_debug_raw_realloc, alifMem_debug_raw_free }
		,{ &runtime.allocators.debug.mem, alifMem_malloc, alifMem_calloc, alifMem_realloc, alifMem_free }
		,{ &runtime.allocators.debug.obj, alifMem_malloc, alifMem_calloc, alifMem_realloc, alifMem_free },
		},
	.debug = {
		{ 'r',{ nullptr, alifMem_raw_malloc, alifMem_raw_calloc, alifMem_raw_realloc, alifMem_raw_free } }
		,{ 'm',{ nullptr, object_malloc, object_calloc, object_realloc, object_free } }
		,{ 'o',{ nullptr, object_malloc, object_calloc, object_realloc, object_free } },
		} ,
	.objArena =
		{ nullptr, alifMem_arenaAlloc, alifMem_arenaFree }
		},
	.memory = { .dumpDebugState = -1 }, .autoTSSKey = { 0 },
};

static int runtimeInitialized = 0;


AlifStatus alifRuntime_initialize()
{
	if (runtimeInitialized) {
		return ALIFSTATUS_OK();
	}
	runtimeInitialized = 1;
}


/* alif_setLocaleFromEnv() is a wrapper around setlocale(category, "") to
 * isolate the idiosyncrasies of different libc implementations. It reads the
 * appropriate environment variable and uses its value to select the locale for
 * 'category'. */
char* alif_setLocaleFromEnv(int category)
{
	char* res;
#ifdef __ANDROID__
	const char* locale;
	const char** pVar;
//#ifdef ALIF_COERCE_C_LOCALE
//	const char* coerce_c_locale;
//#endif
	const char* utf8Locale = "C.UTF-8";
	const char* envVarSet[] = { "LC_ALL", "LC_CTYPE", "LANG", nullptr };

	/* Android setlocale(category, "") doesn't check the environment variables
	 * and incorrectly sets the "C" locale at API 24 and older APIs. We only
	 * check the environment variables listed in envVarSet. */
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

	/* Android uses UTF-8, so explicitly set the locale to C.UTF-8 if none of
	 * LC_ALL, LC_CTYPE, or LANG is set to a non-empty string.
	 * Quote from POSIX section "8.2 Internationalization Variables":
	 * "4. If the LANG environment variable is not set or is set to the empty
	 * string, the implementation-defined default locale shall be used." */

#ifdef ALIF_COERCE_C_LOCALE
	coerce_c_locale = getenv("ALIFCOERCECLOCALE");
	if (coerce_c_locale == nullptr || strcmp(coerce_c_locale, "0") != 0) {
		/* Some other ported code may check the environment variables (e.g. in
		 * extension modules), so we make sure that they match the locale
		 * configuration */
		if (setenv("LC_CTYPE", utf8Locale, 1)) {
			fprintf(stderr, "تحذير: فشل ضبط LC_CTYPE "
				"متغير البيئة %s\n", utf8Locale);
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

 
AlifStatus alif_preInitializeFromConfig(const AlifConfig* _config, const AlifArgv* _args)
{
	//assert(_config != nullptr);

	//AlifStatus status = alifRuntime_initialize();
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}
	//AlifRuntimeState* runtime = &alifRuntime;

	//if (runtime->preinitialized) {
	//	/* Already initialized: do nothing */
	//	return ALIFSTATUS_OK();
	//}

	AlifPreConfig preconfig{};

	//alifPreConfig_initFromConfig(&preconfig, _config);

	if (!_config->parseArgv) {
		//return alif_preInitialize(&preconfig);
	}
	else if (_args == nullptr) {
		AlifArgv configArgs = {
			.argc = _config->argv.length,
			.useCharArgv = 0,
			.charArgv = nullptr,
			.wcharArgv = _config->argv.items
		};
		return alif_preInitializeFromAlifArgv(&preconfig, &configArgs);
	}
	else {
		return alif_preInitializeFromAlifArgv(&preconfig, _args);
	}
}


AlifStatus alif_preInitializeFromAlifArgv(const AlifPreConfig* _srcConfig, const AlifArgv* _args)
{
	AlifStatus status{};

	//status = alifRuntime_initialize();
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}
	//AlifRuntimeState* runtime = &alifRuntime;

	//if (runtime->preinitialized) {
	//	/* If it's already configured: ignored the new configuration */
	//	return ALIFSTATUS_OK();
	//}

	/* Note: preinitialized remains 1 on error, it is only set to 0
	   at exit on success. */
	//runtime->preinitializing = 1;

	AlifPreConfig config{};

	status = alifPreConfig_initFromPreConfig(&config);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	status = alifPreConfig_read(&config, _args);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}

	//status = alifPreConfig_write(&config);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

	//runtime->preinitializing = 0;
	//runtime->preinitialized = 1;
	return ALIFSTATUS_OK();
}


static AlifStatus alifInit_core(const AlifConfig* _srcConfig)
{
	AlifStatus status;
	AlifConfig config = *_srcConfig;

	status = alifConfig_read(&config);

	return status;
}


AlifStatus alifInit_fromConfig(const AlifConfig* _config)
{
	AlifStatus status{};

	status = alifInit_core(_config);

	return ALIFSTATUS_OK();
}
