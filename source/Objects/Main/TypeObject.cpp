#include "alif.h"

#include "alifCore_Abstract.h"
#include "AlifCore_Call.h"
#include "AlifCore_Code.h"
#include "AlifCore_Dict.h"
#include "AlifCore_Frame.h"
#include "AlifCore_Integer.h"
#include "AlifCore_Memory.h"
#include "AlifCore_Lock.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_SymTable.h"
#include "AlifCore_TypeObject.h"
#include "AlifCore_UString.h"
#include "OpCode.h"


#define MCACHE_MAX_ATTR_SIZE    100
#define MCACHE_HASH(version, nameHash)                                 \
        (((unsigned int)(version) ^ (unsigned int)(nameHash))          \
         & ((1 << MCACHE_SIZE_EXP) - 1))

#define MCACHE_HASH_METHOD(type, name)                                  \
    MCACHE_HASH((type)->versionTag, ((int64_t)(name)) >> 3)
#define MCACHE_CACHEABLE_NAME(name)                             \
        ALIFUSTR_CHECK(name) &&                           \
        (ALIFUSTR_GET_LENGTH(name) <= MCACHE_MAX_ATTR_SIZE)
#define NEXT_GLOBALVERSION_TAG _alifDureRun_.types.nextVersionTag
#define NEXT_VERSION_TAG(interp) \
    (interp)->types.nextVersionTag


static inline size_t staticBuiltin_indexGet(AlifTypeObject* _self)
{

	return (size_t)_self->subclasses_ - 1;
}

static inline StaticBuiltinState* staticBuiltin_stateGet(AlifInterpreter* _interp, AlifTypeObject* _self)
{
	return &(_interp->types.builtins[staticBuiltin_indexGet(_self)]);
}

StaticBuiltinState* alifStaticType_getState(AlifInterpreter* _interp, AlifTypeObject* _self)
{
	return staticBuiltin_stateGet(_interp, _self);
}

static inline int is_readying(AlifTypeObject* _type)
{
	if (_type->flags_ & ALIFSUBTPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp_ = alifInterpreter_get();
		StaticBuiltinState* state_ = staticBuiltin_stateGet(interp_, _type);
		return state_->readying;
	}
	return (_type->flags_ & ALIFTPFLAGS_READYING) != 0;
}

static inline AlifObject* lookupType_dict(AlifTypeObject* _self)
{
	if (_self->flags_ & ALIFSUBTPFLAGS_STATIC_BUILTIN) {
		AlifInterpreter* interp_ = alifInterpreter_get();
		StaticBuiltinState* state_ = alifStaticType_getState(interp_, _self);
		return state_->dict;
	}
	return _self->dict_;
}

static inline AlifObject* lookupType_bases(AlifTypeObject* _self)
{
	return _self->bases_;
}

static inline AlifObject* lookupType_mro(AlifTypeObject* _self)
{
	return _self->mro_;
}

static class TypeCache* get_type_cache(void)
{
	AlifInterpreter* interp_ = alifInterpreter_get();
	return &interp_->types.typeCache;
}

#define MAX_VERSIONS_PER_CLASS 1000

static int assign_version_tag(AlifInterpreter* _interp, AlifTypeObject* _type)
{
	if (alifSubType_hasFeature(_type, ALIFTPFLAGS_VALID_VERSION_TAG)) {
		return 1;
	}
	if (!alifSubType_hasFeature(_type, ALIFTPFLAGS_READY)) {
		return 0;
	}
	if (_type->versionsUsed >= MAX_VERSIONS_PER_CLASS) {
		return 0;
	}
	_type->versionsUsed++;
	if (_type->flags_ & ALIFTPFLAGS_IMMUTABLETYPE) {
		if (NEXT_GLOBALVERSION_TAG > ALIFMAX_GLOBALTYPE_VERSIONTAG) {
			return 0;
		}
		_type->versionTag = NEXT_GLOBALVERSION_TAG++;
	}
	else {
		if (NEXT_VERSION_TAG(_interp) == 0) {
			return 0;
		}
		_type->versionTag = NEXT_VERSION_TAG(_interp)++;
	}

	AlifObject* bases = lookupType_bases(_type);
	int64_t n = ALIFTUPLE_GET_SIZE(bases);
	for (int64_t i = 0; i < n; i++) {
		AlifObject* b = ALIFTUPLE_GET_ITEM(bases, i);
		if (!assign_version_tag(_interp, (AlifTypeObject*)b))
			return 0;
	}
	_type->flags_ |= ALIFTPFLAGS_VALID_VERSION_TAG;
	return 1;
}


class TypeNew {
public:
    AlifInitObject* metatype;
    AlifObject* args;
    AlifObject* kwds;
    AlifObject* origDict;
    AlifObject* name;
    AlifObject* bases;
    AlifInitObject* base;
    AlifObject* slots;
    int64_t nSlot;
    int addDict;
    int addWeak;
    int mayAddDict;
    int mayAddWeak;
};

AlifObject* alifNew_type(AlifInitObject* _metatype, AlifObject* _args, AlifObject* _kwds)
{

    AlifObject* name_, * bases_, * origDict;
    //if (!alifArg_ParseTuple(args, "UO!O!:type_.__new__",
    //    &name,
    //    &alifTuple_Type, &bases,
    //    &alifDict_Type, &orig_dict))
    //{
    //    return nullptr;
    //}

    //TypeNew ctx = {
    //    metatype,
    //    args,
    //    kwds,
    //    origDict,
    //    name,
    //    bases,
    //    nullptr,
    //    nullptr,
    //    0,
    //    0,
    //    0,
    //    0,
    //    0 
    //};
    AlifObject* type_ = nullptr;
    //int res = type_new_get_bases(&ctx, &type_);
    //if (res < 0) {
        //return nullptr;
    //}
    //if (res == 1) {
        //return type_;
    //}

    //type_ = type_new_impl(&ctx);
    return type_;
}

static AlifObject* findName_inMro(AlifTypeObject* _type, AlifObject* _name, int* _error)
{
	size_t hash_;
	if (!ALIFUSTR_CHECK(_name))
	{
		hash_ = alifObject_hash(_name);
		if (hash_ == -1) {
			*_error = -1;
			return NULL;
		}
	}

	AlifObject* mro_ = lookupType_mro(_type);
	if (mro_ == NULL) {
		if (!is_readying(_type)) {
			if (alifType_ready(_type) < 0) {
				*_error = -1;
				return NULL;
			}
			mro_ = lookupType_mro(_type);
		}
		if (mro_ == NULL) {
			*_error = 1;
			return NULL;
		}
	}

	AlifObject* res_ = NULL;

	ALIF_INCREF(mro_);
	int64_t n_ = ALIFTUPLE_GET_SIZE(mro_);
	for (int64_t i_ = 0; i_ < n_; i_++) {
		AlifObject* base_ = ALIFTUPLE_GET_ITEM(mro_, i_);
		AlifObject* dict_ = lookupType_dict(((AlifTypeObject*)base_));
		res_ = alifDictGetItem_knownHash(dict_, _name, hash_);
		if (res_ != NULL) {
			break;
		}

	}
	*_error = 0;
done:
	ALIF_DECREF(mro_);
	return res_;
}

static void update_cache(class TypeCacheEntry* _entry, AlifObject* _name, unsigned int _versionTag, AlifObject* _value)
{
	_entry->version_ = _versionTag;
	_entry->value_ = _value;  /* borrowed */
	//OBJECT_STAT_INC_COND(typeCacheCollisions, _entry->name_ != ALIF_NONE && _entry->name_ != _name);
	ALIF_SETREF(_entry->name_, ALIF_NEWREF(_name));
}

AlifObject* alifType_lookup(AlifTypeObject* _type, AlifObject* _name)
{
	AlifObject* res_;
	int error_;
	AlifInterpreter* interp_ = alifInterpreter_get();

	unsigned int h_ = MCACHE_HASH_METHOD(_type, _name);
	class TypeCache* cache_ = get_type_cache();
	class TypeCacheEntry* entry_ = &cache_->hashtable[h_];
	if (entry_->version_ == _type->versionTag &&
		entry_->name_ == _name) {
		//OBJECT_STAT_INC_COND(type_cache_hits, !is_dunder_name(name));
		//OBJECT_STAT_INC_COND(type_cache_dunder_hits, is_dunder_name(name));
		return entry_->value_;
	}
	//OBJECT_STAT_INC_COND(type_cache_misses, !is_dunder_name(name));
	//OBJECT_STAT_INC_COND(type_cache_dunder_misses, is_dunder_name(name));

	int hasVersion = 0;
	int version_ = 0;
	//BEGIN_TYPE_LOCK()
	res_ = findName_inMro(_type, _name, &error_);
	if (MCACHE_CACHEABLE_NAME(_name)) {
		hasVersion = assign_version_tag(interp_, _type);
		version_ = _type->versionTag;
	}
	//END_TYPE_LOCK()

	if (error_) {

		if (error_ == -1) {
			return nullptr;
		}
		return NULL;
	}

	if (hasVersion) {
		update_cache(entry_, _name, version_, res_);
	}
	return res_;
}

AlifObject* alifType_allocNoTrack(AlifTypeObject* _type, AlifSizeT nitems) { // 1936
	AlifObject* obj;
	size_t size = alifSubObject_varSize(_type, nitems + 1);

	const AlifUSizeT presize = alifSubType_preHeaderSize(_type);
	if (_type->flags_ & ALIFTPFLAGS_INLINE_VALUES) {
		size += alifInline_valuesSize(_type);
	}
	char* alloc = (char*)alifMem_objAlloc(size + presize);
	if (alloc == nullptr) {
		//return alifErr_noMemory();
		return nullptr; //
	}
	obj = (AlifObject*)(alloc + presize);
	if (presize) {
		((AlifObject**)alloc)[0] = nullptr;
		((AlifObject**)alloc)[1] = nullptr;
	}
	if (ALIFTYPE_IS_GC(_type)) {
		alifSubObjectGC_link(obj);
	}
	memset(obj, '\0', size);

	if (_type->itemSize == 0) {
		alifSubObject_init(obj, _type);
	}
	else {
		alifSubObject_initVar((AlifVarObject*)obj, _type, nitems);
	}
	if (_type->flags_ & ALIFTPFLAGS_INLINE_VALUES) {
		//alifObject_initInlineValues(obj, _type);
	}
	return obj;
}

AlifObject* alifType_genericAlloc(AlifTypeObject* _type, AlifSizeT nitems) { // 1979
	AlifObject* obj = alifType_allocNoTrack(_type, nitems);
	if (obj == nullptr) {
		return nullptr;
	}

	if (ALIFTYPE_IS_GC(_type)) {
		ALIFOBJECT_GC_TRACK(obj);
	}
	return obj;
}

static AlifIntT typeIsSubType_baseChain(AlifTypeObject* _a, AlifTypeObject* _b) { // 2355
	do {
		if (_a == _b)
			return 1;
		_a = _a->base_;
	} while (_a != nullptr);

	return (_b == &_alifBaseObjectType_);
}

static AlifIntT isSubType_withMro(AlifObject* _mro, AlifTypeObject* _a, AlifTypeObject* _b) { // 2367
	AlifIntT res{};
	if (_mro != nullptr) {
		AlifSizeT i, n;
		n = ALIFTUPLE_GET_SIZE(_mro);
		res = 0;
		for (i = 0; i < n; i++) {
			if (ALIFTUPLE_GET_ITEM(_mro, i) == (AlifObject*)_b) {
				res = 1;
				break;
			}
		}
	}
	else {
		/* a is not completely initialized yet; follow tp_base */
		res = typeIsSubType_baseChain(_a, _b);
	}
	return res;
}

AlifIntT alifType_isSubType(AlifTypeObject* a, AlifTypeObject* b) { // 2392
	return isSubType_withMro(a->mro_, a, b);
}

AlifInitObject _alifTypeType_ = {
    ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
    L"type",                                     
    sizeof(AlifHeapTypeObject),                  
    sizeof(AlifMemberDef),                        
    0,//estructor)type_dealloc,                   
    offsetof(AlifInitObject, vectorCall),      
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
    0,
    0,
    0,
    0,
    0,
    offsetof(AlifInitObject, weakList),       
    0,                                         
    0,                                          
    0,                            
    0,                             
    0,                             
    0,                                         
    0,                                         
    0,                                          
    0,                                          
    offsetof(AlifInitObject, dict_),           
    0,                                
    0,                          
    0,                               
    0,                            
    0,                      
    //.tp_vectorcall = type_vectorcall,
};










AlifObject* alifType_getAttroImpl(AlifTypeObject* type, AlifObject* name, AlifIntT* suppress_missing_attribute) { // 5289
	AlifTypeObject* metatype = ALIF_TYPE(type);
	AlifObject* metaAttribute{}, * attribute{};
	DescrGetFunc metaGet{};
	AlifObject* res{};

	if (!ALIFUSTR_CHECK(name)) {
		//alifErr_format(alifExcTypeError,
		//	"attribute name must be string, not '%.200s'",
		//	ALIF_TYPE(name)->name_);
		return nullptr;
	}

	//if (!alifType_isReady(type)) {
	//	if (alifType_ready(type) < 0)
	//		return nullptr;
	//}

	metaGet = nullptr;

	//meta_attribute = alifType_lookupRef(metatype, name);

	if (metaAttribute != nullptr) {
		metaGet = ALIF_TYPE(metaAttribute)->descrGet;

		if (metaGet != nullptr and alifDescr_isData(metaAttribute)) {

			res = metaGet(metaAttribute, (AlifObject*)type,
				(AlifObject*)metatype);
			ALIF_DECREF(metaAttribute);
			return res;
		}
	}

	//attribute = alifType_lookupRef(type, name);
	if (attribute != nullptr) {
		DescrGetFunc local_get = ALIF_TYPE(attribute)->descrGet;

		ALIF_XDECREF(metaAttribute);

		if (local_get != nullptr) {
			res = local_get(attribute, (AlifObject*)nullptr,
				(AlifObject*)type);
			ALIF_DECREF(attribute);
			return res;
		}

		return attribute;
	}

	if (metaGet != nullptr) {
		AlifObject* res;
		res = metaGet(metaAttribute, (AlifObject*)type,
			(AlifObject*)metatype);
		ALIF_DECREF(metaAttribute);
		return res;
	}

	if (metaAttribute != nullptr) {
		return metaAttribute;
	}

	/* Give up */
	if (suppress_missing_attribute == nullptr) {
		//alifErr_format(alifExcAttributeError,
		//	"type object '%.100s' has no attribute '%U'",
		//	type->name_, name);
	}
	else {
		*suppress_missing_attribute = 1;
	}
	return nullptr;
}


AlifObject* alifType_getAttro(AlifObject* type, AlifObject* name) { // 5381
	return alifType_getAttroImpl((AlifTypeObject*)type, name, nullptr);
}

int alifType_ready(AlifTypeObject* _type) // 7906
{
	if (_type->flags_ & ALIFTPFLAGS_READY) {
		return 0;
	}

	if (!(_type->flags_ & ALIFTPFLAGS_HEAPTYPE)) {
		_type->flags_ |= ALIFTPFLAGS_IMMUTABLETYPE;
		alifSub_setImmortalUntracked((AlifObject*)_type);
	}

	int res;
	//BEGIN_TYPE_LOCK()
		if (!(_type->flags_ & ALIFTPFLAGS_READY)) {
			//res = type_ready(_type, 0);
		}
		else {
			res = 0;
		}
	//END_TYPE_LOCK()
		return res;
}







AlifTypeObject _alifBaseObjectType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	L"object",              
	sizeof(AlifObject),     
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
	0,                      
	0,                    
	0,                    
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_BASETYPE,  
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
	0,              
	0,              
	0,              
	0,              
	0,              
	0,              
	0,              
};
