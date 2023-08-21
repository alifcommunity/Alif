#include "alif.h"

/*#include "alifCore_call.h"          
#include "alifCore_ceval.h"         
#include "alifCore_codecs.h"        
#include "alifCore_context.h"       
#include "alifCore_dict.h"          
#include "alifCore_exceptions.h"    
#include "alifCore_fileUtils.h"     
#include "alifCore_floatObject.h"   
#include "alifCore_genObject.h"     
#include "alifCore_globalObjectsFiniGenerated.h" 
#include "alifCore_import.h"  */      
#include "alifCore_initConfig.h"    
//#include "alifCore_list.h"
//#include "alifCore_long.h"
//#include "alifCore_object.h"
//#include "alifCore_pathConfig.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifcycle.h"   
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
#include "alifCore_unicodeObject.h" 
//#include "alifCore_weakRef.h"       
//#include "opcode.h"

#include <locale.h>               // setlocale()
















#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif


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

AlifRuntimeState alifRuntime
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__ ((section(".alifRuntime")))
#endif
= { .allocators = {
	.standard = {
		{ &alifRuntime.allocators.debug.raw, alifMem_debug_raw_malloc, alifMem_debug_raw_calloc, alifMem_debug_raw_realloc, alifMem_debug_raw_free }
		,{ &alifRuntime.allocators.debug.mem, alifMem_malloc, alifMem_calloc, alifMem_realloc, alifMem_free }
		,{ &alifRuntime.allocators.debug.obj, alifMem_malloc, alifMem_calloc, alifMem_realloc, alifMem_free },
		},
	.debug = {
		{ 'r',{ nullptr, alifMem_rawMalloc, alifMem_rawCalloc, alifMem_rawRealloc, alifMem_rawFree } }
		,{ 'm',{ nullptr, alifObject_malloc, alifObject_calloc, alifObject_realloc, alifObject_free } }
		,{ 'o',{ nullptr, alifObject_malloc, alifObject_calloc, alifObject_realloc, alifObject_free } },
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

	return alifRuntimeState_init(&alifRuntime);
}





























































































































































































































char* alif_setLocaleFromEnv(int category)
{
	char* res;
#ifdef __ANDROID__
	const char* locale;
	const char** pVar;
#ifdef ALIF_COERCE_CPP_LOCALE
	const char* coerceCppLocale;
#endif
	const char* utf8Locale = "C.UTF-8";
	const char* envVarSet[] = { "LC_ALL", "LC_CTYPE", "LANG", nullptr };




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







#ifdef ALIF_COERCE_CPP_LOCALE
	coerceCppLocale = getenv("ALIFCOERCECLOCALE");
	if (coerceCppLocale == nullptr || strcmp(coerceCppLocale, "0") != 0) {



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

 


















































































































































































































































































































































































































































































AlifStatus alif_preInitializeFromAlifArgv(const AlifPreConfig* _srcConfig, const AlifArgv* _args)
{
	AlifStatus status{};

	status = alifRuntime_initialize();
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}
	AlifRuntimeState* runtime = &alifRuntime;

	if (runtime->preinitialized) {
		return ALIFSTATUS_OK();
	}

	runtime->preinitializing = 1;

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


static AlifStatus alifInit_core(AlifRuntimeState* _runtime, const AlifConfig* _srcConfig, AlifThreadState** _tStateP)
{
	AlifStatus status;
	AlifConfig config = *_srcConfig;

	status = alifConfig_read(&config);

	return status;
}


AlifStatus alifInit_fromConfig(const AlifConfig* _config)
{
	if (_config == nullptr) {
		return ALIFSTATUS_ERR("متغير التهيئة فارغ");
	}

	AlifStatus status{};

	status = alifRuntime_initialize();
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	AlifRuntimeState* runtime = &alifRuntime;

	AlifThreadState* tState = nullptr;
	status = alifInit_core(runtime, _config, &tState);
	if (ALIFSTATUS_EXCEPTION(status)) {
		return status;
	}
	//_config = alifInterpreterState_getConfig(tState->interp);

	if (_config->initMain) {
		//status = alifInit_main(tState);
		if (ALIFSTATUS_EXCEPTION(status)) {
			return status;
		}
	}

	return ALIFSTATUS_OK();
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













































void ALIF_NO_RETURN alif_fatalErrorFunc(const char* _func, const char* _msg)
{
	//fatal_error(fileno(stderr), 1, _func, _msg, -1);
}
