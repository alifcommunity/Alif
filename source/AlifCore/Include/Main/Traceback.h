#pragma once





ALIFAPI_FUNC(AlifIntT) alifTraceBack_here(AlifFrameObject*); // 9

 // 13
ALIFAPI_DATA(AlifTypeObject) _alifTraceBackType_;
#define ALIFTRACEBACK_CHECK(_v) ALIF_IS_TYPE(_v, &_alifTraceBackType_)



/* ------------------------------------------------------------------------------ */




class AlifTracebackObject { // 7
public:
	ALIFOBJECT_HEAD;
	AlifTracebackObject* next{};
	AlifFrameObject* frame{};
	AlifIntT lasti{};
	AlifIntT lineno{};
};
