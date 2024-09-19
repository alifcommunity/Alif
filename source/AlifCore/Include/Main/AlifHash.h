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










/* hash function definition */
class AlifHashFuncDef { // 37
public:
	AlifHashT(* const hash)(const void*, AlifSizeT) {};
	const char* name{};
	const AlifIntT hashBits{};
	const AlifIntT seedBits{};
};
