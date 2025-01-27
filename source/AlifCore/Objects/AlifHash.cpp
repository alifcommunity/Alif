#include "alif.h"

#include "AlifCore_Hash.h"






AlifHashSecretT _alifHashSecret_ = { {0} }; // 17

// 19
//#if ALIF_HASH_ALGORITHM == ALIF_HASH_EXTERNAL // need review
extern AlifHashFuncDef _alifHashFunc_;
//#else
//static AlifHashFuncDef _alifHashFunc_;
//#endif


AlifHashT _alif_hashDouble(AlifObject* _inst, double _v) { // 86
	AlifIntT e{}, sign{};
	double m{};
	AlifUHashT x{}, y{};

	if (!isfinite(_v)) {
		if (isinf(_v))
			return _v > 0 ? ALIFHASH_INF : -ALIFHASH_INF;
		else
			return alifObject_genericHash(_inst);
	}

	m = frexp(_v, &e);

	sign = 1;
	if (m < 0) {
		sign = -1;
		m = -m;
	}

	/* process 28 bits at a time;  this should work well both for binary
	   and hexadecimal floating point. */
	x = 0;
	while (m) {
		x = ((x << 28) & ALIFHASH_MODULUS) | x >> (ALIFHASH_BITS - 28);
		m *= 268435456.0;  /* 2**28 */
		e -= 28;
		y = (AlifUHashT)m;  /* pull out integer part */
		m -= y;
		x += y;
		if (x >= ALIFHASH_MODULUS)
			x -= ALIFHASH_MODULUS;
	}

	/* adjust for the exponent;  first reduce it modulo ALIFHASH_BITS */
	e = e >= 0 ? e % ALIFHASH_BITS : ALIFHASH_BITS - 1 - ((-1 - e) % ALIFHASH_BITS);
	x = ((x << e) & ALIFHASH_MODULUS) | x >> (ALIFHASH_BITS - e);

	x = x * sign;
	if (x == (AlifUHashT)-1)
		x = (AlifUHashT)-2;
	return (AlifHashT)x;
}


AlifHashT alif_hashPointer(const void* _ptr) { // 132
	AlifHashT hash = alif_hashPointerRaw(_ptr);
	if (hash == -1) {
		hash = -2;
	}
	return hash;
}

AlifHashT alifObject_genericHash(AlifObject* _obj) { // 142
	return alif_hashPointer(_obj);
}



AlifHashT alif_hashBuffer(const void* _ptr, AlifSizeT _len) { // 148
	AlifHashT x{};
	/*
	  We make the hash of the empty string be 0, rather than using
	  (prefix ^ suffix), since this slightly obfuscates the hash secret
	*/
	if (_len == 0) {
		return 0;
	}

#ifdef ALIF_HASH_STATS
	hashstats[(_len <= ALIF_HASH_STATS_MAX) ? _len : 0]++;
#endif

#if ALIF_HASH_CUTOFF > 0
	if (_len < ALIF_HASH_CUTOFF) {
		/* Optimize hashing of very small strings with inline DJBX33A. */
		AlifUHashT hash;
		const unsigned char* p = _ptr;
		hash = 5381;

		switch (_len) {
			/* ((hash << 5) + hash) + *p == hash * 33 + *p */
		case 7: hash = ((hash << 5) + hash) + *p++; ALIF_FALLTHROUGH;
		case 6: hash = ((hash << 5) + hash) + *p++; ALIF_FALLTHROUGH;
		case 5: hash = ((hash << 5) + hash) + *p++; ALIF_FALLTHROUGH;
		case 4: hash = ((hash << 5) + hash) + *p++; ALIF_FALLTHROUGH;
		case 3: hash = ((hash << 5) + hash) + *p++; ALIF_FALLTHROUGH;
		case 2: hash = ((hash << 5) + hash) + *p++; ALIF_FALLTHROUGH;
		case 1: hash = ((hash << 5) + hash) + *p++; break;
		default:
			ALIF_UNREACHABLE();
		}
		hash ^= _len;
		hash ^= (AlifUHashT)_alifHashSecret_.djbx33a.suffix;
		x = (AlifHashT)hash;
	}
	else
#endif
	{
		x = _alifHashFunc_.hash(_ptr, _len);
	}

	if (x == -1) {
		return -2;
	}
	return x;
}





// 328
#if ALIF_LITTLE_ENDIAN
#  define _LE64TOH(x) ((uint64_t)(x))
#elif defined(__APPLE__)
#  define _LE64TOH(x) OSSwapLittleToHostInt64(x)
#elif defined(HAVE_LETOH64)
#  define _LE64TOH(x) le64toh(x)
#else
#  define _LE64TOH(x) (((uint64_t)(x) << 56) | \
                      (((uint64_t)(x) << 40) & 0xff000000000000ULL) | \
                      (((uint64_t)(x) << 24) & 0xff0000000000ULL) | \
                      (((uint64_t)(x) << 8)  & 0xff00000000ULL) | \
                      (((uint64_t)(x) >> 8)  & 0xff000000ULL) | \
                      (((uint64_t)(x) >> 24) & 0xff0000ULL) | \
                      (((uint64_t)(x) >> 40) & 0xff00ULL) | \
                      ((uint64_t)(x)  >> 56))
#endif

#ifdef _MSC_VER
#  define ROTATE(x, b)  _rotl64(x, b)
#else
#  define ROTATE(x, b) (uint64_t)( ((x) << (b)) | ( (x) >> (64 - (b))) )
#endif

#define HALF_ROUND(a,b,c,d,s,t)     \
    a += b; c += d;                 \
    b = ROTATE(b, s) ^ a;           \
    d = ROTATE(d, t) ^ c;           \
    a = ROTATE(a, 32);

#define SINGLE_ROUND(v0,v1,v2,v3)   \
    HALF_ROUND(v0,v1,v2,v3,13,16);  \
    HALF_ROUND(v2,v1,v0,v3,17,21);

#define DOUBLE_ROUND(v0,v1,v2,v3)   \
    SINGLE_ROUND(v0,v1,v2,v3);      \
    SINGLE_ROUND(v0,v1,v2,v3);


static uint64_t sipHash_13(uint64_t _k0, uint64_t _k1,
	const void* _src, AlifSizeT _srcSZ) { // 367
	uint64_t b = (uint64_t)_srcSZ << 56;
	const uint8_t* in = (const uint8_t*)_src;

	uint64_t v0 = _k0 ^ 0x736f6d6570736575ULL;
	uint64_t v1 = _k1 ^ 0x646f72616e646f6dULL;
	uint64_t v2 = _k0 ^ 0x6c7967656e657261ULL;
	uint64_t v3 = _k1 ^ 0x7465646279746573ULL;

	uint64_t t{};
	uint8_t* pt{};

	while (_srcSZ >= 8) {
		uint64_t mi;
		memcpy(&mi, in, sizeof(mi));
		mi = _LE64TOH(mi);
		in += sizeof(mi);
		_srcSZ -= sizeof(mi);
		v3 ^= mi;
		SINGLE_ROUND(v0, v1, v2, v3);
		v0 ^= mi;
	}

	t = 0;
	pt = (uint8_t*)&t;
	switch (_srcSZ) {
	case 7: pt[6] = in[6]; ALIF_FALLTHROUGH;
	case 6: pt[5] = in[5]; ALIF_FALLTHROUGH;
	case 5: pt[4] = in[4]; ALIF_FALLTHROUGH;
	case 4: memcpy(pt, in, sizeof(uint32_t)); break;
	case 3: pt[2] = in[2]; ALIF_FALLTHROUGH;
	case 2: pt[1] = in[1]; ALIF_FALLTHROUGH;
	case 1: pt[0] = in[0]; break;
	}
	b |= _LE64TOH(t);

	v3 ^= b;
	SINGLE_ROUND(v0, v1, v2, v3);
	v0 ^= b;
	v2 ^= 0xff;
	SINGLE_ROUND(v0, v1, v2, v3);
	SINGLE_ROUND(v0, v1, v2, v3);
	SINGLE_ROUND(v0, v1, v2, v3);

	/* modified */
	t = (v0 ^ v1) ^ (v2 ^ v3);
	return t;
}


#if ALIF_HASH_ALGORITHM == ALIF_HASH_SIPHASH13
static AlifHashT alif_sipHash(const void* _src, AlifSizeT _srcSZ) { // 477
	return (AlifHashT)sipHash_13(
		_LE64TOH(_alifHashSecret_.sipHash.k0), _LE64TOH(_alifHashSecret_.sipHash.k1),
		_src, _srcSZ);
}
AlifHashFuncDef _alifHashFunc_ = {.hash = alif_sipHash,
										 .name = "sipHash_13",
										 .hashBits = 64,
										 .seedBits = 128 };
// need review
//static AlifHashFuncDef _alifHashFunc_ = {.hash = alif_sipHash,
//										 .name = "sipHash_13",
//										 .hashBits = 64,
//										 .seedBits = 128 };
#endif
