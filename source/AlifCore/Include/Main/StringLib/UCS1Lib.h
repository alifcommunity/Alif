

#define STRINGLIB_IS_UNICODE     1

#define FASTSEARCH               ucs1Lib_fastSearch
#define STRINGLIB(_F)            ucs1Lib_##_F
#define STRINGLIB_OBJECT         AlifUStrObject
#define STRINGLIB_SIZEOF_CHAR    1
#define STRINGLIB_MAX_CHAR       0xFFu
#define STRINGLIB_CHAR           AlifUCS1

#define STRINGLIB_FAST_MEMCHR    memchr
#define STRINGLIB_MUTABLE 0
