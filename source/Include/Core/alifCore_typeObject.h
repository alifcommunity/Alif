#pragma once



























class TypeCacheEntry {
public:
	unsigned int version;  
	AlifObject* name;        
	AlifObject* value;       
};
#define MCACHE_SIZE_EXP 12

class TypeCache {
public:
	class TypeCacheEntry hashTable[1 << MCACHE_SIZE_EXP];
};


#define ALIFMAX_STATIC_BUILTINTYPES 200

class StaticBuiltinState {
public:
	AlifTypeObject* type;
	int readying;
	int ready;


	AlifObject* tp_dict;
	AlifObject* tp_subclasses;




	AlifObject* tp_weaklist;
};

class TypesState {
public:


	unsigned int nextVersionTag;

	class TypeCache typeCache;
	size_t numBuiltinsInitialized;
	StaticBuiltinState builtins[ALIFMAX_STATIC_BUILTINTYPES];
};
