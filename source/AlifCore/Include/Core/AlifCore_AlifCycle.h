#pragma once

#include "AlifCore_DureRun.h"


extern AlifObject* alifBuiltin_init(AlifInterpreter*);
int alifSys_create(AlifThread* , AlifObject** );

AlifIntT alifSubGC_init(AlifInterpreter*);



/* -------------------------------- تعريفات لمرة واحدة ------------------------------- */



AlifIntT alif_initFromConfig(AlifConfig*);

char* alif_setLocale(AlifIntT);
