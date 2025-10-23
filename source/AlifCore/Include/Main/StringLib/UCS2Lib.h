
#define STRINGLIB_IS_UNICODE     1

// 6
#define FASTSEARCH               ucs2Lib_fastSearch
#define STRINGLIB(_F)            ucs2Lib_##_F
#define STRINGLIB_OBJECT         AlifUStrObject
#define STRINGLIB_SIZEOF_CHAR    2
#define STRINGLIB_MAX_CHAR       0xFFFFu
#define STRINGLIB_CHAR           AlifUCS2
#define STRINGLIB_ISSPACE        alifUStr_isSpace
#define STRINGLIB_NEW            _alifUStr_fromUCS2
#define STRINGLIB_CHECK_EXACT    ALIFUSTR_CHECKEXACT


#if SIZEOF_WCHAR_T == 2
#define STRINGLIB_FAST_MEMCHR(s, c, n)              \
    (AlifUCS2*)wmemchr((const wchar_t *)(s), c, n)
#endif
#define STRINGLIB_MUTABLE 0
