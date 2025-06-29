

#define STRINGLIB_IS_UNICODE     1

#define FASTSEARCH               asciiLib_fastSearch
#define STRINGLIB(_F)			 asciiLib_##_F
#define STRINGLIB_OBJECT         AlifUStrObject
#define STRINGLIB_SIZEOF_CHAR    1
#define STRINGLIB_MAX_CHAR       0x7Fu
#define STRINGLIB_CHAR           AlifUCS1

#define STRINGLIB_FAST_MEMCHR    memchr
#define STRINGLIB_MUTABLE 0
