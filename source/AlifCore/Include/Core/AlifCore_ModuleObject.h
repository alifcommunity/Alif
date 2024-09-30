#pragma once











class AlifModuleObject { // 17
public:
	ALIFOBJECT_HEAD{};
	AlifObject* dict{};
	AlifModuleDef* def{};
	void* state{};
	AlifObject* weaklist{};
	AlifObject* name{};
	void* gil{};
};




static inline AlifObject* _alifModule_getDict(AlifObject* mod) { // 40
	AlifObject* dict = ((AlifModuleObject*)mod)->dict;
	return dict;  // borrowed reference
}


AlifObject* alifModule_getAttroImpl(AlifModuleObject*, AlifObject*, AlifIntT); // 48
AlifObject* alifModule_getAttro(AlifModuleObject*, AlifObject*); // 49
