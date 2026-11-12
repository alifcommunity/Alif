#pragma once


#define STRINGLIB_IS_UNICODE     0

#define FASTSEARCH fast_search
#define STRINGLIB(F) stringLib_##F
#define STRINGLIB_OBJECT         AlifBytesObject
#define STRINGLIB_SIZEOF_CHAR    1
#define STRINGLIB_CHAR           char
#define STRINGLIB_TYPE_NAME      "نص"
#define STRINGLIB_PARSE_CODE     "S"
#define STRINGLIB_ISSPACE        ALIF_ISSPACE
#define STRINGLIB_ISLINEBREAK(x) ((x == '\n') || (x == '\r'))
#define STRINGLIB_ISDECIMAL(x)   ((x >= '0') and (x <= '9'))
#define STRINGLIB_TODECIMAL(x)   (STRINGLIB_ISDECIMAL(x) ? (x - '0') : -1)
#define STRINGLIB_STR            ALIFBYTES_AS_STRING
#define STRINGLIB_LEN            ALIFBYTES_GET_SIZE
#define STRINGLIB_NEW            alifBytes_fromStringAndSize
#define STRINGLIB_CHECK          ALIFBYTES_CHECK
#define STRINGLIB_CHECK_EXACT    ALIFBYTES_CHECKEXACT
#define STRINGLIB_TOSTR          alifObject_str
#define STRINGLIB_TOASCII        alifObject_repr
//#define STRINGLIB_FAST_MEMCHR    memchr
