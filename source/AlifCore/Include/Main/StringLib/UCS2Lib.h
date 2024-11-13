
#define STRINGLIB_IS_UNICODE     1

// 6
#define FASTSEARCH               ucs2Lib_fastSearch
#define STRINGLIB(F)             ucs2Lib_##F
#define STRINGLIB_OBJECT         AlifUStrObject
#define STRINGLIB_SIZEOF_CHAR    2
#define STRINGLIB_MAX_CHAR       0xFFFFu
#define STRINGLIB_CHAR           AlifUCS2

#if SIZEOF_WCHAR_T == 2
#define STRINGLIB_FAST_MEMCHR(s, c, n)              \
    (AlifUCS2*)wmemchr((const wchar_t *)(s), c, n)
#endif
#define STRINGLIB_MUTABLE 0
