#pragma once






AlifObject* alifImport_getModule(AlifObject*); // 36


AlifObject* alifImport_addModuleRef(const char*); // 47



AlifObject* alifImport_importModuleLevelObject(AlifObject*, AlifObject*,
	AlifObject*, AlifObject*, AlifIntT); // 65



/* --------------------------------------------------------------------------------------- */




class InitTable { // 7
public:
	const char* name{};
	AlifObject* (*initFunc)(void);
};


//extern class InitTable* _alifImportInitTab_; // 12
