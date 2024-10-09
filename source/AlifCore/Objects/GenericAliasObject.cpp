#include "alif.h"

#include "AlifCore_Eval.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"
#include "AlifCore_UStrObject.h"


class GenericAliasObject { // 13
public:
	ALIFOBJECT_HEAD;
	AlifObject* origin{};
	AlifObject* args{};
	AlifObject* parameters{};
	AlifObject* weakRefList{};
	bool starred{};
	VectorCallFunc vectorCall{};
};




static inline AlifObject* set_origClass(AlifObject* _obj, AlifObject* _self) { // 534
	if (_obj != nullptr) {
		if (alifObject_setAttr(_obj, &ALIF_ID(__origClass__), _self) < 0) {
			//if (!alifErr_exceptionMatches(_alifExcAttributeError_) and
			//	!alifErr_exceptionMatches(_alifExcTypeError_))
			//{
			//	ALIF_DECREF(obj);
			//	return nullptr;
			//}
			//alifErr_clear();
		}
	}
	return _obj;
}

static AlifObject* genericAlias_vectorCall(AlifObject* _self,
	AlifObject* const* _args, AlifUSizeT _nargsf, AlifObject* _kwnames) { // 559

	GenericAliasObject* alias_ = (GenericAliasObject*)_self;
	AlifObject* obj_ = alifVectorCall_function(alias_->origin)(alias_->origin, _args, _nargsf, _kwnames);
	return set_origClass(obj_, _self);
}

static inline AlifIntT setup_genericAlias(GenericAliasObject* _alias,
	AlifObject* _origin, AlifObject* _args) { // 765
	if (!ALIFTUPLE_CHECK(_args)) {
		_args = alifTuple_pack(1, _args);
		if (_args == nullptr) return 0;
	}
	else {
		ALIF_INCREF(_args);
	}

	_alias->origin = ALIF_NEWREF(_origin);
	_alias->args = _args;
	_alias->parameters = nullptr;
	_alias->weakRefList = nullptr;

	if (alifVectorCall_function(_origin) != nullptr) {
		_alias->vectorCall = genericAlias_vectorCall;
	}
	else {
		_alias->vectorCall = nullptr;
	}

	return 1;
}




AlifTypeObject _alifGenericAliasType_ = { // 905
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "انواع.اسم_مستعار",
	.basicSize = sizeof(GenericAliasObject),
	//.dealloc = ga_dealloc,
	.vectorCallOffset = offsetof(GenericAliasObject, vectorCall),
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_HAVE_GC | ALIF_TPFLAGS_BASETYPE | ALIF_TPFLAGS_HAVE_VECTORCALL,
	.weakListOffset = offsetof(GenericAliasObject, weakRefList),
	.free = alifObject_gcDel,
};





AlifObject* alif_genericAlias(AlifObject* _origin, AlifObject* _args) { // 931
	GenericAliasObject* _alias = (GenericAliasObject*)alifType_genericAlloc((AlifTypeObject*)&_alifGenericAliasType_, 0);
	if (_alias == nullptr) return nullptr;

	if (!setup_genericAlias(_alias, _origin, _args)) {
		ALIF_DECREF(_alias);
		return nullptr;
	}
	return (AlifObject*)_alias;
}
