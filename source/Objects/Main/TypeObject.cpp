#include "alif.h"

#include "alifCore_Abstract.h"
#include "AlifCore_Call.h"
#include "AlifCore_Code.h"
#include "AlifCore_Dict.h"
#include "AlifCore_Frame.h"
#include "AlifCore_Integer.h"
#include "AlifCore_Memory.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_AlifState.h"
#include "AlifCore_SymTable.h"
#include "AlifCore_TypeObject.h"
#include "AlifCore_UString.h"

#include "OpCode.h"

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

AlifObject* alifNew_type(AlifInitObject* metatype, AlifObject* args, AlifObject* kwds)
{

    AlifObject* name, * bases, * origDict;
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


AlifObject* alifType_allocNoTrack(AlifTypeObject* type, AlifSizeT nitems) { // 1936
	AlifObject* obj;
	size_t size = alifSubObject_varSize(type, nitems + 1);

	const AlifUSizeT presize = alifSubType_preHeaderSize(type);
	if (type->flags_ & ALIFTPFLAGS_INLINE_VALUES) {
		size += alifInline_valuesSize(type);
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
	if (ALIFTYPE_IS_GC(type)) {
		alifSubObjectGC_link(obj);
	}
	memset(obj, '\0', size);

	if (type->itemSize == 0) {
		alifSubObject_init(obj, type);
	}
	else {
		alifSubObject_initVar((AlifVarObject*)obj, type, nitems);
	}
	if (type->flags_ & ALIFTPFLAGS_INLINE_VALUES) {
		//alifObject_initInlineValues(obj, type);
	}
	return obj;
}

AlifObject* alifType_genericAlloc(AlifTypeObject* type, AlifSizeT nitems) { // 1979
	AlifObject* obj = alifType_allocNoTrack(type, nitems);
	if (obj == nullptr) {
		return nullptr;
	}

	if (ALIFTYPE_IS_GC(type)) {
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
