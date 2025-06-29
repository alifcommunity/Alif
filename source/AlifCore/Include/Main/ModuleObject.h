#pragma once




extern AlifTypeObject _alifModuleType_; // 10

// 12
#define ALIFMODULE_CHECK(_op) ALIFOBJECT_TYPECHECK((_op), &_alifModuleType_)
#define ALIFMODULE_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifModuleType_)

AlifObject* alifModule_newObject(AlifObject*); // 16


AlifObject* alifModule_new(const char*); // 20

AlifObject* alifModule_getDict(AlifObject*); // 23

AlifObject* alifModule_getNameObject(AlifObject*); // 25

const char* alifModule_getName(AlifObject*); // 27

AlifObject* alifModule_getFilenameObject(AlifObject*); // 29
AlifModuleDef* alifModule_getDef(AlifObject*); // 30
void* alifModule_getState(AlifObject*); // 31

AlifObject* alifModuleDef_init(AlifModuleDef*); // 35
extern AlifTypeObject _alifModuleDefType_; // 36



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


// 74
#define ALIF_MOD_CREATE 1
#define ALIF_MOD_EXEC 2
#define ALIF_MOD_MULTIPLE_INTERPRETERS 3
#define ALIF_MOD_GIL 4

// 92
#define ALIF_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED ((void *)0)
#define ALIF_MOD_MULTIPLE_INTERPRETERS_SUPPORTED ((void *)1)
#define ALIF_MOD_PER_INTERPRETER_GIL_SUPPORTED ((void *)2)

#define ALIF_MOD_GIL_USED ((void *)0) // 99
#define ALIF_MOD_GIL_NOT_USED ((void *)1)

AlifIntT alifUnstable_moduleSetGIL(AlifObject*, void*); // 104

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
