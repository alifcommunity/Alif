#pragma once


#include "AlifCore_ModuleObject.h"
#include "AlifCore_Lock.h"


#define _ALIF_TYPE_VERSION_INT 1
#define _ALIF_TYPE_VERSION_FLOAT 2
#define _ALIF_TYPE_VERSION_LIST 3
#define _ALIF_TYPE_VERSION_TUPLE 4
#define _ALIF_TYPE_VERSION_STR 5
#define _ALIF_TYPE_VERSION_SET 6
#define _ALIF_TYPE_VERSION_FROZEN_SET 7
#define _ALIF_TYPE_VERSION_DICT 8
#define _ALIF_TYPE_VERSION_BYTEARRAY 9
#define _ALIF_TYPE_VERSION_BYTES 10
#define _ALIF_TYPE_VERSION_COMPLEX 11

#define _ALIF_TYPE_VERSION_NEXT 16

// 32
#define _ALIF_TYPE_BASE_VERSION_TAG (2<<16)
#define ALIF_MAX_GLOBAL_TYPE_VERSION_TAG (_ALIF_TYPE_BASE_VERSION_TAG - 1)

#define ALIFMAX_MANAGED_STATIC_BUILTIN_TYPES 200 // 22
#define ALIFMAX_MANAGED_STATIC_EXT_TYPES 10 // 23
#define ALIF_MAX_MANAGED_STATIC_TYPES \
    (ALIFMAX_MANAGED_STATIC_BUILTIN_TYPES + ALIFMAX_MANAGED_STATIC_EXT_TYPES)


class TypesRuntimeState { // 27
public:
	AlifUIntT nextVersionTag{};
	class {
	public:
		class {
		public:
			AlifTypeObject* type{};
			int64_t interpCount{};
		} types[ALIF_MAX_MANAGED_STATIC_TYPES];
	} managedStatic;
};

class TypeCacheEntry { // 45
public:
	AlifUIntT version{};
	AlifSeqLock sequence{};
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
	unsigned int nextVersionTag{};
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



extern AlifStatus alifTypes_initTypes(AlifInterpreter*); // 144


#define MAX_EQUIV 10

typedef class WrapperBase AlifTypeSlotDef; // 158

static inline AlifObject** _alifStaticType_getWeakRefsListPtr(ManagedStaticTypeState* _state) { // 161
	return &_state->weakList;
}

extern AlifIntT alifStaticType_initBuiltin(AlifInterpreter*, AlifTypeObject*); // 168


extern ManagedStaticTypeState* _alifStaticType_getState(AlifInterpreter*, AlifTypeObject*); // 177


static inline void* _alifType_getModuleState(AlifTypeObject* _type) { // 191
	AlifHeapTypeObject* et = (AlifHeapTypeObject*)_type;
	AlifModuleObject* mod = (AlifModuleObject*)(et->module_);
	return mod->state;
}


ALIFAPI_FUNC(AlifObject*) _alifType_getDict(AlifTypeObject*); // 207

static inline AlifIntT alifType_isReady(AlifTypeObject* _type) { // 221
	return _alifType_getDict(_type) != nullptr;
}

extern AlifObject* _alifType_getAttroImpl(AlifTypeObject*, AlifObject*, AlifIntT*); // 226

extern AlifObject* _alifType_getAttro(AlifObject*, AlifObject*); // 228



ALIFAPI_FUNC(void) _alifType_setVersion(AlifTypeObject*, AlifUIntT); // 269
