#pragma once









AlifObject* alifModule_new(const char*); // 20




class AlifModuleDefBase { // 39
public:
	ALIFOBJECT_HEAD;
	AlifObject* (*init)(void);
	AlifSizeT index;
	AlifObject* copy;
};

// 60
#define ALIFMODULEDEF_HEAD_INIT {	\
    ALIFOBJECT_HEAD_INIT(nullptr),	\
    nullptr,						\
    0,								\
    nullptr,						\
  }


class AlifModuleDefSlot { // 69
public:
	AlifIntT slot{};
	void* value{};
};


class AlifModuleDef { // 107
public:
	AlifModuleDefBase base{};
	const char* name{};
	const char* doc{};
	AlifSizeT size{};
	AlifMethodDef* methods{};
	AlifModuleDefSlot* slots{};
	TraverseProc traverse{};
	Inquiry clear{};
	FreeFunc free{};
};
