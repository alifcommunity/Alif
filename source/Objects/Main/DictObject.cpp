#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_Dict.h"

AlifDictObject* dict_presize(AlifDictObject* dict, int64_t used) {

    dict = (AlifDictObject*)alifMem_objRealloc(dict,
        sizeof(AlifDictValues) * used + sizeof(AlifDictObject));

    dict->items_ = (AlifDictValues*)((char*)&dict->items_ + 8);

    return dict;
}

AlifObject* new_dict(int64_t used){
    
    AlifDictObject* object = (AlifDictObject*)alifMem_objAlloc(
        sizeof(AlifDictObject));
    
    object->_base_.type_ = &typeDict;

    if(used > 0) {
        object = dict_presize(object, used);
    }

    return (AlifObject*)object;
}

AlifObject* alifNew_dict()
{
    return new_dict(0);
}

//AlifDictObject* dict_shrink(AlifDictObject* dict) {
//
//    int64_t capacity_ = dict->capacity_,
//        size_ = dict->size_;
//
//    if (!(size_ < (capacity_ / 2))) {
//        std::wcout << L"يجب ان تكون سعة الفهرس نصف او اقل من حجمه\n" << std::endl;
//        exit(-1);
//    }
//    ((size_t)(*capacity_) >> 3) << 3
//
//    AlifDictObject* newDict = (AlifDictObject*)new_dict(capacity_ / 2);
//
//    memcpy(newDict, dict, sizeof(AlifDictObject) + (size_ * sizeof(AlifDictValues)));
//    newDict->items_ = (AlifDictValues*)((char*)&newDict->items_ + 8);
//    newDict->capacity_ = capacity_ / 2;
//
//    alifMem_objFree(dict);
//
//    return newDict;
//}

AlifDictObject* dict_resize(AlifDictObject* dict) {
    
    if (dict == nullptr) {
        return nullptr;
    }

    int64_t* size_ = &dict->size_,
       * capacity_ = &dict->capacity_;

    if (*size_ >= *capacity_) {

        //The growth pattern is : 0, 4, 8, 16, 24, 32, 40, 52, 64, ...
        *capacity_ = (*capacity_ == 0) ? 1 : ((size_t)*capacity_ + (*capacity_ >> 3) + 6) & ~(size_t)3;

        dict = dict_presize(dict, *capacity_);

    }

    return dict;
}

int dict_lookupItem(AlifDictObject* dict, AlifObject* key, size_t hash, AlifObject** addrValue) {

    AlifDictValues* items_ = dict->items_;
    int64_t size_ = dict->size_,
        index = -1;

    for (int64_t i = 0; i < size_; i++)
    {
        if (items_[i].key->type_ == key->type_ && items_[i].hash == hash) {
            *addrValue = items_[i].value;
            index = i;
            break;
        }
    }
    if (index == -1) {
        *addrValue = nullptr;
    }

    return index;
}

AlifIntT dict_setItem(AlifDictObject* _dict, AlifObject* _key, AlifObject* _value) {

    AlifObject* oldValue;
    size_t hash;

    if (_key->type_ == &_typeUnicode_) {
        hash = ((AlifUStrObject*)_key)->hash;
    }
    else {
        hash = alifObject_hash(_key);
    }

    int64_t index = dict_lookupItem(_dict, _key, hash, &oldValue);

    if (index == -1) {

        int64_t size_ = _dict->size_;
        if ((_dict = dict_resize(_dict)) == nullptr) {
            return -1;
        }
        _dict->items_[size_].hash = hash;
        _dict->items_[size_].key = _key;
        _dict->items_[size_].value = _value;
        _dict->size_++;
    }
    else {
        
        _dict->items_[index].value = _value;
    }

	return 1;
}

//AlifObject* newDict_fromItems() {}

AlifObject* dict_getItem(AlifObject* dict, AlifObject* key) {

    AlifDictObject* dictObj = (AlifDictObject*)dict;

    AlifObject* value;

    size_t hash;
    if (key->type_ == &_typeUnicode_) {
        hash = ((AlifUStrObject*)key)->hash;
    }
    else {
        hash = alifObject_hash(key);
    }

    int64_t index = dict_lookupItem(dictObj, key, hash, &value);

    return value;

}

int alifDict_getItemRefKnownHash(AlifObject* op, AlifObject* key, size_t hash, AlifObject** result)
{
	AlifDictObject* mp = (AlifDictObject*)op;

	AlifObject* value;
#ifdef ALIF_GIL_DISABLED
	size_t ix = alifSub_dict_lookup_threadsafe(mp, key, hash, &value);
#else
	size_t ix = dict_lookupItem(mp, key, hash, &value);
#endif
	if (value == NULL) {
		*result = NULL;
		return 0;  // missing key
	}
#ifdef ALIF_GIL_DISABLED
	* result = value;
#else
	* result = ALIF_NEWREF(value);
#endif
	return 1;  // key is present
}

int alifDict_getItemRef(AlifObject* op, AlifObject* key, AlifObject** result)
{
	if (!(op->type_ == &typeDict)) {
		*result = NULL;
		return -1;
	}

	size_t hash;
	if (key->type_ == &_typeUnicode_) {
		hash = ((AlifUStrObject*)key)->hash;
	}
	else {
		hash = alifObject_hash(key);
	}

	return alifDict_getItemRefKnownHash(op, key, hash, result);
}

bool dict_contain(AlifObject* dict, AlifObject* key) {

    AlifDictObject* dictObj = (AlifDictObject*)dict;

    AlifObject* value;
    size_t hash;
    if (key->type_ == &_typeUnicode_) {
        hash = ((AlifUStrObject*)key)->hash;
    }
    else {
        hash = alifObject_hash(key);
    }

    int64_t index = dict_lookupItem(dictObj, key, hash, &value);

    return (index != -1 && value != nullptr);

}

AlifDictObject* deleteItem_fromIndex(AlifDictObject* dict, int64_t index) {

    for (int64_t i = index; i < dict->size_ - 1; ++i)
    {
        dict->items_[ i] = dict->items_[ i + 1];
    }

    dict->size_--;

    //if (dict->size_ < (dict->capacity_ / 2)) {
    //    
    //    dict = (AlifDictObject*)dict_shrink(dict);

    //}
    return dict;
}

AlifDictObject* deletItem_common(AlifDictObject* dict, size_t hash, 
    int64_t index) {


    if (index != -1) {
    
        dict = deleteItem_fromIndex(dict, index);

    }
    else {
        int64_t indexPos = -1;
        for (int64_t i = 0; i < dict->size_; i++)
        {
            if (dict->items_[i].hash == hash) {
                indexPos = i;
                break;
            }
        }

        if(indexPos != -1){
            dict = deleteItem_fromIndex(dict, indexPos);
        }
    }
    return dict;
}

AlifIntT dict_deleteItem(AlifDictObject* dict, AlifObject* key) {

    size_t hash;
    if (key->type_ == &_typeUnicode_) {
        hash = ((AlifUStrObject*)key)->hash;
    }
    else {
        hash = alifObject_hash(key);
    }

    AlifObject* oldValue;
    int64_t index = dict_lookupItem(dict, key, hash, &oldValue);
    if (index == -1 || oldValue == nullptr) {
        return -1;
    }

    deletItem_common(dict, hash, index);
    
    return 1;
}

bool dict_next(AlifObject* dict, int64_t * posPos, AlifObject** posKey, AlifObject** posValue, size_t *posHash) {

    int64_t index{};
    AlifDictObject* map{};

    map = (AlifDictObject*)dict;
    index = *posPos;

    if (map->size_ <= index || index < 0) {
        return false;
    }

    AlifDictValues value = map->items_[index];

    *posPos = index + 1;

    if (posKey) {
        *posKey = value.key;
    }
    if (posValue) {
        *posValue = value.value;
    }
    if (posHash) {
        if (value.key->type_ == &_typeUnicode_) {
            *posHash = ((AlifUStrObject*)value.key)->hash;
        }
        else {
            *posHash = alifObject_hash(value.key);
        }
    }
    return true;

}

AlifObject* alifDict_popKnownHash(AlifObject* dict, AlifObject* key, size_t hash, AlifObject* deflt)
{
    int64_t ix;
    AlifObject* old_value;
    AlifDictObject* mp;

    mp = (AlifDictObject*)dict;

    if (mp->size_ == 0) {
        if (deflt) {
            return deflt;
        }
        return NULL;
    }
    ix = dict_lookupItem(mp, key, hash, &old_value);
    if (ix == -1)
        return NULL;
    if ( old_value == NULL) {
        if (deflt) {
            return deflt;
        }
        return NULL;
    }
    deletItem_common(mp, hash, ix);

    return old_value;
}

AlifObject* _alifDict_pop(AlifObject* dict, AlifObject* key, AlifObject* deflt)
{
    size_t hash;

    if (((AlifDictObject*)dict)->size_ == 0) {
        if (deflt) {
            return (deflt);
        }
        return NULL;
    }
    if (!(key->type_ == &_typeUnicode_) || (hash = ((AlifUStrObject*)key)->hash) == 0) {
        hash = alifObject_hash(key);
        if (hash == -1)
            return NULL;
    }
    return alifDict_popKnownHash(dict, key, hash, deflt);
}


static AlifObject* dict___contains__(AlifDictObject* self, AlifObject* key)
{
    register AlifDictObject* mp = self;
    size_t hash;
    int64_t ix;
    AlifObject* value;

    if (!(key->type_ == &_typeUnicode_) || (hash = ((AlifUStrObject*)key)->hash) == 0) {
        hash = alifObject_hash(key);
        if (hash == -1)
            return NULL;
    }
    ix = dict_lookupItem(mp, key, hash, &value);
    if ( value == nullptr)
        return ALIF_FALSE;
    return ALIF_TRUE;
}

static AlifObject* dict_get_impl(AlifDictObject* self, AlifObject* key, AlifObject* default_value);

static AlifObject* dict_get(AlifDictObject* self, AlifObject* const* args, int64_t nargs)
{
    AlifObject* return_value = NULL;
    AlifObject* key;
    AlifObject* default_value = ALIF_NONE;

    if (!_alifArg_checkPositional(L"get", nargs, 1, 2)) {
        goto exit;
    }
    key = args[0];
    if (nargs < 2) {
        goto skip_optional;
    }
    default_value = args[1];
skip_optional:
    return_value = dict_get_impl(self, key, default_value);

exit:
    return return_value;
}

static AlifObject* dict_get_impl(AlifDictObject* self, AlifObject* key, AlifObject* default_value)
{
    AlifObject* val = NULL;
    size_t hash;
    int64_t ix;

    if (!(key->type_ == &_typeUnicode_) || (hash = ((AlifUStrObject*)key)->hash) == 0) {
        hash = alifObject_hash(key);
        if (hash == -1)
            return NULL;
    }
    ix = dict_lookupItem(self, key, hash, &val);
    if (ix == -1)
        return NULL;
    if ( val == NULL) {
        val = default_value;
    }
    return val;
}

static AlifObject* dict_popImpl(AlifDictObject* self, AlifObject* key, AlifObject* default_value);

static AlifObject* dict_pop(AlifDictObject* self, AlifObject* const* args, int64_t nargs)
{
    AlifObject* return_value = NULL;
    AlifObject* key;
    AlifObject* default_value = NULL;

    if (!_alifArg_checkPositional(L"pop", nargs, 1, 2)) {
        goto exit;
    }
    key = args[0];
    if (nargs < 2) {
        goto skip_optional;
    }
    default_value = args[1];
skip_optional:
    return_value = dict_popImpl(self, key, default_value);

exit:
    return return_value;
}

static AlifObject* dict_popImpl(AlifDictObject* self, AlifObject* key, AlifObject* default_value)
{
    return _alifDict_pop((AlifObject*)self, key, default_value);
}

//static AlifObject* dict_popItemImpl(AlifDictObject* self)
///*[clinic end generated code: output=e65fcb04420d230d input=1c38a49f21f64941]*/
//{
//    int64_t i, j;
//    AlifObject* res;
//    uint64_t new_version;
//
//    res = alifNew_tuple(2);
//    if (res == NULL)
//        return NULL;
//    if (self->size_ == 0) {
//        return NULL;
//    }
//    ///* Convert split table to combined table */
//    //if (self->ma_keys->dk_kind == DICT_KEYS_SPLIT) {
//    //    if (dictResize( self, DK_LOG_SIZE(self->ma_keys), 1)) {
//    //        return NULL;
//    //    }
//    //}
//    /* Pop last item */
//    AlifObject* key, * value;
//    size_t hash;
//    if (DK_IS_UNICODE(self->ma_keys)) {
//        PyDictUnicodeEntry* ep0 = DK_UNICODE_ENTRIES(self->ma_keys);
//        i = self->ma_keys->dk_nentries - 1;
//        while (i >= 0 && ep0[i].me_value == NULL) {
//            i--;
//        }
//
//        key = ep0[i].me_key;
//        new_version = _PyDict_NotifyEvent(
//            interp, PyDict_EVENT_DELETED, self, key, NULL);
//        hash = unicode_get_hash(key);
//        value = ep0[i].me_value;
//        ep0[i].me_key = NULL;
//        ep0[i].me_value = NULL;
//    }
//    else {
//        PyDictKeyEntry* ep0 = DK_ENTRIES(self->ma_keys);
//        i = self->ma_keys->dk_nentries - 1;
//        while (i >= 0 && ep0[i].me_value == NULL) {
//            i--;
//        }
//
//        key = ep0[i].me_key;
//        new_version = _PyDict_NotifyEvent(
//            interp, PyDict_EVENT_DELETED, self, key, NULL);
//        hash = ep0[i].me_hash;
//        value = ep0[i].me_value;
//        ep0[i].me_key = NULL;
//        ep0[i].me_hash = -1;
//        ep0[i].me_value = NULL;
//    }
//
//    j = lookdict_index(self->ma_keys, hash, i);
//    dictkeys_set_index(self->ma_keys, j, DKIX_DUMMY);
//
//    PyTuple_SET_ITEM(res, 0, key);
//    PyTuple_SET_ITEM(res, 1, value);
//    self->ma_keys->dk_nentries = i;
//    self->ma_used--;
//    self->ma_version_tag = new_version;
//    return res;
//}

int dicts_equal(AlifDictObject* v, AlifDictObject* w) {

    AlifDictObject* dict1 = v,
        * dict2 = w;
    int64_t size1 = dict1->size_,
        size2 = dict2->size_;
    int compare;
    if (size1 != size2) {
        return 0;
    }

    for (int64_t i = 0; i < size1; i++)
    {
        AlifDictValues item1 = dict1->items_[i];
        AlifDictValues item2 = dict2->items_[i];

        if (!(item1.hash == item2.hash)) {
            return 0;
        }

        if (item1.value == nullptr || item2.value == nullptr) {
            // error
        }

        compare = alifObject_richCompareBool(item1.value, item2.value, ALIF_EQ);

        if (compare <= 0 ) {
            return compare;
        }
    }

    return 1;

}

AlifObject* dict_compare(AlifObject* v, AlifObject* w, int op) {

    int compare;
    AlifObject* res = nullptr;

    if ((v->type_ !=& typeDict) || (w->type_ != &typeDict)) {
        // error
    }
    if (op == ALIF_EQ || op == ALIF_NE) {
    
        compare = dicts_equal((AlifDictObject*)v, (AlifDictObject*)w);
        if (compare < 0) {
            return nullptr;
        }
        res = (compare == (op == ALIF_EQ)) ? ALIF_TRUE : ALIF_FALSE;

    }
    return res;

}

size_t dict_length(AlifDictObject* dict) {
    return dict->size_;
}

AlifIntT dict_ass_sub(AlifDictObject* dict, AlifObject* key, AlifObject* value) {


    if (value == nullptr) {
        return dict_deleteItem(dict, key);
    }
    else {
        return dict_setItem(dict, key, value);
    }

}

static AlifObject* keys_lock_held(AlifObject* _dict)
{

	if (_dict == NULL || !(_dict->type_ == &typeDict)) {
		return NULL;
	}
	AlifDictObject* mp_ = (AlifDictObject*)_dict;
	AlifObject* v_{};
	int64_t n_{};

again:
	v_ = alifNew_list(n_);
	if (v_ == NULL)
		return NULL;

	int64_t j_ = 0, pos_ = 0;
	AlifObject* key_;
	while (dict_next((AlifObject*)mp_, &pos_, &key_, nullptr, nullptr)) {
		((AlifListObject*)v_)->items[j_] = ALIF_NEWREF(key_);
		j_++;
	}
	return v_;
}

AlifObject* alifDict_keys(AlifObject* _dict)
{
	AlifObject* res;
	//ALIF_BEGIN_CRITICAL_SECTION(dict);
	res = keys_lock_held(_dict);
	//ALIF_END_CRITICAL_SECTION();

	return res;
}

AlifObject* dict_subscript(AlifDictObject* dict, AlifObject* key) {

    size_t hash;
    if (key->type_ == &_typeUnicode_) {
        hash = ((AlifUStrObject*)key)->hash;
    }
    else {
        hash = alifObject_hash(key);
    }

    int64_t index;
    AlifObject* value{};

    index = dict_lookupItem(dict, key, hash, &value);
    if (index == -1) {
        return nullptr;
    }
    return value;

}

void dict_dealloc(AlifDictObject* dict) {

    alifMem_objFree(dict);

}

static AlifMethodDef mappMethods[] = {
    {L"__contains__", (AlifCFunction)dict___contains__, METHOD_O | METHOD_COEXIST},
    {L"__getitem__", ALIFCFunction_CAST(dict_subscript), METHOD_O | METHOD_COEXIST},
    {L"get", ALIFCFunction_CAST(dict_get), METHOD_FASTCALL},
    {L"pop", ALIFCFunction_CAST(dict_pop), METHOD_FASTCALL},
    {NULL,              NULL}   /* sentinel */
};

AlifSequenceMethods dictAsSequence = {
    0,                          
    0,                        
    0,                         
    0,                         
    0,                        
    0,                          
    0,                          
    (ObjObjProc)dict_contain,                        
};

AlifMappingMethods dictAsMapping = {
    (LenFunc)dict_length, 
    (BinaryFunc)dict_subscript,
    (ObjObjArgProc)dict_ass_sub, 
};

AlifInitObject typeDict = {
    0,
    0,
    0,
    L"dict",
    sizeof(AlifDictObject),
    0,
    (Destructor)dict_dealloc,                 
    0,                                         
    0,                                       
    0,                                          
    0,                       
    0,                            
    &dictAsSequence,
    &dictAsMapping,
    //(HashFunc)alifObject_hashNotImplemented,             
    0,                                          
    0,                                          
    0,            
    0,
    0,
    0,                                          
    0,                                          
    0,  
    0,                            
    0,                              
    dict_compare,                           
    0,                              
    0,                                          
    0,                     
    mappMethods,
    0,                               
    0,                                          
    0,                                         
    0,                                         
    0,                                         
    0,                                          
    0,                                          
    0,                                          
    0,                                 
    0,                      
    0,                                  
    0,                           
};
