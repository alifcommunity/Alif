#pragma once

#define STRINGLIB_FASTSEARCH_H

#ifdef DEBUG
#  define ALIF_SAFE_DOWNCAST(_VALUE, _WIDE, _NARROW) 
#else
#  define ALIF_SAFE_DOWNCAST(_VALUE, _WIDE, _NARROW) ((_NARROW)_VALUE)
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

#define STRINGLIB_BLOOM_ADD(_mask, _ch) \
    ((_mask |= (1UL << ((_ch) & (STRINGLIB_BLOOM_WIDTH -1)))))
#define STRINGLIB_BLOOM(_mask, _ch)     \
    ((_mask &  (1UL << ((_ch) & (STRINGLIB_BLOOM_WIDTH -1)))))

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
static __inline int64_t
#ifdef _WINDOWS64
	__fastcall
#endif
find_char(const STRINGLIB_CHAR* _s, int64_t _n, STRINGLIB_CHAR _ch)
{ // __fastcall لا تعمل على انظمة لينكس
    const STRINGLIB_CHAR* p_, * e_;

    p_ = _s;
    e_ = _s + _n;
    if (_n > MEMCHR_CUT_OFF) {
#ifdef STRINGLIB_FAST_MEMCHR
        p_ = (const STRINGLIB_CHAR*)STRINGLIB_FAST_MEMCHR(_s, _ch, _n);
        if (p_ != nullptr)
            return (p_ - _s);
        return -1;
#else

        const STRINGLIB_CHAR* s1_, * e1_;
        unsigned char needle = _ch & 0xff;

        if (needle_ != 0) {
            do {
                void* candidate_ = memchr(p_, needle_,
                    (e_ - p_) * sizeof(STRINGLIB_CHAR));
                if (candidate_ == nullptr)
                    return -1;
                s1_ = p_;
                p_ = (const STRINGLIB_CHAR*)
                    ALIF_ALIGN_DOWN(candidate_, sizeof(STRINGLIB_CHAR));
                if (*p_ == _ch)
                    return (p_ - _s);
                p_++;
                if (p_ - s1_ > MEMCHR_CUT_OFF)
                    continue;
                if (e_ - p_ <= MEMCHR_CUT_OFF)
                    break;
                e1_ = p_ + MEMCHR_CUT_OFF;
                while (p_ != e1_) {
                    if (*p_ == _ch)
                        return (p_ - _s);
                    p_++;
                }
            } while (e_ - p_ > MEMCHR_CUT_OFF);
        }
#endif
    }
    while (p_ < e_) {
        if (*p_ == _ch)
            return (p_ - _s);
        p_++;
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
static __inline int64_t
#ifdef _WINDOWS64
__fastcall
#endif
rfind_char(const STRINGLIB_CHAR* _s, int64_t _n, STRINGLIB_CHAR _ch)
{
    const STRINGLIB_CHAR* p_;
#ifdef HAVE_MEMRCHR

    if (_n > MEMRCHR_CUT_OFF) {
#if STRINGLIB_SIZEOF_CHAR == 1
        p_ = memrchr(_s, _ch, _n);
        if (p_ != nullptr)
            return (p_ - _s);
        return -1;
#else
        const STRINGLIB_CHAR* s1_;
        int64_t n1_;
        unsigned char needle_ = _ch & 0xff;

        if (needle_ != 0) {
            do {
                void* candidate_ = memrchr(_s, needle_,
                    _n * sizeof(STRINGLIB_CHAR));
                if (candidate_ == nullptr)
                    return -1;
                n1_ = _n;
                p_ = (const STRINGLIB_CHAR*)
                    ALIF_ALIGN_DOWN(candidate_, sizeof(STRINGLIB_CHAR));
                _n = p_ - _s;
                if (*p_ == _ch)
                    return _n;
                if (n1_ - _n > MEMRCHR_CUT_OFF)
                    continue;
                if (_n <= MEMRCHR_CUT_OFF)
                    break;
                s1_ = p_ - MEMRCHR_CUT_OFF;
                while (p_ > s1_) {
                    p_--;
                    if (*p_ == _ch)
                        return (p_ - _s);
                }
                _n = p_ - _s;
            } while (_n > MEMRCHR_CUT_OFF);
        }
#endif
    }
#endif
    p_ = _s + _n;
    while (p_ > _s) {
        p_--;
        if (*p_ == _ch)
            return (p_ - _s);
    }
    return -1;
}

#undef MEMRCHR_CUT_OFF

#if 0 && STRINGLIB_SIZEOF_CHAR == 1
# define LOG(...) printf(__VA_ARGS__)
# define LOG_STRING(_s, _n) printf("\"%.*s\"", (int)(_n), _s)
# define LOG_LINEUP() do {                                         \
    LOG("> "); LOG_STRING(_hayStack, _lenHayStack); LOG("\n> ");    \
    LOG("%*s",(int)(windowLast - _hayStack + 1 - lenNeedle), ""); \
    LOG_STRING(needle_, lenNeedle); LOG("\n");                     \
} while(0)
#else
# define LOG(...)
# define LOG_STRING(_s, _n)
# define LOG_LINEUP()
#endif

template <typename STRINGLIB_CHAR>
int64_t lex_search(const STRINGLIB_CHAR* _needle, int64_t _lenNeedle,
    int64_t* _returnPeriod, int _invertAlphabet)
{
    int64_t maxSuffix = 0;
    int64_t candidate_ = 1;
    int64_t k_ = 0;
    int64_t period_ = 1;

    while (candidate_ + k_ < _lenNeedle) {
        STRINGLIB_CHAR a_ = _needle[candidate_ + k_];
        STRINGLIB_CHAR b_ = _needle[maxSuffix + k_];
        if (_invertAlphabet ? (b_ < a_) : (a_ < b_)) {

            candidate_ += k_ + 1;
            k_ = 0;

            period_ = candidate_ - maxSuffix;
        }
        else if (a_ == b_) {
            if (k_ + 1 != period_) {
                k_++;
            }
            else {

                candidate_ += period_;
                k_ = 0;
            }
        }
        else {
            maxSuffix = candidate_;
            candidate_++;
            k_ = 0;
            period_ = 1;
        }
    }
    *_returnPeriod = period_;
    return maxSuffix;
}

template <typename STRINGLIB_CHAR> int64_t factorize(const STRINGLIB_CHAR* _needle,
    int64_t _lenNeedle,
    int64_t* _returnPeriod)
{
    int64_t cut1_, period1_, cut2_, period2_, cut_, period_;
    cut1_ = lex_search(_needle, _lenNeedle, &period1_, 0);
    cut2_ = lex_search(_needle, _lenNeedle, &period2_, 1);

    if (cut1_ > cut2_) {
        period_ = period1_;
        cut_ = cut1_;
    }
    else {
        period_ = period2_;
        cut_ = cut2_;
    }

    LOG("split: "); LOG_STRING(_needle, cut_);
    LOG(" + "); LOG_STRING(_needle + cut_, _lenNeedle - cut_);
    LOG("\n");

    *_returnPeriod = period_;
    return cut_;
}


#define SHIFT_TYPE uint8_t
#define MAX_SHIFT UINT8_MAX

#define TABLE_SIZE_BITS 6u
#define TABLE_SIZE (1U << TABLE_SIZE_BITS)
#define TABLE_MASK (TABLE_SIZE - 1U)

template <typename STRINGLIB_CHAR>
class PreWork{
public:
    const STRINGLIB_CHAR* needle_;
    int64_t lenNeedle;
    int64_t cut_;
    int64_t period_;
    int64_t gap_;
    int isPeriodic;
    SHIFT_TYPE table_[TABLE_SIZE];
} ;

template <typename STRINGLIB_CHAR>
static void preprocess(const STRINGLIB_CHAR* _needle, int64_t _lenNeedle,
    PreWork<STRINGLIB_CHAR>* _p)
{
    _p->needle_ = _needle;
    _p->lenNeedle = _lenNeedle;
    _p->cut_ = factorize(_needle, _lenNeedle, &(_p->period_));
    _p->isPeriodic = (0 == memcmp(_needle,
        _needle + _p->period_,
        _p->cut_ * STRINGLIB_SIZEOF_CHAR));
    if (_p->isPeriodic) {
        _p->gap_ = 0; // unused
    }
    else {
        _p->period_ = ALIF_MAX(_p->cut_, _lenNeedle - _p->cut_) + 1;

        _p->gap_ = _lenNeedle;
        STRINGLIB_CHAR last_ = _needle[_lenNeedle - 1] & TABLE_MASK;
        for (int64_t i_ = _lenNeedle - 2; i_ >= 0; i_--) {
            STRINGLIB_CHAR x_ = _needle[i_] & TABLE_MASK;
            if (x_ == last_) {
                _p->gap_ = _lenNeedle - 1 - i_;
                break;
            }
        }
    }
    int64_t notFoundShift = ALIF_MIN(_lenNeedle, MAX_SHIFT);
    for (int64_t i_ = 0; i_ < (int64_t)TABLE_SIZE; i_++) {
        _p->table_[i_] = ALIF_SAFE_DOWNCAST(notFoundShift, int64_t, SHIFT_TYPE);
    }
    for (int64_t i_ = _lenNeedle - notFoundShift; i_ < _lenNeedle; i_++) {
        SHIFT_TYPE shift_ = ALIF_SAFE_DOWNCAST(_lenNeedle - 1 - i_,
            int64_t, SHIFT_TYPE);
        _p->table_[_needle[i_] & TABLE_MASK] = shift_;
    }
}

template <typename STRINGLIB_CHAR>
static int64_t two_way(const STRINGLIB_CHAR* _hayStack, int64_t _lenHayStack,
    PreWork<STRINGLIB_CHAR>* _p)
{
    const int64_t lenNeedle = _p->lenNeedle;
    const int64_t cut_ = _p->cut_;
    int64_t period_ = _p->period_;
    const STRINGLIB_CHAR* const needle_ = _p->needle_;
    const STRINGLIB_CHAR* windowLast = _hayStack + lenNeedle - 1;
    const STRINGLIB_CHAR* const haystackEnd = _hayStack + _lenHayStack;
    SHIFT_TYPE* table_ = _p->table_;
    const STRINGLIB_CHAR* window_;
    LOG("===== Two-way: \"%s\" in \"%s\". =====\n", needle_, _hayStack);

    if (_p->isPeriodic) {
        LOG("Needle is periodic.\n");
        int64_t memory_ = 0;
    periodicwindowloop:
        while (windowLast < haystackEnd) {
            for (;;) {
                LOG_LINEUP();
                int64_t shift_ = table_[(*windowLast) & TABLE_MASK];
                windowLast += shift_;
                if (shift_ == 0) {
                    break;
                }
                if (windowLast >= haystackEnd) {
                    return -1;
                }
                LOG("Horspool skip_\n");
            }
        no_shift:
            window_ = windowLast - lenNeedle + 1;
           
            int64_t i_ = ALIF_MAX(cut_, memory_);
            for (; i_ < lenNeedle; i_++) {
                if (needle_[i_] != window_[i_]) {
                    LOG("Right half does not match.\n");
                    windowLast += i_ - cut_ + 1;
                    memory_ = 0;
                    goto periodicwindowloop;
                }
            }
            for (i_ = memory_; i_ < cut_; i_++) {
                if (needle_[i_] != window_[i_]) {
                    LOG("Left half does not match.\n");
                    windowLast += period_;
                    memory_ = lenNeedle - period_;
                    if (windowLast >= haystackEnd) {
                        return -1;
                    }
                    int64_t shift_ = table_[(*windowLast) & TABLE_MASK];
                    if (shift_) {

                        int64_t mem_jump = ALIF_MAX(cut_, memory_) - cut_ + 1;
                        LOG("Skip with Memory.\n");
                        memory_ = 0;
                        windowLast += ALIF_MAX(shift_, mem_jump);
                        goto periodicwindowloop;
                    }
                    goto no_shift;
                }
            }
            LOG("Found a match!\n");
            return window_ - _hayStack;
        }
    }
    else {
        int64_t gap_ = _p->gap_;
        period_ = ALIF_MAX(gap_, period_);
        LOG("Needle is not periodic.\n");
        int64_t gapJumpEnd = ALIF_MIN(lenNeedle, cut_ + gap_);
    windowloop:
        while (windowLast < haystackEnd) {
            for (;;) {
                LOG_LINEUP();
                int64_t shift_ = table_[(*windowLast) & TABLE_MASK];
                windowLast += shift_;
                if (shift_ == 0) {
                    break;
                }
                if (windowLast >= haystackEnd) {
                    return -1;
                }
                LOG("Horspool skip_\n");
            }
            window_ = windowLast - lenNeedle + 1;

            for (int64_t i_ = cut_; i_ < gapJumpEnd; i_++) {
                if (needle_[i_] != window_[i_]) {
                    LOG("Early right half mismatch: jump by gap_.\n");
                    windowLast += gap_;
                    goto windowloop;
                }
            }
            for (int64_t i_ = gapJumpEnd; i_ < lenNeedle; i_++) {
                if (needle_[i_] != window_[i_]) {lenNeedle
                    LOG("Late right half mismatch.\n");
                    windowLast += i_ - cut_ + 1;
                    goto windowloop;
                }
            }
            for (int64_t i_ = 0; i_ < cut_; i_++) {
                if (needle_[i_] != window_[i_]) {
                    LOG("Left half does not match.\n");
                    windowLast += period_;
                    goto windowloop;
                }
            }
            LOG("Found a match!\n");
            return window_ - _hayStack;
        }
    }
    LOG("Not found. Returning -1.\n");
    return -1;
}

template <typename STRINGLIB_CHAR>
static int64_t two_way_find(const STRINGLIB_CHAR* _hayStack,
    int64_t _lenHayStack,
    const STRINGLIB_CHAR* _needle,
    int64_t _lenNeedle)
{
    LOG("###### Finding \"%s\" in \"%s\".\n", _needle, _hayStack);
    PreWork<STRINGLIB_CHAR> p{};
    preprocess(_needle, _lenNeedle, &p);
    return two_way(_hayStack, _lenHayStack, &p);
}

template <typename STRINGLIB_CHAR>
static int64_t two_way_count(const STRINGLIB_CHAR* _hayStack,
    int64_t _lenHayStack,
    const STRINGLIB_CHAR* _needle,
    int64_t _lenNeedle,
    int64_t _maxCount)
{
    LOG("###### Counting \"%s\" in \"%s\".\n", _needle, _hayStack);
    PreWork<STRINGLIB_CHAR> p_{};
    preprocess(_needle, _lenNeedle, &p_);
    int64_t index_ = 0, count_ = 0;
    while (1) {
        int64_t result_;
        result_ = two_way(_hayStack + index_,
            _lenHayStack - index_, &p_);
        if (result_ == -1) {
            return count_;
        }
		count_++;
        if (count_ == _maxCount) {
            return _maxCount;
        }
        index_ += result_ + _lenNeedle;
    }
    return count_;
}

template <typename STRINGLIB_CHAR>
static inline int64_t default_find(const STRINGLIB_CHAR* _s, int64_t _n,
    const STRINGLIB_CHAR* _p, int64_t _m,
    int64_t _maxCount, int _mode)
{
    const int64_t w_ = _n - _m;
    int64_t mLast = _m - 1, count_ = 0;
    int64_t gap_ = mLast;
    const STRINGLIB_CHAR last_ = _p[mLast];
    const STRINGLIB_CHAR* const ss_ = &_s[mLast];

    unsigned long mask_ = 0;
    for (int64_t i_ = 0; i_ < mLast; i_++) {
        STRINGLIB_BLOOM_ADD(mask_, _p[i_]);
        if (_p[i_] == last_) {
            gap_ = mLast - i_ - 1;
        }
    }
    STRINGLIB_BLOOM_ADD(mask_, last_);

    for (int64_t i_ = 0; i_ <= w_; i_++) {
        if (ss_[i_] == last_) {
            int64_t j_;
            for (j_ = 0; j_ < mLast; j_++) {
                if (_s[i_ + j_] != _p[j_]) {
                    break;
                }
            }
            if (j_ == mLast) {
                
                if (_mode != FAST_COUNT) {
                    return i_;
                }
                count_++;
                if (count_ == _maxCount) {
                    return _maxCount;
                }
                i_ = i_ + mLast;
                continue;
            }
            if (!STRINGLIB_BLOOM(mask_, ss_[i_ + 1])) {
                i_ = i_ + _m;
            }
            else {
                i_ = i_ + gap_;
            }
        }
        else {
            if (!STRINGLIB_BLOOM(mask_, ss_[i_ + 1])) {
                i_ = i_ + _m;
            }
        }
    }
    return _mode == FAST_COUNT ? count_ : -1;
}

template <typename STRINGLIB_CHAR>
static int64_t adaptive_find(const STRINGLIB_CHAR* _s, int64_t n,
    const STRINGLIB_CHAR* _p, int64_t _m,
    int64_t _maxCount, int _mode)
{
    const int64_t w = n - _m;
    int64_t mLast = _m - 1, count_ = 0;
    int64_t gap_ = mLast;
    int64_t hits_ = 0, res_;
    const STRINGLIB_CHAR last_ = _p[mLast];
    const STRINGLIB_CHAR* const ss_ = &_s[mLast];

    unsigned long mask_ = 0;
    for (int64_t i_ = 0; i_ < mLast; i_++) {
        STRINGLIB_BLOOM_ADD(mask_, _p[i_]);
        if (_p[i_] == last_) {
            gap_ = mLast - i_ - 1;
        }
    }
    STRINGLIB_BLOOM_ADD(mask_, last_);

    for (int64_t i_ = 0; i_ <= w; i_++) {
        if (ss_[i_] == last_) {
            int64_t j_;
            for (j_ = 0; j_ < mLast; j_++) {
                if (_s[i_ + j_] != _p[j_]) {
                    break;
                }
            }
            if (j_ == mLast) {
                if (_mode != FAST_COUNT) {
                    return i_;
                }
                count_++;
                if (count_ == _maxCount) {
                    return _maxCount;
                }
                i_ = i_ + mLast;
                continue;
            }
            hits_ += j_ + 1;
            if (hits_ > _m / 4 && w - i_ > 2000) {
                if (_mode == FAST_SEARCH) {
                    res_ = two_way_find(_s + i_, n - i_, _p, _m);
                    return res_ == -1 ? -1 : res_ + i_;
                }
                else {
                    res_ = two_way_count(_s + i_, n - i_, _p, _m,
                        _maxCount - count_);
                    return res_ + count_;
                }
            }
            if (!STRINGLIB_BLOOM(mask_, ss_[i_ + 1])) {
                i_ = i_ + _m;
            }
            else {
                i_ = i_ + gap_;
            }
        }
        else {
            if (!STRINGLIB_BLOOM(mask_, ss_[i_ + 1])) {
                i_ = i_ + _m;
            }
        }
    }
    return _mode == FAST_COUNT ? count_ : -1;
}

template <typename STRINGLIB_CHAR>
static int64_t default_rfind(const STRINGLIB_CHAR* _s, int64_t n,
    const STRINGLIB_CHAR* _p, int64_t _m,
    int64_t _maxCount, int _mode)
{
    unsigned long mask_ = 0;
    int64_t i_, j_, mLast = _m - 1, skip_ = _m - 1, w = n - _m;

    STRINGLIB_BLOOM_ADD(mask_, _p[0]);
    for (i_ = mLast; i_ > 0; i_--) {
        STRINGLIB_BLOOM_ADD(mask_, _p[i_]);
        if (_p[i_] == _p[0]) {
            skip_ = i_ - 1;
        }
    }

    for (i_ = w; i_ >= 0; i_--) {
        if (_s[i_] == _p[0]) {
            for (j_ = mLast; j_ > 0; j_--) {
                if (_s[i_ + j_] != _p[j_]) {
                    break;
                }
            }
            if (j_ == 0) {
                return i_;
            }
            if (i_ > 0 && !STRINGLIB_BLOOM(mask_, _s[i_ - 1])) {
                i_ = i_ - _m;
            }
            else {
                i_ = i_ - skip_;
            }
        }
        else {
            if (i_ > 0 && !STRINGLIB_BLOOM(mask_, _s[i_ - 1])) {
                i_ = i_ - _m;
            }
        }
    }
    return -1;
}

template <typename STRINGLIB_CHAR>
static inline int64_t count_char(const STRINGLIB_CHAR* _s, int64_t n,
    const STRINGLIB_CHAR p0, int64_t _maxCount)
{
    int64_t i_, count_ = 0;
    for (i_ = 0; i_ < n; i_++) {
        if (_s[i_] == p0) {
            count_++;
            if (count_ == _maxCount) {
                return _maxCount;
            }
        }
    }
    return count_;
}

template <typename STRINGLIB_CHAR>
int64_t fastSearch(const STRINGLIB_CHAR* _s, int64_t n,
    const STRINGLIB_CHAR* _p, int64_t _m,
    int64_t _maxCount, int _mode)
{
    if (n < _m || (_mode == FAST_COUNT && _maxCount == 0)) {
        return -1;
    }

    if (_m <= 1) {
        if (_m <= 0) {
            return -1;
        }
        if (_mode == FAST_SEARCH)
            return find_char(_s, n, _p[0]);
        else if (_mode == FAST_RSEARCH)
            return rfind_char(_s, n, _p[0]);
        else {
            return count_char(_s, n, _p[0], _maxCount);
        }
    }

    if (_mode != FAST_RSEARCH) {
        if (n < 2500 || (_m < 100 && n < 30000) || _m < 6) {
            return default_find(_s, n, _p, _m, _maxCount, _mode);
        }
        else if ((_m >> 2) * 3 < (n >> 2)) {

            if (_mode == FAST_SEARCH) {
                return two_way_find(_s, n, _p, _m);
            }
            else {
                return two_way_count(_s, n, _p, _m, _maxCount);
            }
        }
        else {
            return adaptive_find(_s, n, _p, _m, _maxCount, _mode);
        }
    }
    else {
        return default_rfind(_s, n, _p, _m, _maxCount, _mode);
    }
}
