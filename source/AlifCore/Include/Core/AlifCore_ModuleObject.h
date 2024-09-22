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





AlifObject* alifModule_getAttroImpl(AlifModuleObject*, AlifObject*, AlifIntT); // 48
AlifObject* alifModule_getAttro(AlifModuleObject*, AlifObject*); // 49
