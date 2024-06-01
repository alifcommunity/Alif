#include "alif.h"
#include "AlifCore_Memory.h"


#define LINEAR_PROBES 9
#define PERTURB_SHIFT 5

static SetEntry* set_lookKey(AlifSetObject* _so, AlifObject* _key, size_t _hash)
{
	SetEntry* table_;
	SetEntry* entry_;
	size_t perTurb = _hash;
	size_t mask_ = _so->mask_;
	size_t i_ = (size_t)_hash & mask_; /* Unsigned for defined overflow behavior */
	int probes_;
	int cmp_;

	while (1) {
		entry_ = &_so->table_[i_];
		probes_ = (i_ + LINEAR_PROBES <= mask_) ? LINEAR_PROBES : 0;
		do {
			if (entry_->hash_ == 0 && entry_->key_ == nullptr)
				return entry_;
			if (entry_->hash_ == _hash) {
				AlifObject* startKey = entry_->key_;
				if (startKey == _key)
					return entry_;
				if ((startKey->type_ == &_alifUStrType_)
					&& (_key->type_ == &_alifUStrType_)
					&& unicode_eq(startKey, _key))
					return entry_;
				table_ = _so->table_;
				ALIF_INCREF(startKey);
				cmp_ = alifObject_richCompareBool(startKey, _key, ALIF_EQ);
				ALIF_DECREF(startKey);
				if (cmp_ < 0)
					return nullptr;
				if (table_ != _so->table_ || entry_->key_ != startKey)
					return set_lookKey(_so, _key, _hash);
				if (cmp_ > 0)
					return entry_;
				mask_ = _so->mask_;
			}
			entry_++;
		} while (probes_--);
		perTurb >>= PERTURB_SHIFT;
		i_ = (i_ * 5 + 1 + perTurb) & mask_;
	}
}

static int set_table_resize(AlifSetObject*, int64_t);

static int set_add_entry(AlifSetObject* _so, AlifObject* _key, size_t _hash)
{
	SetEntry* table_;
	SetEntry* freeSlot;
	SetEntry* entry_;
	size_t perTurb;
	size_t mask_;
	size_t i_;
	int probes_;
	int cmp_;

	ALIF_INCREF(_key);

restart:

	mask_ = _so->mask_;
	i_ = (size_t)_hash & mask_;
	freeSlot = nullptr;
	perTurb = _hash;

	while (1) {
		entry_ = &_so->table_[i_];
		probes_ = (i_ + LINEAR_PROBES <= mask_) ? LINEAR_PROBES : 0;
		do {
			if (entry_->hash_ == 0 && entry_->key_ == nullptr)
				goto foundUnusedOrBummy;
			if (entry_->hash_ == _hash) {
				AlifObject* startKey = entry_->key_;
				if (startKey == _key)
					goto foundActive;
				if ((startKey->type_ == &_alifUStrType_)
					&& (_key->type_ == &_alifUStrType_)
					&& unicode_eq(startKey, _key))
					goto foundActive;
				table_ = _so->table_;
				ALIF_INCREF(startKey);
				cmp_ = alifObject_richCompareBool(startKey, _key, ALIF_EQ);
				ALIF_DECREF(startKey);
				if (cmp_ > 0)
					goto foundActive;
				if (cmp_ < 0)
					goto comparisonError;
				if (table_ != _so->table_ || entry_->key_ != startKey)
					goto restart;
				mask_ = _so->mask_;
			}
			else if (entry_->hash_ == -1) {
				freeSlot = entry_;
			}
			entry_++;
		} while (probes_--);
		perTurb >>= PERTURB_SHIFT;
		i_ = (i_ * 5 + 1 + perTurb) & mask_;
	}

foundUnusedOrBummy:
	if (freeSlot == nullptr)
		goto foundUnused;
	_so->used_++;
	freeSlot->key_ = _key;
	freeSlot->hash_ = _hash;
	return 0;

foundUnused:
	_so->fill_++;
	_so->used_++;
	entry_->key_ = _key;
	entry_->hash_ = _hash;
	if ((size_t)_so->fill_ * 5 < mask_ * 3)
		return 0;
	return set_table_resize(_so, _so->used_ > 50000 ? _so->used_ * 2 : _so->used_ * 4);

foundActive:
	ALIF_DECREF(_key);
	return 0;

comparisonError:
	ALIF_DECREF(_key);
	return -1;
}

static void set_insert_clean(SetEntry* _table, size_t _mask, AlifObject* _key, size_t _hash)
{
	SetEntry* entry_;
	size_t perTurb = _hash;
	size_t i_ = (size_t)_hash & _mask;
	size_t j_;

	while (1) {
		entry_ = &_table[i_];
		if (entry_->key_ == nullptr)
			goto foundNull;
		if (i_ + LINEAR_PROBES <= _mask) {
			for (j_ = 0; j_ < LINEAR_PROBES; j_++) {
				entry_++;
				if (entry_->key_ == nullptr)
					goto foundNull;
			}
		}
		perTurb >>= PERTURB_SHIFT;
		i_ = (i_ * 5 + 1 + perTurb) & _mask;
	}
foundNull:
	entry_->key_ = _key;
	entry_->hash_ = _hash;
}

static int set_table_resize(AlifSetObject* _so, int64_t _minUsed)
{
	SetEntry* oldTable, * newTable, * entry_;
	int64_t oldMask = _so->mask_;
	size_t newMask;
	int isOldTableMalloced;
	SetEntry smallCopy[ALIFSET_MINSIZE];
	size_t newSize = ALIFSET_MINSIZE;
	while (newSize <= (size_t)_minUsed) {
		newSize <<= 1;
	}

	oldTable = _so->table_;
	isOldTableMalloced = oldTable != _so->smallTable;

	if (newSize == ALIFSET_MINSIZE) {
		newTable = _so->smallTable;
		if (newTable == oldTable) {
			if (_so->fill_ == _so->used_) {
				return 0;
			}

			memcpy(smallCopy, oldTable, sizeof(smallCopy));
			oldTable = smallCopy;
		}
	}
	else {
		newTable = (SetEntry*)alifMem_objAlloc(newSize);
		if (newTable == nullptr) {
			return -1;
		}
	}

	memset(newTable, 0, sizeof(SetEntry) * newSize);
	_so->mask_ = newSize - 1;
	_so->table_ = newTable;

	newMask = (size_t)_so->mask_;
	if (_so->fill_ == _so->used_) {
		for (entry_ = oldTable; entry_ <= oldTable + oldMask; entry_++) {
			if (entry_->key_ != nullptr) {
				set_insert_clean(newTable, newMask, entry_->key_, entry_->hash_);
			}
		}
	}
	else {
		_so->fill_ = _so->used_;
		for (entry_ = oldTable; entry_ <= oldTable + oldMask; entry_++) {
			if (entry_->key_ != nullptr
				//&& entry_->key_ != dummy
				) {
				set_insert_clean(newTable, newMask, entry_->key_, entry_->hash_);
			}
		}
	}

	if (isOldTableMalloced)
		alifMem_objFree(oldTable);
	return 0;
}

static int set_contains_entry(AlifSetObject* _so, AlifObject* _key, size_t _hash)
{
	SetEntry* entry_;

	entry_ = set_lookKey(_so, _key, _hash);
	if (entry_ != nullptr)
		return entry_->key_ != nullptr;
	return -1;
}

#define DISCARD_NOTFOUND 0
#define DISCARD_FOUND 1

static int set_discard_entry(AlifSetObject* _so, AlifObject* _key, size_t _hash)
{
	SetEntry* entry_;
	AlifObject* oldKey;

	entry_ = set_lookKey(_so, _key, _hash);
	if (entry_ == nullptr)
		return -1;
	if (entry_->key_ == nullptr)
		return DISCARD_NOTFOUND;
	oldKey = entry_->key_;
	//entry_->key_ = dummy;
	entry_->hash_ = -1;
	_so->used_--;
	ALIF_DECREF(oldKey);
	return DISCARD_FOUND;
}

static int set_merge_lock_held(AlifSetObject* _so, AlifObject* _otherSet)
{
	AlifSetObject* other_;
	AlifObject* key_;
	int64_t i_;
	SetEntry* soEntry;
	SetEntry* otherEntry;

	other_ = (AlifSetObject*)_otherSet;
	if (other_ == _so || other_->used_ == 0)
		return 0;

	if ((_so->fill_ + other_->used_) * 5 >= _so->mask_ * 3) {
		if (set_table_resize(_so, (_so->used_ + other_->used_) * 2) != 0)
			return -1;
	}
	soEntry = _so->table_;
	otherEntry = other_->table_;

	if (_so->fill_ == 0 && _so->mask_ == other_->mask_ && other_->fill_ == other_->used_) {
		for (i_ = 0; i_ <= other_->mask_; i_++, soEntry++, otherEntry++) {
			key_ = otherEntry->key_;
			if (key_ != NULL) {
				soEntry->key_ = ALIF_NEWREF(key_);
				soEntry->hash_ = otherEntry->hash_;
			}
		}
		_so->fill_ = other_->fill_;
		_so->used_ = other_->used_;
		return 0;
	}

	if (_so->fill_ == 0) {
		SetEntry* newTable = _so->table_;
		size_t newMask = (size_t)_so->mask_;
		_so->fill_ = other_->used_;
		_so->used_ = other_->used_;
		for (i_ = other_->mask_ + 1; i_ > 0; i_--, otherEntry++) {
			key_ = otherEntry->key_;
			if (key_ != NULL
				//&& key_ != dummy
				) {
				set_insert_clean(newTable, newMask, ALIF_NEWREF(key_),
					otherEntry->hash_);
			}
		}
		return 0;
	}

	for (i_ = 0; i_ <= other_->mask_; i_++) {
		otherEntry = &other_->table_[i_];
		key_ = otherEntry->key_;
		if (key_ != NULL
			//&& key_ != dummy
			) {
			if (set_add_entry(_so, key_, otherEntry->hash_))
				return -1;
		}
	}
	return 0;
}


static int set_add_key(AlifSetObject* _so, AlifObject* _key)
{
	size_t hash_;

	if (!(_key->type_ == &_alifUStrType_)) {
		hash_ = alifObject_hash(_key);
		if (hash_ == -1)
			return -1;
	}
	return set_add_entry(_so, _key, hash_);
}

static int set_contains_key(AlifSetObject* _so, AlifObject* _key)
{
	size_t hash_;

	if (!(_key->type_ == &_alifUStrType_)
		//|| (hash_ = ALIFUNICODE_CAST(key)->hash) == -1
		) {
		hash_ = alifObject_hash(_key);
		if (hash_ == -1)
			return -1;
	}
	return set_contains_entry(_so, _key, hash_);
}

static int set_discard_key(AlifSetObject* _so, AlifObject* _key)
{
	size_t hash_;

	if (!(_key->type_ == &_alifUStrType_)
		//||
		//(hash_ = ALIFUNICODE_CAST(_key)->hash_) == -1) 
		)
	{
		hash_ = alifObject_hash(_key);
		if (hash_ == -1)
			return -1;
	}
	return set_discard_entry(_so, _key, hash_);
}

static int set_update_dict_lock_held(AlifSetObject* _so, AlifObject* _other)
{

	int64_t dictSize = ((AlifDictObject*)_other)->size_;
	if ((_so->fill_ + dictSize) * 5 >= _so->mask_ * 3) {
		if (set_table_resize(_so, (_so->used_ + dictSize) * 2) != 0) {
			return -1;
		}
	}

	int64_t pos_ = 0;
	AlifObject* key_;
	AlifObject* value_;
	size_t hash_;
	while (dict_next(_other, &pos_, &key_, &value_, &hash_)) {
		if (set_add_entry(_so, key_, hash_)) {
			return -1;
		}
	}
	return 0;
}

static int set_update_iterable_lock_held(AlifSetObject* _so, AlifObject* _other)
{

	AlifObject* it_ = alifObject_getIter(_other);
	if (it_ == nullptr) {
		return -1;
	}

	AlifObject* key_;
	while ((key_ = alifIter_next(it_)) != nullptr) {
		if (set_add_key(_so, key_)) {
			ALIF_DECREF(it_);
			ALIF_DECREF(key_);
			return -1;
		}
		ALIF_DECREF(key_);
	}
	ALIF_DECREF(it_);
	return 0;
}

static int set_update_local(AlifSetObject* _so, AlifObject* _other)
{
	if ((_other->type_ == &_alifSetType_)) {
		int rv{};
		//ALIF_BEGIN_CRITICAL_SECTION(other);
		rv = set_merge_lock_held(_so, _other);
		//ALIF_END_CRITICAL_SECTION();
		return rv;
	}
	else if ((_other->type_ == &typeDict)) {
		int rv;
		//ALIF_BEGIN_CRITICAL_SECTION(_other);
		rv = set_update_dict_lock_held(_so, _other);
		//ALIF_END_CRITICAL_SECTION();
		return rv;
	}
	return set_update_iterable_lock_held(_so, _other);
}

static AlifObject* make_new_set(AlifTypeObject* _type, AlifObject* _iterable)
{
	AlifSetObject* so_{};

	//so_ = (AlifSetObject*)(_type->alloc_(_type, 0));
	so_ = (AlifSetObject*)alifMem_objAlloc(alifSubObject_varSize(&_alifSetType_, 1));
	if (so_ == nullptr)
		return nullptr;

	so_->fill_ = 0;
	so_->used_ = 0;
	so_->mask_ = ALIFSET_MINSIZE - 1;
	so_->table_ = so_->smallTable;
	so_->hash_ = -1;
	so_->finger_ = 0;
	so_->weakreFlist = nullptr;

	if (_iterable != nullptr) {
		if (set_update_local(so_, _iterable)) {
			ALIF_DECREF(so_);
			return nullptr;
		}
	}

	return (AlifObject*)so_;
}

static AlifObject* make_new_set_baseType(AlifTypeObject* type, AlifObject* iterable)
{
	//if (type != &AlifSet_Type && type != &AlifFrozenSet_Type) {
		//if (alifType_IsSubtype(type, &_alifSetType_))
	type = &_alifSetType_;
	//else
		//type = &AlifFrozenSet_Type;
//}
	return make_new_set(type, iterable);
}

static AlifObject* set_or(AlifSetObject* _so, AlifObject* _other)
{
	AlifSetObject* result_;

	if (!(((AlifObject*)_so)->type_ == &_alifSetType_) || !(_other->type_ == &_alifSetType_))
		return ALIF_NOTIMPLEMENTED;

	result_ = (AlifSetObject*)set_copy(_so, NULL);
	if (result_ == NULL) {
		return NULL;
	}
	if (ALIF_IS((AlifObject*)_so, _other)) {
		return (AlifObject*)result_;
	}
	if (set_update_local(result_, _other)) {
		ALIF_DECREF(result_);
		return NULL;
	}
	return (AlifObject*)result_;
}

AlifTypeObject _alifSetType_ = {
	0,
	0,
	0,
	L"set",                              /* tp_name */
	sizeof(AlifSetObject),                /* tp_basicsize */
	0,                                  /* tp_itemsize */
	/* methods */
	0, // (destructor)set_dealloc,            /* tp_dealloc */
	0,                                  /* tp_vectorcall_offset */
	0,                                  /* tp_getattr */
	0,                                  /* tp_setattr */
	0,                                  /* tp_as_async */
	0, // (reprfunc)set_repr,                 /* tp_repr */
	0, //&set_as_number,                     /* tp_as_number */
	0, //&set_as_sequence,                   /* tp_as_sequence */
	0,                                  /* tp_as_mapping */
	0, // Object_HashNotImplemented,        /* tp_hash */
	0,                                  /* tp_call */
	0,                                  /* tp_str */
	0, //yObject_GenericGetAttr,            /* tp_getattro */
	0,                                  /* tp_setattro */
	0,                                  /* tp_as_buffer */
	0,       /* tp_flags */
	0, //set_doc,                            /* tp_doc */
	0, //(traverseproc)set_traverse,         /* tp_traverse */
	0, //(inquiry)set_clear_internal,        /* tp_clear */
	0, //(richcmpfunc)set_richcompare,       /* tp_richcompare */
	0, //offsetof(SetObject, weakreflist), /* tp_weaklistoffset */
	0, //(getiterfunc)set_iter,              /* tp_iter */
	0,                                  /* tp_iternext */
	0, //set_methods,                        /* tp_methods */
	0,                                  /* tp_members */
	0,                                  /* tp_getset */
	0,                                  /* tp_base */
	0,                                  /* tp_dict */
	0,                                  /* tp_descr_get */
	0,                                  /* tp_descr_set */
	0,                                  /* tp_dictoffset */
	0, //(initproc)set_init,                 /* tp_init */
	0, //Type_GenericAlloc,                /* tp_alloc */
	0, //set_new,                            /* tp_new */
	0, //Object_GC_Del,                    /* tp_free */
	0, //.tp_vectorcall = set_vectorcall,
};

AlifObject* alifNew_set(AlifObject* _iterable)
{
	return make_new_set(&_alifSetType_, _iterable);
}

#define SET_COPY_METHODDEF    \
    {"copy", (PyCFunction)set_copy, METH_NOARGS, set_copy__doc__},

static AlifObject*
set_copy_impl(AlifSetObject*);

static AlifObject* set_copy(AlifSetObject* _so, AlifObject* ALIF_UNUSED(ignored))
{
	AlifObject* return_value = NULL;

	//ALIF_BEGIN_CRITICAL_SECTION(_so);
	return_value = set_copy_impl(_so);
	//ALIF_END_CRITICAL_SECTION();

	return return_value;
}

static AlifObject* set_copy_impl(AlifSetObject* _so)
{
	AlifObject* copy_ = make_new_set_baseType(ALIF_TYPE(_so), NULL);
	if (copy_ == NULL) {
		return NULL;
	}
	if (set_merge_lock_held((AlifSetObject*)copy_, (AlifObject*)_so) < 0) {
		ALIF_DECREF(copy_);
		return NULL;
	}
	return copy_;
}

int alifSet_discard(AlifObject* _set, AlifObject* _key)
{
	if (!(_set->type_ == &_alifSetType_)) {
		return -1;
	}

	int rv;
	//ALIF_BEGIN_CRITICAL_SECTION(set);
	rv = set_discard_key((AlifSetObject*)_set, _key);
	//ALIF_END_CRITICAL_SECTION();
	return rv;
}

int alifSet_contains(AlifObject* _anySet, AlifObject* _key)
{
	if (!(_anySet->type_ == &_alifSetType_)) {
		return -1;
	}

	int rv;
	//ALIF_BEGIN_CRITICAL_SECTION(anyset);
	rv = set_contains_key((AlifSetObject*)_anySet, _key);
	//ALIF_END_CRITICAL_SECTION();
	return rv;
}

int alifSet_add(AlifObject* _anySet, AlifObject* _key)
{
	if (!(_anySet->type_ == &_alifSetType_) &&
		(ALIF_REFCNT(_anySet) != 1)) {
		return -1;
	}

	int rv;
	//ALIF_BEGIN_CRITICAL_SECTION(anyset);
	rv = set_add_key((AlifSetObject*)_anySet, _key);
	//ALIF_END_CRITICAL_SECTION();
	return rv;
}
