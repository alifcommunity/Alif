#pragma once

#define STRINGLIB_FASTSEARCH_H

#ifdef DEBUG
#  define ALIF_SAFE_DOWNCAST(VALUE, WIDE, NARROW) 
#else
#  define ALIF_SAFE_DOWNCAST(VALUE, WIDE, NARROW) ((NARROW)VALUE)
#endif

// in file alifConfig.h in 64
#define LONG_BIT        32

#define FAST_COUNT 0
#define FAST_SEARCH 1
#define FAST_RSEARCH 2

#if LONG_BIT >= 128
#define STRINGLIB_BLOOM_WIDTH 128
#elif LONG_BIT >= 64
#define STRINGLIB_BLOOM_WIDTH 64
#elif LONG_BIT >= 32
#define STRINGLIB_BLOOM_WIDTH 32
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


// هذه يجب ان تتغير على حسب النوع المدخل في الفانكشن
#define STRINGLIB_SIZEOF_CHAR 2

// تم استخدام template بدل التعريفات في ملف ucs2lib.h and ucs4lib.h
// لن يتم حذف ملفين ucs2lib.h and ucs4lib.h الى ان يتم اخذ منها جميع البيانات ويستغنى عنها كليا
template <typename STRINGLIB_CHAR>
static __inline int64_t __fastcall find_char(const STRINGLIB_CHAR* s, int64_t n, STRINGLIB_CHAR ch)
{
    const STRINGLIB_CHAR* p, * e;

    p = s;
    e = s + n;
    if (n > MEMCHR_CUT_OFF) {
#ifdef STRINGLIB_FAST_MEMCHR
        p = (const STRINGLIB_CHAR*)STRINGLIB_FAST_MEMCHR(s, ch, n);
        if (p != NULL)
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
                void* candidate = memchr(p, needle,
                    (e - p) * sizeof(STRINGLIB_CHAR));
                if (candidate == NULL)
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

template <typename STRINGLIB_CHAR>
static __inline int64_t __fastcall rfind_char(const STRINGLIB_CHAR* s, int64_t n, STRINGLIB_CHAR ch)
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
        if (p != NULL)
            return (p - s);
        return -1;
#else
        /* use memrchr if we can choose a needle without too many likely
           false positives */
        const STRINGLIB_CHAR* s1;
        int64_t n1;
        unsigned char needle = ch & 0xff;
        /* If looking for a multiple of 256, we'd have too
           many false positives looking for the '\0' byte in UCS2
           and UCS4 representations. */
        if (needle != 0) {
            do {
                void* candidate = memrchr(s, needle,
                    n * sizeof(STRINGLIB_CHAR));
                if (candidate == NULL)
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

/* Change to a 1 to see logging comments walk through the algorithm. */
#if 0 && STRINGLIB_SIZEOF_CHAR == 1
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

template <typename STRINGLIB_CHAR>
int64_t lex_search(const STRINGLIB_CHAR* needle, int64_t len_needle,
    int64_t* return_period, int invert_alphabet)
{
    /* Do a lexicographic search. Essentially this:
           >>> max(needle[i:] for i in range(len(needle)+1))
       Also find the period of the right half.   */
    int64_t max_suffix = 0;
    int64_t candidate = 1;
    int64_t k = 0;
    // The period of the right half.
    int64_t period = 1;

    while (candidate + k < len_needle) {
        // each loop increases candidate + k + max_suffix
        STRINGLIB_CHAR a = needle[candidate + k];
        STRINGLIB_CHAR b = needle[max_suffix + k];
        // check if the suffix at candidate is better than max_suffix
        if (invert_alphabet ? (b < a) : (a < b)) {
            // Fell short of max_suffix.
            // The next k + 1 characters are non-increasing
            // from candidate, so they won't start_ a maximal suffix.
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

template <typename STRINGLIB_CHAR> int64_t factorize(const STRINGLIB_CHAR* needle,
    int64_t len_needle,
    int64_t* return_period)
{
    /* Do a "critical factorization", making it so that:
       >>> needle = (left := needle[:cut]) + (right := needle[cut:])
       where the "local period" of the cut is maximal.

       The local period of the cut is the minimal length of a string w
       such that (left endswith w or w endswith left)
       and (right startswith w or w startswith left).

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

    int64_t cut1, period1, cut2, period2, cut, period;
    cut1 = lex_search(needle, len_needle, &period1, 0);
    cut2 = lex_search(needle, len_needle, &period2, 1);

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


#define SHIFT_TYPE uint8_t
#define MAX_SHIFT UINT8_MAX

#define TABLE_SIZE_BITS 6u
#define TABLE_SIZE (1U << TABLE_SIZE_BITS)
#define TABLE_MASK (TABLE_SIZE - 1U)

template <typename STRINGLIB_CHAR>
class PreWork{
public:
    const STRINGLIB_CHAR* needle;
    int64_t len_needle;
    int64_t cut;
    int64_t period;
    int64_t gap;
    int isPeriodic;
    SHIFT_TYPE table[TABLE_SIZE];
} ;

template <typename STRINGLIB_CHAR>
static void preprocess(const STRINGLIB_CHAR* needle, int64_t len_needle,
    PreWork<STRINGLIB_CHAR>* p)
{
    p->needle = needle;
    p->len_needle = len_needle;
    p->cut = factorize(needle, len_needle, &(p->period));
    p->isPeriodic = (0 == memcmp(needle,
        needle + p->period,
        p->cut * STRINGLIB_SIZEOF_CHAR));
    if (p->isPeriodic) {
        p->gap = 0; // unused
    }
    else {
        // A lower bound on the period
        p->period = max(p->cut, len_needle - p->cut) + 1;
        // The gap between the last character and the previous
        // occurrence of an equivalent character (modulo TABLE_SIZE)
        p->gap = len_needle;
        STRINGLIB_CHAR last = needle[len_needle - 1] & TABLE_MASK;
        for (int64_t i = len_needle - 2; i >= 0; i--) {
            STRINGLIB_CHAR x = needle[i] & TABLE_MASK;
            if (x == last) {
                p->gap = len_needle - 1 - i;
                break;
            }
        }
    }
    // Fill up a compressed Boyer-Moore "Bad Character" table
    int64_t not_found_shift = min(len_needle, MAX_SHIFT);
    for (int64_t i = 0; i < (int64_t)TABLE_SIZE; i++) {
        p->table[i] = ALIF_SAFE_DOWNCAST(not_found_shift, int64_t, SHIFT_TYPE);
    }
    for (int64_t i = len_needle - not_found_shift; i < len_needle; i++) {
        SHIFT_TYPE shift = ALIF_SAFE_DOWNCAST(len_needle - 1 - i,
            int64_t, SHIFT_TYPE);
        p->table[needle[i] & TABLE_MASK] = shift;
    }
}

template <typename STRINGLIB_CHAR>
static int64_t two_way(const STRINGLIB_CHAR* haystack, int64_t len_haystack,
    PreWork<STRINGLIB_CHAR>* p)
{
    const int64_t len_needle = p->len_needle;
    const int64_t cut = p->cut;
    int64_t period = p->period;
    const STRINGLIB_CHAR* const needle = p->needle;
    const STRINGLIB_CHAR* window_last = haystack + len_needle - 1;
    const STRINGLIB_CHAR* const haystack_end = haystack + len_haystack;
    SHIFT_TYPE* table = p->table;
    const STRINGLIB_CHAR* window;
    LOG("===== Two-way: \"%s\" in \"%s\". =====\n", needle, haystack);

    if (p->isPeriodic) {
        LOG("Needle is periodic.\n");
        int64_t memory = 0;
    periodicwindowloop:
        while (window_last < haystack_end) {
            for (;;) {
                LOG_LINEUP();
                int64_t shift = table[(*window_last) & TABLE_MASK];
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
           
            int64_t i = max(cut, memory);
            for (; i < len_needle; i++) {
                if (needle[i] != window[i]) {
                    LOG("Right half does not match.\n");
                    window_last += i - cut + 1;
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
                    int64_t shift = table[(*window_last) & TABLE_MASK];
                    if (shift) {
                        // A mismatch has been identified to the right
                        // of where i will next start_, so we can jump
                        // at least as far as if the mismatch occurred
                        // on the first comparison.
                        int64_t mem_jump = max(cut, memory) - cut + 1;
                        LOG("Skip with Memory.\n");
                        memory = 0;
                        window_last += max(shift, mem_jump);
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
        int64_t gap = p->gap;
        period = max(gap, period);
        LOG("Needle is not periodic.\n");
        int64_t gap_jump_end = min(len_needle, cut + gap);
    windowloop:
        while (window_last < haystack_end) {
            for (;;) {
                LOG_LINEUP();
                int64_t shift = table[(*window_last) & TABLE_MASK];
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

            for (int64_t i = cut; i < gap_jump_end; i++) {
                if (needle[i] != window[i]) {
                    LOG("Early right half mismatch: jump by gap.\n");
                    window_last += gap;
                    goto windowloop;
                }
            }
            for (int64_t i = gap_jump_end; i < len_needle; i++) {
                if (needle[i] != window[i]) {
                    LOG("Late right half mismatch.\n");
                    window_last += i - cut + 1;
                    goto windowloop;
                }
            }
            for (int64_t i = 0; i < cut; i++) {
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

template <typename STRINGLIB_CHAR>
static int64_t two_way_find(const STRINGLIB_CHAR* haystack,
    int64_t len_haystack,
    const STRINGLIB_CHAR* needle,
    int64_t len_needle)
{
    LOG("###### Finding \"%s\" in \"%s\".\n", needle, haystack);
    PreWork<STRINGLIB_CHAR> p{};
    preprocess(needle, len_needle, &p);
    return two_way(haystack, len_haystack, &p);
}

template <typename STRINGLIB_CHAR>
static int64_t two_way_count(const STRINGLIB_CHAR* haystack,
    int64_t len_haystack,
    const STRINGLIB_CHAR* needle,
    int64_t len_needle,
    int64_t maxcount)
{
    LOG("###### Counting \"%s\" in \"%s\".\n", needle, haystack);
    PreWork<STRINGLIB_CHAR> p{};
    preprocess(needle, len_needle, &p);
    int64_t index = 0, count = 0;
    while (1) {
        int64_t result;
        result = two_way(haystack + index,
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

template <typename STRINGLIB_CHAR>
static inline int64_t default_find(const STRINGLIB_CHAR* s, int64_t n,
    const STRINGLIB_CHAR* p, int64_t m,
    int64_t maxcount, int mode)
{
    const int64_t w = n - m;
    int64_t mlast = m - 1, count = 0;
    int64_t gap = mlast;
    const STRINGLIB_CHAR last = p[mlast];
    const STRINGLIB_CHAR* const ss = &s[mlast];

    unsigned long mask = 0;
    for (int64_t i = 0; i < mlast; i++) {
        STRINGLIB_BLOOM_ADD(mask, p[i]);
        if (p[i] == last) {
            gap = mlast - i - 1;
        }
    }
    STRINGLIB_BLOOM_ADD(mask, last);

    for (int64_t i = 0; i <= w; i++) {
        if (ss[i] == last) {
            /* candidate match */
            int64_t j;
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

template <typename STRINGLIB_CHAR>
static int64_t adaptive_find(const STRINGLIB_CHAR* s, int64_t n,
    const STRINGLIB_CHAR* p, int64_t m,
    int64_t maxcount, int mode)
{
    const int64_t w = n - m;
    int64_t mlast = m - 1, count = 0;
    int64_t gap = mlast;
    int64_t hits = 0, res;
    const STRINGLIB_CHAR last = p[mlast];
    const STRINGLIB_CHAR* const ss = &s[mlast];

    unsigned long mask = 0;
    for (int64_t i = 0; i < mlast; i++) {
        STRINGLIB_BLOOM_ADD(mask, p[i]);
        if (p[i] == last) {
            gap = mlast - i - 1;
        }
    }
    STRINGLIB_BLOOM_ADD(mask, last);

    for (int64_t i = 0; i <= w; i++) {
        if (ss[i] == last) {
            /* candidate match */
            int64_t j;
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
                    res = two_way_find(s + i, n - i, p, m);
                    return res == -1 ? -1 : res + i;
                }
                else {
                    res = two_way_count(s + i, n - i, p, m,
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

template <typename STRINGLIB_CHAR>
static int64_t default_rfind(const STRINGLIB_CHAR* s, int64_t n,
    const STRINGLIB_CHAR* p, int64_t m,
    int64_t maxcount, int mode)
{
    /* create compressed boyer-moore delta 1 table */
    unsigned long mask = 0;
    int64_t i, j, mlast = m - 1, skip = m - 1, w = n - m;

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

template <typename STRINGLIB_CHAR>
static inline int64_t count_char(const STRINGLIB_CHAR* s, int64_t n,
    const STRINGLIB_CHAR p0, int64_t maxcount)
{
    int64_t i, count = 0;
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

template <typename STRINGLIB_CHAR>
int64_t fastSearch(const STRINGLIB_CHAR* s, int64_t n,
    const STRINGLIB_CHAR* p, int64_t m,
    int64_t maxcount, int mode)
{
    if (n < m || (mode == FAST_COUNT && maxcount == 0)) {
        return -1;
    }

    /* look for special cases */
    if (m <= 1) {
        if (m <= 0) {
            return -1;
        }
        /* use special case for 1-character strings */
        if (mode == FAST_SEARCH)
            return find_char(s, n, p[0]);
        else if (mode == FAST_RSEARCH)
            return rfind_char(s, n, p[0]);
        else {
            return count_char(s, n, p[0], maxcount);
        }
    }

    if (mode != FAST_RSEARCH) {
        if (n < 2500 || (m < 100 && n < 30000) || m < 6) {
            return default_find(s, n, p, m, maxcount, mode);
        }
        else if ((m >> 2) * 3 < (n >> 2)) {
            /* 33% threshold, but don't overflow. */
            /* For larger problems where the needle isn't a huge
               percentage of the size_ of the haystack, the relatively
               expensive O(m) startup cost of the two-way algorithm
               will surely pay off. */
            if (mode == FAST_SEARCH) {
                return two_way_find(s, n, p, m);
            }
            else {
                return two_way_count(s, n, p, m, maxcount);
            }
        }
        else {
            /* To ensure that we have good worst-case behavior,
               here's an adaptive version of the algorithm, where if
               we match O(m) characters without any matches of the
               entire needle, then we predict that the startup cost of
               the two-way algorithm will probably be worth it. */
            return adaptive_find(s, n, p, m, maxcount, mode);
        }
    }
    else {
        /* FAST_RSEARCH */
        return default_rfind(s, n, p, m, maxcount, mode);
    }
}