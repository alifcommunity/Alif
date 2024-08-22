

#define STRINGLIB_IS_UNICODE     1

#define STRINGLIB(_F)            ucs1Lib_##_F
#define STRINGLIB_OBJECT         AlifUStrObject
#define STRINGLIB_SIZEOF_CHAR    1
#define STRINGLIB_MAX_CHAR       0xFFu
#define STRINGLIB_CHAR           AlifUCS1
