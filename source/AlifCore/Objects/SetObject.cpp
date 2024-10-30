#include "alif.h"
#include "AlifCore_ObjectAlloc.h""


#define LINEAR_PROBES 9 // 69

#define PERTURB_SHIFT 5 // 73

static AlifIntT set_tableResize(AlifSetObject*, AlifSizeT);

static AlifIntT set_addEntry(AlifSetObject* _so, AlifObject* _key, AlifHashT _hash) { // 123
	SetEntry* table;
	SetEntry* freesLot;
	SetEntry* entry;
	size_t perturb;
	size_t mask;
	size_t i_;                       /* Unsigned for defined overflow behavior */
	AlifIntT probes;
	AlifIntT cmp_;

	ALIF_INCREF(_key);

restart:

	mask = _so->mask;
	i_ = (size_t)_hash & mask;
	freesLot = nullptr;
	perturb = _hash;

	while (1) {
		entry = &_so->table[i_];
		probes = (i_ + LINEAR_PROBES <= mask) ? LINEAR_PROBES : 0;
		do {
			if (entry->hash == 0 and entry->key == nullptr)
				goto foundUnusedOrDummy;
			if (entry->hash == _hash) {
				AlifObject* startkey = entry->key;
				if (startkey == _key)
					goto foundActive;
				if (ALIFUSTR_CHECKEXACT(startkey)
					and ALIFUSTR_CHECKEXACT(_key) and alifUStr_eq(startkey, _key) )
					goto foundActive;
				table = _so->table;
				ALIF_INCREF(startkey);
				cmp_ = alifObject_richCompareBool(startkey, _key, ALIF_EQ);
				ALIF_DECREF(startkey);
				if (cmp_ > 0)
					goto foundActive;
				if (cmp_ < 0)
					goto comparisonError;
				if (table != _so->table || entry->key != startkey)
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
	if ((size_t)_so->fill * 5 < mask * 3)
		return 0;
	return set_tableResize(_so, _so->used > 50000 ? _so->used * 2 : _so->used * 4);

foundActive:
	ALIF_DECREF(_key);
	return 0;

comparisonError:
	ALIF_DECREF(_key);
	return -1;
}

static AlifIntT set_addKey(AlifSetObject* _so, AlifObject* _key) { // 366
	AlifHashT hash = alifObject_hashFast(_key);
	if (hash == -1) {
		return -1;
	}
	return set_addEntry(_so, _key, hash);
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


AlifIntT alifSet_add(AlifObject* _anySet, AlifObject* _key) { // 2658
	if (!ALIFSET_CHECK(_anySet) and
		(!ALIFFROZENSET_CHECK(_anySet) or ALIF_REFCNT(_anySet) != 1)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	AlifIntT rv_;
	ALIF_BEGIN_CRITICAL_SECTION(_anySet);
	rv_ = set_addKey((AlifSetObject*)_anySet, _key);
	ALIF_END_CRITICAL_SECTION();
	return rv_;
}
