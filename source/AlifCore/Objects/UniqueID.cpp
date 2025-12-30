#include "alif.h"

#include "AlifCore_Dict.h"
#include "AlifCore_Lock.h"
#include "AlifCore_State.h"
#include "AlifCore_Object.h"
#include "AlifCore_UniqueID.h"



#define POOL_MIN_SIZE 8 // 15

// 17
#define LOCK_POOL(_pool) alifMutex_lockFlags(&_pool->mutex, AlifLockFlags_::Alif_Lock_Dont_Detach)
#define UNLOCK_POOL(_pool) ALIFMUTEX_UNLOCK(&_pool->mutex)


static AlifIntT resizeInterp_typeIDPool(AlifUniqueIDPool* pool) { // 20
	if ((size_t)pool->size > ALIF_SIZET_MAX / (2 * sizeof(*pool->table))) {
		return -1;
	}

	AlifSizeT new_size = pool->size * 2;
	if (new_size < POOL_MIN_SIZE) {
		new_size = POOL_MIN_SIZE;
	}

	AlifUniqueIDEntry* table = (AlifUniqueIDEntry*)alifMem_dataRealloc(pool->table, new_size * sizeof(*pool->table));

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
	if (_thread->refCounts.isFinalized) return -1;

	AlifUniqueIDPool* pool = &_thread->base.interpreter->uniqueIDs;
	AlifSizeT size = alifAtomic_loadSize(&pool->size);

	AlifSizeT* refCnts = (AlifSizeT*)alifMem_dataRealloc(_thread->refCounts.vals, size * sizeof(AlifSizeT));
	if (refCnts == nullptr)	return -1;

	AlifSizeT oldSize = _thread->refCounts.size;
	if (oldSize < size) {
		memset(refCnts + oldSize, 0, (size - oldSize) * sizeof(AlifSizeT));
	}

	_thread->refCounts.vals = refCnts;
	_thread->refCounts.size = size;
	return 0;
}



AlifSizeT _alifObject_assignUniqueId(AlifObject* _obj) { // 76
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifUniqueIDPool* pool = &interp->uniqueIDs;

	LOCK_POOL(pool);
	if (pool->freelist == nullptr) {
		if (resizeInterp_typeIDPool(pool) < 0) {
			UNLOCK_POOL(pool);
			return -1;
		}
	}

	AlifUniqueIDEntry* entry = pool->freelist;
	pool->freelist = entry->next;
	entry->obj = _obj;
	alifObject_setDeferredRefcount(_obj);
	AlifSizeT unicodeID = (entry - pool->table);
	UNLOCK_POOL(pool);
	return unicodeID;
}





void _alifObject_releaseUniqueId(AlifSizeT _uniqueID) { // 102
	AlifInterpreter* interp = _alifInterpreter_get();
	AlifUniqueIDPool* pool = &interp->uniqueIDs;

	LOCK_POOL(pool);
	AlifUniqueIDEntry* entry = &pool->table[_uniqueID];
	entry->next = pool->freelist;
	pool->freelist = entry;

	UNLOCK_POOL(pool);
}


static AlifSizeT clear_uniqueID(AlifObject* _obj) { // 115
	AlifSizeT id = -1;
	if (ALIFTYPE_CHECK(_obj)) {
		if (alifType_hasFeature((AlifTypeObject*)_obj, ALIF_TPFLAGS_HEAPTYPE)) {
			AlifHeapTypeObject* ht = (AlifHeapTypeObject*)_obj;
			id = ht->uniqueID;
			ht->uniqueID = -1;
		}
	}
	else if (ALIFCODE_CHECK(_obj)) {
		AlifCodeObject* co = (AlifCodeObject*)_obj;
		id = co->uniqueID;
		co->uniqueID = -1;
	}
	else if (ALIFDICT_CHECK(_obj)) {
		AlifDictObject* mp = (AlifDictObject*)_obj;
		id = _alifDict_uniqueId(mp);
		mp->watcherTag &= ~(UINT64_MAX << DICT_UNIQUE_ID_SHIFT);
	}
	return id;
}


void _alifObject_disablePerThreadRefcounting(AlifObject* _obj) { // 134
	AlifSizeT id = clear_uniqueID(_obj);
	if (id >= 0) {
		_alifObject_releaseUniqueId(id);
	}
}



void _alifObject_threadIncrefSlow(AlifObject* _obj, AlifSizeT _uniqueID) { // 143
	AlifThreadImpl* thread = (AlifThreadImpl*)_alifThread_get();
	if (_uniqueID < 0 or resize_localRefCounts(thread) < 0) {
		ALIF_INCREF(_obj);
		return;
	}
	thread->refCounts.vals[_uniqueID]++;
}
