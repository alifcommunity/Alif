

#define STRINGLIB_IS_UNICODE     1

// 6
#define FASTSEARCH               ucs4Lib_fastSearch
#define STRINGLIB(_F)			 ucs4Lib_##_F
#define STRINGLIB_OBJECT         AlifUStrObject
#define STRINGLIB_SIZEOF_CHAR    4
#define STRINGLIB_MAX_CHAR       0x10FFFFu
#define STRINGLIB_CHAR           AlifUCS4 
#define STRINGLIB_ISSPACE        alifUStr_isSpace
#define STRINGLIB_NEW            _alifUStr_fromUCS4
#define STRINGLIB_CHECK_EXACT    ALIFUSTR_CHECKEXACT


#if SIZEOF_WCHAR_T == 4
#define STRINGLIB_FAST_MEMCHR(s, c, n)              \
    (AlifUCS4*)wmemchr((const wchar_t *)(s), c, n)
#endif
#define STRINGLIB_MUTABLE 0
