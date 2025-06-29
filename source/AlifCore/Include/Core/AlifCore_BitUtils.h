#pragma once



#ifdef _MSC_VER
#  include <intrin.h>             // _byteswap_uint64()
#endif




static inline uint32_t
_alif_bswap32(uint32_t word) { // 48
#if defined(ALIF_HAVE_BUILTIN_BSWAP) or ALIF_HAS_BUILTIN(__builtin_bswap32)
	return __builtin_bswap32(word);
#elif defined(_MSC_VER)
	return _byteswap_ulong(word);
#else
	// Portable implementation which doesn't rely on circular bit shift
	return (((word & UINT32_C(0x000000FF)) << 24)
		| ((word & UINT32_C(0x0000FF00)) << 8)
		| ((word & UINT32_C(0x00FF0000)) >> 8)
		| ((word & UINT32_C(0xFF000000)) >> 24));
#endif
}




static inline AlifIntT alifBit_length(unsigned long x) { // 145
#if (defined(__clang__) or defined(__GNUC__))
	if (x != 0) {
		return (int)sizeof(unsigned long) * 8 - __builtin_clzl(x);
	}
	else {
		return 0;
	}
#elif defined(_MSC_VER)
	unsigned long msb;
	if (_BitScanReverse(&msb, x)) {
		return (int)msb + 1;
	}
	else {
		return 0;
	}
#else
	const int BIT_LENGTH_TABLE[32] = {
		0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5
	};
	int msb = 0;
	while (x >= 32) {
		msb += 6;
		x >>= 6;
	}
	msb += BIT_LENGTH_TABLE[x];
	return msb;
#endif
}
