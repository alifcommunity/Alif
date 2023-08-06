#include "alif.h"
#include "alifCore_initConfig.h"
#include "alifCore_alifCycle.h"
#include "alifCore_runtime.h"




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

	AlifRuntimeState runtime {};
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((section(".alifRuntime")))
#endif
  

static int runtimeInitialized = 0;


AlifStatus AlifRuntime_initialize(void) {


	if (runtimeInitialized) {
		return ALIFSTATUS_OK();
	}
	runtimeInitialized = 1;
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

	AlifPreConfig preconfig;

	alifPreConfig_initFromConfig(&preconfig, _config);

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

	//status = alifPreConfig_read(&config, _args);
	//if (ALIFSTATUS_EXCEPTION(status)) {
	//	return status;
	//}

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
