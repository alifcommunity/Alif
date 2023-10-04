#pragma once









#include "alifCore_runtime.h"

/* Forward declarations */
class AlifArgv;















extern void alif_initVersion();





extern void alifSys_readPreInitWarnOptions(AlifWideStringList*);
extern void alifSys_readPreInitXOptions(AlifConfig*);



//extern AlifStatus alif_hashRandomizationInit(const AlifConfig*);

//extern AlifStatus alifTime_init();






















extern void alifGILState_init(AlifInterpreterState*);





extern void alif_preInitializeFromAlifArgv(const AlifPreConfig*, const AlifArgv*);


extern void alif_preInitializeFromConfig(const AlifConfig*,const AlifArgv*);

































extern int alif_coerceLegacyLocale(int);

ALIFAPI_FUNC(char*) alif_setLocaleFromEnv(int);
