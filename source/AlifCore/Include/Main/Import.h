#pragma once






ALIFAPI_FUNC(AlifObject*) alifImport_getModule(AlifObject*); // 36


ALIFAPI_FUNC(AlifObject*) alifImport_addModuleRef(const char*); // 47

ALIFAPI_FUNC(AlifObject*) alifImport_importModule(const char*); // 51

ALIFAPI_FUNC(AlifObject*) alifImport_importModuleLevelObject(AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifIntT); // 65


ALIFAPI_FUNC(AlifObject*) alifImport_import(AlifObject*); // 78

/* --------------------------------------------------------------------------------------- */


ALIFMODINIT_FUNC alifInit__imp(void); // 5

class InitTable { // 7
public:
	const char* name{};
	AlifObject* (*initFunc)(void);
};


ALIFAPI_DATA(class InitTable*) _alifImportInitTable_; // 12


class Frozen { // 15
public:
	const char* name{};                 /* ASCII encoded string */
	const unsigned char* code{};
	AlifIntT size{};
	AlifIntT isPackage{};
};
