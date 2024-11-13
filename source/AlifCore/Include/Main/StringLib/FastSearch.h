

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
{ // 115
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
		AlifSizeT n1;
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
 // 181
/* Change to a 1 to see logging comments walk through the algorithm. */
#if 0 and STRINGLIB_SIZEOF_CHAR == 1
# define LOG(...) printf(__VA_ARGS__)
# define LOG_STRING(s, n) printf("\"%.*s\"", (int)(n), s)
# define LOG_LINEUP() do {                                         \
    LOG("> "); LOG_STRING(haystack, len_haystack); LOG("\n> ");    \
    LOG("%*s",(int)(window_last - haystack + 1 - len_needle), ""); \
    LOG_STRING(needle, len_needle); LOG("\n");                     \
} while(0)
#else
# define LOG(...)
# define LOG_STRING(s, n)
# define LOG_LINEUP()
#endif


ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(lexSearch)(const STRINGLIB_CHAR* needle, AlifSizeT len_needle,
	AlifSizeT* return_period, int invert_alphabet) { // 196
	/* Do a lexicographic search. Essentially this:
		   >>> max(needle[i:] for i in range(len(needle)+1))
	   Also find the period of the right half.   */
	AlifSizeT max_suffix = 0;
	AlifSizeT candidate = 1;
	AlifSizeT k = 0;
	// The period of the right half.
	AlifSizeT period = 1;

	while (candidate + k < len_needle) {
		// each loop increases candidate + k + max_suffix
		STRINGLIB_CHAR a = needle[candidate + k];
		STRINGLIB_CHAR b = needle[max_suffix + k];
		// check if the suffix at candidate is better than max_suffix
		if (invert_alphabet ? (b < a) : (a < b)) {
			// Fell short of max_suffix.
			// The next k + 1 characters are non-increasing
			// from candidate, so they won't start a maximal suffix.
			candidate += k + 1;
			k = 0;
			// We've ruled out any period smaller than what's
			// been scanned since max_suffix.
			period = candidate - max_suffix;
		}
		else if (a == b) {
			if (k + 1 != period) {
				// Keep scanning the equal strings
				k++;
			}
			else {
				// Matched a whole period.
				// Start matching the next period.
				candidate += period;
				k = 0;
			}
		}
		else {
			// Did better than max_suffix, so replace it.
			max_suffix = candidate;
			candidate++;
			k = 0;
			period = 1;
		}
	}
	*return_period = period;
	return max_suffix;
}

ALIF_LOCAL_INLINE(AlifSizeT)
STRINGLIB(factorize)(const STRINGLIB_CHAR* needle,
	AlifSizeT len_needle, AlifSizeT* return_period) { // 248
	/* Do a "critical factorization", making it so that:
	   >>> needle = (left := needle[:cut]) + (right := needle[cut:])
	   where the "local period" of the cut is maximal.

	   The local period of the cut is the minimal length of a string w
	   such that (left endswith w or w endswith left)
	   and (right startswith w or w startswith right).

	   The Critical Factorization Theorem says that this maximal local
	   period is the global period of the string.

	   Crochemore and Perrin (1991) show that this cut can be computed
	   as the later of two cuts: one that gives a lexicographically
	   maximal right half, and one that gives the same with the
	   with respect to a reversed alphabet-ordering.

	   This is what we want to happen:
		   >>> x = "GCAGAGAG"
		   >>> cut, period = factorize(x)
		   >>> x[:cut], (right := x[cut:])
		   ('GC', 'AGAGAG')
		   >>> period  # right half period
		   2
		   >>> right[period:] == right[:-period]
		   True

	   This is how the local period lines up in the above example:
				GC | AGAGAG
		   AGAGAGC = AGAGAGC
	   The length of this minimal repetition is 7, which is indeed the
	   period of the original string. */

	AlifSizeT cut1{}, period1{}, cut2{}, period2{}, cut{}, period{};
	cut1 = STRINGLIB(lexSearch)(needle, len_needle, &period1, 0);
	cut2 = STRINGLIB(lexSearch)(needle, len_needle, &period2, 1);

	// Take the later cut.
	if (cut1 > cut2) {
		period = period1;
		cut = cut1;
	}
	else {
		period = period2;
		cut = cut2;
	}

	LOG("split: "); LOG_STRING(needle, cut);
	LOG(" + "); LOG_STRING(needle + cut, len_needle - cut);
	LOG("\n");

	*return_period = period;
	return cut;
}


 // 308
#define SHIFT_TYPE uint8_t
#define MAX_SHIFT UINT8_MAX

#define TABLE_SIZE_BITS 6u
#define TABLE_SIZE (1U << TABLE_SIZE_BITS)
#define TABLE_MASK (TABLE_SIZE - 1U)


typedef struct STRINGLIB(pre) { // 315
	const STRINGLIB_CHAR* needle;
	AlifSizeT len_needle{};
	AlifSizeT cut{};
	AlifSizeT period{};
	AlifSizeT gap{};
	AlifIntT isPeriodic{};
	SHIFT_TYPE table[TABLE_SIZE];
} STRINGLIB(preWork);

static void
STRINGLIB(preProcess)(const STRINGLIB_CHAR* needle, AlifSizeT len_needle,
	STRINGLIB(preWork)* p) { // 326
	p->needle = needle;
	p->len_needle = len_needle;
	p->cut = STRINGLIB(factorize)(needle, len_needle, &(p->period));
	p->isPeriodic = (0 == memcmp(needle,
		needle + p->period,
		p->cut * STRINGLIB_SIZEOF_CHAR));
	if (p->isPeriodic) {
	}
	else {
		// A lower bound on the period
		p->period = ALIF_MAX(p->cut, len_needle - p->cut) + 1;
	}
	// The gap between the last character and the previous
	// occurrence of an equivalent character (modulo TABLE_SIZE)
	p->gap = len_needle;
	STRINGLIB_CHAR last = needle[len_needle - 1] & TABLE_MASK;
	for (AlifSizeT i = len_needle - 2; i >= 0; i--) {
		STRINGLIB_CHAR x = needle[i] & TABLE_MASK;
		if (x == last) {
			p->gap = len_needle - 1 - i;
			break;
		}
	}
	// Fill up a compressed Boyer-Moore "Bad Character" table
	AlifSizeT not_found_shift = ALIF_MIN(len_needle, MAX_SHIFT);
	for (AlifSizeT i = 0; i < (AlifSizeT)TABLE_SIZE; i++) {
		p->table[i] = ALIF_SAFE_DOWNCAST(not_found_shift,
			AlifSizeT, SHIFT_TYPE);
	}
	for (AlifSizeT i = len_needle - not_found_shift; i < len_needle; i++) {
		SHIFT_TYPE shift = ALIF_SAFE_DOWNCAST(len_needle - 1 - i,
			AlifSizeT, SHIFT_TYPE);
		p->table[needle[i] & TABLE_MASK] = shift;
	}
}


static AlifSizeT
STRINGLIB(twoWay)(const STRINGLIB_CHAR* haystack, AlifSizeT len_haystack,
	STRINGLIB(preWork)* p) { // 369
	// Crochemore and Perrin's (1991) Two-Way algorithm.
	// See http://www-igm.univ-mlv.fr/~lecroq/string/node26.html#SECTION00260
	const AlifSizeT len_needle = p->len_needle;
	const AlifSizeT cut = p->cut;
	AlifSizeT period = p->period;
	const STRINGLIB_CHAR* const needle = p->needle;
	const STRINGLIB_CHAR* window_last = haystack + len_needle - 1;
	const STRINGLIB_CHAR* const haystack_end = haystack + len_haystack;
	SHIFT_TYPE* table = p->table;
	const STRINGLIB_CHAR* window;
	LOG("===== Two-way: \"%s\" in \"%s\". =====\n", needle, haystack);

	AlifSizeT gap = p->gap;
	AlifSizeT gap_jump_end = ALIF_MIN(len_needle, cut + gap);
	if (p->isPeriodic) {
		LOG("Needle is periodic.\n");
		AlifSizeT memory = 0;
	periodicwindowloop:
		while (window_last < haystack_end) {
			for (;;) {
				LOG_LINEUP();
				AlifSizeT shift = table[(*window_last) & TABLE_MASK];
				window_last += shift;
				if (shift == 0) {
					break;
				}
				if (window_last >= haystack_end) {
					return -1;
				}
				LOG("Horspool skip\n");
			}
		no_shift:
			window = window_last - len_needle + 1;
			AlifSizeT i = ALIF_MAX(cut, memory);
			for (; i < len_needle; i++) {
				if (needle[i] != window[i]) {
					if (i < gap_jump_end) {
						LOG("Early right half mismatch: jump by gap.\n");
						window_last += gap;
					}
					else {
						LOG("Late right half mismatch: jump by n (>gap)\n");
						window_last += i - cut + 1;
					}
					memory = 0;
					goto periodicwindowloop;
				}
			}
			for (i = memory; i < cut; i++) {
				if (needle[i] != window[i]) {
					LOG("Left half does not match.\n");
					window_last += period;
					memory = len_needle - period;
					if (window_last >= haystack_end) {
						return -1;
					}
					AlifSizeT shift = table[(*window_last) & TABLE_MASK];
					if (shift) {
						// A mismatch has been identified to the right
						// of where i will next start, so we can jump
						// at least as far as if the mismatch occurred
						// on the first comparison.
						AlifSizeT mem_jump = ALIF_MAX(cut, memory) - cut + 1;
						LOG("Skip with Memory.\n");
						memory = 0;
						window_last += ALIF_MAX(shift, mem_jump);
						goto periodicwindowloop;
					}
					goto no_shift;
				}
			}
			LOG("Found a match!\n");
			return window - haystack;
		}
	}
	else {
		period = ALIF_MAX(gap, period);
		LOG("Needle is not periodic.\n");
	windowloop:
		while (window_last < haystack_end) {
			for (;;) {
				LOG_LINEUP();
				AlifSizeT shift = table[(*window_last) & TABLE_MASK];
				window_last += shift;
				if (shift == 0) {
					break;
				}
				if (window_last >= haystack_end) {
					return -1;
				}
				LOG("Horspool skip\n");
			}
			window = window_last - len_needle + 1;
			AlifSizeT i = cut;
			for (; i < len_needle; i++) {
				if (needle[i] != window[i]) {
					if (i < gap_jump_end) {
						LOG("Early right half mismatch: jump by gap.\n");
						window_last += gap;
					}
					else {
						LOG("Late right half mismatch: jump by n (>gap)\n");
						window_last += i - cut + 1;
					}
					goto windowloop;
				}
			}
			for (AlifSizeT i = 0; i < cut; i++) {
				if (needle[i] != window[i]) {
					LOG("Left half does not match.\n");
					window_last += period;
					goto windowloop;
				}
			}
			LOG("Found a match!\n");
			return window - haystack;
		}
	}
	LOG("Not found. Returning -1.\n");
	return -1;
}


static AlifSizeT
STRINGLIB(twoWayFind)(const STRINGLIB_CHAR* haystack, AlifSizeT len_haystack,
	const STRINGLIB_CHAR* needle, AlifSizeT len_needle) { // 505
	LOG("###### Finding \"%s\" in \"%s\".\n", needle, haystack);
	STRINGLIB(preWork) p {};
	STRINGLIB(preProcess)(needle, len_needle, &p);
	return STRINGLIB(twoWay)(haystack, len_haystack, &p);
}

static AlifSizeT
STRINGLIB(twoWayCount)(const STRINGLIB_CHAR* haystack,
	AlifSizeT len_haystack, const STRINGLIB_CHAR* needle,
	AlifSizeT len_needle, AlifSizeT maxcount) { // 518
	LOG("###### Counting \"%s\" in \"%s\".\n", needle, haystack);
	STRINGLIB(preWork) p {};
	STRINGLIB(preProcess)(needle, len_needle, &p);
	AlifSizeT index = 0, count = 0;
	while (1) {
		AlifSizeT result;
		result = STRINGLIB(twoWay)(haystack + index,
			len_haystack - index, &p);
		if (result == -1) {
			return count;
		}
		count++;
		if (count == maxcount) {
			return maxcount;
		}
		index += result + len_needle;
	}
	return count;
}


 // 545
#undef SHIFT_TYPE
#undef NOT_FOUND
#undef SHIFT_OVERFLOW
#undef TABLE_SIZE_BITS
#undef TABLE_SIZE
#undef TABLE_MASK

#undef LOG
#undef LOG_STRING
#undef LOG_LINEUP


static inline AlifSizeT
STRINGLIB(defaultFind)(const STRINGLIB_CHAR* s, AlifSizeT n,
	const STRINGLIB_CHAR* p, AlifSizeT m,
	AlifSizeT maxcount, int mode) { // 556
	const AlifSizeT w = n - m;
	AlifSizeT mlast = m - 1, count = 0;
	AlifSizeT gap = mlast;
	const STRINGLIB_CHAR last = p[mlast];
	const STRINGLIB_CHAR* const ss = &s[mlast];

	unsigned long mask = 0;
	for (AlifSizeT i = 0; i < mlast; i++) {
		STRINGLIB_BLOOM_ADD(mask, p[i]);
		if (p[i] == last) {
			gap = mlast - i - 1;
		}
	}
	STRINGLIB_BLOOM_ADD(mask, last);

	for (AlifSizeT i = 0; i <= w; i++) {
		if (ss[i] == last) {
			/* candidate match */
			AlifSizeT j;
			for (j = 0; j < mlast; j++) {
				if (s[i + j] != p[j]) {
					break;
				}
			}
			if (j == mlast) {
				/* got a match! */
				if (mode != FAST_COUNT) {
					return i;
				}
				count++;
				if (count == maxcount) {
					return maxcount;
				}
				i = i + mlast;
				continue;
			}
			/* miss: check if next character is part of pattern */
			if (!STRINGLIB_BLOOM(mask, ss[i + 1])) {
				i = i + m;
			}
			else {
				i = i + gap;
			}
		}
		else {
			/* skip: check if next character is part of pattern */
			if (!STRINGLIB_BLOOM(mask, ss[i + 1])) {
				i = i + m;
			}
		}
	}
	return mode == FAST_COUNT ? count : -1;
}


static AlifSizeT
STRINGLIB(adaptiveFind)(const STRINGLIB_CHAR* s, AlifSizeT n,
	const STRINGLIB_CHAR* p, AlifSizeT m,
	AlifSizeT maxcount, int mode) { // 616
	const AlifSizeT w = n - m;
	AlifSizeT mlast = m - 1, count = 0;
	AlifSizeT gap = mlast;
	AlifSizeT hits = 0, res;
	const STRINGLIB_CHAR last = p[mlast];
	const STRINGLIB_CHAR* const ss = &s[mlast];

	unsigned long mask = 0;
	for (AlifSizeT i = 0; i < mlast; i++) {
		STRINGLIB_BLOOM_ADD(mask, p[i]);
		if (p[i] == last) {
			gap = mlast - i - 1;
		}
	}
	STRINGLIB_BLOOM_ADD(mask, last);

	for (AlifSizeT i = 0; i <= w; i++) {
		if (ss[i] == last) {
			/* candidate match */
			AlifSizeT j;
			for (j = 0; j < mlast; j++) {
				if (s[i + j] != p[j]) {
					break;
				}
			}
			if (j == mlast) {
				/* got a match! */
				if (mode != FAST_COUNT) {
					return i;
				}
				count++;
				if (count == maxcount) {
					return maxcount;
				}
				i = i + mlast;
				continue;
			}
			hits += j + 1;
			if (hits > m / 4 && w - i > 2000) {
				if (mode == FAST_SEARCH) {
					res = STRINGLIB(twoWayFind)(s + i, n - i, p, m);
					return res == -1 ? -1 : res + i;
				}
				else {
					res = STRINGLIB(twoWayCount)(s + i, n - i, p, m,
						maxcount - count);
					return res + count;
				}
			}
			/* miss: check if next character is part of pattern */
			if (!STRINGLIB_BLOOM(mask, ss[i + 1])) {
				i = i + m;
			}
			else {
				i = i + gap;
			}
		}
		else {
			/* skip: check if next character is part of pattern */
			if (!STRINGLIB_BLOOM(mask, ss[i + 1])) {
				i = i + m;
			}
		}
	}
	return mode == FAST_COUNT ? count : -1;
}


static AlifSizeT
STRINGLIB(defaultRfind)(const STRINGLIB_CHAR* s, AlifSizeT n,
	const STRINGLIB_CHAR* p, AlifSizeT m,
	AlifSizeT maxcount, int mode) { // 689
	/* create compressed boyer-moore delta 1 table */
	unsigned long mask = 0;
	AlifSizeT i{}, j{}, mlast = m - 1, skip = m - 1, w = n - m;

	/* process pattern[0] outside the loop */
	STRINGLIB_BLOOM_ADD(mask, p[0]);
	/* process pattern[:0:-1] */
	for (i = mlast; i > 0; i--) {
		STRINGLIB_BLOOM_ADD(mask, p[i]);
		if (p[i] == p[0]) {
			skip = i - 1;
		}
	}

	for (i = w; i >= 0; i--) {
		if (s[i] == p[0]) {
			/* candidate match */
			for (j = mlast; j > 0; j--) {
				if (s[i + j] != p[j]) {
					break;
				}
			}
			if (j == 0) {
				/* got a match! */
				return i;
			}
			/* miss: check if previous character is part of pattern */
			if (i > 0 && !STRINGLIB_BLOOM(mask, s[i - 1])) {
				i = i - m;
			}
			else {
				i = i - skip;
			}
		}
		else {
			/* skip: check if previous character is part of pattern */
			if (i > 0 && !STRINGLIB_BLOOM(mask, s[i - 1])) {
				i = i - m;
			}
		}
	}
	return -1;
}


static inline AlifSizeT
STRINGLIB(countChar)(const STRINGLIB_CHAR* s, AlifSizeT n,
	const STRINGLIB_CHAR p0, AlifSizeT maxcount) { // 739
	AlifSizeT i, count = 0;
	for (i = 0; i < n; i++) {
		if (s[i] == p0) {
			count++;
			if (count == maxcount) {
				return maxcount;
			}
		}
	}
	return count;
}

static inline AlifSizeT
STRINGLIB(countCharNoMaxCount)(const STRINGLIB_CHAR* s, AlifSizeT n,
	const STRINGLIB_CHAR p0)
	/* A specialized function of count_char that does not cut off at a maximum.
	   As a result, the compiler is able to vectorize the loop. */
{ // 756
	AlifSizeT count = 0;
	for (AlifSizeT i = 0; i < n; i++) {
		if (s[i] == p0) {
			count++;
		}
	}
	return count;
}





 // 772
ALIF_LOCAL_INLINE(AlifSizeT)
FASTSEARCH(const STRINGLIB_CHAR* s, AlifSizeT n,
	const STRINGLIB_CHAR* p, AlifSizeT m,
	AlifSizeT maxcount, int mode)
{
	if (n < m or (mode == FAST_COUNT and maxcount == 0)) {
		return -1;
	}

	/* look for special cases */
	if (m <= 1) {
		if (m <= 0) {
			return -1;
		}
		/* use special case for 1-character strings */
		if (mode == FAST_SEARCH)
			return STRINGLIB(findChar)(s, n, p[0]);
		else if (mode == FAST_RSEARCH)
			return STRINGLIB(rFindChar)(s, n, p[0]);
		else {
			if (maxcount == ALIF_SIZET_MAX) {
				return STRINGLIB(countCharNoMaxCount)(s, n, p[0]);
			}
			return STRINGLIB(countChar)(s, n, p[0], maxcount);
		}
	}

	if (mode != FAST_RSEARCH) {
		if (n < 2500 or (m < 100 and n < 30000) or m < 6) {
			return STRINGLIB(defaultFind)(s, n, p, m, maxcount, mode);
		}
		else if ((m >> 2) * 3 < (n >> 2)) {
			/* 33% threshold, but don't overflow. */
			/* For larger problems where the needle isn't a huge
			   percentage of the size of the haystack, the relatively
			   expensive O(m) startup cost of the two-way algorithm
			   will surely pay off. */
			if (mode == FAST_SEARCH) {
				return STRINGLIB(twoWayFind)(s, n, p, m);
			}
			else {
				return STRINGLIB(twoWayCount)(s, n, p, m, maxcount);
			}
		}
		else {
			/* To ensure that we have good worst-case behavior,
			   here's an adaptive version of the algorithm, where if
			   we match O(m) characters without any matches of the
			   entire needle, then we predict that the startup cost of
			   the two-way algorithm will probably be worth it. */
			return STRINGLIB(adaptiveFind)(s, n, p, m, maxcount, mode);
		}
	}
	else {
		/* FAST_RSEARCH */
		return STRINGLIB(defaultRfind)(s, n, p, m, maxcount, mode);
	}
}
