

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
//#include "alifCore_import.h"
#include "alifCore_initConfig.h"
//#include "alifCore_list.h"
//#include "alifCore_long.h"
//#include "alifCore_object.h"
//#include "alifCore_pathcConfig.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifLifeCycle.h"
//#include "alifCore_alifMem.h"
//#include "alifCore_alifState.h"
#include "alifCore_runtime.h"
//#include "alifCore_runtimeInit.h"
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







































//AlifRuntimeState alifRuntime
//#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
//__attribute__((section(".AlifRuntime")))
//#endif
//= ALIFRUNTIMESTATE_INIT(alifRuntime);
//ALIF_COMP_DIAG_POP













































































































































































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
