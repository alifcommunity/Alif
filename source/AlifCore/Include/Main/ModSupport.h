#pragma once




ALIFAPI_FUNC(AlifIntT)alifArg_parseTuple(AlifObject*, const char*, ...); // 10
ALIFAPI_FUNC(AlifIntT) alifArg_parseTupleAndKeywords(AlifObject*, AlifObject*, const char*, const char* const *, ...); // 11
ALIFAPI_FUNC(AlifIntT) alifArg_unpackTuple(AlifObject*, const char*, AlifSizeT, AlifSizeT, ...); // 18
ALIFAPI_FUNC(AlifObject*) alif_buildValue(const char*, ...); // 19


ALIFAPI_FUNC(AlifIntT) alifModule_addObjectRef(AlifObject*, const char*, AlifObject*); // 26


ALIFAPI_FUNC(AlifIntT) alifModule_add(AlifObject*, const char*, AlifObject*); // 31


ALIFAPI_FUNC(AlifIntT) alifModule_addIntConstant(AlifObject*, const char*, long); // 39

ALIFAPI_FUNC(AlifIntT) alifModule_addType(AlifObject*, AlifTypeObject*); // 44


#define ALIFMODULE_ADDINTMACRO(_m, _name, _c) alifModule_addIntConstant((_m), #_name, (_c)) // 47


ALIFAPI_FUNC(AlifIntT) alifModule_addFunctions(AlifObject*, AlifMethodDef*); // 53
ALIFAPI_FUNC(AlifIntT) alifModule_execDef(AlifObject*, AlifModuleDef*); // 54

#define ALIF_CLEANUP_SUPPORTED 0x20000 // 57

 // 59
// we chose random version
#define ALIF_API_VERSION 104
#define ALIF_API_STRING "104"






 /* The ALIF_ABI_VERSION for the lifetime of
 Alif 5, it will stay at the value of 5; changes to the limited API
 must be performed in a strictly backwards-compatible manner. */
 // 108
#define ALIF_ABI_VERSION 5
#define ALIF_ABI_STRING "5"


ALIFAPI_FUNC(AlifObject*) alifModule_fromDefAndSpec2(AlifModuleDef*, AlifObject*, AlifIntT); // 123

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
