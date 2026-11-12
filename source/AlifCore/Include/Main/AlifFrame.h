#pragma once





ALIFAPI_FUNC(AlifCodeObject*) alifFrame_getCode(AlifFrameObject*);



/* --------------------------------------------------------------------------------- */



ALIFAPI_DATA(AlifTypeObject) _alifFrameType_; // 5
ALIFAPI_DATA(AlifTypeObject) _alifFrameLocalsProxyType_;

#define ALIFFRAME_CHECK(op) ALIF_IS_TYPE((op), &_alifFrameType_) // 8
#define ALIFFRAMELOCALSPROXY_CHECK(op) ALIF_IS_TYPE((op), &_alifFrameLocalsProxyType_)






ALIFAPI_FUNC(AlifIntT) alifUnstable_interpreterFrameGetLine(class AlifInterpreterFrame*); // 37
