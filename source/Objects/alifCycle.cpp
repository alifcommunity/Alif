#include "alif.h"
#include "alifCore_initConfig.h"

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


AlifStatus alifInit_fromConfig(const AlifConfig* config)
{
	AlifStatus status{};

	status = alifInit_core(config);

	return ALIFSTATUS_OK();
}
