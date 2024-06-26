#include "alif.h"

#include "AlifCore_Memory.h"
#include "AlifCore_Dict.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_Interpreter.h"


#define ALIFDICT_LOG_MINSIZE 3
#define ALIFDICT_MINSIZE 8

#define USABLE_FRACTION(n) ((n << 1)/3)



// Forwards
static AlifIntT setItem_lockHeld(AlifDictObject*, AlifObject*, AlifObject*);




AlifDictObject* dict_presize(AlifDictObject* dict, int64_t used) {


	if (dict->capacity_ > 1) {
		dict->items_ = (AlifDictValues*)alifMem_objRealloc(dict->items_,
			sizeof(AlifDictValues) * used + sizeof(AlifDictObject));
	}
	else {

		dict->items_ = (AlifDictValues*)alifMem_objAlloc(sizeof(AlifDictValues) * used);

		//dict->items_ = (AlifDictValues*)((char*)&dict->items_ + 8);
	}

    return dict;
}

AlifObject* new_dict(int64_t used){
    
    AlifDictObject* object = (AlifDictObject*)alifMem_objAlloc(
        sizeof(AlifDictObject));
    
    object->_base_.type_ = &_alifDictType_;

	ALIF_INCREF(object);

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

	ALIF_NEWREF(_key);
	ALIF_NEWREF(_value);

    if (_key->type_ == &_alifUStrType_) {
        hash = ((AlifUStrObject*)_key)->hash_;
    }
    else {
        hash = alifObject_hash(_key);
    }

    int64_t index = dict_lookupItem(_dict, _key, hash, &oldValue);

    if (index == -1) {

		int64_t size_ = _dict->size_;
		_dict = dict_resize(_dict);
		if (_dict == nullptr) {
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
    if (key->type_ == &_alifUStrType_) {
        hash = ((AlifUStrObject*)key)->hash_;
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
	if (value == nullptr) {
		*result = nullptr;
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
	if (!(op->type_ == &_alifDictType_)) {
		*result = nullptr;
		return -1;
	}

	size_t hash;
	if (key->type_ == &_alifUStrType_) {
		hash = ((AlifUStrObject*)key)->hash_;
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
    if (key->type_ == &_alifUStrType_) {
        hash = ((AlifUStrObject*)key)->hash_;
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
    if (key->type_ == &_alifUStrType_) {
        hash = ((AlifUStrObject*)key)->hash_;
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

bool alifDict_next(AlifObject* _dict, AlifSizeT * _popPos, AlifObject** _posKey, AlifObject** _posValue, AlifUSizeT *_posHash) {

    int64_t index{};
    AlifDictObject* map{};

    map = (AlifDictObject*)_dict;
    index = *_popPos;

    if (map->size_ <= index || index < 0) {
        return false;
    }

    AlifDictValues value = map->items_[index];

    *_popPos = index + 1;

    if (_posKey) {
        *_posKey = value.key;
    }
    if (_posValue) {
        *_posValue = value.value;
    }
    if (_posHash) {
        if (value.key->type_ == &_alifUStrType_) {
            *_posHash = ((AlifUStrObject*)value.key)->hash_;
        }
        else {
            *_posHash = alifObject_hash(value.key);
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
        return nullptr;
    }
    ix = dict_lookupItem(mp, key, hash, &old_value);
    if (ix == -1)
        return nullptr;
    if ( old_value == nullptr) {
        if (deflt) {
            return deflt;
        }
        return nullptr;
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
        return nullptr;
    }
    if (!(key->type_ == &_alifUStrType_) || (hash = ((AlifUStrObject*)key)->hash_) == 0) {
        hash = alifObject_hash(key);
        if (hash == -1)
            return nullptr;
    }
    return alifDict_popKnownHash(dict, key, hash, deflt);
}


static AlifObject* dict___contains__(AlifDictObject* self, AlifObject* key)
{
    register AlifDictObject* mp = self;
    size_t hash;
    int64_t ix;
    AlifObject* value;

    if (!(key->type_ == &_alifUStrType_) || (hash = ((AlifUStrObject*)key)->hash_) == 0) {
        hash = alifObject_hash(key);
        if (hash == -1)
            return nullptr;
    }
    ix = dict_lookupItem(mp, key, hash, &value);
    if ( value == nullptr)
        return ALIF_FALSE;
    return ALIF_TRUE;
}

static AlifObject* dict_get_impl(AlifDictObject* self, AlifObject* key, AlifObject* default_value);

static AlifObject* dict_get(AlifDictObject* self, AlifObject* const* args, int64_t nargs)
{
    AlifObject* return_value = nullptr;
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
    AlifObject* val = nullptr;
    size_t hash;
    int64_t ix;

    if (!(key->type_ == &_alifUStrType_) || (hash = ((AlifUStrObject*)key)->hash_) == 0) {
        hash = alifObject_hash(key);
        if (hash == -1)
            return nullptr;
    }
    ix = dict_lookupItem(self, key, hash, &val);
    if (ix == -1)
        return nullptr;
    if ( val == nullptr) {
        val = default_value;
    }
    return val;
}

static AlifObject* dict_popImpl(AlifDictObject* self, AlifObject* key, AlifObject* default_value);

static AlifObject* dict_pop(AlifDictObject* self, AlifObject* const* args, int64_t nargs)
{
    AlifObject* return_value = nullptr;
    AlifObject* key;
    AlifObject* default_value = nullptr;

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
//    if (res == nullptr)
//        return nullptr;
//    if (self->size_ == 0) {
//        return nullptr;
//    }
//    ///* Convert split table to combined table */
//    //if (self->ma_keys->dk_kind == DICT_KEYS_SPLIT) {
//    //    if (dictResize( self, DK_LOG_SIZE(self->ma_keys), 1)) {
//    //        return nullptr;
//    //    }
//    //}
//    /* Pop last item */
//    AlifObject* key, * value;
//    size_t hash;
//    if (DK_IS_UNICODE(self->ma_keys)) {
//        PyDictUnicodeEntry* ep0 = DK_UNICODE_ENTRIES(self->ma_keys);
//        i = self->ma_keys->dk_nentries - 1;
//        while (i >= 0 && ep0[i].me_value == nullptr) {
//            i--;
//        }
//
//        key = ep0[i].me_key;
//        new_version = _PyDict_NotifyEvent(
//            interp, PyDict_EVENT_DELETED, self, key, nullptr);
//        hash = unicode_get_hash(key);
//        value = ep0[i].me_value;
//        ep0[i].me_key = nullptr;
//        ep0[i].me_value = nullptr;
//    }
//    else {
//        PyDictKeyEntry* ep0 = DK_ENTRIES(self->ma_keys);
//        i = self->ma_keys->dk_nentries - 1;
//        while (i >= 0 && ep0[i].me_value == nullptr) {
//            i--;
//        }
//
//        key = ep0[i].me_key;
//        new_version = _PyDict_NotifyEvent(
//            interp, PyDict_EVENT_DELETED, self, key, nullptr);
//        hash = ep0[i].me_hash;
//        value = ep0[i].me_value;
//        ep0[i].me_key = nullptr;
//        ep0[i].me_hash = -1;
//        ep0[i].me_value = nullptr;
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

    if ((v->type_ !=& _alifDictType_) || (w->type_ != &_alifDictType_)) {
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

	if (_dict == nullptr || !(_dict->type_ == &_alifDictType_)) {
		return nullptr;
	}
	AlifDictObject* mp_ = (AlifDictObject*)_dict;
	AlifObject* v_{};
	int64_t n_{};

again:
	v_ = alifNew_list(n_);
	if (v_ == nullptr)
		return nullptr;

	int64_t j_ = 0, pos_ = 0;
	AlifObject* key_;
	while (alifDict_next((AlifObject*)mp_, &pos_, &key_, nullptr, nullptr)) {
		((AlifListObject*)v_)->items_[j_] = ALIF_NEWREF(key_);
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
    if (key->type_ == &_alifUStrType_) {
        hash = ((AlifUStrObject*)key)->hash_;
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

// للمراجعة
static AlifObject* dictNew_presized(AlifInterpreter* _interp, AlifSizeT _minUsed, bool _unicode) { // 2081
	const uint8_t log2MaxPresize = 17;
	const AlifSizeT maxPresize = ((AlifSizeT)1) << log2MaxPresize;
	uint8_t log2NewSize{};
	AlifDictKeysObject* newKeys{};

	if (_minUsed <= USABLE_FRACTION(ALIFDICT_MINSIZE)) {
		return alifNew_dict();
	}

	if (_minUsed > USABLE_FRACTION(maxPresize)) {
		log2NewSize = log2MaxPresize;
	}
	else {
		//log2NewSize = estimateLog2_keySize(_minUsed);
	}

	//newKeys = new_keys_object(_interp, log2NewSize, _unicode);
	//if (newKeys == nullptr) return nullptr;

	//return new_dict(_interp, newKeys, nullptr, 0, 0);
	return nullptr; // 
}

AlifObject* alifDict_fromItems(AlifObject* const* _keys, AlifSizeT _keysOffset,
	AlifObject* const* _values, AlifSizeT _valuesOffset, AlifSizeT _length) { // 2116

	bool unicode = true;
	AlifObject* const* ks = _keys;
	AlifInterpreter* interp = alifInterpreter_get();

	for (AlifSizeT i = 0; i < _length; i++) {
		if (!ALIFUSTR_CHECKEXACT(*ks)) {
			unicode = false;
			break;
		}
		ks += _keysOffset;
	}

	AlifObject* dict = dictNew_presized(interp, _length, unicode);
	if (dict == nullptr) {
		return nullptr;
	}

	ks = _keys;
	AlifObject* const* vs = _values;

	for (AlifSizeT i = 0; i < _length; i++) {
		AlifObject* key = *ks;
		AlifObject* value = *vs;
		if (setItem_lockHeld((AlifDictObject*)dict, key, value) < 0) {
			ALIF_DECREF(dict);
			return nullptr;
		}
		ks += _keysOffset;
		vs += _valuesOffset;
	}

	return dict;
}

// للمراجعة
static AlifIntT setItemTake2_lockHeld(AlifDictObject* mp, AlifObject* key, AlifObject* value) { // 2456

	AlifSizeT hash{};
	if (!ALIFUSTR_CHECKEXACT(key) /* or (hash = uStr_getHash(key)) == -1*/) {
		hash = alifObject_hash(key);
		if (hash == -1) {
			ALIF_DECREF(key);
			ALIF_DECREF(value);
			return -1;
		}
	}

	AlifInterpreter* interp = alifInterpreter_get();

	//if (mp->keys == ALIF_EMPTY_KEYS) {
	//	return insert_toEmptyDict(interp, mp, key, hash, value);
	//}

	//return insert_dict(interp, mp, key, hash, value);
	return 0; //
}

static AlifIntT setItem_lockHeld(AlifDictObject* mp, AlifObject* key, AlifObject* value) { // 2512
	return setItemTake2_lockHeld(mp, ALIF_NEWREF(key), ALIF_NEWREF(value));
}

static AlifMethodDef mappMethods[] = {
    {L"__contains__", (AlifCFunction)dict___contains__, METHOD_O | METHOD_COEXIST},
    {L"__getitem__", ALIFCFunction_CAST(dict_subscript), METHOD_O | METHOD_COEXIST},
    {L"get", ALIFCFunction_CAST(dict_get), METHOD_FASTCALL},
    {L"pop", ALIFCFunction_CAST(dict_pop), METHOD_FASTCALL},
    {nullptr,              nullptr}   /* sentinel */
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

AlifInitObject _alifDictType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
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
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC | ALIFTPFLAGS_BASETYPE |
	ALIFTPFLAGS_LIST_SUBCLASS | ALIFSUBTPFLAGS_MATCH_SELF | ALIFTPFLAGS_SEQUENCE,
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
