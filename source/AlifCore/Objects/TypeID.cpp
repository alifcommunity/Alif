#include "alif.h"

#include "AlifCore_State.h"
#include "AlifCore_Object.h"
#include "AlifCore_TypeID.h"









static AlifIntT resize_localRefCounts(AlifThreadImpl* _thread) { // 50
	if (_thread->types.isFinalized) return -1;

	AlifTypeIDPool* pool = &_thread->base.interpreter->typeIDs;
	AlifSizeT size = alifAtomic_loadSize(&pool->size);

	AlifSizeT* refCnts = (AlifSizeT*)alifMem_dataRealloc(_thread->types.refCounts, size * sizeof(AlifSizeT));
	if (refCnts == nullptr)	return -1;

	AlifSizeT oldSize = _thread->types.size;
	if (oldSize < size) {
		memset(refCnts + oldSize, 0, (size - oldSize) * sizeof(AlifSizeT));
	}

	_thread->types.refCounts = refCnts;
	_thread->types.size = size;
	return 0;
}




void alifType_releaseID(AlifHeapTypeObject* type) { // 99
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifTypeIDPool* pool = &interp->typeIDs;

	if (type->uniqueID < 0) {
		return;
	}

	//LOCK_POOL(pool);
	AlifTypeIDEntry* entry = &pool->table[type->uniqueID];
	entry->next = pool->freelist;
	pool->freelist = entry;

	type->uniqueID = -1;
	//UNLOCK_POOL(pool);
}




void alifType_incRefSlow(AlifHeapTypeObject* type) { // 120
	AlifThreadImpl* thread = (AlifThreadImpl*)_alifThread_get();
	if (type->uniqueID < 0 or resize_localRefCounts(thread) < 0) {
		// just incref the type directly.
		ALIF_INCREF(type);
		return;
	}
	thread->types.refCounts[type->uniqueID]++;
}
