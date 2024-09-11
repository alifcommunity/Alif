#pragma once


#define ALIFBOOL_CHECK(_x) ALIF_IS_TYPE((_x), &_alifBoolType_) // 12


AlifLongObject _alifFalseClass_; // 17
AlifLongObject _alifTrueClass_; // 18



#define ALIF_FALSE ALIFOBJECT_CAST(&_alifFalseClass_) // 25
#define ALIF_TRUE ALIFOBJECT_CAST(&_alifTrueClass_) // 26
