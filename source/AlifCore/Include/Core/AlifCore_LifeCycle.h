#pragma once

#include "AlifCore_Runtime.h"




extern AlifIntT _alif_isLocaleCoercionTarget(const char*); // 26

extern AlifObject* alifBuiltin_init(AlifInterpreter*); // 32
extern AlifStatus alifSys_create(AlifThread*, AlifObject**); // 33


extern AlifIntT _alifSys_updateConfig(AlifThread*); // 38




extern AlifIntT _alifSignal_init(AlifIntT _installSignalHandlers); // 48

extern AlifStatus alifGILState_init(AlifInterpreter*); // 63

extern AlifStatus _alif_preInitializeFromAlifArgv(const AlifPreConfig*, const class AlifArgv*); // 69

extern AlifStatus _alif_preInitializeFromConfig(const AlifConfig*, const AlifArgv*); // 72

extern AlifIntT _alif_coerceLegacyLocale(AlifIntT); // 108
extern AlifIntT _alif_legacyLocaleDetected(AlifIntT);

char* _alif_setLocaleFromEnv(AlifIntT); // 112

