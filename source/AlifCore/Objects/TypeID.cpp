#include "alif.h"

#include "AlifCore_Lock.h"
#include "AlifCore_State.h"
#include "AlifCore_Object.h"
#include "AlifCore_TypeID.h"



#define POOL_MIN_SIZE 8 // 15

 // 17
#define LOCK_POOL(_pool) alifMutex_lockFlags(&_pool->mutex, AlifLockFlags_::Alif_Lock_Dont_Detach)
#define UNLOCK_POOL(_pool) ALIFMUTEX_UNLOCK(&_pool->mutex)


static AlifIntT resizeInterp_typeIDPool(AlifTypeIDPool* pool) { // 20
	if ((size_t)pool->size > ALIF_SIZET_MAX / (2 * sizeof(*pool->table))) {
		return -1;
	}

	AlifSizeT new_size = pool->size * 2;
	if (new_size < POOL_MIN_SIZE) {
		new_size = POOL_MIN_SIZE;
	}

	AlifTypeIDEntry* table = (AlifTypeIDEntry*)alifMem_dataRealloc(pool->table, new_size * sizeof(*pool->table));

	if (table == nullptr) {
		return -1;
	}

	AlifSizeT start = pool->size;
	for (AlifSizeT i = start; i < new_size - 1; i++) {
		table[i].next = &table[i + 1];
	}
	table[new_size - 1].next = nullptr;

	pool->table = table;
	pool->freelist = &table[start];
	alifAtomic_storeSize(&pool->size, new_size);
	return 0;
}



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



void _alifType_assignId(AlifHeapTypeObject* type) { // 76
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifTypeIDPool* pool = &interp->typeIDs;

	LOCK_POOL(pool);
	if (pool->freelist == nullptr) {
		if (resizeInterp_typeIDPool(pool) < 0) {
			type->uniqueID = -1;
			UNLOCK_POOL(pool);
			return;
		}
	}

	AlifTypeIDEntry* entry = pool->freelist;
	pool->freelist = entry->next;
	entry->type = type;
	alifObject_setDeferredRefcount((AlifObject*)type);
	type->uniqueID = (entry - pool->table);
	UNLOCK_POOL(pool);
}





void alifType_releaseID(AlifHeapTypeObject* type) { // 99
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifTypeIDPool* pool = &interp->typeIDs;

	if (type->uniqueID < 0) {
		return;
	}

	LOCK_POOL(pool);
	AlifTypeIDEntry* entry = &pool->table[type->uniqueID];
	entry->next = pool->freelist;
	pool->freelist = entry;

	type->uniqueID = -1;
	UNLOCK_POOL(pool);
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
