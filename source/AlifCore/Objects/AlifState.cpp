#include "alif.h"

//#include "AlifCore_AlifEval.h"
//#include "AlifCore_Code.h"
//#include "AlifCore_Frame.h"
//#include "AlifCore_InitConfig.h"
//#include "AlifObject.h"
#include "AlifCore_LifeCycle.h"
//#include "AlifCore_AlifState.h"
#include "AlifCore_DureRunInit.h"



#ifdef HAVE_LOCAL_THREAD
	ALIF_LOCAL_THREAD AlifThread* _alifTSSThread_ = nullptr; // 68
#endif



//------------------------------------------------
// the thread state bound to the current OS thread
//------------------------------------------------


static inline AlifIntT threadTSS_Init(AlifTssT* _key) { // 127
	return alifThreadTSS_create(_key);
}




/* ---------------------------------------- AlifCycle ---------------------------------------- */


static const AlifDureRun initial = ALIF_DURERUNSTATE_INIT(_alifDureRun_); // 393



static void init_dureRun(AlifDureRun* _dureRun) { // 411

	_dureRun->mainThreadID = alifThread_getThreadID();

	_dureRun->selfInitialized = 1;
}

AlifIntT alifDureRunState_init(AlifDureRun* _dureRun) { // 441

	if (_dureRun->selfInitialized) {
		memcpy(_dureRun, &initial, sizeof(*_dureRun));
	}

	if (threadTSS_Init(&_dureRun->autoTSSKey) != 0) {
		alifDureRun_fini(_dureRun);
		return -1;
	}
	//if (alifThread_tssCreate(&_dureRun->trashTSSKey) != 0) {
	//	alifDureRun_fini(_dureRun);
	//	return -1;
	//}

	init_dureRun(_dureRun);

	return 1;
}


