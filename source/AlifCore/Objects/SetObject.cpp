#include "alif.h"
#include "AlifCore_ObjectAlloc.h"


// was static but shows error
extern AlifObject _dummyStruct_; // 59 // alif

#define DUMMY (&_dummyStruct_) // 61


#define LINEAR_PROBES 9 // 69

#define PERTURB_SHIFT 5 // 73

static SetEntry* set_lookKey(AlifSetObject* _so, AlifObject* _key, AlifHashT _hash) { // 76
	SetEntry* table{};
	SetEntry* entry{};
	AlifUSizeT perturb = _hash;
	AlifUSizeT mask = _so->mask;
	AlifUSizeT i_ = (AlifUSizeT)_hash & mask; /* Unsigned for defined overflow behavior */
	AlifIntT probes{};
	AlifIntT cmp_{};

	while (1) {
		entry = &_so->table[i_];
		probes = (i_ + LINEAR_PROBES <= mask) ? LINEAR_PROBES : 0;
		do {
			if (entry->hash == 0 and entry->key_ == nullptr)
				return entry;
			if (entry->hash == _hash) {
				AlifObject* startKey = entry->key_;
				if (startKey == _key)
					return entry;
				if (ALIFUSTR_CHECKEXACT(startKey)
					and ALIFUSTR_CHECKEXACT(_key)
					and alifUStr_eq(startKey, _key))
					return entry;
				table = _so->table;
				ALIF_INCREF(startKey);
				cmp_ = alifObject_richCompareBool(startKey, _key, ALIF_EQ);
				ALIF_DECREF(startKey);
				if (cmp_ < 0)
					return nullptr;
				if (table != _so->table or entry->key_ != startKey)
					return set_lookKey(_so, _key, _hash);
				if (cmp_ > 0)
					return entry;
				mask = _so->mask;
			}
			entry++;
		} while (probes--);
		perturb >>= PERTURB_SHIFT;
		i_ = (i_ * 5 + 1 + perturb) & mask;
	}
}

static AlifIntT set_tableResize(AlifSetObject*, AlifSizeT); // 120

static AlifIntT set_addEntry(AlifSetObject* _so, AlifObject* _key, AlifHashT _hash) { // 123
	SetEntry* table{};
	SetEntry* freesLot{};
	SetEntry* entry{};
	AlifUSizeT perturb{};
	AlifUSizeT mask{};
	AlifUSizeT i_{};                       /* Unsigned for defined overflow behavior */
	AlifIntT probes{};
	AlifIntT cmp_{};

	ALIF_INCREF(_key);

restart:

	mask = _so->mask;
	i_ = (AlifUSizeT)_hash & mask;
	freesLot = nullptr;
	perturb = _hash;

	while (1) {
		entry = &_so->table[i_];
		probes = (i_ + LINEAR_PROBES <= mask) ? LINEAR_PROBES : 0;
		do {
			if (entry->hash == 0 and entry->key_ == nullptr)
				goto foundUnusedOrDummy;
			if (entry->hash == _hash) {
				AlifObject* startKey = entry->key_;
				if (startKey == _key)
					goto foundActive;
				if (ALIFUSTR_CHECKEXACT(startKey)
					and ALIFUSTR_CHECKEXACT(_key) and alifUStr_eq(startKey, _key) )
					goto foundActive;
				table = _so->table;
				ALIF_INCREF(startKey);
				cmp_ = alifObject_richCompareBool(startKey, _key, ALIF_EQ);
				ALIF_DECREF(startKey);
				if (cmp_ > 0)
					goto foundActive;
				if (cmp_ < 0)
					goto comparisonError;
				if (table != _so->table or entry->key_ != startKey)
					goto restart;
				mask = _so->mask;
			}
			else if (entry->hash == -1) {
				freesLot = entry;
			}
			entry++;
		} while (probes--);
		perturb >>= PERTURB_SHIFT;
		i_ = (i_ * 5 + 1 + perturb) & mask;
	}

foundUnusedOrDummy:
	if (freesLot == nullptr)
		goto foundUnused;
	alifAtomic_storeSizeRelaxed(&_so->used, _so->used + 1);
	freesLot->key_ = _key;
	freesLot->hash = _hash;
	return 0;

foundUnused:
	_so->fill++;
	alifAtomic_storeSizeRelaxed(&_so->used, _so->used + 1);
	entry->key_ = _key;
	entry->hash = _hash;
	if ((AlifUSizeT)_so->fill * 5 < mask * 3)
		return 0;
	return set_tableResize(_so, _so->used > 50000 ? _so->used * 2 : _so->used * 4);

foundActive:
	ALIF_DECREF(_key);
	return 0;

comparisonError:
	ALIF_DECREF(_key);
	return -1;
}


static AlifIntT set_tableResize(AlifSetObject* _so, AlifSizeT _minUsed) { // 253
	SetEntry* oldTable{}, * newTable{}, * entry{};
	AlifSizeT oldMask = _so->mask;
	AlifUSizeT newMask{};
	AlifIntT isOldTableMalloced{};
	SetEntry smallCopy[ALIFSET_MINSIZE];


	/* Find the smallest table size > minused. */
	/* XXX speed-up with intrinsics */
	AlifUSizeT newSize = ALIFSET_MINSIZE;
	while (newSize <= (AlifUSizeT)_minUsed) {
		newSize <<= 1; // The largest possible value is ALIF_SIZET_MAX + 1.
	}

	/* Get space for a new table. */
	oldTable = _so->table;
	isOldTableMalloced = oldTable != _so->smallTable;

	if (newSize == ALIFSET_MINSIZE) {
		/* A large table is shrinking, or we can't get any smaller. */
		newTable = _so->smallTable;
		if (newTable == oldTable) {
			if (_so->fill == _so->used) {
				/* No dummies, so_ no point doing anything. */
				return 0;
			}
			memcpy(smallCopy, oldTable, sizeof(smallCopy));
			oldTable = smallCopy;
		}
	}
	else {
		newTable = (SetEntry*)alifMem_dataAlloc(newSize * sizeof(SetEntry));
		if (newTable == nullptr) {
			//alifErr_noMemory();
			return -1;
		}
	}

	/* Make the set empty, using the new table. */
	memset(newTable, 0, sizeof(SetEntry) * newSize);
	_so->mask = newSize - 1;
	_so->table = newTable;

	/* Copy the data over; this is refcount-neutral for active entries;
	   dummy entries aren't copied over, of course */
	newMask = (AlifUSizeT)_so->mask;
	if (_so->fill == _so->used) {
		for (entry = oldTable; entry <= oldTable + oldMask; entry++) {
			if (entry->key_ != nullptr) {
				set_insertClean(newTable, newMask, entry->key_, entry->hash);
			}
		}
	}
	else {
		_so->fill = _so->used;
		for (entry = oldTable; entry <= oldTable + oldMask; entry++) {
			if (entry->key_ != nullptr and entry->key_ != DUMMY) {
				set_insertClean(newTable, newMask, entry->key_, entry->hash);
			}
		}
	}

	if (isOldTableMalloced) alifMem_dataFree(oldTable);
	return 0;
}

static AlifIntT set_containsEntry(AlifSetObject* _so, AlifObject* _key, AlifHashT _hash) { // 333
	SetEntry* entry{};

	entry = set_lookKey(_so, _key, _hash);
	if (entry != nullptr)
		return entry->key_ != nullptr;
	return -1;
}

static AlifIntT set_addKey(AlifSetObject* _so, AlifObject* _key) { // 366
	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		return -1;
	}
	return set_addEntry(_so, _key, hash);
}

static AlifIntT set_containsKey(AlifSetObject* _so, AlifObject* _key) { // 376
	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		return -1;
	}
	return set_containsEntry(_so, _key, hash);
}

static AlifIntT setUpdateIterable_lockHeld(AlifSetObject *_so, AlifObject *_other) { // 961

    AlifObject *it_ = alifObject_getIter(_other);
    if (it_ == nullptr) {
        return -1;
    }

    AlifObject *key_;
    while ((key_ = alifIter_next(it_)) != nullptr) {
        if (set_addKey(_so, key_)) {
            ALIF_DECREF(it_);
            ALIF_DECREF(key_);
            return -1;
        }
        ALIF_DECREF(key_);
    }
    ALIF_DECREF(it_);
    //if (alifErr_occurred())
        //return -1;
    return 0;
}

static AlifIntT setUpdate_lockHeld(AlifSetObject *_so, AlifObject *_other) { // 986
    if (ALIFANYSET_CHECK(_other)) {
        return setMerge_lockHeld(_so, _other);
    }
    else if (ALIFDICT_CHECKEXACT(_other)) {
        return setUpdateDict_lockHeld(_so, _other);
    }
    return setUpdateIterable_lockHeld(_so, _other);
}

// set_update for a `so` that is only visible to the current thread
static AlifIntT setUpdate_local(AlifSetObject *so, AlifObject *_other) { // 999
    if (ALIFANYSET_CHECK(_other)) {
		AlifIntT rv_{};
        ALIF_BEGIN_CRITICAL_SECTION(_other);
        rv_ = setMerge_lockHeld(so, _other);
        ALIF_END_CRITICAL_SECTION();
        return rv_;
    }
    else if (ALIFDICT_CHECKEXACT(_other)) {
		AlifIntT rv_{};
        ALIF_BEGIN_CRITICAL_SECTION(_other);
        rv_ = setUpdateDict_lockHeld(so, _other);
        ALIF_END_CRITICAL_SECTION();
        return rv_;
    }
    return setUpdateIterable_lockHeld(so, _other);
}

static AlifObject* make_newSet(AlifTypeObject* _type, AlifObject* _iterable) { // 1077
	AlifSetObject* so_{};

	so_ = (AlifSetObject*)_type->alloc(_type, 0);
	if (so_ == nullptr)
		return nullptr;

	so_->fill = 0;
	so_->used = 0;
	so_->mask = ALIFSET_MINSIZE - 1;
	so_->table = so_->smallTable;
	so_->hash = -1;
	so_->finger = 0;
	so_->weakreList = nullptr;

	if (_iterable != nullptr) {
		if (set_update_local(so_, _iterable)) {
			ALIF_DECREF(so_);
			return nullptr;
		}
	}

	return (AlifObject*)so_;
}

AlifTypeObject _alifSetType_ = { // 2449
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مميزة",                           
	.basicSize = sizeof(AlifSetObject),
	.itemSize = 0,
};



AlifTypeObject _alifFrozenSetType_ = { // 2539
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مميزة_مجمدة",
	.basicSize = sizeof(AlifSetObject),
	.itemSize = 0,
};

AlifIntT alifSet_contains(AlifObject* _anySet, AlifObject* _key) { // 2628
	if (!ALIFANYSET_CHECK(_anySet)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	AlifIntT rv_{};
	ALIF_BEGIN_CRITICAL_SECTION(_anySet);
	rv_ = set_containsKey((AlifSetObject*)_anySet, _key);
	ALIF_END_CRITICAL_SECTION();
	return rv_;
}

AlifIntT alifSet_add(AlifObject* _anySet, AlifObject* _key) { // 2658
	if (!ALIFSET_CHECK(_anySet) and
		(!ALIFFROZENSET_CHECK(_anySet) or ALIF_REFCNT(_anySet) != 1)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	AlifIntT rv_{};
	ALIF_BEGIN_CRITICAL_SECTION(_anySet);
	rv_ = set_addKey((AlifSetObject*)_anySet, _key);
	ALIF_END_CRITICAL_SECTION();
	return rv_;
}



static AlifTypeObject _alifSetDummyType_ = { // 2743
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "<مفتاح_وهمي> نوع",
	.basicSize = 0,
	.itemSize = 0,
	.flags = ALIF_TPFLAGS_DEFAULT, /*tp_flags */
};

AlifObject _dummyStruct_ = ALIFOBJECT_HEAD_INIT(&_alifSetDummyType_); // 2766
