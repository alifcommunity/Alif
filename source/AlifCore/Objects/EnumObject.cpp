#include "alif.h"
#include "AlifCore_Call.h"
#include "AlifCore_Long.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"




class EnumObject { // 17
public:
	ALIFOBJECT_HEAD;
	AlifSizeT enIndex{};
	AlifObject* enSit{};   
	AlifObject* enResult{};      
	AlifObject* enLongIndex{};  
	AlifObject* one{};            
};


static AlifObject* enum_newImpl(AlifTypeObject* type,
	AlifObject* iterable, AlifObject* start) { // 44
	EnumObject* en{};

	en = (EnumObject*)type->alloc(type, 0);
	if (en == nullptr)
		return nullptr;
	if (start != nullptr) {
		start = alifNumber_index(start);
		if (start == nullptr) {
			ALIF_DECREF(en);
			return nullptr;
		}
		en->enIndex = alifLong_asSizeT(start);
		if (en->enIndex == -1 and alifErr_occurred()) {
			alifErr_clear();
			en->enIndex = ALIF_SIZET_MAX;
			en->enLongIndex = start;
		}
		else {
			en->enLongIndex = nullptr;
			ALIF_DECREF(start);
		}
	}
	else {
		en->enIndex = 0;
		en->enLongIndex = nullptr;
	}
	en->enSit = alifObject_getIter(iterable);
	if (en->enSit == nullptr) {
		ALIF_DECREF(en);
		return nullptr;
	}
	en->enResult = alifTuple_pack(2, ALIF_NONE, ALIF_NONE);
	if (en->enResult == nullptr) {
		ALIF_DECREF(en);
		return nullptr;
	}
	en->one = _alifLong_getOne();    /* borrowed reference */
	return (AlifObject*)en;
}

static AlifIntT check_keyword(AlifObject* kwnames,
	AlifIntT index, const char* name) { // 87
	AlifObject* kw = ALIFTUPLE_GET_ITEM(kwnames, index);
	//if (!_alifUStr_equalToASCIIString(kw, name)) {
	if (!alifUStr_equalToUTF8(kw, name)) {
		alifErr_format(_alifExcTypeError_,
			"'%S' معامل غير صحيح لـ تعداد()", kw);
		return 0;
	}
	return 1;
}


static AlifObject* enumerate_vectorCall(AlifObject* type,
	AlifObject* const* args, AlifUSizeT nargsf, AlifObject* kwnames) { // 100
	AlifTypeObject* tp = ALIFTYPE_CAST(type);
	AlifSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);
	AlifSizeT nkwargs = 0;
	if (kwnames != nullptr) {
		nkwargs = ALIFTUPLE_GET_SIZE(kwnames);
	}

	// Manually implement enumerate(iterable, start=...)
	if (nargs + nkwargs == 2) {
		if (nkwargs == 1) {
			if (!check_keyword(kwnames, 0, "البداية")) {
				return nullptr;
			}
		}
		else if (nkwargs == 2) {
			AlifObject* kw0 = ALIFTUPLE_GET_ITEM(kwnames, 0);
			if (_alifUStr_equalToASCIIString(kw0, "البداية")) {
				if (!check_keyword(kwnames, 1, "التكرار")) {
					return nullptr;
				}
				return enum_newImpl(tp, args[1], args[0]);
			}
			if (!check_keyword(kwnames, 0, "التكرار") or
				!check_keyword(kwnames, 1, "البداية")) {
				return nullptr;
			}

		}
		return enum_newImpl(tp, args[0], args[1]);
	}

	if (nargs + nkwargs == 1) {
		if (nkwargs == 1 and !check_keyword(kwnames, 0, "التكرار")) {
			return nullptr;
		}
		return enum_newImpl(tp, args[0], nullptr);
	}

	if (nargs == 0) {
		alifErr_setString(_alifExcTypeError_,
			"تعداد() معامل ضروري مفقود 'التكرار'");
		return nullptr;
	}

	alifErr_format(_alifExcTypeError_,
		"تعداد() يحتاج معاملين على الأكثر (المعطى %d)", nargs + nkwargs);
	return nullptr;
}



static void enum_dealloc(EnumObject* _en) { // 152
	alifObject_gcUnTrack(_en);
	ALIF_XDECREF(_en->enSit);
	ALIF_XDECREF(_en->enResult);
	ALIF_XDECREF(_en->enLongIndex);
	ALIF_TYPE(_en)->free(_en);
}


static AlifObject* enum_nextLong(EnumObject* _en,
	AlifObject* _nextItem) { // 171
	AlifObject* result = _en->enResult;
	AlifObject* next_index{};
	AlifObject* stepped_up{};
	AlifObject* old_index{};
	AlifObject* old_item{};

	if (_en->enLongIndex == nullptr) {
		_en->enLongIndex = alifLong_fromSizeT(ALIF_SIZET_MAX);
		if (_en->enLongIndex == nullptr) {
			ALIF_DECREF(_nextItem);
			return nullptr;
		}
	}
	next_index = _en->enLongIndex;
	stepped_up = alifNumber_add(next_index, _en->one);
	if (stepped_up == nullptr) {
		ALIF_DECREF(_nextItem);
		return nullptr;
	}
	_en->enLongIndex = stepped_up;

	if (ALIF_REFCNT(result) == 1) {
		ALIF_INCREF(result);
		old_index = ALIFTUPLE_GET_ITEM(result, 0);
		old_item = ALIFTUPLE_GET_ITEM(result, 1);
		ALIFTUPLE_SET_ITEM(result, 0, next_index);
		ALIFTUPLE_SET_ITEM(result, 1, _nextItem);
		ALIF_DECREF(old_index);
		ALIF_DECREF(old_item);
		if (!ALIFOBJECT_GC_IS_TRACKED(result)) {
			ALIFOBJECT_GC_TRACK(result);
		}
		return result;
	}
	result = alifTuple_new(2);
	if (result == nullptr) {
		ALIF_DECREF(next_index);
		ALIF_DECREF(_nextItem);
		return nullptr;
	}
	ALIFTUPLE_SET_ITEM(result, 0, next_index);
	ALIFTUPLE_SET_ITEM(result, 1, _nextItem);
	return result;
}

static AlifObject* enum_next(EnumObject* _en) { // 222
	AlifObject* next_index{};
	AlifObject* next_item{};
	AlifObject* result = _en->enResult;
	AlifObject* it = _en->enSit;
	AlifObject* old_index{};
	AlifObject* old_item{};

	next_item = (*ALIF_TYPE(it)->iterNext)(it);
	if (next_item == nullptr)
		return nullptr;

	if (_en->enIndex == ALIF_SIZET_MAX)
		return enum_nextLong(_en, next_item);

	next_index = alifLong_fromSizeT(_en->enIndex);
	if (next_index == nullptr) {
		ALIF_DECREF(next_item);
		return nullptr;
	}
	_en->enIndex++;

	if (ALIF_REFCNT(result) == 1) {
		ALIF_INCREF(result);
		old_index = ALIFTUPLE_GET_ITEM(result, 0);
		old_item = ALIFTUPLE_GET_ITEM(result, 1);
		ALIFTUPLE_SET_ITEM(result, 0, next_index);
		ALIFTUPLE_SET_ITEM(result, 1, next_item);
		ALIF_DECREF(old_index);
		ALIF_DECREF(old_item);
		if (!ALIFOBJECT_GC_IS_TRACKED(result)) {
			ALIFOBJECT_GC_TRACK(result);
		}
		return result;
	}
	result = alifTuple_new(2);
	if (result == nullptr) {
		ALIF_DECREF(next_index);
		ALIF_DECREF(next_item);
		return nullptr;
	}
	ALIFTUPLE_SET_ITEM(result, 0, next_index);
	ALIFTUPLE_SET_ITEM(result, 1, next_item);
	return result;
}



AlifTypeObject _alifEnumType_ = { // 290
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "تعداد",
	.basicSize = sizeof(EnumObject),
	/* methods */
	.dealloc = (Destructor)enum_dealloc,  
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE,
	//.traverse = (TraverseProc)enum_traverse,
	.iter = alifObject_selfIter,
	.iterNext = (IterNextFunc)enum_next,
	//.methods = _enumMethods_,
	.alloc = alifType_genericAlloc,
	//.new_ = enum_new,
	.free = alifObject_gcDel,
	.vectorCall = (VectorCallFunc)enumerate_vectorCall
};


/* ------------------ Reversed Object ------------------ */


class ReversedObject {
public:
	ALIFOBJECT_HEAD{};
	AlifSizeT index{};
	AlifObject* seq{};
};


static AlifObject* reversed_newImpl(AlifTypeObject* _type, AlifObject* _seq) {
	AlifSizeT n{};
	AlifObject* reversedMeth{};
	ReversedObject* ro{};

	reversedMeth = _alifObject_lookupSpecial(_seq, &ALIF_STR(__reversed__));
	if (reversedMeth == ALIF_NONE) {
		ALIF_DECREF(reversedMeth);
		alifErr_format(_alifExcTypeError_,
			"'%.200s' الكائن غير قابل للعكس",
			ALIF_TYPE(_seq)->name);
		return nullptr;
	}
	if (reversedMeth != nullptr) {
		AlifObject *res = _alifObject_callNoArgs(reversedMeth);
		ALIF_DECREF(reversedMeth);
		return res;
	}
	else if (alifErr_occurred())
		return nullptr;

	if (!alifSequence_check(_seq)) {
		alifErr_format(_alifExcTypeError_,
			"'%.200s' الكائن غير قابل للعكس",
			ALIF_TYPE(_seq)->name);
		return nullptr;
	}

	n = alifSequence_size(_seq);
	if (n == -1)
		return nullptr;

	ro = (ReversedObject*)_type->alloc(_type, 0);
	if (ro == nullptr)
		return nullptr;

	ro->index = n-1;
	ro->seq = ALIF_NEWREF(_seq);
	return (AlifObject*)ro;
}


static AlifObject* reversed_vectorCall(AlifObject* _type, AlifObject* const* _args,
	AlifUSizeT _nargsf, AlifObject* _kwNames) {
	if (!_ALIFARG_NOKWNAMES("معكوس", _kwNames)) {
		return nullptr;
	}

	AlifSizeT nargs = ALIFVECTORCALL_NARGS(_nargsf);
	if (!_ALIFARG_CHECKPOSITIONAL("معكوس", nargs, 1, 1)) {
		return nullptr;
	}

	return reversed_newImpl(ALIFTYPE_CAST(_type), _args[0]);
}






static void reversed_dealloc(ReversedObject* _ro) {
	alifObject_gcUnTrack(_ro);
	ALIF_XDECREF(_ro->seq);
	ALIF_TYPE(_ro)->free(_ro);
}




static AlifObject* reversed_next(ReversedObject* _ro) {
	AlifObject* item{};
	AlifSizeT index = _ro->index;

	if (index >= 0) {
		item = alifSequence_getItem(_ro->seq, index);
		if (item != nullptr) {
			_ro->index--;
			return item;
		}
		if (alifErr_exceptionMatches(_alifExcIndexError_) or
			alifErr_exceptionMatches(_alifExcStopIteration_))
			alifErr_clear();
	}
	_ro->index = -1;
	ALIF_CLEAR(_ro->seq);
	return nullptr;
}






AlifTypeObject _alifReversedType_ = {
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "معكوس",
	.basicSize = sizeof(ReversedObject),
	/* methods */
	.dealloc = (Destructor)reversed_dealloc,
	.getAttro = alifObject_genericGetAttr,
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC |
	ALIF_TPFLAGS_BASETYPE,
	//.traverse = (TraverseProc)reversed_traverse,
	.iter = alifObject_selfIter,
	.iterNext = (IterNextFunc)reversed_next,
	//.methods = _reversedIterMethods_,
	.alloc = alifType_genericAlloc,
	//.new_ = reversed_new,
	.free = alifObject_gcDel,
	.vectorCall = (VectorCallFunc)reversed_vectorCall,
};
