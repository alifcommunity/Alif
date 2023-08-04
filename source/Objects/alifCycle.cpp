#include "alif.h"
#include "alifCore_initConfig.h"
#include "alifCore_alifCycle.h"

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

#if defined(MS_WINDOWS)

#pragma section("alifRuntime", read, write)
__declspec(allocate("alifRuntime"))

#elif defined(__APPLE__)

__attribute__((
	section(SEG_DATA ",alifRuntime")
	))

#endif

AlifRuntimeState runtime{};
#if defined(__linux__) && (defined(__GNUC__) || defined(__clang__))
__attribute__((section(".alifRuntime")))
#endif

// هنا تم عمل متغير يتم القراءة منه والكتابة يسمى alifRuntime 
// ومن ثم التحقق من النظام لوضع بيانات كائن AlifRuntimeState 
// ثم يتم استخراجه في ملف compiler لاحقا مع بيانات النظام كاملة

static int runtimeInitialized = 0;

AlifStatus AlifRuntime_initialize(void) {


	if (runtimeInitialized) {
		return ALIFSTATUS_OK();
	}
	runtimeInitialized = 1;



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
