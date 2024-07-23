#include "alif.h"

#include "AlifCore_AlifEval.h"
#include "AlifCore_ModSupport.h"
#include "AlifCore_Object.h"


class GenericAliasObject {
public:
	ALIFOBJECT_HEAD;
	AlifObject* origin_{};
	AlifObject* args_{};
	AlifObject* parameters_{};
	AlifObject* weakRefList{};
	bool starred_;
	VectorCallFunc vectorCall;
};






AlifTypeObject _alifGenericAliasType_ = {
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0)
	L"types.GenericAlias",
	0,
	sizeof(GenericAliasObject),
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
	alifObject_genericGetAttr,
	0,
	0,
	ALIFTPFLAGS_DEFAULT | ALIFTPFLAGS_HAVE_GC | ALIFTPFLAGS_BASETYPE | ALIFTPFLAGS_HAVE_VECTORCALL,
	0,
	0,
	0,
	0,
	offsetof(GenericAliasObject, weakRefList),
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
	0,
	offsetof(GenericAliasObject, vectorCall),
};



static inline AlifObject* set_origClass(AlifObject* _obj, AlifObject* _self) { // 590
	if (_obj != nullptr) {
		AlifObject* name_ = alifUStr_decodeStringToUTF8(L"__orig_class__");
		if (alifObject_setAttr(_obj, name_, _self) < 0) {
			//if (!alifErr_exceptionMatches(alifExcAttributeError) and
			//	!alifErr_exceptionMatches(alifExcTypeError))
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
	AlifObject* const* _args, AlifUSizeT _nargsf, AlifObject* _kwnames) { // 615

	GenericAliasObject* alias_ = (GenericAliasObject*)_self;
	AlifObject* obj_ = alifVectorCall_function(alias_->origin_)(alias_->origin_, _args, _nargsf, _kwnames);
	return set_origClass(obj_, _self);
}

static inline AlifIntT setup_genericAlias(GenericAliasObject* _alias, AlifObject* _origin, AlifObject* _args) { // 821
	if (!ALIFTUPLE_CHECK(_args)) {
		_args = tuple_pack(1, _args);
		if (_args == nullptr) return 0;
	} else {
		ALIF_INCREF(_args);
	}

	_alias->origin_ = ALIF_NEWREF(_origin);
	_alias->args_ = _args;
	_alias->parameters_ = nullptr;
	_alias->weakRefList = nullptr;

	if (alifVectorCall_function(_origin) != nullptr) {
		_alias->vectorCall = genericAlias_vectorCall;
	} else {
		_alias->vectorCall = nullptr;
	}

	return 1;
}

AlifObject* alif_genericAlias(AlifObject* _origin, AlifObject* _args) { // 987
	GenericAliasObject* _alias = (GenericAliasObject*)alifType_genericAlloc((AlifTypeObject*)&_alifGenericAliasType_, 0);
	if (_alias == nullptr) return nullptr;

	if (!setup_genericAlias(_alias, _origin, _args)) {
		ALIF_DECREF(_alias);
		return nullptr;
	}
	return (AlifObject*)_alias;
}
