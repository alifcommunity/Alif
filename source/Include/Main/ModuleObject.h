#pragma once


extern AlifTypeObject _alifModuleType_;

#define ALIFMODULE_CHECK(op) ALIFOBJECT_TYPECHECK((op), &_alifModuleType_)
#define ALIFMODULE_CHECKEXACT(op) ALIF_IS_TYPE((op), &_alifModuleType_)


AlifObject* alifModule_getNameObject(AlifObject*); // 25


class AlifModuleDefBase { // 39
public:
	ALIFOBJECT_HEAD;
	AlifObject* (*init)(void);
	AlifSizeT index;
	AlifObject* copy;
};


#define ALIFMODULEDEF_HEAD_INIT {  \
    ALIFOBJECT_HEAD_INIT(nullptr), \
    nullptr, /* m_init */       \
    0,        /* m_index */      \
    nullptr, /* m_copy */       \
  }


class AlifModuleDefSlot { // 69
public:
	AlifIntT slot{};
	void* value{};
};



class AlifModuleDef { // 107
public:
	AlifModuleDefBase base{};
	const wchar_t* name{};
	const wchar_t* doc{};
	AlifSizeT size{};
	AlifMethodDef* methods{};
	AlifModuleDefSlot* slots{};
	TraverseProc traverse{};
	Inquiry clear{};
	FreeFunc free{};
};

AlifObject* alifModule_newObject(AlifObject*);
AlifObject* alifNew_module(const wchar_t* );

AlifObject* alifModule_getDict(AlifObject*);


AlifObject* alifModuleDef_init(AlifModuleDef*);
