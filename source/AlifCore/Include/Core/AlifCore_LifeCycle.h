#pragma once

#include "AlifCore_Runtime.h"


extern AlifObject* alifBuiltin_init(AlifInterpreter*); // 32
extern AlifStatus alifSys_create(AlifThread*, AlifObject**); // 33


extern AlifIntT _alifSys_updateConfig(AlifThread*); // 38




extern AlifIntT _alifSignal_init(AlifIntT _installSignalHandlers); // 48

extern AlifStatus alifGILState_init(AlifInterpreter*); // 63

extern AlifStatus _alif_preInitializeFromAlifArgv(const AlifPreConfig*, const class AlifArgv*); // 69




char* alif_setLocale(AlifIntT); // 112

