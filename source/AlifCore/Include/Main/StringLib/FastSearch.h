

#define FAST_COUNT 0
#define FAST_SEARCH 1
#define FAST_RSEARCH 2

#if LONG_BIT >= 128
#define STRINGLIB_BLOOM_WIDTH 128
#elif LONG_BIT >= 64
#define STRINGLIB_BLOOM_WIDTH 64
#elif LONG_BIT >= 32
#define STRINGLIB_BLOOM_WIDTH 32
#else
#error "LONG_BIT is smaller than 32"
#endif

#define STRINGLIB_BLOOM_ADD(mask, ch) \
    ((mask |= (1UL << ((ch) & (STRINGLIB_BLOOM_WIDTH -1)))))
#define STRINGLIB_BLOOM(mask, ch)     \
    ((mask &  (1UL << ((ch) & (STRINGLIB_BLOOM_WIDTH -1)))))

#ifdef STRINGLIB_FAST_MEMCHR
#  define MEMCHR_CUT_OFF 15
#else
#  define MEMCHR_CUT_OFF 40
#endif

 // 49
ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(findChar)(const STRINGLIB_CHAR* s, AlifSizeT n, STRINGLIB_CHAR ch)
{
	const STRINGLIB_CHAR* p, * e;

	p = s;
	e = s + n;
	if (n > MEMCHR_CUT_OFF) {
#ifdef STRINGLIB_FAST_MEMCHR
		p = (const STRINGLIB_CHAR*)STRINGLIB_FAST_MEMCHR(s, ch, n);
		if (p != nullptr)
			return (p - s);
		return -1;
#else
		/* use memchr if we can choose a needle without too many likely
		   false positives */
		const STRINGLIB_CHAR* s1, * e1;
		unsigned char needle = ch & 0xff;
		/* If looking for a multiple of 256, we'd have too
		   many false positives looking for the '\0' byte in UCS2
		   and UCS4 representations. */
		if (needle != 0) {
			do {
				void* candidate = (void*)memchr(p, needle,
					(e - p) * sizeof(STRINGLIB_CHAR));
				if (candidate == nullptr)
					return -1;
				s1 = p;
				p = (const STRINGLIB_CHAR*)
					ALIF_ALIGN_DOWN(candidate, sizeof(STRINGLIB_CHAR));
				if (*p == ch)
					return (p - s);
				/* False positive */
				p++;
				if (p - s1 > MEMCHR_CUT_OFF)
					continue;
				if (e - p <= MEMCHR_CUT_OFF)
					break;
				e1 = p + MEMCHR_CUT_OFF;
				while (p != e1) {
					if (*p == ch)
						return (p - s);
					p++;
				}
			} while (e - p > MEMCHR_CUT_OFF);
		}
#endif
	}
	while (p < e) {
		if (*p == ch)
			return (p - s);
		p++;
	}
	return -1;
}

#undef MEMCHR_CUT_OFF

#if STRINGLIB_SIZEOF_CHAR == 1
#  define MEMRCHR_CUT_OFF 15
#else
#  define MEMRCHR_CUT_OFF 40
#endif


ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(rFindChar)(const STRINGLIB_CHAR* s, AlifSizeT n, STRINGLIB_CHAR ch)
{
	const STRINGLIB_CHAR* p;
#ifdef HAVE_MEMRCHR
	/* memrchr() is a GNU extension, available since glibc 2.1.91.  it
	   doesn't seem as optimized as memchr(), but is still quite
	   faster than our hand-written loop below. There is no wmemrchr
	   for 4-byte chars. */

	if (n > MEMRCHR_CUT_OFF) {
#if STRINGLIB_SIZEOF_CHAR == 1
		p = memrchr(s, ch, n);
		if (p != nullptr)
			return (p - s);
		return -1;
#else
		/* use memrchr if we can choose a needle without too many likely
		   false positives */
		const STRINGLIB_CHAR* s1;
		Py_ssize_t n1;
		unsigned char needle = ch & 0xff;
		/* If looking for a multiple of 256, we'd have too
		   many false positives looking for the '\0' byte in UCS2
		   and UCS4 representations. */
		if (needle != 0) {
			do {
				void* candidate = memrchr(s, needle,
					n * sizeof(STRINGLIB_CHAR));
				if (candidate == nullptr)
					return -1;
				n1 = n;
				p = (const STRINGLIB_CHAR*)
					ALIF_ALIGN_DOWN(candidate, sizeof(STRINGLIB_CHAR));
				n = p - s;
				if (*p == ch)
					return n;
				/* False positive */
				if (n1 - n > MEMRCHR_CUT_OFF)
					continue;
				if (n <= MEMRCHR_CUT_OFF)
					break;
				s1 = p - MEMRCHR_CUT_OFF;
				while (p > s1) {
					p--;
					if (*p == ch)
						return (p - s);
				}
				n = p - s;
			} while (n > MEMRCHR_CUT_OFF);
		}
#endif
	}
#endif  /* HAVE_MEMRCHR */
	p = s + n;
	while (p > s) {
		p--;
		if (*p == ch)
			return (p - s);
	}
	return -1;
}

#undef MEMRCHR_CUT_OFF
