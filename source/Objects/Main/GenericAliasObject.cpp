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
	ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
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
	0,
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



static inline AlifObject* set_origClass(AlifObject* obj, AlifObject* self) { // 590
	if (obj != nullptr) {
		AlifObject* name = alifUStr_decodeStringToUTF8(L"__orig_class__");
		if (alifObject_setAttr(obj, name, self) < 0) {
			//if (!alifErr_exceptionMatches(alifExcAttributeError) and
			//	!alifErr_exceptionMatches(alifExcTypeError))
			//{
			//	ALIF_DECREF(obj);
			//	return nullptr;
			//}
			//alifErr_clear();
		}
	}
	return obj;
}

static AlifObject* genericAlias_vectorCall(AlifObject* self,
	AlifObject* const* args, AlifUSizeT nargsf, AlifObject* kwnames) { // 615

	GenericAliasObject* alias = (GenericAliasObject*)self;
	AlifObject* obj = alifVectorCall_function(alias->origin_)(alias->origin_, args, nargsf, kwnames);
	return set_origClass(obj, self);
}

static inline AlifIntT setup_genericAlias(GenericAliasObject* alias, AlifObject* origin, AlifObject* args) { // 821
	if (!ALIFTUPLE_CHECK(args)) {
		args = tuple_pack(1, args);
		if (args == nullptr) return 0;
	} else {
		ALIF_INCREF(args);
	}

	alias->origin_ = ALIF_NEWREF(origin);
	alias->args_ = args;
	alias->parameters_ = nullptr;
	alias->weakRefList = nullptr;

	if (alifVectorCall_function(origin) != nullptr) {
		alias->vectorCall = genericAlias_vectorCall;
	} else {
		alias->vectorCall = nullptr;
	}

	return 1;
}

AlifObject* alif_genericAlias(AlifObject* _origin, AlifObject* _args) { // 987
	GenericAliasObject* alias = (GenericAliasObject*)alifType_genericAlloc((AlifTypeObject*)&_alifGenericAliasType_, 0);
	if (alias == nullptr) return nullptr;

	if (!setup_genericAlias(alias, _origin, _args)) {
		ALIF_DECREF(alias);
		return nullptr;
	}
	return (AlifObject*)alias;
}
