#pragma once









#include "alifCore_runtime.h"

/* Forward declarations */
class AlifArgv;















extern void alif_initVersion();





extern AlifStatus alifSys_readPreInitWarnOptions(AlifWideStringList*);
extern AlifStatus alifSys_readPreInitXOptions(AlifConfig*);



//extern AlifStatus alif_hashRandomizationInit(const AlifConfig*);

//extern AlifStatus alifTime_init();






















extern AlifStatus alifGILState_init(AlifInterpreterState*);





extern AlifStatus alif_preInitializeFromAlifArgv(const AlifPreConfig*, const AlifArgv*);


extern AlifStatus alif_preInitializeFromConfig(const AlifConfig*,const AlifArgv*);

































extern int alif_coerceLegacyLocale(int);

ALIFAPI_FUNC(char*) alif_setLocaleFromEnv(int);
