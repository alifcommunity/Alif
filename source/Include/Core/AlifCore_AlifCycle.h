#pragma once

#include "AlifCore_DureRun.h"


int alifSys_create(AlifThread* , AlifObject** );

AlifIntT alifSubGC_init(AlifInterpreter*);



/* -------------------------------- تعريفات لمرة واحدة ------------------------------- */



AlifIntT alif_initFromConfig(AlifConfig*);
