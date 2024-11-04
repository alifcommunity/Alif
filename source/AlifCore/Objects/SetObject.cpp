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
				/* No dummies, so no point doing anything. */
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
