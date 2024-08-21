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
