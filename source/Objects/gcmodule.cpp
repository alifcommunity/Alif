
























#include "alif.h"
//#include "alifCore_context.h"
//#include "alifCore_dict.h"         
#include "alifCore_initConfig.h"
#include "alifCore_interp.h"        
//#include "alifCore_object.h"
//#include "alifCore_alifErrors.h"
#include "alifCore_alifState.h"       
//#include "alifCore_weakRef.h"       
//#include "alifDTrace.h"

typedef class GCRuntimeState GCState;


























































































#define GEN_HEAD(_gcState, n) (&(_gcState)->generations[n].head)










void alifGC_initState(GCState* _gcState)
{
#define INIT_HEAD(GEN) \
    do { \
        GEN.head.gCNext = (uintptr_t)&GEN.head; \
        GEN.head.gCPrev = (uintptr_t)&GEN.head; \
    } while (0)

	for (int i = 0; i < NUM_GENERATIONS; i++) {
		//assert(gcstate->generations[i].count == 0);
		INIT_HEAD(_gcState->generations[i]);
	};
	_gcState->generation0 = GEN_HEAD(_gcState, 0);
	INIT_HEAD(_gcState->permanentGeneration);

#undef INIT_HEAD
}
