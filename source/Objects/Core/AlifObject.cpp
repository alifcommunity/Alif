#include "alif.h"

#include "AlifCore_Object.h"
#include "AlifCore_Memory.h"

void alif_incRef(AlifObject* _o)
{
	ALIF_XINCREF(_o);
}

void alif_decRef(AlifObject* _o)
{
	ALIF_XDECREF(_o);
}

void alifSub_incRef(AlifObject* _o)
{
	ALIF_INCREF(_o);
}

void alifSub_decRef(AlifObject* _o)
{
	ALIF_DECREF(_o);
}

void alif_setImmortal(AlifObject* object)
{
	if (object) {
		object->ref_ = 0xffff;
	}
}

AlifObject* alifObject_init(AlifObject* _op, AlifInitObject* _type) {

	if (_op == nullptr) {
		std::wcout << L"لا يمكن تهيئة كائن غير موجود\n" << std::endl;
		exit(-1);
	}

	alifSubObject_init(_op, _type);

	return _op;
}

AlifVarObject* alifObject_InitVar(AlifVarObject* _op, AlifTypeObject* _tp, int64_t _size)
{
	if (_op == nullptr) {
		std::wcout << L"لا يمكن تهيئة كائن غير موجود\n" << std::endl;
		exit(-1);
	}

	alifSubObject_initVar(_op, _tp, _size);
	return _op;
}

AlifObject* alifObject_new(AlifTypeObject* _tp)
{
	AlifObject* op_ = (AlifObject*)alifMem_objAlloc(_tp->basicSize);
	if (op_ == nullptr) {
		std::wcout << L"لا يمكن تهيئة كائن غير موجود\n" << std::endl;
		exit(-1);
	}
	alifSubObject_init(op_, _tp);
	return op_;
}

AlifVarObject* alifSubObject_newVar(AlifTypeObject* _tp, int64_t _nItems)
{
	AlifVarObject* op_;
	const size_t size_ = alifSubObject_varSize(_tp, _nItems);
	op_ = (AlifVarObject*)alifMem_objAlloc(size_);
	if (op_ == nullptr) {
		std::wcout << L"لا يمكن تهيئة كائن غير موجود\n" << std::endl;
		exit(-1);
	}
	alifSubObject_initVar(op_, _tp, _nItems);
	return op_;
}

int _alifSwappedOp_[] = { ALIF_GT, ALIF_GE, ALIF_EQ, ALIF_NE, ALIF_LT, ALIF_LE };

static const wchar_t* const _opstrings_[] = { L"<", L"<=", L"==", L"!=", L">", L">=" };

static AlifObject* do_richCompare(AlifObject* _v, AlifObject* _w, int _op)
{
	RichCmpFunc f_;
	AlifObject* res_;
	int checkedReverseOp = 0;

	if (!ALIF_IS_TYPE(_v, ALIF_TYPE(_w)) &&
		//alifType_isSubtype(ALIF_TYPE(_w), ALIF_TYPE(_v)) &&
		(f_ = ALIF_TYPE(_w)->richCompare) != nullptr) {
		checkedReverseOp = 1;
		res_ = (*f_)(_w, _v, _alifSwappedOp_[_op]);
		//if (res_ != ALIF_NOTIMPLEMENT)
			//return res_;
		ALIF_DECREF(res_);
	}
	if ((f_ = ALIF_TYPE(_v)->richCompare) != nullptr) {
		res_ = (*f_)(_v, _w, _op);
		//if (res_ != ALIF_NOTIMPLEMENT)
			//return res_;
		ALIF_DECREF(res_);
	}
	if (!checkedReverseOp && (f_ = ALIF_TYPE(_w)->richCompare) != nullptr) {
		res_ = (*f_)(_w, _v, _alifSwappedOp_[_op]);
		//if (res_ != ALIF_NOTIMPLEMENT)
			//return res_;
		ALIF_DECREF(res_);
	}
	/* If neither object implements it, provide a sensible default
	   for == and !=, but raise an exception for ordering. */
	switch (_op) {
	case ALIF_EQ:
		res_ = (_v == _w) ? ALIF_TRUE : ALIF_FALSE;
		break;
	case ALIF_NE:
		res_ = (_v != _w) ? ALIF_TRUE : ALIF_FALSE;
		break;
	default:
		//Err_Format(tstate, Exc_TypeError,
			//"'%s' not supported between instances of '%.100s' and '%.100s'",
			//_opstrings_[op],
			//ALIF_TYPE(v)->name,
			//ALIF_TYPE(w)->name);
		return nullptr;
	}
	return ALIF_NEWREF(res_);
}

AlifObject* alifObject_richCompare(AlifObject* _v, AlifObject* _w, int _op)
{

	if (_v == nullptr || _w == nullptr) {

		return nullptr;
	}
	AlifObject* res_ = do_richCompare( _v, _w, _op);
	return res_;
}

int alifObject_richCompareBool(AlifObject* _v, AlifObject* _w, int _op)
{
	AlifObject* res_;
	int ok_;

	if (_v == _w) {
		if (_op == ALIF_EQ)
			return 1;
		else if (_op == ALIF_NE)
			return 0;
	}

	res_ = alifObject_richCompare(_v, _w, _op);
	if (res_ == nullptr)
		return -1;
	if (res_->type_ == &typeBool)
		ok_ = (res_ == ALIF_TRUE);
	else
		ok_ = alifObject_isTrue(res_);
	ALIF_DECREF(res_);
	return ok_;
}

int64_t alifObject_hashNotImplemented(AlifObject* _v)
{
	//Err_Format(Exc_TypeError, "unhashable type: '%.200s'",
		//ALIF_TYPE(_v)->name_);
	return -1;
}

int64_t alifObject_hash(AlifObject* _v)
{
	AlifTypeObject* tp_ = ALIF_TYPE(_v);
	if (tp_->hash_ != nullptr)
		return (*tp_->hash_)(_v);

	//if (!alifType_isReady(tp)) {
		//if (alifType_ready(tp) < 0)
			//return -1;
		//if (tp_->hash_ != nullptr)
			//return (*tp_->hash_)(_v);
	//}
	return alifObject_hashNotImplemented(_v);
}

AlifObject* alifObject_getAttrString(AlifObject* _v, const wchar_t* _name)
{
	AlifObject* w_{}, * res_{};

	if (ALIF_TYPE(_v)->getAttr != nullptr)
		return (*ALIF_TYPE(_v)->getAttr)(_v, (wchar_t*)_name);
	w_ = alifUStr_decodeUTF8Stateful(_name, wcslen(_name), nullptr, nullptr);
	if (w_ == nullptr)
		return NULL;
	//res_ = alifObject_getAttr(_v, w_);
	ALIF_DECREF(w_);
	return res_;
}

AlifObject** alifSubObject_computedDictPointer(AlifObject* _obj)
{
	AlifTypeObject* tp_ = ALIF_TYPE(_obj);

	int64_t dictOffset = tp_->dictOffset;
	if (dictOffset == 0) {
		return nullptr;
	}

	if (dictOffset < 0) {

		int64_t tsize_ = ALIF_SIZE(_obj);
		if (tsize_ < 0) {
			tsize_ = -tsize_;
		}
		size_t size_ = alifSubObject_varSize(tp_, tsize_);
		dictOffset += (int64_t)size_;

	}
	return (AlifObject**)((char*)_obj + dictOffset);
}

AlifObject* alifObject_selfIter(AlifObject* _obj)
{
	return ALIF_NEWREF(_obj);
}

int alifObject_isTrue(AlifObject* _v)
{
	int64_t res_;
	if (_v == ALIF_TRUE)
		return 1;
	if (_v == ALIF_FALSE)
		return 0;
	if (_v == ALIF_NONE)
		return 0;
	else if (ALIF_TYPE(_v)->asNumber != nullptr &&
		ALIF_TYPE(_v)->asNumber->boolean_ != nullptr)
		res_ = (*ALIF_TYPE(_v)->asNumber->boolean_)(_v);
	else if (ALIF_TYPE(_v)->asMapping != nullptr &&
		ALIF_TYPE(_v)->asMapping->length_ != nullptr)
		res_ = (*ALIF_TYPE(_v)->asMapping->length_)(_v);
	else if (ALIF_TYPE(_v)->asSequence != nullptr &&
		ALIF_TYPE(_v)->asSequence->length_ != nullptr)
		res_ = (*ALIF_TYPE(_v)->asSequence->length_)(_v);
	else
		return 1;
	/* if it is negative, it should be either -1 or -2 */
	return (res_ > 0) ? 1 : res_;
}

int alifObject_not(AlifObject* _v)
{
	int res_;
	res_ = alifObject_isTrue(_v);
	if (res_ < 0)
		return res_;
	return res_ == 0;
}

int alifCallable_check(AlifObject* _x)
{
	if (_x == nullptr)
		return 0;
	return ALIF_TYPE(_x)->call_ != nullptr;
}

static AlifObject* none_repr(AlifObject* _op)
{
	return alifUStr_decodeUTF8Stateful(L"None", wcslen(L"None"), nullptr, nullptr);
}

static void none_dealloc(AlifObject* _none)
{

	alifSub_setImmortal(_none);
}

static AlifObject* none_new(AlifTypeObject* _type, AlifObject* _args, AlifObject* _kwargs)
{
	if (((AlifVarObject*)_args)->size_ || (_kwargs && ((AlifDictObject*)_kwargs)->size_)) {
		return NULL;
	}
	ALIF_RETURN_NONE;
}

static int none_bool(AlifObject* _v)
{
	return 0;
}

size_t none_hash(AlifObject* v)
{
	// تم تحديد ترميز القيم true = 1 , false = 2 , none = 3 هكذا 
	return 3;
}


AlifTypeObject _alifNotImplementedType_ = { // need review

	0,// for var obj
	0,// for var obj
	0,// fro var obj
	L"NotImplemented",
	0,
	0,
	none_dealloc, //
	0,
	0,
	0,
	0,
	& _noneAsNumber_, // 
	0,
	0,
	(HashFunc)none_hash, // 
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
	0,
	0,
	0,
	0,
	none_new, // 
};

AlifObject _alifNotImplemented_ = ALIFOBJECT_HEAD_INIT(&_alifNotImplementedType_); // need review


AlifNumberMethods _noneAsNumber_ = {
	0,                   
	0,                        
	0,                        
	0,                         
	0,                      
	0,                     
	0,                        
	0,                        
	0,                        
	(Inquiry)none_bool,         
};

AlifTypeObject _typeNone_ = {
	
	0,// for var obj
	0,// for var obj
	0,// fro var obj
	L"NoneType",
	0,
	0,
	none_dealloc,  
	0,                  
	0,             
	0,             
	0,  
	& _noneAsNumber_,
	0,                 
	0,                
	(HashFunc)none_hash,
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
	0,             
	0,              
	0,        
	0,         
	none_new,
};

AlifObject _alifNoneStruct_ = ALIFOBJECT_HEAD_INIT(&_typeNone_);

static inline void new_reference(AlifObject* _op)
{
	_op->ref_ = 1;
}

void alifSub_newReference(AlifObject* _op)
{
	new_reference(_op);
}

void alifSub_setImmortalUntracked(AlifObject* _op)
{
	_op->ref_ = ALIF_IMMORTAL_REFCENT;
}


void alifSub_setImmortal(AlifObject* _op)
{
	//if (alifObject_IS_GC(op) && alifSubObject_GC_IS_TRACKED(_op)) {
		//alifSubObject_GC_UNTRACK(_op);
	//}
	alifSub_setImmortalUntracked(_op);
}

void alifSub_dealloc(AlifObject* _op)
{
	AlifTypeObject* type_ = ALIF_TYPE(_op);
	Destructor dealloc = type_->dealloc_;

	(*dealloc)(_op);

}

AlifObject* alif_newRef(AlifObject* _obj)
{
	return alifSub_newRef(_obj);
}

AlifObject* alif_xNewRef(AlifObject* _obj)
{
	return alifSub_xNewRef(_obj);
}

int alif_is(AlifObject* _x, AlifObject* _y)
{
	return (_x == _y);
}

int alif_isNone(AlifObject* _x)
{
	return alif_is(_x, ALIF_NONE);
}

int alif_isTrue(AlifObject* _x)
{
	return alif_is(_x, ALIF_TRUE);
}

int alif_isFalse(AlifObject* _x)
{
	return alif_is(_x, ALIF_FALSE);
}

void alifSub_setRefcnt(AlifObject* _ob, int64_t _ref)
{
	ALIF_SETREF(_ob, _ref);
}
