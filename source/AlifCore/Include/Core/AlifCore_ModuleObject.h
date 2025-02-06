#pragma once






extern AlifIntT _alifModuleSpec_isInitializing(AlifObject*); // 13




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


static inline AlifModuleDef* _alifModule_getDef(AlifObject* mod) { // 30
	return ((AlifModuleObject*)mod)->def;
}

static inline void* _alifModule_getState(AlifObject* mod) { // 35
	return ((AlifModuleObject*)mod)->state;
}

static inline AlifObject* _alifModule_getDict(AlifObject* mod) { // 40
	AlifObject* dict = ((AlifModuleObject*)mod)->dict;
	return dict;  // borrowed reference
}


AlifObject* alifModule_getAttroImpl(AlifModuleObject*, AlifObject*, AlifIntT); // 48
AlifObject* alifModule_getAttro(AlifObject*, AlifObject*); // 49
