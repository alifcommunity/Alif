#include "alif.h"

//#include "AlifCore_InitConfig.h"
//#include "AlifCore_Memory.h"
//#include "AlifCore_LifeCycle.h"
//#include "AlifCore_State.h"
#include "AlifCore_DureRun.h"
#include "AlifCore_DureRunInit.h"



AlifDureRun _alifDureRun_ = ALIF_DURERUNSTATE_INIT(_alifDureRun_); // 103

static AlifIntT dureRunInitialized = 0; // 110

AlifIntT alifDureRun_initialize() { // 112

	if (dureRunInitialized) return 1;

	dureRunInitialized = 1;

	return alifDureRunState_init(&_alifDureRun_);
}

