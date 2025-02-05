#include "alif.h"
#include "AlifCore_ObjectAlloc.h"
#include "AlifCore_Dict.h"
#include "AlifCore_SetObject.h"

#include "clinic/SetObject.cpp.h"

// was static but shows error
extern AlifObject _dummyStruct_; // 59 //* alif

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
			if (entry->hash == 0 and entry->key == nullptr)
				return entry;
			if (entry->hash == _hash) {
				AlifObject* startKey = entry->key;
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
				if (table != _so->table or entry->key != startKey)
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
			if (entry->hash == 0 and entry->key == nullptr)
				goto foundUnusedOrDummy;
			if (entry->hash == _hash) {
				AlifObject* startKey = entry->key;
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
				if (table != _so->table or entry->key != startKey)
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
	freesLot->key = _key;
	freesLot->hash = _hash;
	return 0;

foundUnused:
	_so->fill++;
	alifAtomic_storeSizeRelaxed(&_so->used, _so->used + 1);
	entry->key = _key;
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

static void set_insertClean(SetEntry* _table, AlifUSizeT _mask,
	AlifObject* _key, AlifHashT _hash) { // 218
	SetEntry* entry{};
	AlifUSizeT perturb = _hash;
	AlifUSizeT i_ = (AlifUSizeT)_hash & _mask;
	AlifUSizeT j_{};

	while (1) {
		entry = &_table[i_];
		if (entry->key == nullptr)
			goto foundNull;
		if (i_ + LINEAR_PROBES <= _mask) {
			for (j_ = 0; j_ < LINEAR_PROBES; j_++) {
				entry++;
				if (entry->key == nullptr)
					goto foundNull;
			}
		}
		perturb >>= PERTURB_SHIFT;
		i_ = (i_ * 5 + 1 + perturb) & _mask;
	}
foundNull:
	entry->key = _key;
	entry->hash = _hash;
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
			if (entry->key != nullptr) {
				set_insertClean(newTable, newMask, entry->key, entry->hash);
			}
		}
	}
	else {
		_so->fill = _so->used;
		for (entry = oldTable; entry <= oldTable + oldMask; entry++) {
			if (entry->key != nullptr and entry->key != DUMMY) {
				set_insertClean(newTable, newMask, entry->key, entry->hash);
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
		return entry->key != nullptr;
	return -1;
}

#define DISCARD_NOTFOUND 0
#define DISCARD_FOUND 1

static AlifIntT set_discardEntry(AlifSetObject* _so, AlifObject* _key, AlifHashT _hash) { // 347
	SetEntry* entry{};
	AlifObject* oldKey{};

	entry = set_lookKey(_so, _key, _hash);
	if (entry == nullptr)
		return -1;
	if (entry->key == nullptr)
		return DISCARD_NOTFOUND;
	oldKey = entry->key;
	entry->key = DUMMY;
	entry->hash = -1;
	alifAtomic_storeSizeRelaxed(&_so->used, _so->used - 1);
	ALIF_DECREF(oldKey);
	return DISCARD_FOUND;
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

static AlifIntT set_discardKey(AlifSetObject* _so, AlifObject* _key) { // 386
	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		return -1;
	}
	return set_discardEntry(_so, _key, hash);
}



static AlifIntT set_next(AlifSetObject* so,
	AlifSizeT* pos_ptr, SetEntry** entry_ptr) { // 468
	AlifSizeT i{};
	AlifSizeT mask{};
	SetEntry* entry{};

	i = *pos_ptr;
	mask = so->mask;
	entry = &so->table[i];
	while (i <= mask and (entry->key == nullptr or entry->key == DUMMY)) {
		i++;
		entry++;
	}
	*pos_ptr = i + 1;
	if (i > mask) return 0;
	*entry_ptr = entry;
	return 1;
}

static void set_dealloc(AlifObject* _self) { // 492
	AlifSetObject* so = ALIFSET_CAST(_self);
	SetEntry* entry{};
	AlifSizeT used = so->used;

	alifObject_gcUnTrack(so);
	ALIF_TRASHCAN_BEGIN(so, set_dealloc);
	if(so->weakRefList != nullptr) alifObject_clearWeakRefs((AlifObject*)so);

	for (entry = so->table; used > 0; entry++) {
		if (entry->key and entry->key != DUMMY) {
			used--;
			ALIF_DECREF(entry->key);
		}
	}
	if (so->table != so->smallTable) alifMem_dataFree(so->table);
	ALIF_TYPE(so)->free(so);
	ALIF_TRASHCAN_END;
}

static AlifSizeT set_len(AlifObject* _self) { // 571
	AlifSetObject* so = ALIFSET_CAST(_self);
	return alifAtomic_loadSizeRelaxed(&so->used);
}

static AlifIntT setMerge_lockHeld(AlifSetObject* _so, AlifObject* _otherSet) { // 578
	AlifSetObject* other{};
	AlifObject* key{};
	AlifSizeT i_{};
	SetEntry* soEntry{};
	SetEntry* otherEntry{};

	other = ALIFSET_CAST(_otherSet);
	if (other == _so or other->used == 0)
		return 0;
	if ((_so->fill + other->used) * 5 >= _so->mask * 3) {
		if (set_tableResize(_so, (_so->used + other->used) * 2) != 0)
			return -1;
	}
	soEntry = _so->table;
	otherEntry = other->table;

	if (_so->fill == 0 and _so->mask == other->mask and other->fill == other->used) {
		for (i_ = 0; i_ <= other->mask; i_++, soEntry++, otherEntry++) {
			key = otherEntry->key;
			if (key != nullptr) {
				soEntry->key = ALIF_NEWREF(key);
				soEntry->hash = otherEntry->hash;
			}
		}
		_so->fill = other->fill;
		alifAtomic_storeSizeRelaxed(&_so->used, other->used);
		return 0;
	}

	if (_so->fill == 0) {
		SetEntry* newtable = _so->table;
		AlifUSizeT newMask = (AlifUSizeT)_so->mask;
		_so->fill = other->used;
		alifAtomic_storeSizeRelaxed(&_so->used, other->used);
		for (i_ = other->mask + 1; i_ > 0; i_--, otherEntry++) {
			key = otherEntry->key;
			if (key != nullptr and key != DUMMY) {
				set_insertClean(newtable, newMask, ALIF_NEWREF(key),
					otherEntry->hash);
			}
		}
		return 0;
	}

	for (i_ = 0; i_ <= other->mask; i_++) {
		otherEntry = &other->table[i_];
		key = otherEntry->key;
		if (key != nullptr and key != DUMMY) {
			if (set_addEntry(_so, key, otherEntry->hash))
				return -1;
		}
	}
	return 0;
}



static AlifObject* set_popImpl(AlifSetObject* so) { // 660
	SetEntry* entry = so->table + (so->finger & so->mask);
	SetEntry* limit = so->table + so->mask;
	AlifObject* key{};

	if (so->used == 0) {
		//alifErr_setString(_alifExcKeyError_, "pop from an empty set");
		return nullptr;
	}
	while (entry->key == nullptr or entry->key == DUMMY) {
		entry++;
		if (entry > limit)
			entry = so->table;
	}
	key = entry->key;
	entry->key = DUMMY;
	entry->hash = -1;
	alifAtomic_storeSizeRelaxed(&so->used, so->used - 1);
	so->finger = entry - so->table + 1;
	return key;
}




class SetIterObject { // 778
public:
	ALIFOBJECT_HEAD;
	AlifSetObject* set{};
	AlifSizeT used{};
	AlifSizeT pos{};
	AlifSizeT len{};
};

static void setIter_dealloc(AlifObject* _self) { // 786
	SetIterObject* si = (SetIterObject*)_self;
	ALIFOBJECT_GC_UNTRACK(si);
	ALIF_XDECREF(si->set);
	alifObject_gcDel(si);
}


static AlifObject* setIter_iterNext(AlifObject* _self) { // 839
	SetIterObject* si = (SetIterObject*)_self;
	AlifObject* key = nullptr;
	AlifSizeT i{}, mask{};
	SetEntry* entry{};
	AlifSetObject* so = si->set;

	if (so == nullptr)
		return nullptr;

	AlifSizeT soUsed = alifAtomic_loadSize(&so->used);
	AlifSizeT siUsed = alifAtomic_loadSize(&si->used);
	if (siUsed != soUsed) {
		//alifErr_setString(_alifExcRuntimeError_,
		//	"Set changed size during iteration");
		si->used = -1;
		return nullptr;
	}

	ALIF_BEGIN_CRITICAL_SECTION(so);
	i = si->pos;
	entry = so->table;
	mask = so->mask;
	while (i <= mask and (entry[i].key == nullptr or entry[i].key == DUMMY)) {
		i++;
	}
	if (i <= mask) {
		key = ALIF_NEWREF(entry[i].key);
	}
	ALIF_END_CRITICAL_SECTION();
	si->pos = i + 1;
	if (key == nullptr) {
		si->set = nullptr;
		ALIF_DECREF(so);
		return nullptr;
	}
	si->len--;
	return key;
}

AlifTypeObject _alifSetIterType_ = { // 881
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مميزة_تكرار",
	.basicSize = sizeof(SetIterObject),
	.itemSize = 0,
	/* methods */
	.dealloc = setIter_dealloc,
	.vectorCallOffset = 0,
	.getAttr = 0,
	.setAttr = 0,
	.repr = 0,
	.asNumber = 0,
	.asSequence = 0,
	.asMapping = 0,
	.hash = 0,
	.getAttro = alifObject_genericGetAttr,
	.setAttro = 0,
	.asBuffer = 0,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC,
	//.traverse = setIter_traverse,
	.richCompare = 0,
	.weakListOffset = 0,
	.iter = alifObject_selfIter,
	.iterNext = setIter_iterNext,
};


static AlifObject* set_iter(AlifObject* _so) { // 914
	AlifSizeT size = set_len(_so);
	SetIterObject* si = ALIFOBJECT_GC_NEW(SetIterObject, &_alifSetIterType_);
	if (si == nullptr) return nullptr;
	si->set = (AlifSetObject*)ALIF_NEWREF(_so);
	si->used = size;
	si->pos = 0;
	si->len = size;
	ALIFOBJECT_GC_TRACK(si);
	return (AlifObject*)si;
}

static AlifIntT setUpdateDict_lockHeld(AlifSetObject* _so, AlifObject* _other) { // 930
	AlifSizeT dictSize = ALIFDICT_GET_SIZE(_other);
	if ((_so->fill + dictSize) * 5 >= _so->mask * 3) {
		if (set_tableResize(_so, (_so->used + dictSize) * 2) != 0) {
			return -1;
		}
	}

	AlifSizeT pos = 0;
	AlifObject* key{};
	AlifObject* value{};
	AlifHashT hash{};
	while (_alifDict_next(_other, &pos, &key, &value, &hash)) {
		if (set_addEntry(_so, key, hash)) {
			return -1;
		}
	}
	return 0;
}

static AlifIntT setUpdateIterable_lockHeld(AlifSetObject *_so, AlifObject *_other) { // 961

	AlifObject* it_ = alifObject_getIter(_other);
    if (it_ == nullptr) {
        return -1;
    }

	AlifObject* key_{};
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

// set_update for a `_so` that is only visible to the current thread
static AlifIntT set_updateLocal(AlifSetObject *_so, AlifObject *_other) { // 999
    if (ALIFANYSET_CHECK(_other)) {
		AlifIntT rv_{};
        ALIF_BEGIN_CRITICAL_SECTION(_other);
        rv_ = setMerge_lockHeld(_so, _other);
        ALIF_END_CRITICAL_SECTION();
        return rv_;
    }
    else if (ALIFDICT_CHECKEXACT(_other)) {
		AlifIntT rv_{};
        ALIF_BEGIN_CRITICAL_SECTION(_other);
        rv_ = setUpdateDict_lockHeld(_so, _other);
        ALIF_END_CRITICAL_SECTION();
        return rv_;
    }
    return setUpdateIterable_lockHeld(_so, _other);
}


static AlifIntT set_updateInternal(AlifSetObject* _so, AlifObject* _other) { // 1019
	if (ALIFANYSET_CHECK(_other)) {
		if (ALIF_IS((AlifObject*)_so, _other)) {
			return 0;
		}
		AlifIntT rv{};
		ALIF_BEGIN_CRITICAL_SECTION2(_so, _other);
		rv = setMerge_lockHeld(_so, _other);
		ALIF_END_CRITICAL_SECTION2();
		return rv;
	}
	else if (ALIFDICT_CHECKEXACT(_other)) {
		AlifIntT rv{};
		ALIF_BEGIN_CRITICAL_SECTION2(_so, _other);
		rv = setUpdateDict_lockHeld(_so, _other);
		ALIF_END_CRITICAL_SECTION2();
		return rv;
	}
	else {
		AlifIntT rv{};
		ALIF_BEGIN_CRITICAL_SECTION(_so);
		rv = setUpdateIterable_lockHeld(_so, _other);
		ALIF_END_CRITICAL_SECTION();
		return rv;
	}
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
	so_->weakRefList = nullptr;

	if (_iterable != nullptr) {
		if (set_updateLocal(so_, _iterable)) {
			ALIF_DECREF(so_);
			return nullptr;
		}
	}

	return (AlifObject*)so_;
}

static AlifObject* set_new(AlifTypeObject* _type,
	AlifObject* _args, AlifObject* _kwds) { // 1166
	return make_newSet(_type, nullptr);
}


static AlifObject* set_ior(AlifObject* _self, AlifObject* _other) { // 1331
	if (!ALIFANYSET_CHECK(_other))
		return ALIF_NOTIMPLEMENTED;

	AlifSetObject* so = ALIFSET_CAST(_self);
	if (set_updateInternal(so, _other)) {
		return nullptr;
	}
	return ALIF_NEWREF(so);
}



static AlifNumberMethods _asNumber_ = { // 2411
	.add_ = 0,
	//.subtract = set_sub,
	//0,                                  /*multiply*/
	//0,                                  /*remainder*/
	//0,                                  /*divmod*/
	//0,                                  /*power*/
	//0,                                  /*negative*/
	//0,                                  /*positive*/
	//0,                                  /*absolute*/
	//0,                                  /*bool*/
	//0,                                  /*invert*/
	//0,                                  /*lshift*/
	//0,                                  /*rshift*/
	//.and_ = set_and,
	//.xor_ = set_xor,
	//.or_ = set_or,
	//0,                                  /*int*/
	//0,                                  /*reserved*/
	//0,                                  /*float*/
	//0,                                  /*inplace_add*/
	//.inplaceSubtract = set_isub,
	//0,                                  /*inplace_multiply*/
	//0,                                  /*inplace_remainder*/
	//0,                                  /*inplace_power*/
	//0,                                  /*inplace_lshift*/
	//0,                                  /*inplace_rshift*/
	//.inplaceAnd = set_iand,
	//.inplaceXor = set_ixor,
	.inplaceOr = set_ior,
};


AlifTypeObject _alifSetType_ = { // 2449
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مميزة",
	.basicSize = sizeof(AlifSetObject),
	.itemSize = 0,
	.dealloc = set_dealloc,

	.asNumber = &_asNumber_,


	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
		ALIF_TPFLAGS_BASETYPE | _ALIF_TPFLAGS_MATCH_SELF,


	.iter = set_iter,

	.alloc = alifType_genericAlloc,
	.new_ = set_new,
	.free = alifObject_gcDel,
};



AlifTypeObject _alifFrozenSetType_ = { // 2539
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "مميزة_مجمدة",
	.basicSize = sizeof(AlifSetObject),
	.itemSize = 0,
};


AlifObject* alifSet_new(AlifObject* _iterable) { // 2588
	return make_newSet(&_alifSetType_, _iterable);
}

AlifObject* alifFrozenSet_new(AlifObject* _iterable) { // 2594
	return make_newSet(&_alifFrozenSetType_, _iterable);
}



AlifSizeT alifSet_size(AlifObject* _anyset) { // 2600
	if (!ALIFANYSET_CHECK(_anyset)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	return set_len(_anyset);
}


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

AlifIntT alifSet_discard(AlifObject* _set, AlifObject* _key) { // 2643
 	if (!ALIFSET_CHECK(_set)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	AlifIntT rv_{};
	ALIF_BEGIN_CRITICAL_SECTION(_set);
	rv_ = set_discardKey((AlifSetObject*)_set, _key);
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

AlifIntT alifSet_nextEntry(AlifObject* _set,
	AlifSizeT* _pos, AlifObject** _key, AlifHashT* _hash) { // 2674
	SetEntry* entry{};

	if (!ALIFANYSET_CHECK(_set)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	if (set_next((AlifSetObject*)_set, _pos, &entry) == 0)
		return 0;
	*_key = entry->key;
	*_hash = entry->hash;
	return 1;
}


AlifIntT _alifSet_nextEntryRef(AlifObject* _set,
	AlifSizeT* _pos, AlifObject** _key, AlifHashT* _hash) { // 2689
	SetEntry* entry{};

	if (!ALIFANYSET_CHECK(_set)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	if (set_next((AlifSetObject*)_set, _pos, &entry) == 0)
		return 0;
	*_key = ALIF_NEWREF(entry->key);
	*_hash = entry->hash;
	return 1;
}


AlifObject* alifSet_pop(AlifObject* _set) { // 2706
	if (!ALIFSET_CHECK(_set)) {
		//ALIFERR_BADINTERNALCALL();
		return nullptr;
	}
	return set_pop((AlifSetObject*)_set, nullptr);
}



static AlifTypeObject _alifSetDummyType_ = { // 2743
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "<مفتاح_وهمي> نوع",
	.basicSize = 0,
	.itemSize = 0,
	.flags = ALIF_TPFLAGS_DEFAULT, /*tp_flags */
};

AlifObject _dummyStruct_ = ALIFOBJECT_HEAD_INIT(&_alifSetDummyType_); // 2766
