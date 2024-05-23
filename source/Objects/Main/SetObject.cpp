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
                if ((startKey->type_ == &_typeUnicode_)
                    && (_key->type_ == &_typeUnicode_)
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
    SetEntry* table;
    SetEntry* freeslot;
    SetEntry* entry;
    size_t perturb;
    size_t mask;
    size_t i;                    
    int probes;
    int cmp;

    //ALIFSUB_CRITICAL_SECTION_ASSERT_OBJECT_LOCKED(_so);

    ALIF_INCREF(_key);

restart:

    mask = _so->mask_;
    i = (size_t)_hash & mask;
    freeslot = nullptr;
    perturb = _hash;

    while (1) {
        entry = &_so->table_[i];
        probes = (i + LINEAR_PROBES <= mask) ? LINEAR_PROBES : 0;
        do {
            if (entry->hash_ == 0 && entry->key_ == nullptr)
                goto found_unused_or_dummy;
            if (entry->hash_ == _hash) {
                AlifObject* startkey = entry->key_;
                if (startkey == _key)
                    goto found_active;
                if ((startkey->type_ == &_typeUnicode_)
                    && (_key->type_ == &_typeUnicode_)
                    && unicode_eq(startkey, _key))
                    goto found_active;
                table = _so->table_;
                ALIF_INCREF(startkey);
                cmp = alifObject_richCompareBool(startkey, _key, ALIF_EQ);
                ALIF_DECREF(startkey);
                if (cmp > 0)
                    goto found_active;
                if (cmp < 0)
                    goto comparison_error;
                if (table != _so->table_ || entry->key_ != startkey)
                    goto restart;
                mask = _so->mask_;
            }
            else if (entry->hash_ == -1) {
                freeslot = entry;
            }
            entry++;
        } while (probes--);
        perturb >>= PERTURB_SHIFT;
        i = (i * 5 + 1 + perturb) & mask;
    }

found_unused_or_dummy:
    if (freeslot == nullptr)
        goto found_unused;
    _so->used_++;
    freeslot->key_ = _key;
    freeslot->hash_ = _hash;
    return 0;

found_unused:
    _so->fill_++;
    _so->used_++;
    entry->key_ = _key;
    entry->hash_ = _hash;
    if ((size_t)_so->fill_ * 5 < mask * 3)
        return 0;
    return set_table_resize(_so, _so->used_ > 50000 ? _so->used_ * 2 : _so->used_ * 4);

found_active:
    ALIF_DECREF(_key);
    return 0;

comparison_error:
    ALIF_DECREF(_key);
    return -1;
}

static void set_insert_clean(SetEntry* _table, size_t _mask, AlifObject* _key, size_t _hash)
{
    SetEntry* entry;
    size_t perturb = _hash;
    size_t i = (size_t)_hash & _mask;
    size_t j;

    while (1) {
        entry = &_table[i];
        if (entry->key_ == nullptr)
            goto foundNull;
        if (i + LINEAR_PROBES <= _mask) {
            for (j = 0; j < LINEAR_PROBES; j++) {
                entry++;
                if (entry->key_ == nullptr)
                    goto foundNull;
            }
        }
        perturb >>= PERTURB_SHIFT;
        i = (i * 5 + 1 + perturb) & _mask;
    }
foundNull:
    entry->key_ = _key;
    entry->hash_ = _hash;
}

static int set_table_resize(AlifSetObject* _so, int64_t _minUsed)
{
    SetEntry* oldTable, * newTable, * entry;
    int64_t oldmask = _so->mask_;
    size_t newmask;
    int is_oldtable_malloced;
    SetEntry small_copy[ALIFSET_MINSIZE];
    size_t newsize = ALIFSET_MINSIZE;
    while (newsize <= (size_t)_minUsed) {
        newsize <<= 1;
    }

    oldTable = _so->table_;
    is_oldtable_malloced = oldTable != _so->smallTable;

    if (newsize == ALIFSET_MINSIZE) {
        newTable = _so->smallTable;
        if (newTable == oldTable) {
            if (_so->fill_ == _so->used_) {
                return 0;
            }

            memcpy(small_copy, oldTable, sizeof(small_copy));
            oldTable = small_copy;
        }
    }
    else {
        newTable = (SetEntry*)alifMem_objAlloc(newsize);
        if (newTable == nullptr) {
            return -1;
        }
    }

    memset(newTable, 0, sizeof(SetEntry) * newsize);
    _so->mask_ = newsize - 1;
    _so->table_ = newTable;

    newmask = (size_t)_so->mask_;
    if (_so->fill_ == _so->used_) {
        for (entry = oldTable; entry <= oldTable + oldmask; entry++) {
            if (entry->key_ != nullptr) {
                set_insert_clean(newTable, newmask, entry->key_, entry->hash_);
            }
        }
    }
    else {
        _so->fill_ = _so->used_;
        for (entry = oldTable; entry <= oldTable + oldmask; entry++) {
            if (entry->key_ != nullptr 
                //&& entry->key_ != dummy
                ) {
                set_insert_clean(newTable, newmask, entry->key_, entry->hash_);
            }
        }
    }

    if (is_oldtable_malloced)
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

static int set_add_key(AlifSetObject* _so, AlifObject* _key)
{
    size_t hash;

    if (!(_key->type_ == &_typeUnicode_)) {
        hash = alifObject_hash(_key);
        if (hash == -1)
            return -1;
    }
    return set_add_entry(_so, _key, hash);
}

static int set_contains_key(AlifSetObject* _so, AlifObject* _key)
{
    size_t hash_;

    if (!(_key->type_ == &_typeUnicode_) 
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

    if (!(_key->type_ == &_typeUnicode_)
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

    int64_t dictsize = ((AlifDictObject*)_other)->size_;
    if ((_so->fill_ + dictsize) * 5 >= _so->mask_ * 3) {
        if (set_table_resize(_so, (_so->used_ + dictsize) * 2) != 0) {
            return -1;
        }
    }

    int64_t pos = 0;
    AlifObject* key;
    AlifObject* value;
    size_t hash;
    while (dict_next(_other, &pos, &key, &value, &hash)) {
        if (set_add_entry(_so, key, hash)) {
            return -1;
        }
    }
    return 0;
}

static int set_update_iterable_lock_held(AlifSetObject* _so, AlifObject* _other)
{
    //ALIFSUB_CRITICAL_SECTION_ASSERT_OBJECT_LOCKED(_so);

    AlifObject* it = alifObject_getIter(_other);
    if (it == nullptr) {
        return -1;
    }

    AlifObject* key;
    while ((key = alifIter_next(it)) != nullptr) {
        if (set_add_key(_so, key)) {
            ALIF_DECREF(it);
            ALIF_DECREF(key);
            return -1;
        }
        ALIF_DECREF(key);
    }
    ALIF_DECREF(it);
    return 0;
}

static int set_update_local(AlifSetObject* _so, AlifObject* _other)
{
    if ((_other->type_ == &_TypeSet_)) {
        int rv{};
        //ALIF_BEGIN_CRITICAL_SECTION(other);
        //rv = set_merge_lock_held(_so, _other);
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
    AlifSetObject* so_;

    so_ = (AlifSetObject*)_type->alloc_(_type, 0);
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



AlifTypeObject _TypeSet_ = {
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
    return make_new_set(&_TypeSet_, _iterable);
}

int alifSet_discard(AlifObject* _set, AlifObject* _key)
{
    if (!(_set->type_ == &_TypeSet_)) {
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
    if (!(_anySet->type_ == &_TypeSet_)) {
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
    if (!(_anySet->type_ == &_TypeSet_) &&
        ( ALIF_REFCNT(_anySet) != 1)) {
        return -1;
    }

    int rv;
    //ALIF_BEGIN_CRITICAL_SECTION(anyset);
    rv = set_add_key((AlifSetObject*)_anySet, _key);
    //ALIF_END_CRITICAL_SECTION();
    return rv;
}