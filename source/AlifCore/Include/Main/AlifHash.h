#pragma once




#define ALIF_HASH_EXTERNAL 0
#define ALIF_HASH_SIPHASH24 1
#define ALIF_HASH_FNV 2
#define ALIF_HASH_SIPHASH13 3


#ifndef ALIF_HASH_ALGORITHM
#  ifndef HAVE_ALIGNED_REQUIRED
#    define ALIF_HASH_ALGORITHM ALIF_HASH_SIPHASH13
#  else
#    define ALIF_HASH_ALGORITHM ALIF_HASH_FNV
#  endif /* uint64_t && uint32_t && aligned */
#endif









/* ---------------------------------------------------------------------------------------- */




#if SIZEOF_VOID_P >= 8
#define ALIFHASH_BITS 61
#else
#define ALIFHASH_BITS 31
#endif


#define ALIFHASH_MODULUS (((AlifUSizeT)1 << ALIFHASH_BITS) - 1)
#define ALIFHASH_INF 314159


AlifHashT _alif_hashDouble(AlifObject*, double); // 30


/* hash function definition */
class AlifHashFuncDef { // 37
public:
	AlifHashT(* const hash)(const void*, AlifSizeT) {};
	const char* name{};
	const AlifIntT hashBits{};
	const AlifIntT seedBits{};
};


AlifHashT alif_hashPointer(const void*); // 46
AlifHashT alifObject_genericHash(AlifObject*); // 47
