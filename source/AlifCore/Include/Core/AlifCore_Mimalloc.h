#pragma once

















enum AlifMimallocHeapID_ { // 12
	Alif_Mimalloc_Heap_Mem = 0,
	Alif_Mimalloc_Heap_Object = 1,
	Alif_Mimalloc_Heap_GC = 2,
	Alif_Mimalloc_Heap_GCPre = 3,
	Alif_Mimalloc_Heap_Count
};



#include "AlifCore_Memory.h"



#include "mimalloc/mimalloc.h"
#include "mimalloc/mimalloc/types.h"
#include "mimalloc/mimalloc/internal.h"








#ifdef ALIF_GIL_DISABLED
class MimallocInterpState { // 54
public:
	mi_abandoned_pool_t abandonedPool{};
};

class MimallocThreadState {
public:
	mi_heap_t* currentObjectHeap{};
	mi_heap_t heaps[AlifMimallocHeapID_::Alif_Mimalloc_Heap_Count];
	mi_tld_t tld{};
	AlifIntT initialized{};
	LListNode pageList{};
};
#endif







/* ----------------------------------------- AlifCore_Memory ------------------------------------------------- */

extern void alifMem_processDelayed(AlifThread*); // 126
