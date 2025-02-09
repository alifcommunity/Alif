#pragma once




AlifIntT alifArg_parseTuple(AlifObject*, const char*, ...); // 10

AlifIntT alifArg_unpackTuple(AlifObject*, const char*, AlifSizeT, AlifSizeT, ...); // 18
AlifObject* alif_buildValue(const char*, ...); // 19


AlifIntT alifModule_addObjectRef(AlifObject*, const char*, AlifObject*); // 26




AlifIntT alifModule_addType(AlifObject*, AlifTypeObject*); // 44



AlifIntT alifModule_addFunctions(AlifObject*, AlifMethodDef*); // 53
AlifIntT alifModule_execDef(AlifObject*, AlifModuleDef*); // 54

#define ALIF_CLEANUP_SUPPORTED 0x20000 // 57

 // 59
#define ALIF_API_VERSION 5
#define ALIF_API_STRING "5"


AlifObject* alifModule_fromDefAndSpec2(AlifModuleDef*, AlifObject*, AlifIntT); // 123

// 131
#define ALIFMODULE_FROMDEFANDSPEC(_module, _spec) \
    alifModule_fromDefAndSpec2((_module), (_spec), ALIF_API_VERSION)


/* ----------------------------------------------------------------------------------------------------------- */


class AlifOnceFlag { // 7
public:
	uint8_t v{};
};

class AlifArgParser { // 11
public:
	const char* format{};
	const char* const* keywords{};
	const char* fname{};
	const char* customMsg{};
	AlifOnceFlag once{};
	AlifIntT isKwTupleOwned{};
	AlifIntT pos{};
	AlifIntT min{};
	AlifIntT max{};
	AlifObject* kwTuple{};
	AlifArgParser* next{};
};
