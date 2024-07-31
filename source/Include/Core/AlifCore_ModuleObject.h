#pragma once



class AlifModuleObject {
public:
	ALIFOBJECT_HEAD;
	AlifObject* dict{};
	AlifModuleDef* def{};
	void* state{};
	AlifObject* weakList{};
	AlifObject* name{};
};










AlifObject* alifModule_getAttroImpl(AlifModuleObject*, AlifObject*, AlifIntT);
AlifObject* alifModule_getAttro(AlifModuleObject*, AlifObject*); 


static inline AlifObject* alifSubModule_getDict(AlifObject* _mod) {
	AlifObject* dict_ = ((AlifModuleObject*)_mod)->dict;
	return dict_;  // borrowed reference
}
