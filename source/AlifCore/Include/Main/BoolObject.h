#pragma once


#define ALIFBOOL_CHECK(_x) ALIF_IS_TYPE((_x), &_alifBoolType_) // 12


ALIFAPI_DATA(AlifLongObject) _alifFalseClass_; // 17
ALIFAPI_DATA(AlifLongObject) _alifTrueClass_; // 18



#define ALIF_FALSE ALIFOBJECT_CAST(&_alifFalseClass_) // 25
#define ALIF_TRUE ALIFOBJECT_CAST(&_alifTrueClass_) // 26






 // 38
#define ALIF_RETURN_TRUE return ALIF_TRUE
#define ALIF_RETURN_FALSE return ALIF_FALSE



ALIFAPI_FUNC(AlifObject*)  alifBool_fromLong(long); // 42




