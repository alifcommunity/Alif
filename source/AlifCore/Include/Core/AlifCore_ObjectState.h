#pragma once


#include "AlifCore_FreeListState.h"
#include "AlifCore_HashTable.h" 

/* Reference tracer state */
class RefTracerRuntimeState { // 198
public:
	AlifRefTracer tracerFunc{};
	void* tracerData{};
};
