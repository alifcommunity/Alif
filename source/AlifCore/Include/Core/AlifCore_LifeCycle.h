#pragma once

#include "AlifCore_DureRun.h"


extern AlifObject* alifBuiltin_init(AlifInterpreter*); // 32
extern AlifIntT alifSys_create(AlifThread*, AlifObject**); // 33


extern AlifIntT _alifSys_updateConfig(AlifThread*); // 38




extern AlifIntT _alifSignal_init(AlifIntT _installSignalHandlers); // 48

extern AlifIntT alifGILState_init(AlifInterpreter*); // 63






char* alif_setLocale(AlifIntT); // 112

