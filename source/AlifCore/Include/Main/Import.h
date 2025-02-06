#pragma once






AlifObject* alifImport_getModule(AlifObject*); // 36


AlifObject* alifImport_addModuleRef(const char*); // 47



AlifObject* alifImport_importModuleLevelObject(AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifIntT); // 65



/* --------------------------------------------------------------------------------------- */


AlifObject* alifInit__imp(void); // 5

class InitTable { // 7
public:
	const char* name{};
	AlifObject* (*initFunc)(void);
};


extern class InitTable* _alifImportInitTable_; // 12


class Frozen { // 15
public:
	const char* name{};                 /* ASCII encoded string */
	const unsigned char* code{};
	AlifIntT size{};
	AlifIntT isPackage{};
};
