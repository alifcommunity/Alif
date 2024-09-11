#pragma once


#define ALIFBOOL_CHECK(_x) ALIF_IS_TYPE((_x), &_alifBoolType_)


AlifLongObject _alifFalseClass_; // 17
AlifLongObject _alifTrueClass_; // 18



#  define ALIF_FALSE ALIFOBJECT_CAST(&_alifFalseClass_)
#  define ALIF_TRUE ALIFOBJECT_CAST(&_alifTrueClass_)
