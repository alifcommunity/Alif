#pragma once


#include "AlifCore_ModuleObject.h"
#include "AlifCore_Lock.h"



#define ALIFMAX_MANAGED_STATIC_BUILTIN_TYPES 200 // 22
#define ALIFMAX_MANAGED_STATIC_EXT_TYPES 10 // 23




class TypeCacheEntry { // 45
public:
	AlifUIntT version{};
#ifdef ALIF_GIL_DISABLED
	AlifSeqLock sequence{};
#endif
	AlifObject* name{};        // reference to exactly a str or None
	AlifObject* value{};       // borrowed reference or nullptr
};

#define MCACHE_SIZE_EXP 12

class TypeCache { // 56
public:
	TypeCacheEntry hashTable[1 << MCACHE_SIZE_EXP]{};
};





class ManagedStaticTypeState { // 60
public:
	AlifTypeObject* type{};
	AlifIntT isBuiltin{};
	AlifIntT readying{};
	AlifIntT ready{};

	AlifObject* dict{};
	AlifObject* subclasses{};
	AlifObject* weakList{};
};

class TypesState { // 78
public:
	class {
	public:
		AlifUSizeT numInitialized{};
		ManagedStaticTypeState initialized[ALIFMAX_MANAGED_STATIC_BUILTIN_TYPES]{};
	} builtins;
	class {
	public:
		AlifUSizeT numInitialized{};
		AlifUSizeT nextIndex{};
		ManagedStaticTypeState initialized[ALIFMAX_MANAGED_STATIC_EXT_TYPES]{};
	} forExtensions;
	AlifMutex mutex{};
};

AlifObject* alifType_getDict(AlifTypeObject*); // 207

static inline AlifIntT alifType_isReady(AlifTypeObject* _type) { // 221
	return alifType_getDict(_type) != nullptr;
}

extern AlifObject* alifType_getAttroImpl(AlifTypeObject* , AlifObject* , AlifIntT* ); // 226

extern AlifObject* alifType_getAttro(AlifObject* , AlifObject* ); // 228
