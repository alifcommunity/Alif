#include "alif.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Memory.h"
#include "AlifCore_AlifCycle.h"

#ifdef _WINDOWS
#include <windows.h>
#endif


AlifRuntime alifRuntime{};


/*
	هذا الملف ليس له عمل في الوقت الحالي ولكن سيتم إستخدامه في المستقبل
*/


void alif_initFromConfig(AlifConfig* _config) {

	alifConfig_read(_config);
}
