#pragma once


#include "AlifCore_ModuleObject.h"
#include "AlifCore_Lock.h"


 // 17
#define ALIF_TYPE_BASE_VERSION_TAG (2<<16)
#define ALIF_MAX_GLOBAL_TYPE_VERSION_TAG (ALIF_TYPE_BASE_VERSION_TAG - 1)

#define ALIFMAX_MANAGED_STATIC_BUILTIN_TYPES 200 // 22
#define ALIFMAX_MANAGED_STATIC_EXT_TYPES 10 // 23
#define ALIF_MAX_MANAGED_STATIC_TYPES \
    (ALIFMAX_MANAGED_STATIC_BUILTIN_TYPES + ALIFMAX_MANAGED_STATIC_EXT_TYPES)


class TypesDureRunState { // 27
public:
	AlifUIntT nextVersionTag{};
	class {
	public:
		class {
		public:
			AlifTypeObject* type{};
			AlifTypeObject def{};
			int64_t interpCount{};
		} types[ALIF_MAX_MANAGED_STATIC_TYPES];
	} managedStatic;
};

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
	AlifUIntT nextVersionTag{};
	TypeCache typeCache{};
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
