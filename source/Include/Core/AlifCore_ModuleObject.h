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
AlifObject* alifModule_getAttro(AlifModuleObject*, AlifObject*); // 49
