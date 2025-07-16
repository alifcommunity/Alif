#pragma once

#include "AlifCore_HashTable.h"



extern AlifIntT _alifImport_fixupBuiltin(AlifThread*, AlifObject*, const char*, AlifObject*); // 26


AlifObject* _alifImport_getModuleAttr(AlifObject*, AlifObject*); // 34
AlifObject* _alifImport_getModuleAttrString(const char*, const char*); // 37

class ImportDureRunState { // 40
public:
	class InitTable* initTable{};

	AlifSizeT lastModuleIndex{};
	class {
	public:
		AlifMutex mutex{};
		AlifHashTableT* hashtable{};
	} extensions;
};

class ImportState { // 63
public:
	AlifObject* modules{};

	AlifObject* modulesByIndex{};

	AlifObject* importLib{};
	AlifObject* importFunc{};
	//class {
	//public:
	//	AlifIntT importLevel{};
	//	AlifTimeT accumulated{};
	//	AlifIntT header{};
	//} findAndLoad;
};

// 119
/*
#define IMPORTS_INIT \
    { \
        .findAndLoad = { \
            .header = 1, \
        }, \
    }
*/


AlifSizeT alifImport_getNextModuleIndex(); // 129
extern const char* alifImport_resolveNameWithPackageContext(const char*);
extern const char* _alifImport_swapPackageContext(const char* newcontext); // 131


extern AlifObject* alifImport_initModules(AlifInterpreter*); // 136
extern AlifObject* _alifImport_getModules(AlifInterpreter*); // 137

extern AlifIntT _alifImport_initDefaultImportFunc(AlifInterpreter*); // 142
extern AlifIntT _alifImport_isDefaultImportFunc(AlifInterpreter*, AlifObject*); // 143

extern AlifIntT alifImport_init(); // 161

extern AlifIntT _alifImport_initCore(AlifThread*, AlifObject*, AlifIntT); // 165

extern AlifObject* _alifImport_getBuiltinModuleNames(void); // 174


class ModuleAlias { // 176
public:
	const char* name{};                 /* ASCII encoded string */
	const char* orig{};                 /* ASCII encoded string */
};


extern const Frozen* _alifImportFrozenBootstrap_; // 182


extern const ModuleAlias* _alifImportFrozenAliases_; // 186
