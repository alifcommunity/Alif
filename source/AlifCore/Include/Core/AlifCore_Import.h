#pragma once

#include "AlifCore_HashTable.h"



extern AlifIntT _alifImport_fixupBuiltin(AlifThread*, AlifObject*, const char*, AlifObject*); // 26


ALIFAPI_FUNC(AlifObject*) _alifImport_getModuleAttr(AlifObject*, AlifObject*); // 34
ALIFAPI_FUNC(AlifObject*) _alifImport_getModuleAttrString(const char*, const char*); // 37

class ImportRuntimeState { // 40
public:
	class InitTable* initTable{};

	AlifSizeT lastModuleIndex{};
	class {
	public:
		AlifMutex mutex{};
		AlifHashTableT* hashtable{};
	} extensions;
};


// 107
//* alif
// we use cpp way to set value while initialize a new ImportState class
#ifdef HAVE_DLOPEN
#  include <dlfcn.h>              // RTLD_NOW, RTLD_LAZY
#  if HAVE_DECL_RTLD_NOW
#    define _ALIF_DLOPEN_FLAGS RTLD_NOW
#  else
#    define _ALIF_DLOPEN_FLAGS RTLD_LAZY
#  endif
#endif

class ImportState { // 63
public:
	AlifObject* modules{};

	AlifObject* modulesByIndex{};

	AlifObject* importLib{};

	AlifIntT overrideMultiInterpExtensionsCheck{};
#ifdef HAVE_DLOPEN
	AlifIntT dlopenflags{_ALIF_DLOPEN_FLAGS};
#endif
	AlifObject* importFunc{};
	class {
	public:
		AlifIntT importLevel{};
		AlifTimeT accumulated{};
		AlifIntT header{1};
	} findAndLoad;
};



//* alif
//* delete
// 119
//#define IMPORTS_INIT \
//	{ \
//		DLOPENFLAGS_INIT \
//		.findAndLoad = { \
//			.header = 1, \
//		}, \
//	}



AlifSizeT alifImport_getNextModuleIndex(); // 129
extern const char* alifImport_resolveNameWithPackageContext(const char*);
extern const char* _alifImport_swapPackageContext(const char* newcontext); // 131

extern AlifIntT _alifImport_getDLOpenFlags(AlifInterpreter*); // 134
extern void _alifImport_setDLOpenFlags(AlifInterpreter*, AlifIntT);

extern AlifObject* alifImport_initModules(AlifInterpreter*); // 136
extern AlifObject* _alifImport_getModules(AlifInterpreter*); // 137

extern AlifIntT _alifImport_initDefaultImportFunc(AlifInterpreter*); // 142
extern AlifIntT _alifImport_isDefaultImportFunc(AlifInterpreter*, AlifObject*); // 143

extern AlifStatus _alifImport_init(); // 161

extern AlifStatus _alifImport_initCore(AlifThread*, AlifObject*, AlifIntT); // 165

extern AlifStatus _alifImport_initExternal(AlifThread*); // 170

extern AlifObject* _alifImport_getBuiltinModuleNames(void); // 174


class ModuleAlias { // 176
public:
	const char* name{};                 /* ASCII encoded string */
	const char* orig{};                 /* ASCII encoded string */
};


ALIFAPI_DATA(const Frozen*) _alifImportFrozenBootstrap_; // 182


extern const ModuleAlias* _alifImportFrozenAliases_; // 186


extern AlifIntT _alifImport_checkSubinterpIncompatibleExtensionAllowed(const char*); // 189
