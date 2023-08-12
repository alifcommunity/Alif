#include "alif.h"
#include "alifCore_initConfig.h"
#include "alifCore_alifMem.h"
#include "alifCore_alifState.h"

#undef ALIF_IS

int alif_is(AlifObject* x, AlifObject* y)
{
	return (x == y);
}
