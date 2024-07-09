#pragma once


#define ALIFTYPE_BASE_VERSIONTAG (2<<16)
#define ALIFMAX_GLOBALTYPE_VERSIONTAG (ALIFTYPE_BASE_VERSIONTAG - 1)


class TypesRuntimeState {
public:
	unsigned int nextVersionTag;
};

class TypeCacheEntry {
public:
	unsigned int version_;  
	AlifObject* name_;        
	AlifObject* value_;       
};

#define MCACHE_SIZE_EXP 12

class TypeCache {
public:
	class TypeCacheEntry hashtable[1 << MCACHE_SIZE_EXP];
};


#define ALIFMAX_STATIC_BUILTINTYPES 200

class StaticBuiltinState {
public:
	AlifTypeObject* type;
	int readying;
	int ready;
	AlifObject* dict;
	AlifObject* subclasses;
	AlifObject* weaklist;
} ;

class TypesState {
public:
	unsigned int nextVersionTag;
	class TypeCache typeCache;
	size_t numBuiltinsInitialized;
	StaticBuiltinState builtins[ALIFMAX_STATIC_BUILTINTYPES];
	uint8_t mutex;
};

AlifObject* alifSubType_getDict(AlifTypeObject* );

static inline int alifType_isReady(AlifTypeObject* _type)
{
	return alifSubType_getDict(_type) != nullptr;
}

AlifObject* alifType_getAttroImpl(AlifTypeObject*, AlifObject* , AlifIntT* );
AlifObject* alifType_getAttro(AlifObject* , AlifObject*);


extern AlifObject* alifType_getAttroImpl(AlifTypeObject*, AlifObject*, AlifIntT*);
extern AlifObject* alifType_getAttro(AlifObject*, AlifObject*); // 179
