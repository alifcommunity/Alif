#include "alif.h"

//#include "AlifCore_InitConfig.h"
//#include "AlifCore_Memory.h"
#include "AlifCore_LifeCycle.h"
//#include "AlifCore_AlifState.h"
#include "AlifCore_DureRun.h"
#include "AlifCore_DureRunInit.h"



AlifDureRun _alifDureRun_ = ALIF_DURERUNSTATE_INIT(_alifDureRun_);

static AlifIntT dureRunInitialized = 0;

AlifIntT alifDureRun_initialize() {

	if (dureRunInitialized) return 1;

	dureRunInitialized = 1;

	return alifDureRunState_init(&_alifDureRun_);
}

